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

static const char *desc [] = {
	[COPY_ELF_HEADER] = "COPY_ELF_HEADER",
	[EVAL_ELF_HEADER] = "EVAL_ELF_HEADER",
	[NEXT_PROGRAM_HEADER] = "NEXT_PROGRAM_HEADER",
	[COPY_PROGRAM_HEADER] = "COPY_PROGRAM_HEADER",
	[EVAL_PROGRAM_HEADER] = "EVAL_PROGRAM_HEADER",
	[SKIP_PROGRAM_HEADER] = "SKIP_PROGRAM_HEADER",
	[PROGRAM_HEADER_DONE] = "PROGRAM_HEADER_DONE",
	[NEXT_SEGMENT] = "NEXT_SEGMENT",
	[SEGMENT_SKIP] = "SEGMENT_SKIP",
	[SEGMENT_SKIP_DONE] = "SEGMENT_SKIP_DONE",
	[COPY_SEGMENT] = "COPY_SEGMENT",
	[COPY_DONE] = "COPY_DONE",
	[ERROR_SEGMENTS] = "ERROR_SEGMENTS",
	[ERROR_SEGMENTS_ALLOCATION] = "ERROR_SEGMENTS_ALLOCATION",
	[ERROR_PROGRAM_HEADER_OFFSET] = "ERROR_PROGRAM_HEADER_OFFSET",
	[ERROR_PROGRAM_HEADER_ENTRY_SIZE] = "ERROR_PROGRAM_HEADER_ENTRY_SIZE",
};

const char *load_elf_state_description(
	const load_elf_context *self
)
{
	size_t index = self->state;

	if (index < sizeof(desc) / sizeof(desc [0])) {
		return desc [index];
	} else {
		return "INVALID STATE";
	}
}
