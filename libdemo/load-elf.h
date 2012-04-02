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

#include <elf.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifndef LOAD_ELF_H
#define LOAD_ELF_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum {
	COPY_ELF_HEADER = 0,
	EVAL_ELF_HEADER,
	NEXT_PROGRAM_HEADER,
	COPY_PROGRAM_HEADER,
	EVAL_PROGRAM_HEADER,
	SKIP_PROGRAM_HEADER,
	PROGRAM_HEADER_DONE,
	NEXT_SEGMENT,
	SEGMENT_SKIP,
	SEGMENT_SKIP_DONE,
	COPY_SEGMENT,
	COPY_DONE,
	ERROR_SEGMENTS,
	ERROR_SEGMENTS_ALLOCATION,
	ERROR_PROGRAM_HEADER_OFFSET,
	ERROR_PROGRAM_HEADER_ENTRY_SIZE
} load_elf_state;

typedef struct {
	uintptr_t file_offset;
	uintptr_t dest_begin;
	uintptr_t dest_end;
} load_elf_segment;

typedef void (*load_elf_copy)(
	void *arg,
	void *dest,
	const void *src,
	size_t n
);

typedef load_elf_segment *(*load_elf_allocate_segments)(
	void *arg,
	size_t count
);

typedef bool (*load_elf_check_segments)(
	void *arg,
	const load_elf_segment *segments,
	size_t count
);

typedef void (*load_elf_start)(void *arg, void *entry);

typedef struct {
	void *arg;
	load_elf_copy copy;
	load_elf_allocate_segments allocate_segments;
	load_elf_check_segments check_segments;
	load_elf_start start;
} load_elf_config;

typedef struct {
	load_elf_state state;
	load_elf_state next_state;
	Elf32_Ehdr elf_header;
	Elf32_Phdr program_header;
	size_t segment_index;
	uintptr_t file_offset;
	uintptr_t src_current;
	uintptr_t src_end;
	uintptr_t dest_current;
	uintptr_t dest_end;
	load_elf_copy copy;
	load_elf_segment *segments;
	const load_elf_config *config;
} load_elf_context;

void load_elf_copy_with_memcpy(
	void *arg,
	void *dest,
	const void *src,
	size_t n
);

void load_elf_copy_nothing(
	void *arg,
	void *dest,
	const void *src,
	size_t n
);

void load_elf_init(
	load_elf_context *self,
	const load_elf_config *config
);

bool load_elf_process(
	load_elf_context *self,
	const void *src,
	size_t n
);

void load_elf_final(load_elf_context *self);

const char *load_elf_state_description(
	const load_elf_context *self
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LOAD_ELF_H */
