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

void quicc_bd_rx_init(
	quicc_bd_rx_context *self,
	size_t bd_count,
	quicc_bd_fill fill,
	void *handler_arg
)
{
	size_t bd_size = bd_count * sizeof(quicc_bd);
	quicc_bd *bd_begin = rtems_heap_allocate_aligned_with_boundary(bd_size, 32, 0);
	size_t per_bd_arg_size = bd_count * sizeof(void *);
	void **per_bd_arg_begin = malloc(per_bd_arg_size);
	size_t i = 0;

	assert(bd_begin != NULL);
	assert(per_bd_arg_begin != NULL);
	assert(bd_count > 0);
	assert(quicc_is_power_of_two((int) bd_count));

	for (i = 0; i < bd_count; ++i) {
		per_bd_arg_begin [i] = (*fill)(handler_arg, bd_begin + i, i == bd_count - 1);
	}

	self->index_mask = bd_count - 1;
	self->bd_begin = bd_begin;
	self->per_bd_arg_begin = per_bd_arg_begin;
}

void quicc_bd_rx_process(
	quicc_bd_rx_context *self,
	quicc_bd_process process,
	void *handler_arg
)
{
	volatile quicc_bd *bd_begin = self->bd_begin;
	void **per_bd_arg_begin = self->per_bd_arg_begin;
	size_t current = 0;
	size_t index_mask = self->index_mask;

	while (true) {
		volatile quicc_bd *bd = bd_begin + current;
		void **per_bd_arg = per_bd_arg_begin + current;

		*per_bd_arg = (*process)(handler_arg, bd, *per_bd_arg);

		current = (current + 1) & index_mask;
	}
}

void quicc_bd_tx_init(quicc_bd_tx_context *self, size_t bd_count, int frame_fragments_max)
{
	size_t bd_size = bd_count * sizeof(quicc_bd);
	quicc_bd *bd_begin = rtems_heap_allocate_aligned_with_boundary(bd_size, 32, 0);
	size_t per_bd_arg_size = bd_count * sizeof(void *);
	void **per_bd_arg_begin = malloc(per_bd_arg_size);

	if (frame_fragments_max <= 0) {
		frame_fragments_max = (int) bd_count - 1;
	}

	assert(bd_begin != NULL);
	assert(per_bd_arg_begin != NULL);
	assert(bd_count > 0);
	assert((int) bd_count >= frame_fragments_max);
	assert(quicc_is_power_of_two((int) bd_count));

	memset(bd_begin, 0, bd_size);
	bd_begin [bd_count - 1].status = QUICC_BD_W;

	memset(per_bd_arg_begin, 0, per_bd_arg_size);

	self->current = 0;
	self->first_in_frame = 0;
	self->index_mask = bd_count - 1;
	self->bd_begin = bd_begin;
	self->per_bd_arg_begin = per_bd_arg_begin;
	self->frame_fragments_available = frame_fragments_max;
	self->frame_fragments_max = frame_fragments_max;
}

void quicc_bd_tx_submit_and_wait(
	quicc_bd_tx_context *self,
	uint32_t bd_status,
	void *bd_buffer,
	void *bd_arg,
	quicc_bd_wait_and_free wait_and_free,
	quicc_bd_compact compact,
	void *handler_arg
)
{
	size_t current = self->current;
	size_t first_in_frame = self->first_in_frame;
	size_t index_mask = self->index_mask;
	volatile quicc_bd *bd_begin = self->bd_begin;
	void **per_bd_arg_begin = self->per_bd_arg_begin;
	int frame_fragments_available = self->frame_fragments_available;

	if (__builtin_expect(frame_fragments_available <= 0, 0)) {
		size_t discard = first_in_frame;
		size_t last = current;
		size_t end = (last + 1) & index_mask;

		per_bd_arg_begin [last] = bd_arg;

		bd_status = bd_begin [discard].status & QUICC_BD_W;
		bd_buffer = NULL;
		bd_arg = NULL;

		do {
			void *discard_bd_arg = per_bd_arg_begin [discard];
			per_bd_arg_begin [discard] = NULL;

			bd_begin [discard].status &= QUICC_BD_W;

			(*compact)(
				handler_arg,
				discard_bd_arg,
				&bd_status,
				&bd_buffer,
				&bd_arg
			);

			discard = (discard + 1) & index_mask;
		} while (discard != end);

		frame_fragments_available = self->frame_fragments_max - 1;
		current = first_in_frame;
	}

	volatile quicc_bd *bd_current = bd_begin + current;
	bool is_last_fragment = (bd_status & QUICC_BD_L) != 0;
	bool is_first_fragment = current == first_in_frame;
	uint32_t bd_wrap = current != index_mask ? 0 : QUICC_BD_W;
	uint32_t bd_ready = (is_last_fragment || !is_first_fragment) ? QUICC_BD_TX_R : 0;

	bd_current->buffer = (uint32_t) bd_buffer;
	bd_current->status = bd_status | bd_wrap | bd_ready;

	per_bd_arg_begin [current] = bd_arg;

	if (is_last_fragment && !is_first_fragment) {
		volatile quicc_bd *bd_first_in_frame = bd_begin + first_in_frame;

		bd_first_in_frame->status |= QUICC_BD_TX_R;
	}

	current = (current + 1) & index_mask;

	if (is_last_fragment) {
		self->first_in_frame = current;
		self->frame_fragments_available = self->frame_fragments_max;
	} else {
		self->frame_fragments_available = frame_fragments_available - 1;
	}

	self->current = current;

	(*wait_and_free)(handler_arg, bd_begin + current, per_bd_arg_begin [current]);
}
