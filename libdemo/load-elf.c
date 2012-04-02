/*
 * Copyright (c) 2011 embedded brains GmbH.  All rights reserved.
 *
 *  embedded brains GmbH
 *  Obere Lagerstr. 30
 *  82178 Puchheim
 *  Germany
 *  <info@embedded-brains.de>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "load-elf.h"

#include <string.h>

void load_elf_copy_with_memcpy(
	void *arg,
	void *dest,
	const void *src,
	size_t n
)
{
	memcpy(dest, src, n);
}

void load_elf_copy_nothing(
	void *arg,
	void *dest,
	const void *src,
	size_t n
)
{
	/* Do nothing */
}

void load_elf_init(
	load_elf_context *self,
	const load_elf_config *config
)
{
	memset(self, 0, sizeof(*self));
	self->next_state = EVAL_ELF_HEADER;
	self->dest_current = (uintptr_t) &self->elf_header;
	self->dest_end = self->dest_current + sizeof(self->elf_header);
	self->copy = load_elf_copy_with_memcpy;
	self->config = config;
}

static void reset_segment_index(load_elf_context *self)
{
	self->segment_index = (size_t) -1;
}

static void do_copy(load_elf_context *self)
{
	const load_elf_config *cfg = self->config;
	uintptr_t available = self->src_end - self->src_current;
	uintptr_t remaining = self->dest_end - self->dest_current;
	uintptr_t size = available < remaining ? available : remaining;

	(*self->copy)(
		cfg->arg,
		(void *) self->dest_current,
		(const void *) self->src_current,
		size
	);

	self->file_offset += size;
	self->src_current += size;
	self->dest_current += size;

	if (self->dest_current == self->dest_end) {
		self->state = self->next_state;
	}
}

static void do_eval_elf_header(load_elf_context *self)
{
	const load_elf_config *cfg = self->config;
	Elf32_Ehdr *e = &self->elf_header;

	if (e->e_phentsize < sizeof(Elf32_Phdr)) {
		self->state = ERROR_PROGRAM_HEADER_ENTRY_SIZE;
	} else if (e->e_ehsize != e->e_phoff) {
		self->state = ERROR_PROGRAM_HEADER_OFFSET;
	} else {
		self->segments = (*cfg->allocate_segments)(
			cfg->arg,
			e->e_phnum
		);

		if (self->segments != NULL) {
			reset_segment_index(self);
			self->state = NEXT_PROGRAM_HEADER;
		} else {
			self->state = ERROR_SEGMENTS_ALLOCATION;
		}
	}
}

static void do_eval_program_header(load_elf_context *self)
{
	Elf32_Ehdr *e = &self->elf_header;
	Elf32_Phdr *p = &self->program_header;

	if (p->p_type == PT_LOAD) {
		load_elf_segment *seg = &self->segments [self->segment_index];

		seg->file_offset = p->p_offset;
		seg->dest_begin = p->p_paddr;
		seg->dest_end = p->p_paddr + p->p_filesz;
	} else {
		--e->e_phnum;
		--self->segment_index;
	}

	self->state = SKIP_PROGRAM_HEADER;
	self->next_state = NEXT_PROGRAM_HEADER;
	self->dest_current = 0;
	self->dest_end = e->e_phentsize - sizeof(Elf32_Phdr);
	self->copy = load_elf_copy_nothing;
}

static void do_next_program_header(load_elf_context *self)
{
	Elf32_Ehdr *e = &self->elf_header;

	++self->segment_index;
	if (self->segment_index < e->e_phnum) {
		self->state = COPY_PROGRAM_HEADER;
		self->next_state = EVAL_PROGRAM_HEADER;
		self->dest_current = (uintptr_t) &self->program_header;
		self->dest_end = self->dest_current + sizeof(self->program_header);
		self->copy = load_elf_copy_with_memcpy;
	} else {
		self->state = PROGRAM_HEADER_DONE;
	}
}

static void do_program_header_done(load_elf_context *self)
{
	const load_elf_config *cfg = self->config;
	load_elf_segment *segments = self->segments;
	Elf32_Ehdr *e = &self->elf_header;

	/* Insertion sort */
	int n = e->e_phnum;
	int i = 0;
	for (i = 1; i < n; ++i) {
		load_elf_segment key = segments [i];
		int j = 0;
		for (j = i - 1; j >= 0 && segments [j].file_offset > key.file_offset; --j) {
			segments [j + 1] = segments [j];
		}
		segments [j + 1] = key;
	}

	if ((*cfg->check_segments)(cfg->arg, segments, e->e_phnum)) {
		reset_segment_index(self);
		self->state = NEXT_SEGMENT;
	} else {
		self->state = ERROR_SEGMENTS;
	}
}

static void do_next_segment(load_elf_context *self)
{
	Elf32_Ehdr *e = &self->elf_header;

	++self->segment_index;
	if (self->segment_index < e->e_phnum) {
		load_elf_segment *seg = &self->segments [self->segment_index];

		self->state = SEGMENT_SKIP;
		self->next_state = SEGMENT_SKIP_DONE;
		self->dest_current = 0;
		self->dest_end = seg->file_offset - self->file_offset;
		self->copy = load_elf_copy_nothing;
	} else {
		self->state = COPY_DONE;
	}
}

static void do_segment_skip_done(load_elf_context *self)
{
	const load_elf_config *cfg = self->config;
	load_elf_segment *seg = &self->segments [self->segment_index];

	self->state = COPY_SEGMENT;
	self->next_state = NEXT_SEGMENT;
	self->dest_current = seg->dest_begin;
	self->dest_end = seg->dest_end;
	self->copy = cfg->copy;
}

bool load_elf_process(
	load_elf_context *self,
	const void *src,
	size_t n
)
{
	bool done = false;

	self->src_current = (uintptr_t) src;
	self->src_end = self->src_current + n;
	while (self->src_current != self->src_end) {
		switch (self->state) {
			case COPY_ELF_HEADER:
			case COPY_PROGRAM_HEADER:
			case SKIP_PROGRAM_HEADER:
			case SEGMENT_SKIP:
			case COPY_SEGMENT:
				do_copy(self);
				break;
			case EVAL_ELF_HEADER:
				do_eval_elf_header(self);
				break;
			case EVAL_PROGRAM_HEADER:
				do_eval_program_header(self);
				break;
			case NEXT_PROGRAM_HEADER:
				do_next_program_header(self);
				break;
			case PROGRAM_HEADER_DONE:
				do_program_header_done(self);
				break;
			case NEXT_SEGMENT:
				do_next_segment(self);
				break;
			case SEGMENT_SKIP_DONE:
				do_segment_skip_done(self);
				break;
			default:
				self->src_current = self->src_end;
				done = true;
				break;
		}
	}

	return done;
}

void load_elf_final(load_elf_context *self)
{
	if (self->state == COPY_DONE) {
		const load_elf_config *cfg = self->config;
		uintptr_t entry = self->elf_header.e_entry;

		(*cfg->start)(cfg->arg, (void *) entry);
	}
}
