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

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <rtems.h>

#include <libcpu/powerpc-utility.h>

#include <mpc83xx/mpc83xx.h>

#include <uid/nand-chip.h>

#define SIZE_4K (2 * PAGE_SIZE)

#define OOB_SIZE_4K (2 * OOB_SIZE)

static volatile bed_elbc *const elbc = ELBC;

static const uint32_t bank = BANK;

static uint8_t *const base_address = (uint8_t *) BASE_ADDRESS;

static const uint32_t fmr = ELBC_FMR_CWTO(0xf) | ELBC_FMR_ECCM | ELBC_FMR_AL(0x1);

static uint8_t stage_1_4k_data[2 * PAGE_SIZE] __attribute__((aligned(SIZE_4K)));

static uint8_t tmp_4k_data[2 * PAGE_SIZE];

static uint8_t data_buf[PAGE_SIZE];

static uint8_t stage_1_4k_oob[2 * OOB_SIZE];

static uint8_t tmp_4k_oob[2 * OOB_SIZE];

static uint8_t oob_buf[OOB_SIZE];

extern const uint8_t _binary_stage_1_bin_start[];

extern const uint8_t _binary_stage_1_bin_size[];

static const bed_elbc_config elbc_config = {
	.info = &bed_nand_device_info_8_bit_3_3_V[0],
	.elbc = ELBC,
	.base_address = (uint8_t *) BASE_ADDRESS,
	.bank = BANK,
	.obtain = bed_default_obtain,
	.release = bed_default_release,
	.select_chip = bed_default_select_chip
};

void bsp_reset(void)
{
	while (1);
}

static bed_address get_empty_page(const bed_partition *part, bed_address current_page)
{
	bed_address next_page;
	bed_status status;

	do {
		next_page = (current_page + bed_page_size(part)) % bed_size(part);

		bool is_block_begin = next_page % bed_block_size(part) == 0;
		if (is_block_begin) {
			status = bed_erase(part, next_page, BED_ERASE_MARK_BAD_ON_ERROR);
		} else {
			status = BED_SUCCESS;
		}

		current_page = next_page;
	} while (status != BED_SUCCESS);

	return next_page;
}

static void copy_stage_1_to_4k_buffer(void)
{
	size_t n = (size_t) _binary_stage_1_bin_size;
	size_t m = n >= SIZE_4K ? SIZE_4K : n;
	const uint8_t *src = _binary_stage_1_bin_start;
	memcpy(&stage_1_4k_data[0], src, m);
	memset(&stage_1_4k_data[m], 0xff, SIZE_4K - m);
}

static void write_page_auto_oob(
	const bed_partition *part,
	bed_address page,
	const uint8_t *master
)
{
	bed_status status = bed_write(part, page, master, PAGE_SIZE);
	assert(status == BED_SUCCESS);
	assert((bed_elbc_read_status(elbc, bank, fmr, base_address) & BED_NAND_STATUS_FAIL) == 0);
}

static void write_page(
	const bed_partition *part,
	bed_address page,
	const uint8_t *data,
	uint8_t *oob
)
{
	bed_oob_request oob_request = {
		.mode = BED_OOB_MODE_RAW,
		.offset = 0,
		.size = OOB_SIZE,
		.data = oob
	};
	bed_status status = bed_write_oob(part, page, data, PAGE_SIZE, &oob_request);
	assert(status == BED_SUCCESS);
	assert((bed_elbc_read_status(elbc, bank, fmr, base_address) & BED_NAND_STATUS_FAIL) == 0);
}

static bool check_read_status(bed_status status, bool expect_micron_ecc_ok)
{
#ifdef USE_MICRON_WITH_INTERNAL_ECC
	uint32_t expected_status = expect_micron_ecc_ok ? 0 : BED_NAND_STATUS_FAIL;
	bool ok = status == BED_SUCCESS
		&& (bed_elbc_read_status(elbc, bank, fmr, base_address) & BED_NAND_STATUS_FAIL) == expected_status;

	bed_elbc_read_mode(elbc, bank, fmr);

	return ok;
#else
	(void) status;
	(void) expect_micron_ecc_ok;

	return true;
#endif
}

static bool read_page(
	const bed_partition *part,
	bed_address page,
	uint8_t *data,
	uint8_t *oob,
	bool expect_micron_ecc_ok
)
{
	memset(data, 0xa5, PAGE_SIZE);
	memset(oob, 0x5a, OOB_SIZE);

	bed_oob_request oob_request = {
		.mode = BED_OOB_MODE_RAW,
		.offset = 0,
		.size = OOB_SIZE,
		.data = oob
	};
	bed_status status = bed_read_oob(part, page, data, PAGE_SIZE, &oob_request);
	bool read_oob_ok = check_read_status(status, expect_micron_ecc_ok);

	status = bed_read(part, page, data, PAGE_SIZE);
	bool read_data_ok = check_read_status(status, expect_micron_ecc_ok);

	return read_oob_ok && read_data_ok;
}

static void enable_micron_ecc(void)
{
#ifdef USE_MICRON_WITH_INTERNAL_ECC
	bed_elbc_enable_micron_ecc(elbc, bank, fmr, base_address, true);
	assert(bed_elbc_is_micron_ecc_enabled(elbc, bank, fmr, base_address));
#endif
}

static void disable_micron_ecc(void)
{
#ifdef USE_MICRON_WITH_INTERNAL_ECC
	bed_elbc_enable_micron_ecc(elbc, bank, fmr, base_address, false);
	assert(!bed_elbc_is_micron_ecc_enabled(elbc, bank, fmr, base_address));
#endif
}

static void reset_nand_chip(void)
{
	bed_elbc_reset(elbc, bank, fmr);
	rtems_task_wake_after(1);
}

static void store_stage_1(const bed_partition *part)
{
	size_t n = (size_t) _binary_stage_1_bin_size;
	size_t m = n >= SIZE_4K ? SIZE_4K : n;
	const uint8_t *src = _binary_stage_1_bin_start;

	printf("store begin\n");

	assert(SIZE_4K == 4096);

	bed_address page_0 = get_empty_page(part, bed_size(part) - bed_page_size(part));
	bed_address page_1 = get_empty_page(part, page_0);

	disable_micron_ecc();

	write_page_auto_oob(part, page_0, &stage_1_4k_data[0]);
	write_page_auto_oob(part, page_1, &stage_1_4k_data[PAGE_SIZE]);

	enable_micron_ecc();

	read_page(part, page_0, &tmp_4k_data[0], &stage_1_4k_oob[0], false);
	read_page(part, page_1, &tmp_4k_data[PAGE_SIZE], &stage_1_4k_oob[OOB_SIZE], false);

	assert(memcmp(&stage_1_4k_data[0], &tmp_4k_data[0], SIZE_4K) == 0);

	n -= m;
	src += m;

	memset(&oob_buf[0], 0xff, OOB_SIZE);

	while (n > 0) {
		m = n >= PAGE_SIZE ? PAGE_SIZE : n;

		memcpy(&data_buf[0], src, m);
		memset(&data_buf[m], 0xff, PAGE_SIZE - m);

		n -= m;
		src += m;

		page_1 = get_empty_page(part, page_1);
		assert(page_1 != 0);

#ifdef USE_MICRON_WITH_INTERNAL_ECC
		write_page(part, page_1, &data_buf[0], &oob_buf[0]);
#else
		write_page_auto_oob(part, page_1, &data_buf[0]);
#endif

		bed_oob_request oob_request = {
			.mode = BED_OOB_MODE_RAW,
			.offset = 0,
			.size = OOB_SIZE,
			.data = &tmp_4k_oob[0]
		};
		bed_status status = bed_read_oob(part, page_1, &tmp_4k_data[0], PAGE_SIZE, &oob_request);
		bool read_ok = check_read_status(status, true);
		assert(read_ok);

		assert(memcmp(&data_buf[0], &tmp_4k_data[0], PAGE_SIZE) == 0);
	}

	printf("store complete\n");
}

static void Init(rtems_task_argument arg)
{
	bed_status status = BED_SUCCESS;
	bed_elbc_context elbc_context;
	bed_partition *part = &elbc_context.part;
	bed_partition stage_1_part;
	bed_partition fs_part;

	bed_elbc_init_module(
		ELBC,
		0,
		ELBC_LARGE_PAGE_BR(BASE_ADDRESS),
		MT29F2G08ABAEA_LCLK_72_5_MHZ_OR
	);

#ifdef USE_MICRON_WITH_INTERNAL_ECC
	bed_elbc_init_with_no_chip_detection(&elbc_context, &elbc_config, BLOCK_COUNT, BLOCK_SIZE, PAGE_SIZE);
#else
	bed_elbc_init(&elbc_context, &elbc_config);
#endif

	status = bed_partition_create(&stage_1_part, part, STAGE_1_BLOCK_BEGIN * BLOCK_SIZE, STAGE_1_BLOCK_COUNT * BLOCK_SIZE);
	assert(status == BED_SUCCESS);

	status = bed_partition_create(&fs_part, part, FS_BLOCK_BEGIN * BLOCK_SIZE, FS_BLOCK_COUNT * BLOCK_SIZE);
	assert(status == BED_SUCCESS);

	copy_stage_1_to_4k_buffer();
	store_stage_1(&stage_1_part);

#if 0
	status = bed_erase_all(&fs_part, BED_ERASE_MARK_BAD_ON_ERROR);
	assert(status == BED_SUCCESS);
#endif

	exit(0);
}

#define CONFIGURE_MICROSECONDS_PER_TICK 1000

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER

#define CONFIGURE_USE_IMFS_AS_BASE_FILESYSTEM

#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS 32

#define CONFIGURE_UNLIMITED_OBJECTS

#define CONFIGURE_UNIFIED_WORK_AREAS

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE

#define CONFIGURE_STACK_CHECKER_ENABLED

#define CONFIGURE_INIT

#include <rtems/confdefs.h>
