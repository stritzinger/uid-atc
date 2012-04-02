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

#include <bsp/irq.h>

static void quicc_irq_default(void *arg)
{
	printk("spurious QUICC interrupt: %i\n", (int) arg);
}

static void quicc_irq_dispatch(
	const quicc_context *self,
	volatile quicc_irq *irq_regs,
	volatile uint32_t *vec_reg
)
{
	uint32_t vec = QUICC_IRQ_VEC_GET(*vec_reg);

	while (vec != QUICC_IRQ_ERROR) {
		(*self->interrupt_handler [vec])(self->interrupt_arg [vec]);
		vec = QUICC_IRQ_VEC_GET(*vec_reg);
	}
}

static void quicc_irq_dispatch_high(void *arg)
{
	const quicc_context *self = arg;
	volatile quicc_irq *irq_regs = &self->regs->irq;

	quicc_irq_dispatch(self, irq_regs, &irq_regs->chivec);
}

static void quicc_irq_dispatch_low(void *arg)
{
	const quicc_context *self = arg;
	volatile quicc_irq *irq_regs = &self->regs->irq;

	quicc_irq_dispatch(self, irq_regs, &irq_regs->civec);
}

static bool quicc_irq_is_initialized(const quicc_context *self)
{
	return self->interrupt_handler [QUICC_IRQ_ERROR] == quicc_irq_default;
}

static void quicc_irq_set_default_handler(quicc_context *self, size_t irq)
{
	self->interrupt_handler [irq] = quicc_irq_default;
	self->interrupt_arg [irq] = (void *) irq;
}

void quicc_irq_init(quicc_context *self)
{
	rtems_status_code sc = RTEMS_SUCCESSFUL;

	if (!quicc_irq_is_initialized(self)) {
		volatile quicc_irq *irq_regs = &self->regs->irq;
		size_t i = 0;
		size_t n = sizeof(self->interrupt_handler) / sizeof(self->interrupt_handler [0]);
		uint32_t default_prio = 0x05309770;

		for (i = 0; i < n; ++i) {
			quicc_irq_set_default_handler(self, i);
		}

		irq_regs->cicr = 0;
		irq_regs->cicnr = 0;
		irq_regs->cricr = 0;
		irq_regs->cipxcc = default_prio;
		irq_regs->cipycc = default_prio;
		irq_regs->cipzcc = default_prio;
		irq_regs->cipwcc = default_prio;
		irq_regs->ciprta = default_prio;
		irq_regs->ciprtb = default_prio;
		irq_regs->cimr = 0;
		irq_regs->crimr = 0;

		sc = rtems_interrupt_handler_install(
			BSP_IPIC_IRQ_QUICC_HI,
			"QUICC High",
			RTEMS_INTERRUPT_UNIQUE,
			quicc_irq_dispatch_high,
			self
		);
		assert(sc == RTEMS_SUCCESSFUL);

		sc = rtems_interrupt_handler_install(
			BSP_IPIC_IRQ_QUICC_LO,
			"QUICC Low",
			RTEMS_INTERRUPT_UNIQUE,
			quicc_irq_dispatch_low,
			self
		);
		assert(sc == RTEMS_SUCCESSFUL);
	}
}

void quicc_irq_handler_install(
	quicc_context *self,
	quicc_interrupt irq,
	rtems_interrupt_handler handler,
	void *handler_arg
)
{
	self->interrupt_handler [irq] = handler;
	self->interrupt_arg [irq] = handler_arg;
	quicc_irq_enable(self, irq);
}

void quicc_irq_handler_remove(quicc_context *self, quicc_interrupt irq)
{
	quicc_irq_disable(self, irq);
	quicc_irq_set_default_handler(self, irq);
}

typedef struct {
	uint8_t bit : 5;
	uint8_t reg : 1;
} quicc_irq_mask;

#define QUICC_IRQ_MASK(b, r) { .bit = 31 - b, .reg = r }

static const quicc_irq_mask quicc_irq_mask_table [] = {
	[QUICC_IRQ_SPI_2] = QUICC_IRQ_MASK(16, 0),
	[QUICC_IRQ_SPI_1] = QUICC_IRQ_MASK(17, 0),
	[QUICC_IRQ_RTT] = QUICC_IRQ_MASK(18, 0),
	[QUICC_IRQ_SDMA] = QUICC_IRQ_MASK(25, 0),
	[QUICC_IRQ_USB] = QUICC_IRQ_MASK(26, 0),
	[QUICC_IRQ_TIMER_1] = QUICC_IRQ_MASK(27, 0),
	[QUICC_IRQ_TIMER_2] = QUICC_IRQ_MASK(28, 0),
	[QUICC_IRQ_TIMER_3] = QUICC_IRQ_MASK(29, 0),
	[QUICC_IRQ_TIMER_4] = QUICC_IRQ_MASK(30, 0),
	[QUICC_IRQ_PTP_1] = QUICC_IRQ_MASK(0, 1),
	[QUICC_IRQ_VT] = QUICC_IRQ_MASK(3, 1),
	[QUICC_IRQ_RTC] = QUICC_IRQ_MASK(4, 1),
	[QUICC_IRQ_EXT_1] = QUICC_IRQ_MASK(8, 1),
	[QUICC_IRQ_EXT_2] = QUICC_IRQ_MASK(9, 1),
	[QUICC_IRQ_EXT_3] = QUICC_IRQ_MASK(10, 1),
	[QUICC_IRQ_EXT_4] = QUICC_IRQ_MASK(11, 1),
	[QUICC_IRQ_UCC_1] = QUICC_IRQ_MASK(0, 0),
	[QUICC_IRQ_UCC_2] = QUICC_IRQ_MASK(1, 0),
	[QUICC_IRQ_UCC_3] = QUICC_IRQ_MASK(2, 0),
	[QUICC_IRQ_UCC_4] = QUICC_IRQ_MASK(3, 0),
	[QUICC_IRQ_MCC_1] = QUICC_IRQ_MASK(4, 0),
	[QUICC_IRQ_UCC_5] = QUICC_IRQ_MASK(8, 0),
	[QUICC_IRQ_UCC_6] = QUICC_IRQ_MASK(9, 0),
	[QUICC_IRQ_UCC_7] = QUICC_IRQ_MASK(10, 0),
	[QUICC_IRQ_UCC_8] = QUICC_IRQ_MASK(11, 0),
	[QUICC_IRQ_MCC_2] = QUICC_IRQ_MASK(12, 0)
};

static uint32_t quicc_irq_get_mask_bit(quicc_irq_mask mask)
{
	return 1U << mask.bit;
}

static volatile uint32_t *quicc_irq_get_mask_reg(const quicc_context *self, quicc_irq_mask mask)
{
	volatile quicc_irq *irq_regs = &self->regs->irq;

	return mask.reg == 0 ? &irq_regs->cimr : &irq_regs->crimr;
}

void quicc_irq_enable(const quicc_context *self, quicc_interrupt irq)
{
	quicc_irq_mask mask = quicc_irq_mask_table [irq];
	uint32_t bit = quicc_irq_get_mask_bit(mask);
	volatile uint32_t *mask_reg = quicc_irq_get_mask_reg(self, mask);
	rtems_interrupt_level level;

	rtems_interrupt_disable(level);
	*mask_reg |= bit;
	rtems_interrupt_enable(level);
}

void quicc_irq_disable(const quicc_context *self, quicc_interrupt irq)
{
	quicc_irq_mask mask = quicc_irq_mask_table [irq];
	uint32_t bit = quicc_irq_get_mask_bit(mask);
	volatile uint32_t *mask_reg = quicc_irq_get_mask_reg(self, mask);
	rtems_interrupt_level level;

	rtems_interrupt_disable(level);
	*mask_reg &= ~bit;
	rtems_interrupt_enable(level);
}

quicc_interrupt quicc_irq_set_highest_priority(const quicc_context *self, quicc_interrupt irq)
{
	rtems_interrupt_level level;
	volatile uint32_t *cicr_reg = &self->regs->irq.cicr;
	uint32_t cicr = 0;

	rtems_interrupt_disable(level);
	cicr = *cicr_reg;
	*cicr_reg = QUICC_IRQ_CICR_HP_SET(cicr, irq);
	rtems_interrupt_enable(level);

	return QUICC_IRQ_CICR_HP_GET(cicr);
}
