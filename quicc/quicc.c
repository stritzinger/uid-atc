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
#include <string.h>

#include <rtems.h>

#include <libcpu/powerpc-utility.h>

#include "quicc.h"
#include "quicc-microcode-mpc830x.h"

static quicc_context context = {
	.regs = &QUICC,
	.iram_size = 48 * 1024,
	.muram_size = 16 * 1024,
	.microcode_size = sizeof(MPC8306_R1_0_UC_PATCH),
	.microcode_begin = &MPC8306_R1_0_UC_PATCH [0],
	.clock_frequency_in_hz = 200000000,
	.free_snum_bitfield = 0xfff
};

static void clear_muram(quicc_context *self)
{
	volatile quicc *q = self->regs;
	size_t n = self->muram_size / sizeof(q->muram.data [0]);
	size_t i = 0;

	for (i = 0; i < n; ++i) {
		q->muram.data [i] = 0;
	}
}

static void load_microcode(quicc_context *self)
{
	volatile quicc *q = self->regs;
	uint32_t *microcode = self->microcode_begin;
	size_t w = sizeof(q->iram.idata);
	size_t n = self->microcode_size / w;
	size_t m = self->iram_size / w;
	size_t i = 0;

	q->iram.iready = 0;
	q->iram.iadd = QUICC_IRAM_IADD_AIE;
	for (i = 0; i < n; ++i) {
		q->iram.idata = microcode [i];
	}
	for (i = n; i < m; ++i) {
		q->iram.idata = 0;
	}
	q->engine.cercr = QUICC_ENGINE_CERCR_MEE | QUICC_ENGINE_CERCR_IEE;
	q->iram.iready = QUICC_IRAM_IREADY_IREADY;
}

static void lock(const quicc_context *self)
{
	rtems_status_code sc = RTEMS_SUCCESSFUL;

	sc = rtems_semaphore_obtain(self->mutex_id, RTEMS_WAIT, RTEMS_NO_TIMEOUT);
	assert(sc == RTEMS_SUCCESSFUL);
}

static void unlock(const quicc_context *self)
{
	rtems_status_code sc = RTEMS_SUCCESSFUL;

	sc = rtems_semaphore_release(self->mutex_id);
	assert(sc == RTEMS_SUCCESSFUL);
}

void quicc_execute_command(
	const quicc_context *self,
	quicc_cmd_opcode opcode,
	quicc_cmd_subblock subblock,
	quicc_cmd_protocol protocol,
	uint32_t data
)
{
	volatile quicc *q = self->regs;
	uint32_t cecr = 0;

	lock(self);

	if (opcode != QUICC_CMD_RESET) {
		q->engine.cecdr = data;
	}
	q->engine.cecr = opcode | subblock | protocol | QUICC_ENGINE_CECR_FLG;

	do {
		cecr = q->engine.cecr;
	} while ((cecr & QUICC_ENGINE_CECR_FLG) != 0);

	unlock(self);
}

void quicc_reset(quicc_context *self)
{
	quicc_execute_command(
		self,
		QUICC_CMD_RESET,
		QUICC_CMD_SUBBLOCK_INVALID,
		QUICC_CMD_PROTOCOL_INVALID,
		0
	);
}

static void timestamp_init(quicc_context *self)
{
	volatile quicc *q = self->regs;
	uint32_t ps = self->clock_frequency_in_hz / 1000000 - 2;

	q->engine.cetscr = QUICC_ENGINE_CETSCR_RTE1
		| QUICC_ENGINE_CETSCR_CETPS1(ps)
		| QUICC_ENGINE_CETSCR_RTE2
		| QUICC_ENGINE_CETSCR_CETPS2(ps);
}

static void sdma_init(quicc_context *self)
{
	volatile quicc *q = self->regs;
	ssize_t offset = quicc_muram_allocate_offset(self, 2048, 4096);
	assert(offset >= 0);

	q->sdma.sdaqr = 0;
	q->sdma.sdaqmr = 0;
	q->sdma.sdebcr = QUICC_SDMA_SDEBCR_BA((uint32_t) offset);
	q->sdma.sdsr = QUICC_SDMA_SDSR_BER_1 | QUICC_SDMA_SDSR_BER_2;
	q->sdma.sdmr = QUICC_SDMA_SDMR_GLB_1_MSK | QUICC_SDMA_SDMR_CEN(0x3);
}

quicc_context *quicc_init(void)
{
	rtems_status_code sc = RTEMS_SUCCESSFUL;
	quicc_context *self = &context;

	if (self->mutex_id == RTEMS_ID_NONE) {
		sc = rtems_semaphore_create(
			rtems_build_name('Q', 'U', 'I', 'C'),
			1,
			RTEMS_BINARY_SEMAPHORE | RTEMS_PRIORITY | RTEMS_INHERIT_PRIORITY,
			0,
			&self->mutex_id
		);
		assert(sc == RTEMS_SUCCESSFUL);

		clear_muram(self);
		load_microcode(self);
		quicc_reset(self);
		timestamp_init(self);
		sdma_init(self);
	}

	return self;
}

void quicc_set_mii_clock_source(quicc_context *self, int ucc_index)
{
	volatile quicc *regs = self->regs;
	uint32_t cmxgcr = regs->cmx.cmxgcr;
	uint32_t index = (uint32_t) ucc_index;

	assert(index < QUICC_UCC_COUNT);

	regs->cmx.cmxgcr = QUICC_CMX_CMXGCR_MEM_SET(cmxgcr, index);
}

ssize_t quicc_muram_allocate_offset(quicc_context *self, size_t size, size_t align)
{
	ssize_t offset = -1;
	size_t aligned_free = 0;

	lock(self);
	aligned_free = align + ((self->muram_offset - 1) & ~(align - 1));
	if (size <= aligned_free - self->muram_size) {
		self->muram_offset = aligned_free + size;
		offset = (ssize_t) aligned_free;
	}
	unlock(self);

	return offset;
}

void *quicc_muram_offset_to_address(const quicc_context *self, ssize_t offset)
{
	if (offset > 0) {
		intptr_t begin = (intptr_t) &self->regs->muram.data [0];

		return (void *) (begin + offset);
	} else {
		return NULL;
	}
}

ssize_t quicc_muram_address_to_offset(const quicc_context *self, void *address)
{
	if (address != NULL) {
		intptr_t offset = (intptr_t) address - (intptr_t) &self->regs->muram.data [0];

		return offset;
	} else {
		return -1;
	}
}

int quicc_snum_allocate_index(quicc_context *self)
{
	int snum_index = -1;

	lock(self);
	snum_index = __builtin_ffs((int) self->free_snum_bitfield) - 1;
	if (snum_index >= 0) {
		self->free_snum_bitfield &= ~(1U << snum_index);
	}
	unlock(self);

	return snum_index;
}

void quicc_snum_free_index(quicc_context *self, int snum_index)
{
	lock(self);
	self->free_snum_bitfield |= 1U << snum_index;
	unlock(self);
}

/*
 * See "QUICC Engine Block Reference Manual with Protocol Interworking" section
 * I.4.8 "SERIAL Number (SNUM)".
 */
static const uint8_t index_to_snum [] = {
	0x88,
	0x89,
	0x98,
	0x99,
	0xa8,
	0xa9,
	0xb8,
	0xb9,
	0xc8,
	0xc9,
	0xd8,
	0xd9
};

int quicc_index_to_snum(int snum_index)
{
	size_t n = sizeof(index_to_snum) / sizeof(index_to_snum [0]);
	size_t i = (size_t) snum_index;

	return i < n ? index_to_snum [i] : 0;
}
