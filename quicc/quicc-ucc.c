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

static const uintptr_t ucc_offsets [] = {
	0x2000,
	0x3000,
	0x2200,
	0x3200,
	0x2400,
	0x3400,
	0x2600,
	0x3600
};

volatile quicc_ucc *quicc_ucc_index_to_regs(const quicc_context *self, int ucc_index)
{
	size_t n = sizeof(ucc_offsets) / sizeof(ucc_offsets [0]);
	size_t i = (size_t) ucc_index;

	return i < n ? (volatile quicc_ucc *) (((uintptr_t) self->regs) + ucc_offsets [i]) : NULL;
}

typedef struct {
	uint8_t index : 3;
	uint8_t shift : 5;
} cmxucr_geometry;

static const cmxucr_geometry cmxucr_geometries [] = {
	{ .index = 0, .shift = 16 },
	{ .index = 2, .shift = 16 },
	{ .index = 0, .shift = 0 },
	{ .index = 2, .shift = 0 },
	{ .index = 1, .shift = 16 },
	{ .index = 3, .shift = 16 },
	{ .index = 1, .shift = 0 },
	{ .index = 3, .shift = 0 },
};

#define CMXUCR_CLK_COUNT 12

static const uint8_t cmxucr_clocks [] [CMXUCR_CLK_COUNT] = {
	{
		QUICC_CLK_NONE,
		QUICC_BRG_1,
		QUICC_BRG_2,
		QUICC_BRG_7,
		QUICC_BRG_8,
		QUICC_CLK_9,
		QUICC_CLK_10,
		QUICC_CLK_11,
		QUICC_CLK_12,
		QUICC_CLK_15,
		QUICC_CLK_16
	}, {
		QUICC_CLK_NONE,
		QUICC_BRG_5,
		QUICC_BRG_6,
		QUICC_BRG_7,
		QUICC_BRG_8,
		QUICC_CLK_13,
		QUICC_CLK_14,
		QUICC_CLK_19,
		QUICC_CLK_20,
		QUICC_CLK_15,
		QUICC_CLK_16
	}, {
		QUICC_CLK_NONE,
		QUICC_BRG_9,
		QUICC_BRG_10,
		QUICC_BRG_15,
		QUICC_BRG_16,
		QUICC_CLK_3,
		QUICC_CLK_4,
		QUICC_CLK_17,
		QUICC_CLK_18,
		QUICC_CLK_7,
		QUICC_CLK_8,
		QUICC_CLK_16
	}, {
		QUICC_CLK_NONE,
		QUICC_BRG_13,
		QUICC_BRG_14,
		QUICC_BRG_15,
		QUICC_BRG_16,
		QUICC_CLK_5,
		QUICC_CLK_6,
		QUICC_CLK_21,
		QUICC_CLK_22,
		QUICC_CLK_7,
		QUICC_CLK_8,
		QUICC_CLK_16
	}
};

static int get_clk_value(int index, quicc_clock clk)
{
	const uint8_t *values = &cmxucr_clocks [index] [0];
	int value = -1;
	int i = 0;

	for (i = 0; value == -1 && i < CMXUCR_CLK_COUNT; ++i) {
		if (values [i] == clk) {
			value = i;
		}
	}

	return value;
}

bool quicc_ucc_set_clock_source(const quicc_context *self, int ucc_index, quicc_clock rx_clk, quicc_clock tx_clk)
{
	size_t i = (size_t) ucc_index;
	size_t n = sizeof(cmxucr_geometries) / sizeof(cmxucr_geometries [0]);
	bool ok = i < n;

	if (ok) {
		cmxucr_geometry geo = cmxucr_geometries [i];
		int rx_clk_value = get_clk_value(geo.index, rx_clk);
		int tx_clk_value = get_clk_value(geo.index, tx_clk);

		ok = rx_clk_value >= 0 && tx_clk_value >= 0;

		if (ok) {
			int shift = geo.shift;
			uint32_t mask = 0xffffU << shift;
			uint32_t value = ((uint32_t) ((rx_clk_value << 4) | tx_clk_value)) << shift;
			volatile uint32_t *cmxucr = &self->regs->cmx.cmxucr [geo.index];

			*cmxucr = (*cmxucr & ~mask) | value;
		}
	}

	return ok;
}

static const quicc_interrupt interrupts [] = {
	QUICC_IRQ_UCC_1,
	QUICC_IRQ_UCC_2,
	QUICC_IRQ_UCC_3,
	QUICC_IRQ_UCC_4,
	QUICC_IRQ_UCC_5,
	QUICC_IRQ_UCC_6,
	QUICC_IRQ_UCC_7,
	QUICC_IRQ_UCC_8
};

quicc_interrupt quicc_ucc_index_to_interrupt(int ucc_index)
{
	size_t n = sizeof(interrupts) / sizeof(interrupts [0]);
	size_t i = (size_t) ucc_index;

	return i < n ? interrupts [i] : QUICC_IRQ_ERROR;
}
