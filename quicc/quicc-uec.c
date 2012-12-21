/*
 * Copyright (c) 2011-2012 embedded brains GmbH.  All rights reserved.
 *
 *  embedded brains GmbH
 *  Obere Lagerstr. 30
 *  82178 Puchheim
 *  Germany
 *  <rtems@embedded-brains.de>
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rtems.com/license/LICENSE.
 */

#include "quicc.h"

#include <assert.h>
#include <stdlib.h>

#include <rtems/malloc.h>

#include <libcpu/powerpc-utility.h>

/* Must be an integral multiple of 128 and the virtual receive FIFO size */
#define UEC_MRBLR 1536

static const uint8_t thread_counts [] = {
	[QUICC_UEC_THREAD_COUNT_1] = 1,
	[QUICC_UEC_THREAD_COUNT_2] = 2,
	[QUICC_UEC_THREAD_COUNT_4] = 4,
	[QUICC_UEC_THREAD_COUNT_6] = 6,
	[QUICC_UEC_THREAD_COUNT_8] = 8
};

uint16_t quicc_uec_mii_read(const quicc_uec_context *uec_context, int phy, int reg)
{
	const quicc_ucf_context *ucf_context = uec_context->ucf_context;
	volatile quicc_ucf *ucf_regs = ucf_context->ucf_regs;

	ucf_regs->miimadd = QUICC_UCF_MIIMADD_PHY_ADDR((uint32_t) phy)
		| QUICC_UCF_MIIMADD_REG_ADDR((uint32_t) reg);
	ucf_regs->miimcom = 0;
	ucf_regs->miimcom;
	ucf_regs->miimcom = QUICC_UCF_MIIMCOM_READ_CYCLE;
	ucf_regs->miimcom;
	while ((ucf_regs->miimind & (QUICC_UCF_MIIMIND_INVALID | QUICC_UCF_MIIMIND_BUSY)) != 0) {
		/* Wait */
	}

	return QUICC_UCF_MIIMSTAT_DATA_GET(ucf_regs->miimstat);
}

void quicc_uec_mii_write(const quicc_uec_context *uec_context, int phy, int reg, uint16_t data)
{
	const quicc_ucf_context *ucf_context = uec_context->ucf_context;
	volatile quicc_ucf *ucf_regs = ucf_context->ucf_regs;

	ucf_regs->miimadd = QUICC_UCF_MIIMADD_PHY_ADDR((uint32_t) phy)
		| QUICC_UCF_MIIMADD_REG_ADDR((uint32_t) reg);
	ucf_regs->miimcom = 0;
	ucf_regs->miimcom;
	ucf_regs->miimcon = QUICC_UCF_MIIMCON_DATA(data);
	ucf_regs->miimcon;
	while ((ucf_regs->miimind & QUICC_UCF_MIIMIND_BUSY) != 0) {
		/* Wait */
	}
}

void quicc_uec_set_mac_address(const quicc_uec_context *uec_context, const uint8_t *mac_address)
{
	const quicc_ucf_context *ucf_context = uec_context->ucf_context;
	volatile quicc_ucf *ucf_regs = ucf_context->ucf_regs;

	ucf_regs->macstnaddr1 = (((uint32_t) mac_address [5]) << 24)
		| (((uint32_t) mac_address [4]) << 16)
		| (((uint32_t) mac_address [3]) << 8)
		| (((uint32_t) mac_address [2]));
	ucf_regs->macstnaddr2 = (((uint32_t) mac_address [1]) << 24)
		| (((uint32_t) mac_address [0]) << 16);
};

void quicc_uec_mac_enable(const quicc_uec_context *uec_context, quicc_direction dir)
{
	const quicc_ucf_context *ucf_context = uec_context->ucf_context;
	volatile quicc_ucf *ucf_regs = ucf_context->ucf_regs;
	uint32_t maccfg1 = ucf_regs->maccfg1;

	if (quicc_direction_includes_receive(dir)) {
		maccfg1 |= QUICC_UCF_MACCFG1_RX_EN;
	}
	if (quicc_direction_includes_transmit(dir)) {
		maccfg1 |= QUICC_UCF_MACCFG1_TX_EN;
	}

	ucf_regs->maccfg1 = maccfg1;
}

void quicc_uec_mac_disable(const quicc_uec_context *uec_context, quicc_direction dir)
{
	const quicc_ucf_context *ucf_context = uec_context->ucf_context;
	volatile quicc_ucf *ucf_regs = ucf_context->ucf_regs;
	uint32_t maccfg1 = ucf_regs->maccfg1;

	if (quicc_direction_includes_receive(dir)) {
		maccfg1 &= ~QUICC_UCF_MACCFG1_RX_EN;
	}
	if (quicc_direction_includes_transmit(dir)) {
		maccfg1 &= ~QUICC_UCF_MACCFG1_TX_EN;
	}

	ucf_regs->maccfg1 = maccfg1;
}

void quicc_uec_execute_command(const quicc_uec_context *uec_context, quicc_cmd_opcode opcode, uint32_t data)
{
	const quicc_ucf_context *ucf_context = uec_context->ucf_context;
	const quicc_context *context = ucf_context->context;
	int ucc_index = ucf_context->config->index;

	quicc_execute_command(
		context,
		opcode,
		quicc_ucc_index_to_fast_subblock(ucc_index),
		QUICC_CMD_PROTOCOL_ETHERNET,
		data
	);
}

void quicc_uec_start(const quicc_uec_context *uec_context, quicc_direction dir)
{
	if (quicc_direction_includes_receive(dir)) {
		quicc_uec_execute_command(uec_context, QUICC_CMD_FAST_RESTART_RX, 0);
	}
	if (quicc_direction_includes_transmit(dir)) {
		quicc_uec_execute_command(uec_context, QUICC_CMD_FAST_RESTART_TX, 0);
	}
}

void quicc_uec_stop(const quicc_uec_context *uec_context, quicc_direction dir)
{
	const quicc_ucf_context *ucf_context = uec_context->ucf_context;
	quicc_uec_rx_gparam *rx_global_param = uec_context->rx_global_param;
	volatile quicc_ucf *ucf_regs = ucf_context->ucf_regs;

	if (quicc_direction_includes_receive(dir)) {
		rx_global_param->typeorlen_rxgstpack &= ~QUICC_UEC_RX_GPARAM_RXGSTPACK;

		do {
			quicc_uec_execute_command(uec_context, QUICC_CMD_FAST_GRACEFUL_STOP_RX, 0);
		} while ((rx_global_param->typeorlen_rxgstpack & QUICC_UEC_RX_GPARAM_RXGSTPACK) == 0);
	}

	if (quicc_direction_includes_transmit(dir)) {
		ucf_regs->ucce = QUICC_UCF_UCCE_UEC_GRA;
		quicc_uec_execute_command(uec_context, QUICC_CMD_FAST_GRACEFUL_STOP_TX, 0);
		while ((ucf_regs->ucce & QUICC_UCF_UCCE_UEC_GRA) == 0) {
			/* Wait */
		}
	}
}

void quicc_uec_config_mode_enter(const quicc_uec_context *uec_context, quicc_direction dir)
{
	const quicc_ucf_context *ucf_context = uec_context->ucf_context;

	quicc_uec_stop(uec_context, dir);
	quicc_ucf_disable(ucf_context, dir);
}

void quicc_uec_config_mode_leave(const quicc_uec_context *uec_context, quicc_direction dir)
{
	const quicc_ucf_context *ucf_context = uec_context->ucf_context;

	quicc_uec_start(uec_context, dir);
	quicc_ucf_enable(ucf_context, dir);
}

void quicc_uec_enable_promiscuous_mode(
	const quicc_uec_context *self,
	bool enable
)
{
	const quicc_ucf_context *ucf_context = self->ucf_context;
	volatile quicc_ucf *ucf_regs = ucf_context->ucf_regs;
	uint32_t pro_flag = QUICC_UCF_UPSMR_UEC_PRO;
	uint32_t pro_new = enable ? pro_flag : 0;
	uint32_t upsmr = ucf_regs->upsmr;

	ucf_regs->upsmr = (upsmr & ~pro_flag) | pro_new;
}

void quicc_uec_set_interface_mode(
	const quicc_uec_context *self,
	quicc_uec_interface_type interface_type,
	quicc_uec_speed speed,
	bool full_duplex
)
{
	const quicc_ucf_context *ucf_context = self->ucf_context;
	volatile quicc_ucf *ucf_regs = ucf_context->ucf_regs;
	uint32_t maccfg2 = ucf_regs->maccfg2;
	uint32_t upsmr = ucf_regs->upsmr;
	uint32_t ifm = 0;
	uint32_t fdx_flag = QUICC_UCF_MACCFG2_FDX;
	uint32_t fdx_new = full_duplex ? fdx_flag : 0;

	upsmr &= ~(QUICC_UCF_UPSMR_UEC_RPM | QUICC_UCF_UPSMR_UEC_TBIM | QUICC_UCF_UPSMR_UEC_R10M | QUICC_UCF_UPSMR_UEC_RMM);

	switch (speed) {
		case QUICC_UEC_SPEED_10:
			ifm = 0x1;
			switch (interface_type) {
				case QUICC_UEC_INTERFACE_TYPE_MII:
					break;
				case QUICC_UEC_INTERFACE_TYPE_RMII:
					upsmr |= QUICC_UCF_UPSMR_UEC_R10M | QUICC_UCF_UPSMR_UEC_RMM;
					break;
				default:
					assert(0);
					break;
			}
			break;
		case QUICC_UEC_SPEED_100:
			ifm = 0x1;
			switch (interface_type) {
				case QUICC_UEC_INTERFACE_TYPE_MII:
					break;
				case QUICC_UEC_INTERFACE_TYPE_RMII:
					upsmr |= QUICC_UCF_UPSMR_UEC_RMM;
					break;
				default:
					assert(0);
					break;
			}
			break;
		default:
			assert(0);
			break;
	}

	maccfg2 = QUICC_UCF_MACCFG2_IFM_SET(maccfg2, ifm);
	maccfg2 = (maccfg2 & ~fdx_flag) | fdx_new;

	ucf_regs->maccfg2 = maccfg2;
	ucf_regs->upsmr = upsmr;
}

static void init_mac(quicc_uec_context *self, quicc_ucf_context *ucf_context, const quicc_uec_config *uec_config)
{
	volatile quicc_ucf *ucf_regs = ucf_context->ucf_regs;

	ucf_regs->upsmr = QUICC_UCF_UPSMR_UEC_HSE | QUICC_UCF_UPSMR_UEC_MUST_BE_ONE;
	ucf_regs->maccfg1 = 0;
	ucf_regs->maccfg2 = QUICC_UCF_MACCFG2_PREL(7)
		| QUICC_UCF_MACCFG2_MUST_BE_ONE
		| QUICC_UCF_MACCFG2_LC
		| QUICC_UCF_MACCFG2_PAD_CRC;

	quicc_uec_set_interface_mode(self, uec_config->interface_type, uec_config->speed, true);
}

static void init_mii(quicc_uec_context *self, quicc_ucf_context *ucf_context, const quicc_uec_config *uec_config)
{
	quicc_context *context = ucf_context->context;
	volatile quicc_ucf *ucf_regs = ucf_context->ucf_regs;

	quicc_set_mii_clock_source(context, ucf_context->config->index);
	ucf_regs->miimcfg = QUICC_UCF_MIIMCFG_CLOCK_SELECT(0x7);
}

static void init_bd(quicc_uec_context *self, quicc_ucf_context *ucf_context, const quicc_uec_config *uec_config)
{
	volatile quicc_ucf *ucf_regs = ucf_context->ucf_regs;
	quicc_bd *bd = NULL;
	size_t rx_bd_count = uec_config->rx_bd_count;
	size_t rx_bd_size = rx_bd_count * sizeof(*bd);
	size_t tx_bd_count = uec_config->tx_bd_count;
	void **per_bd_arg_begin = NULL;
	size_t i = 0;

	assert(rx_bd_count >= 8);
	assert(rx_bd_count % 4 == 0);

	assert(tx_bd_count >= 2);
	assert(tx_bd_count % 4 == 0);

	assert(UEC_MRBLR % QUICC_UCF_URFS_RFS_GET(ucf_regs->urfs) == 0);

	quicc_bd_rx_init(&self->rx_bd_context, rx_bd_count, uec_config->fill_rx_bd, uec_config->bd_arg);
	quicc_bd_tx_init(&self->tx_bd_context, tx_bd_count, -1);
}

static uint32_t get_rx_thread_data_struct(quicc_uec_context *self, quicc_context *context, const quicc_uec_config *uec_config)
{
	size_t thread_count = thread_counts [uec_config->rx_thread_count];
	size_t size = thread_count * sizeof(*self->rx_thread_data_struct);

	/* FIXME: Why not align to 128? */
	ssize_t offset = quicc_muram_allocate_offset(context, size, 256);
	quicc_uec_rx_tds *tds = quicc_muram_offset_to_address(context, offset);

	assert(offset > 0);

	self->rx_thread_data_struct = tds;

	return (uint32_t) offset;
}

static uint32_t get_rx_bd_queue(quicc_uec_context *self, quicc_context *context, const quicc_uec_config *uec_config)
{
	size_t bd_queue_size = sizeof(*self->rx_bd_queue);
	size_t size = bd_queue_size + sizeof(*self->rx_bd_prefetch);
	ssize_t offset = quicc_muram_allocate_offset(context, size, 8);
	quicc_uec_rx_bd_queue *bd_queue = quicc_muram_offset_to_address(context, offset);

	assert(offset > 0);

	bd_queue->ebbdptr = (uint32_t) self->rx_bd_context.bd_begin;

	self->rx_bd_queue = bd_queue;
	self->rx_bd_prefetch = (quicc_uec_rx_bd_prefetch *) (((char *) bd_queue) + bd_queue_size);

	return (uint32_t) offset;
}

static void init_rx_param(quicc_uec_context *self, quicc_ucf_context *ucf_context, const quicc_uec_config *uec_config)
{
	quicc_context *context = ucf_context->context;
	size_t size = sizeof(*self->rx_global_param);
	ssize_t offset = quicc_muram_allocate_offset(context, size, 64);
	quicc_uec_rx_gparam *gparam = quicc_muram_offset_to_address(context, offset);

	assert(offset > 0);

	self->rx_global_param = gparam;

	gparam->rqptr = get_rx_thread_data_struct(self, context, uec_config);
	gparam->typeorlen_rxgstpack = QUICC_UEC_RX_GPARAM_TYPE_OR_LEN(3072);
	gparam->busyvector_rstate = QUICC_UEC_RX_GPARAM_RSTATE_BMR_GBL | QUICC_UEC_RX_GPARAM_RSTATE_BMR_BO(0x2);
	assert(uec_config->max_rx_buf_len > 0);
	assert(uec_config->max_rx_buf_len % 128 == 0);
	gparam->ovskip_mrblr = QUICC_UEC_RX_GPARAM_MRBLR(uec_config->max_rx_buf_len);
	gparam->rdbqptr = get_rx_bd_queue(self, context, uec_config);
	gparam->mflr_minflr = QUICC_UEC_RX_GPARAM_MFLR(1518) | QUICC_UEC_RX_GPARAM_MINFLR(64);
	gparam->maxd1_maxd2 = QUICC_UEC_RX_GPARAM_MAXD1(1520) | QUICC_UEC_RX_GPARAM_MAXD2(1520);
	gparam->vlantype_tci = QUICC_UEC_RX_GPARAM_VLAN_TYPE(0x8100);
}

static uint32_t get_tx_thread_data_struct(quicc_uec_context *self, quicc_context *context, const quicc_uec_config *uec_config)
{
	size_t thread_count = thread_counts [uec_config->tx_thread_count];
	size_t size = thread_count * sizeof(*self->tx_thread_data_struct);
	ssize_t offset = quicc_muram_allocate_offset(context, size, 128);
	quicc_uec_tx_tds *tds = quicc_muram_offset_to_address(context, offset);

	assert(offset > 0);

	self->tx_thread_data_struct = tds;

	return (uint32_t) offset;
}

static uint32_t get_tx_send_queue_desc(quicc_uec_context *self, quicc_context *context, const quicc_uec_config *uec_config)
{
	size_t size = sizeof(*self->tx_send_queue_desc);
	ssize_t offset = quicc_muram_allocate_offset(context, size, 32);
	quicc_uec_tx_sqqd *sqqd = quicc_muram_offset_to_address(context, offset);
	volatile quicc_bd *tx_bd_begin = self->tx_bd_context.bd_begin;

	assert(offset > 0);

	self->tx_send_queue_desc = sqqd;

	sqqd->bd_ring_base = (uint32_t) tx_bd_begin;
	sqqd->last_bd_completed_addr = (uint32_t) (tx_bd_begin + uec_config->tx_bd_count);

	return (uint32_t) offset;
}

static void init_tx_param(quicc_uec_context *self, quicc_ucf_context *ucf_context, const quicc_uec_config *uec_config)
{
	quicc_context *context = ucf_context->context;
	size_t size = sizeof(*self->tx_global_param);
	ssize_t offset = quicc_muram_allocate_offset(context, size, 64);
	quicc_uec_tx_gparam *gparam = quicc_muram_offset_to_address(context, offset);

	assert(offset > 0);

	self->tx_global_param = gparam;

	gparam->temorder = QUICC_UEC_TX_GPARAM_TEMORDER_START_OF_FRAME | QUICC_UEC_TX_GPARAM_TEMORDER_START_OF_BD;
	gparam->sqptr = get_tx_send_queue_desc(self, context, uec_config);
	gparam->tstate = QUICC_UEC_TX_GPARAM_TSTATE_BMR_GBL | QUICC_UEC_TX_GPARAM_TSTATE_BMR_BO(0x2);
	gparam->tqptr = get_tx_thread_data_struct(self, context, uec_config);
}

static void init_rx_tx_param(quicc_uec_context *self, quicc_ucf_context *ucf_context, const quicc_uec_config *uec_config)
{
	quicc_context *context = ucf_context->context;
	size_t size = sizeof(*self->rx_tx_param);
	ssize_t offset = quicc_muram_allocate_offset(context, size, 4);
	quicc_uec_rx_tx_param *param = quicc_muram_offset_to_address(context, offset);
	int rx_thread_count = thread_counts [uec_config->rx_thread_count];
	int tx_thread_count = thread_counts [uec_config->tx_thread_count];
	int rx_global_snum_index = quicc_snum_allocate_index(context);
	int rx_global_snum = quicc_index_to_snum(rx_global_snum_index);
	uint32_t risc_alloc = QUICC_UEC_RX_TX_PARAM_RISC_ALLOC(0x1);
	int i = 0;

	assert(offset > 0);
	assert(rx_global_snum_index >= 0);

	self->rx_tx_param = param;

	param->reserved_00 = 0x0630ff00;
	param->hcen_llks = 0x04000000;
	param->rgf_tgf_rxgparmpage_riscalloc = QUICC_UEC_RX_TX_PARAM_RGF(rx_thread_count)
		| QUICC_UEC_RX_TX_PARAM_TGF(tx_thread_count)
		| (uint32_t) quicc_muram_address_to_offset(context, self->rx_global_param)
		| risc_alloc;
	param->rxgsnum_rxvfifoblksz_riscalloc = QUICC_UEC_RX_TX_PARAM_SNUM(rx_global_snum)
		| QUICC_UEC_RX_TX_PARAM_RX_VFIFO_BLK_SZ(0x0)
		| risc_alloc;

	for (i = 0; i < rx_thread_count; ++i) {
		int snum_index = quicc_snum_allocate_index(context);
		int snum = quicc_index_to_snum(snum_index);
		ssize_t offset = quicc_muram_allocate_offset(context, sizeof(quicc_uec_rx_tparam), 128);

		assert(snum_index >= 0);

		param->rxthread [i] = QUICC_UEC_RX_TX_PARAM_SNUM(snum) | (uint32_t) offset | risc_alloc;
	}

	param->txgprampage = (uint32_t) quicc_muram_address_to_offset(context, self->tx_global_param) | risc_alloc;

	for (i = 0; i < tx_thread_count; ++i) {
		int snum_index = quicc_snum_allocate_index(context);
		int snum = quicc_index_to_snum(snum_index);
		ssize_t offset = quicc_muram_allocate_offset(context, sizeof(quicc_uec_tx_tparam), 64);

		assert(snum_index >= 0);
		assert(offset > 0);

		param->txthread [i] = QUICC_UEC_RX_TX_PARAM_SNUM(snum) | (uint32_t) offset | risc_alloc;
	}
}

static void execute_rx_tx_init(quicc_uec_context *self, quicc_ucf_context *ucf_context)
{
	quicc_context *context = ucf_context->context;
	uint32_t data = (uint32_t) quicc_muram_address_to_offset(context, self->rx_tx_param);

	ppc_synchronize_data();

	quicc_uec_execute_command(self, QUICC_CMD_FAST_INIT_RX_TX, data);
}

void quicc_uec_init(quicc_uec_context *self, quicc_ucf_context *ucf_context, const quicc_uec_config *uec_config)
{
	self->ucf_context = ucf_context;
	self->config = uec_config;

	init_mac(self, ucf_context, uec_config);
	init_mii(self, ucf_context, uec_config);
	init_bd(self, ucf_context, uec_config);
	init_rx_param(self, ucf_context, uec_config);
	init_tx_param(self, ucf_context, uec_config);
	init_rx_tx_param(self, ucf_context, uec_config);
	execute_rx_tx_init(self, ucf_context);
}
