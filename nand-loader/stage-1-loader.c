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

#include <stdlib.h>

#include <rtems.h>

#include <uid/uid.h>

extern const uint8_t _binary_stage_1_bin_start[];

extern const uint8_t _binary_stage_1_bin_size[];

static void format_yaffs_partition(void)
{
#if 0
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

	status = bed_partition_create(&fs_part, part, FS_BLOCK_BEGIN * BLOCK_SIZE, FS_BLOCK_COUNT * BLOCK_SIZE);
	assert(status == BED_SUCCESS);

	status = bed_erase_all(&fs_part, BED_ERASE_MARK_BAD_ON_ERROR);
	assert(status == BED_SUCCESS);
#endif
}

static void Init(rtems_task_argument arg)
{
	uid_stage_1_update(&_binary_stage_1_bin_start[0], (size_t) _binary_stage_1_bin_size);
	format_yaffs_partition();

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
