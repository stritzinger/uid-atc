#ifndef QUICC_H
#define QUICC_H

#include <rtems.h>
#include <rtems/irq-extension.h>

#include "quicc-regs.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define QUICC_UCC_COUNT 8

typedef enum {
	QUICC_CMD_RESET = QUICC_ENGINE_CECR_RST,
	QUICC_CMD_FAST_INIT_RX_TX = QUICC_ENGINE_CECR_OPCODE(0x00),
	QUICC_CMD_FAST_INIT_RX = QUICC_ENGINE_CECR_OPCODE(0x01),
	QUICC_CMD_FAST_INIT_TX = QUICC_ENGINE_CECR_OPCODE(0x02),
	QUICC_CMD_FAST_ENTER_HUNT_MODE = QUICC_ENGINE_CECR_OPCODE(0x03),
	QUICC_CMD_FAST_STOP_TX = QUICC_ENGINE_CECR_OPCODE(0x04),
	QUICC_CMD_FAST_GRACEFUL_STOP_TX = QUICC_ENGINE_CECR_OPCODE(0x05),
	QUICC_CMD_FAST_RESTART_TX = QUICC_ENGINE_CECR_OPCODE(0x06),
	QUICC_CMD_FAST_L2_SWITCH_COMMAND = QUICC_ENGINE_CECR_OPCODE(0x07),
	QUICC_CMD_FAST_SET_GROUP_ADDRESS = QUICC_ENGINE_CECR_OPCODE(0x08),
	QUICC_CMD_FAST_INSERT_CELL = QUICC_ENGINE_CECR_OPCODE(0x09),
	QUICC_CMD_FAST_ATM_TRANSMIT = QUICC_ENGINE_CECR_OPCODE(0x0a),
	QUICC_CMD_FAST_CELL_POOL_GET = QUICC_ENGINE_CECR_OPCODE(0x0b),
	QUICC_CMD_FAST_CELL_POOL_PUT = QUICC_ENGINE_CECR_OPCODE(0x0c),
	QUICC_CMD_FAST_IMA_HOST_CMD = QUICC_ENGINE_CECR_OPCODE(0x0d),
	QUICC_CMD_FAST_PUSHSCHED = QUICC_ENGINE_CECR_OPCODE(0x0f),
	QUICC_CMD_FAST_ATM_MULTITHREAD_INIT = QUICC_ENGINE_CECR_OPCODE(0x11),
	QUICC_CMD_FAST_ASSIGN_PAGE = QUICC_ENGINE_CECR_OPCODE(0x12),
	QUICC_CMD_FAST_SET_LAST_RECV_REQ_THRESHOLD = QUICC_ENGINE_CECR_OPCODE(0x13),
	QUICC_CMD_FAST_START_FLOW_CONTROL = QUICC_ENGINE_CECR_OPCODE(0x14),
	QUICC_CMD_FAST_STOP_FLOW_CONTROL = QUICC_ENGINE_CECR_OPCODE(0x15),
	QUICC_CMD_FAST_ASSIGN_PAGE_TO_DEVICE = QUICC_ENGINE_CECR_OPCODE(0x16),
	QUICC_CMD_FAST_GRACEFUL_STOP_RX = QUICC_ENGINE_CECR_OPCODE(0x1a),
	QUICC_CMD_FAST_RESTART_RX = QUICC_ENGINE_CECR_OPCODE(0x1b)
} quicc_cmd_opcode;

typedef enum {
	QUICC_CMD_SUBBLOCK_GENERAL = QUICC_ENGINE_CECR_SBC(0x1f0),
	QUICC_CMD_SUBBLOCK_INVALID = QUICC_ENGINE_CECR_SBC(0x000),
	QUICC_CMD_SUBBLOCK_MCC_1 = QUICC_ENGINE_CECR_SBC(0x1c0),
	QUICC_CMD_SUBBLOCK_MCC_2 = QUICC_ENGINE_CECR_SBC(0x1d0),
	QUICC_CMD_SUBBLOCK_SPI_1 = QUICC_ENGINE_CECR_SBC(0x0a0),
	QUICC_CMD_SUBBLOCK_SPI_2 = QUICC_ENGINE_CECR_SBC(0x0b0),
	QUICC_CMD_SUBBLOCK_TIMER = QUICC_ENGINE_CECR_SBC(0x0f0),
	QUICC_CMD_SUBBLOCK_UCCFAST_1 = QUICC_ENGINE_CECR_SBC(0x100),
	QUICC_CMD_SUBBLOCK_UCCFAST_2 = QUICC_ENGINE_CECR_SBC(0x110),
	QUICC_CMD_SUBBLOCK_UCCFAST_3 = QUICC_ENGINE_CECR_SBC(0x120),
	QUICC_CMD_SUBBLOCK_UCCFAST_4 = QUICC_ENGINE_CECR_SBC(0x130),
	QUICC_CMD_SUBBLOCK_UCCFAST_5 = QUICC_ENGINE_CECR_SBC(0x140),
	QUICC_CMD_SUBBLOCK_UCCFAST_6 = QUICC_ENGINE_CECR_SBC(0x150),
	QUICC_CMD_SUBBLOCK_UCCFAST_7 = QUICC_ENGINE_CECR_SBC(0x160),
	QUICC_CMD_SUBBLOCK_UCCFAST_8 = QUICC_ENGINE_CECR_SBC(0x170),
	QUICC_CMD_SUBBLOCK_UCCSLOW_1 = QUICC_ENGINE_CECR_SBC(0x000),
	QUICC_CMD_SUBBLOCK_UCCSLOW_2 = QUICC_ENGINE_CECR_SBC(0x010),
	QUICC_CMD_SUBBLOCK_UCCSLOW_3 = QUICC_ENGINE_CECR_SBC(0x020),
	QUICC_CMD_SUBBLOCK_UCCSLOW_4 = QUICC_ENGINE_CECR_SBC(0x030),
	QUICC_CMD_SUBBLOCK_UCCSLOW_5 = QUICC_ENGINE_CECR_SBC(0x040),
	QUICC_CMD_SUBBLOCK_UCCSLOW_6 = QUICC_ENGINE_CECR_SBC(0x050),
	QUICC_CMD_SUBBLOCK_UCCSLOW_7 = QUICC_ENGINE_CECR_SBC(0x060),
	QUICC_CMD_SUBBLOCK_UCCSLOW_8 = QUICC_ENGINE_CECR_SBC(0x070),
	QUICC_CMD_SUBBLOCK_USB = QUICC_ENGINE_CECR_SBC(0x190)
} quicc_cmd_subblock;

typedef enum {
	QUICC_CMD_PROTOCOL_ATM_POS = QUICC_ENGINE_CECR_MCN(0xa),
	QUICC_CMD_PROTOCOL_ETHERNET = QUICC_ENGINE_CECR_MCN(0xc),
	QUICC_CMD_PROTOCOL_HDLC_TRANSPARENT = QUICC_ENGINE_CECR_MCN(0),
	QUICC_CMD_PROTOCOL_INVALID = QUICC_ENGINE_CECR_MCN(0),
	QUICC_CMD_PROTOCOL_L2_SWITCH = QUICC_ENGINE_CECR_MCN(0xd)
} quicc_cmd_protocol;

typedef enum {
	QUICC_CLK_NONE,
	QUICC_BRG_1,
	QUICC_BRG_2,
	QUICC_BRG_3,
	QUICC_BRG_4,
	QUICC_BRG_5,
	QUICC_BRG_6,
	QUICC_BRG_7,
	QUICC_BRG_8,
	QUICC_BRG_9,
	QUICC_BRG_10,
	QUICC_BRG_11,
	QUICC_BRG_12,
	QUICC_BRG_13,
	QUICC_BRG_14,
	QUICC_BRG_15,
	QUICC_BRG_16,
	QUICC_CLK_1,
	QUICC_CLK_2,
	QUICC_CLK_3,
	QUICC_CLK_4,
	QUICC_CLK_5,
	QUICC_CLK_6,
	QUICC_CLK_7,
	QUICC_CLK_8,
	QUICC_CLK_9,
	QUICC_CLK_10,
	QUICC_CLK_11,
	QUICC_CLK_12,
	QUICC_CLK_13,
	QUICC_CLK_14,
	QUICC_CLK_15,
	QUICC_CLK_16,
	QUICC_CLK_17,
	QUICC_CLK_18,
	QUICC_CLK_19,
	QUICC_CLK_20,
	QUICC_CLK_21,
	QUICC_CLK_22,
	QUICC_CLK_23,
	QUICC_CLK_24,
	QUICC_CLK_INVALID
} quicc_clock;

typedef enum {
	QUICC_IRQ_ERROR,
	QUICC_IRQ_SPI_2,
	QUICC_IRQ_SPI_1,
	QUICC_IRQ_RTT,
	QUICC_IRQ_SDMA = 10,
	QUICC_IRQ_USB,
	QUICC_IRQ_TIMER_1,
	QUICC_IRQ_TIMER_2,
	QUICC_IRQ_TIMER_3,
	QUICC_IRQ_TIMER_4,
	QUICC_IRQ_PTP_1 = 17,
	QUICC_IRQ_VT = 20,
	QUICC_IRQ_RTC,
	QUICC_IRQ_EXT_1 = 25,
	QUICC_IRQ_EXT_2,
	QUICC_IRQ_EXT_3,
	QUICC_IRQ_EXT_4,
	QUICC_IRQ_UCC_1 = 32,
	QUICC_IRQ_UCC_2,
	QUICC_IRQ_UCC_3,
	QUICC_IRQ_UCC_4,
	QUICC_IRQ_MCC_1,
	QUICC_IRQ_UCC_5 = 40,
	QUICC_IRQ_UCC_6,
	QUICC_IRQ_UCC_7,
	QUICC_IRQ_UCC_8,
	QUICC_IRQ_MCC_2
} quicc_interrupt;

#define QUICC_IRQ_COUNT 64

typedef struct {
	volatile quicc *regs;
	rtems_id mutex_id;
	size_t iram_size;
	size_t muram_size;
	size_t muram_offset;
	size_t microcode_size;
	uint32_t *microcode_begin;
	uint32_t clock_frequency_in_hz;
	uint32_t free_snum_bitfield;
	rtems_interrupt_handler interrupt_handler [QUICC_IRQ_COUNT];
	void *interrupt_arg [QUICC_IRQ_COUNT];
} quicc_context;

static inline bool quicc_is_power_of_two(int value)
{
	int bit = __builtin_ffs(value) - 1;

	return (1 << bit) == value;
}

quicc_context *quicc_init(void);

void quicc_reset(quicc_context *self);

void quicc_set_mii_clock_source(quicc_context *self, int ucc_index);

ssize_t quicc_muram_allocate_offset(quicc_context *self, size_t size, size_t align);

void *quicc_muram_offset_to_address(const quicc_context *self, ssize_t offset);

ssize_t quicc_muram_address_to_offset(const quicc_context *self, void *address);

int quicc_snum_allocate_index(quicc_context *self);

void quicc_snum_free_index(quicc_context *self, int snum_index);

int quicc_index_to_snum(int snum_index);

volatile quicc_ucc *quicc_ucc_index_to_regs(const quicc_context *self, int ucc_index);

quicc_cmd_subblock quicc_ucc_index_to_fast_subblock(int ucc_index);

quicc_interrupt quicc_ucc_index_to_interrupt(int ucc_index);

void quicc_irq_init(quicc_context *self);

void quicc_irq_handler_install(
	quicc_context *self,
	quicc_interrupt irq,
	rtems_interrupt_handler handler,
	void *handler_arg
);

void quicc_irq_handler_remove(quicc_context *self, quicc_interrupt irq);

void quicc_irq_enable(const quicc_context *self, quicc_interrupt irq);

void quicc_irq_disable(const quicc_context *self, quicc_interrupt irq);

/**
 * @brief Sets the highest priority interrupt.
 *
 * Interrupts are disabled to make this operation atomic.
 *
 * @return Previous highest priority interrupt.
 */
quicc_interrupt quicc_irq_set_highest_priority(const quicc_context *self, quicc_interrupt irq);

void quicc_execute_command(
	const quicc_context *self,
	quicc_cmd_opcode opcode,
	quicc_cmd_subblock subblock,
	quicc_cmd_protocol protocol,
	uint32_t data
);

/**
 * @param[in] self QUICC context obtained by quicc_init().
 * @param[in] ucc_index UCC index, use 0 for UCC 1, etc.
 * @param[in] rx_clk Clock source for receive path.
 * @param[in] tx_clk Clock source for transmit path.
 *
 * @retval true Valid clock source setting.
 * @retval false Invalid index or unavailable clock sources.
 */
bool quicc_ucc_set_clock_source(const quicc_context *self, int ucc_index, quicc_clock rx_clk, quicc_clock tx_clk);

typedef enum {
	QUICC_UCF_ETHERNET_FAST,
	QUICC_UCF_ETHERNET_GIGA
} quicc_ucf_type;

typedef struct {
	int index;
	quicc_clock rx_clk;
	quicc_clock tx_clk;
	quicc_ucf_type type;
} quicc_ucf_config;

typedef struct {
	volatile quicc_ucf *ucf_regs;
	quicc_context *context;
	void *rx_virtual_fifo;
	void *tx_virtual_fifo;
	const quicc_ucf_config *config;
} quicc_ucf_context;

void quicc_ucf_init(quicc_ucf_context *self, const quicc_ucf_config *config);

typedef enum {
	QUICC_DIR_NONE = 0x0,
	QUICC_DIR_RX = 0x1,
	QUICC_DIR_TX = 0x2,
	QUICC_DIR_RX_AND_TX = 0x3
} quicc_direction;

static inline bool quicc_direction_includes_receive(quicc_direction dir)
{
	return (dir & QUICC_DIR_RX) != 0;
}

static inline bool quicc_direction_includes_transmit(quicc_direction dir)
{
	return (dir & QUICC_DIR_TX) != 0;
}

void quicc_ucf_enable(const quicc_ucf_context *self, quicc_direction dir);

void quicc_ucf_disable(const quicc_ucf_context *self, quicc_direction dir);

typedef void (*quicc_bd_wait)(void *handler_arg);

typedef void *(*quicc_bd_process)(void *handler_arg, volatile quicc_bd *bd, void *bd_arg);

typedef void *(*quicc_bd_fill)(void *handler_arg, quicc_bd *bd, bool last);

typedef void (*quicc_bd_compact)(
	void *handler_arg,
	void *discard_bd_arg,
	uint32_t *compact_bd_status,
	void **compact_bd_buffer,
	void **compact_bd_arg
);

typedef void (*quicc_bd_wait_and_free)(void *handler_arg, volatile quicc_bd *bd, void *bd_arg);

typedef struct {
	size_t index_mask;
	volatile quicc_bd *bd_begin;
	void **per_bd_arg_begin;
} quicc_bd_rx_context;

typedef struct {
	size_t current;
	size_t first_in_frame;
	size_t index_mask;
	volatile quicc_bd *bd_begin;
	void **per_bd_arg_begin;
	int frame_fragments_available;
	int frame_fragments_max;
	quicc_bd_wait_and_free wait_and_free;
	quicc_bd_compact compact;
	void *handler_arg;
} quicc_bd_tx_context;

void quicc_bd_rx_init(
	quicc_bd_rx_context *self,
	size_t bd_count,
	quicc_bd_fill fill,
	void *handler_arg
);

void quicc_bd_rx_process(
	quicc_bd_rx_context *self,
	quicc_bd_process process,
	void *handler_arg
);

void quicc_bd_tx_init(
	quicc_bd_tx_context *self,
	size_t bd_count,
	int frame_fragments_max
);

void quicc_bd_tx_submit_and_wait(
	quicc_bd_tx_context *self,
	uint32_t bd_status,
	void *bd_buffer,
	void *bd_arg,
	quicc_bd_wait_and_free wait_and_free,
	quicc_bd_compact compact,
	void *handler_arg
);

static inline bool quicc_bd_tx_can_submit(quicc_bd_tx_context *self)
{
	size_t current = self->current;
	size_t index_mask = self->index_mask;
	size_t next = (current + 1) & index_mask;
	volatile quicc_bd *bd = self->bd_begin + next;

	return (bd->status & QUICC_BD_TX_R) == 0;
}

typedef enum {
	QUICC_UEC_SPEED_10,
	QUICC_UEC_SPEED_100,
	QUICC_UEC_SPEED_1000
} quicc_uec_speed;

typedef enum {
	QUICC_UEC_THREAD_COUNT_1 = 0x1,
	QUICC_UEC_THREAD_COUNT_2 = 0x2,
	QUICC_UEC_THREAD_COUNT_4 = 0x0,
	QUICC_UEC_THREAD_COUNT_6 = 0x3,
	QUICC_UEC_THREAD_COUNT_8 = 0x4
} quicc_uec_thread_count;

typedef enum {
	QUICC_UEC_INTERFACE_TYPE_MII,
	QUICC_UEC_INTERFACE_TYPE_RMII
} quicc_uec_interface_type;

typedef struct {
	quicc_uec_speed speed;
	quicc_uec_interface_type interface_type;
	quicc_uec_thread_count rx_thread_count;
	quicc_uec_thread_count tx_thread_count;
	size_t rx_bd_count;
	size_t tx_bd_count;
	uint32_t max_rx_buf_len;
	void *bd_arg;
	quicc_bd_fill fill_rx_bd;
	int phy_address;
} quicc_uec_config;

typedef struct {
	quicc_ucf_context *ucf_context;
	quicc_bd_rx_context rx_bd_context;
	quicc_bd_tx_context tx_bd_context;
	quicc_uec_rx_gparam *rx_global_param;
	quicc_uec_tx_gparam *tx_global_param;
	quicc_uec_rx_tds *rx_thread_data_struct;
	quicc_uec_rx_bd_queue *rx_bd_queue;
	quicc_uec_rx_bd_prefetch *rx_bd_prefetch;
	quicc_uec_tx_sqqd *tx_send_queue_desc;
	quicc_uec_tx_tds *tx_thread_data_struct;
	quicc_uec_rx_tx_param *rx_tx_param;;
	const quicc_uec_config *config;
} quicc_uec_context;

void quicc_uec_init(quicc_uec_context *self, quicc_ucf_context *ucf_context, const quicc_uec_config *uec_config);

void quicc_uec_execute_command(const quicc_uec_context *uec_context, quicc_cmd_opcode opcode, uint32_t data);

void quicc_uec_enable_promiscuous_mode(const quicc_uec_context *uec_context, bool enable);

void quicc_uec_set_mac_address(const quicc_uec_context *uec_context, const uint8_t *mac_address);

void quicc_uec_set_interface_mode(
	const quicc_uec_context *self,
	quicc_uec_interface_type interface_type,
	quicc_uec_speed speed,
	bool full_duplex
);

void quicc_uec_mac_enable(const quicc_uec_context *uec_context, quicc_direction dir);

void quicc_uec_mac_disable(const quicc_uec_context *uec_context, quicc_direction dir);

void quicc_uec_start(const quicc_uec_context *uec_context, quicc_direction dir);

void quicc_uec_stop(const quicc_uec_context *uec_context, quicc_direction dir);

void quicc_uec_config_mode_enter(const quicc_uec_context *uec_context, quicc_direction dir);

void quicc_uec_config_mode_leave(const quicc_uec_context *uec_context, quicc_direction dir);

uint16_t quicc_uec_mii_read(const quicc_uec_context *uec_context, int phy, int reg);

void quicc_uec_mii_write(const quicc_uec_context *uec_context, int phy, int reg, uint16_t data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* QUICC_H */
