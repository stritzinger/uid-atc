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

#include <assert.h>

#include "quicc.h"

static const quicc_cmd_subblock subblocks [] = {
	QUICC_CMD_SUBBLOCK_UCCFAST_1,
	QUICC_CMD_SUBBLOCK_UCCFAST_2,
	QUICC_CMD_SUBBLOCK_UCCFAST_3,
	QUICC_CMD_SUBBLOCK_UCCFAST_4,
	QUICC_CMD_SUBBLOCK_UCCFAST_5,
	QUICC_CMD_SUBBLOCK_UCCFAST_6,
	QUICC_CMD_SUBBLOCK_UCCFAST_7,
	QUICC_CMD_SUBBLOCK_UCCFAST_8
};

quicc_cmd_subblock quicc_ucc_index_to_fast_subblock(int ucc_index)
{
	size_t n = sizeof(subblocks) / sizeof(subblocks [0]);
	size_t i = (size_t) ucc_index;

	return i < n ? subblocks [i] : QUICC_CMD_SUBBLOCK_INVALID;
}

void quicc_ucf_init(quicc_ucf_context *self, const quicc_ucf_config *config)
{
	quicc_context *context = quicc_init();
	volatile quicc_ucf *ucf_regs = &quicc_ucc_index_to_regs(context, config->index)->fast;
	uint32_t mode = 0x10U | (config->rx_clk != QUICC_CLK_NONE ? 0x2U : 0x0U) | (config->tx_clk != QUICC_CLK_NONE ? 0x1U : 0x0U);
	uint32_t gumr = 0;

	self->context = context;
	self->ucf_regs = ucf_regs;
	self->config = config;

	ucf_regs->guemr = QUICC_UCF_GUEMR_MODE(mode);

	switch (config->type) {
		case QUICC_UCF_ETHERNET_FAST:
		case QUICC_UCF_ETHERNET_GIGA:
			gumr = QUICC_UCF_GUMR_MODE(0xc);
			break;
		default:
			assert(0);
			break;
	}
	ucf_regs->gumr = gumr;

	if (config->type == QUICC_UCF_ETHERNET_FAST) {
		size_t fifo_size = 512;
		size_t fifo_size_1_4 = (1 * fifo_size) / 4;
		size_t fifo_size_2_4 = (2 * fifo_size) / 4;
		size_t fifo_size_3_4 = (3 * fifo_size) / 4;
		size_t fifo_align = 8;
		/* FIXME: Why +8? */
		ssize_t rx_fifo_offset = quicc_muram_allocate_offset(context, fifo_size + 8, fifo_align);
		ssize_t tx_fifo_offset = quicc_muram_allocate_offset(context, fifo_size, fifo_align);

		assert(rx_fifo_offset > 0);
		assert(tx_fifo_offset > 0);

		/* Virtual receive FIFO setup */
		self->rx_virtual_fifo = quicc_muram_offset_to_address(context, rx_fifo_offset);
		ucf_regs->urfb = QUICC_UCF_URFB_ADDR((uint32_t) rx_fifo_offset);
		ucf_regs->urfs = QUICC_UCF_URFS_RFS(fifo_size);
		ucf_regs->urfet_urfset = QUICC_UCF_URFET_URFSET_RFET(fifo_size_2_4) | QUICC_UCF_URFET_URFSET_RSFET(fifo_size_3_4);

		/* Virtual transmit FIFO setup */
		self->tx_virtual_fifo = quicc_muram_offset_to_address(context, tx_fifo_offset);
		ucf_regs->utfb = QUICC_UCF_UTFB_ADDR((uint32_t) tx_fifo_offset);
		ucf_regs->utfs = QUICC_UCF_UTFS_TFS(fifo_size);
		ucf_regs->utfet = QUICC_UCF_UTFET_TFET(fifo_size_2_4);
		ucf_regs->utftt = QUICC_UCF_UTFET_TFET(fifo_size_1_4);
	} else {
		assert(0);
	}

	quicc_ucc_set_clock_source(context, config->index, config->rx_clk, config->tx_clk);

	/* Disable and clear interrupts */
	ucf_regs->uccm = 0x0;
	ucf_regs->ucce = 0xffffffff;
}

void quicc_ucf_enable(const quicc_ucf_context *self, quicc_direction dir)
{
	uint32_t gumr = self->ucf_regs->gumr;

	if (quicc_direction_includes_receive(dir)) {
		gumr |= QUICC_UCF_GUMR_ENR;
	}
	if (quicc_direction_includes_transmit(dir)) {
		gumr |= QUICC_UCF_GUMR_ENT;
	}

	self->ucf_regs->gumr = gumr;
}

void quicc_ucf_disable(const quicc_ucf_context *self, quicc_direction dir)
{
	uint32_t gumr = self->ucf_regs->gumr;

	if (quicc_direction_includes_receive(dir)) {
		gumr &= ~QUICC_UCF_GUMR_ENR;
	}
	if (quicc_direction_includes_transmit(dir)) {
		gumr &= ~QUICC_UCF_GUMR_ENT;
	}

	self->ucf_regs->gumr = gumr;
}
