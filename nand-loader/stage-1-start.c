/*
 * Copyright (c) 2013-2014 embedded brains GmbH.  All rights reserved.
 *
 *  embedded brains GmbH
 *  Obere Lagerstr. 30
 *  82178 Puchheim
 *  Germany
 *  <rtems@embedded-brains.de>
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

#include "nand-chip.h"

#include <assert.h>
#include <string.h>

#include <libcpu/powerpc-utility.h>

#include <mpc83xx/mpc83xx.h>

#include <bsp/bootcard.h>
#include <bsp/start.h>
#include <bsp/linker-symbols.h>

BSP_START_TEXT_SECTION static inline void bed_elbc_write_workaround(volatile uint8_t *last);

BSP_START_TEXT_SECTION static inline void bed_elbc_init_module(
	volatile bed_elbc *elbc,
	uint32_t bank,
	uint32_t br,
	uint32_t or_timing
);

BSP_START_TEXT_SECTION static inline void bed_elbc_set_ecc_mode(
	volatile bed_elbc *elbc,
	uint32_t bank,
	bed_elbc_ecc_mode mode
);

BSP_START_TEXT_SECTION static inline uint32_t bed_elbc_finalize_module(
	volatile bed_elbc *elbc,
	uint32_t bank,
	bool has_large_pages,
	bool needs_3_page_cycles,
	bed_elbc_ecc_mode mode
);

BSP_START_TEXT_SECTION static inline uint32_t bed_elbc_wait(volatile bed_elbc *elbc, uint32_t flags);

BSP_START_TEXT_SECTION static inline uint32_t bed_elbc_execute(volatile bed_elbc *elbc, uint32_t bank, uint32_t fmr);

BSP_START_TEXT_SECTION static inline void bed_elbc_enable_micron_ecc(
	volatile bed_elbc *elbc,
	uint32_t bank,
	uint32_t fmr,
	volatile uint8_t *base_address,
	bool enable
);

BSP_START_TEXT_SECTION static inline bool bed_elbc_is_micron_ecc_enabled(
	volatile bed_elbc *elbc,
	uint32_t bank,
	uint32_t fmr,
	volatile uint8_t *base_address
);

BSP_START_TEXT_SECTION static inline uint8_t bed_elbc_read_status(
	volatile bed_elbc *elbc,
	uint32_t bank,
	uint32_t fmr,
	volatile uint8_t *base_address
);

BSP_START_TEXT_SECTION static inline void bed_elbc_read_mode(
	volatile bed_elbc *elbc,
	uint32_t bank,
	uint32_t fmr
);

void __wrap_boot_card(const char *cmdline);

void __real_boot_card(const char *cmdline);

extern char nand_loader_section_data_end[];

BSP_START_TEXT_SECTION void nand_loader_reset(void)
{
	while (1);
}

BSP_START_TEXT_SECTION static void enable_micron_ecc(volatile bed_elbc *elbc, uint32_t fmr)
{
#ifdef USE_MICRON_WITH_INTERNAL_ECC
	bed_elbc_enable_micron_ecc(elbc, BANK, fmr, (uint8_t *) BASE_ADDRESS, true);
	if (!bed_elbc_is_micron_ecc_enabled(elbc, BANK, fmr, (uint8_t *) BASE_ADDRESS)) {
		nand_loader_reset();
	}
#else
	(void) elbc;
	(void) fmr;
#endif
}

BSP_START_TEXT_SECTION static void check_micron_ecc_status(volatile bed_elbc *elbc, uint32_t fmr)
{
#ifdef USE_MICRON_WITH_INTERNAL_ECC
	if ((bed_elbc_read_status(elbc, BANK, fmr, (uint8_t *) BASE_ADDRESS) & BED_NAND_STATUS_FAIL) != 0) {
		nand_loader_reset();
	}

	bed_elbc_read_mode(elbc, BANK, fmr);
#else
	(void) elbc;
	(void) fmr;
#endif
}

BSP_START_TEXT_SECTION static uint32_t elbc_fcm_init(volatile bed_elbc *elbc)
{
	uint32_t fmr;

	bed_elbc_init_module(
		elbc,
		0,
		ELBC_LARGE_PAGE_BR(BASE_ADDRESS),
		MT29F2G08ABAEA_LCLK_72_5_MHZ_OR
	);

	fmr = bed_elbc_finalize_module(
		elbc,
		BANK,
		HAS_LARGE_PAGES,
		NEEDS_3_PAGE_CYCLES,
		ELBC_ECC_MODE_NONE
	);

	enable_micron_ecc(elbc, fmr);

	return fmr;
}

BSP_START_TEXT_SECTION static void elbc_fcm_read_page(
	volatile bed_elbc *elbc,
	uint32_t fmr,
	uint32_t page,
	uint32_t *dst,
	size_t dst_word_count,
	uint8_t **oob
)
{
	uint32_t ltesr;
	uint32_t buffer_index;
	uint32_t *current_buffer;

#if HAS_LARGE_PAGES
	elbc->fir = (ELBC_FIR_OP_CW0 << ELBC_FIR_OP0_SHIFT)
		| (ELBC_FIR_OP_CA << ELBC_FIR_OP1_SHIFT)
		| (ELBC_FIR_OP_PA << ELBC_FIR_OP2_SHIFT)
		| (ELBC_FIR_OP_CW1 << ELBC_FIR_OP3_SHIFT)
		| (ELBC_FIR_OP_RBW << ELBC_FIR_OP4_SHIFT);
	elbc->fcr = ((BED_NAND_CMD_READ_PAGE & 0xffU) << ELBC_FCR_CMD0_SHIFT)
		| ((BED_NAND_CMD_READ_PAGE_2 & 0xffU) << ELBC_FCR_CMD1_SHIFT);
	elbc->fbar = page >> 6;
	elbc->fpar = ELBC_FPAR_LP_PI(page);
	elbc->fbcr = 0;

	buffer_index = (page & 0x1) << 2;
	current_buffer = (uint32_t *) BASE_ADDRESS + buffer_index * (1024 / sizeof(current_buffer[0]));
	*oob = (uint8_t *) current_buffer + PAGE_SIZE;
#else
	#error "TODO"
#endif

	ltesr = bed_elbc_execute(elbc, BANK, fmr);
	if ((ltesr & ELBC_LTE_NAND_STATUS) == ELBC_LTE_CC) {
		size_t i;
		for (i = 0; i < dst_word_count; ++i) {
			dst[i] = current_buffer[i];
		}

		check_micron_ecc_status(elbc, fmr);
	} else {
		nand_loader_reset();
	}
}

BSP_START_TEXT_SECTION void __wrap_boot_card(const char *cmdline)
{
	volatile bed_elbc *elbc = ELBC;
	uint32_t fmr;
	uint32_t *load = (uint32_t *) bsp_section_text_begin;
	uintptr_t end = (uintptr_t) nand_loader_section_data_end;
	uint32_t page = (uintptr_t) load / PAGE_SIZE;
	uint8_t *oob;

	fmr = elbc_fcm_init(elbc);

	while (page < STAGE_1_PAGES && (uintptr_t) load < end) {
		size_t remaining = end - (uintptr_t) load;
		size_t remaining_current = remaining >= PAGE_SIZE ? PAGE_SIZE : remaining;
		size_t current_word_count = (remaining_current + sizeof(load[0]) - 1) / sizeof(load[0]);

		elbc_fcm_read_page(elbc, fmr, page, load, current_word_count, &oob);

		if (page % PAGES_PER_BLOCK != 0 || oob[0] == 0xff) {
			++page;
			load += current_word_count;
		} else {
			/* This is a bad block */
			page += PAGES_PER_BLOCK;
		}
	}

	__real_boot_card(cmdline);
}
