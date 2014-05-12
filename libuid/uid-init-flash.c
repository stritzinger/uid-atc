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
#include <stdio.h>

#include <rtems/libio.h>

#include <bed/bed.h>
#include <bed/bed-yaffs.h>

#include <yaffs/rtems_yaffs.h>

#include "nand-chip.h"

static const bed_elbc_config elbc_config = {
	.info = &bed_nand_device_info_8_bit_3_3_V[0],
	.elbc = ELBC,
	.base_address = (uint8_t *) BASE_ADDRESS,
	.bank = BANK,
	.obtain = bed_default_obtain,
	.release = bed_default_release,
	.select_chip = bed_default_select_chip
};

static bed_elbc_context elbc_context;

static bed_partition fs_part;

static int uid_yaffs_mount(rtems_filesystem_mount_table_entry_t *mt_entry, const void *data)
{
	static struct yaffs_dev flash_device;
	static const rtems_yaffs_mount_data mount_data = {
		.dev = &flash_device
	};

	bed_status status = bed_yaffs_initialize_device(&fs_part, &flash_device);
	assert(status == BED_SUCCESS);

	return rtems_yaffs_mount_handler(mt_entry, &mount_data);
}

bool uid_init_flash_file_system(const char *mount_path)
{
	bed_status status = bed_elbc_init(&elbc_context, &elbc_config);
	assert(status == BED_SUCCESS);

	bed_partition *part = &elbc_context.part;
	assert(OOB_SIZE == bed_oob_size(part));

	status = bed_partition_create(&fs_part, part, FS_BLOCK_BEGIN * BLOCK_SIZE, FS_BLOCK_COUNT * BLOCK_SIZE);
	assert(status == BED_SUCCESS);

	int rv = rtems_filesystem_register(RTEMS_FILESYSTEM_TYPE_YAFFS, uid_yaffs_mount);
	assert(rv == 0);

	rv = mount_and_make_target_path(
		NULL,
		mount_path,
		RTEMS_FILESYSTEM_TYPE_YAFFS,
		RTEMS_FILESYSTEM_READ_WRITE,
		NULL
	);

	return rv == 0;
}

void uid_format_flash_file_system(void)
{
	bed_status status = bed_yaffs_format(&fs_part);
	assert(status == BED_SUCCESS);
}

void uid_print_bad_blocks(void)
{
	bed_print_bad_blocks(&elbc_context.part, (bed_printer) fprintf, stdout);
}
