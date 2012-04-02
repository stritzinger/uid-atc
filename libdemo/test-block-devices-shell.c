/**
 * @file
 *
 * @brief Block device shell commands.
 */

/*
 * Copyright (c) 2010-2011 embedded brains GmbH.  All rights reserved.
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>

#include <rtems.h>
#include <rtems/diskdevs.h>
#include <rtems/shell.h>
#include <rtems/media.h>
#include <rtems/bdbuf.h>
#include <rtems/cpuuse.h>

#include "test.h"

#define ASSERT_SC(sc) assert((sc) == RTEMS_SUCCESSFUL)

static void get_device_data(dev_t *dev, rtems_blkdev_bnum *block_count, uint32_t *block_size)
{
	rtems_status_code sc = RTEMS_SUCCESSFUL;
	rtems_disk_device *dd = NULL;

	sc = rtems_media_get_device_identifier("/dev/hda", dev);
	ASSERT_SC(sc);

	dd = rtems_disk_obtain(*dev);
	assert(dd != NULL);

	*block_count = dd->size;
	*block_size = dd->block_size;

	sc = rtems_disk_release(dd);
	ASSERT_SC(sc);
}

static int do_bdls(int argc, char **argv)
{
	rtems_status_code sc = RTEMS_SUCCESSFUL;
	dev_t dev = (dev_t) -1;
	rtems_disk_device *dd = NULL;
	
	while (sc == RTEMS_SUCCESSFUL && (dd = rtems_disk_next(dev)) != NULL) {
		printf(
			"%08" PRIx32 ":%08" PRIx32 " %c %s UC=%u BB=%" PRIu32 " BC=%" PRIu32 " BS=%" PRIu32 " MBS=%" PRIu32 "\n",
			rtems_filesystem_dev_major_t(dev),
			rtems_filesystem_dev_minor_t(dev),
			dd->phys_dev == dd ? 'P' : 'L',
			dd->name,
			dd->uses,
			dd->start,
			dd->size,
			dd->block_size,
			dd->media_block_size
		);
		dev = rtems_disk_get_device_identifier(dd);
		sc = rtems_disk_release(dd);
	}

	return 0;
}

static rtems_shell_cmd_t bdls_command = {
	"bdls",
	"bdls",
	"bsp",
	do_bdls,
	NULL,
	NULL
};

static int do_bddump(int argc, char **argv)
{
	dev_t dev = 0;
	int i = 0;
	int j = 0;

	rtems_status_code sc = rtems_media_get_device_identifier("/dev/hda", &dev);
	ASSERT_SC(sc);

	rtems_blkdev_bnum block = 0;
	if (argc == 2) {
		block = strtoul(argv [1], NULL, 10);
	}

	rtems_bdbuf_purge_dev(dev);

	rtems_bdbuf_buffer *bd = NULL;
	sc = rtems_bdbuf_read(dev, block, &bd);
	ASSERT_SC(sc);

	for (i = 0; i < 16; ++i) {
		for (j = 0; j < 32; ++j) {
			printf("%02x ", bd->buffer [i * 32 + j]);
		}
		printf("\n");
	}

	sc = rtems_bdbuf_release(bd);
	ASSERT_SC(sc);

	return 0;
}

static rtems_shell_cmd_t bddump_command = {
	"bddump",
	"bddump",
	"bsp",
	do_bddump,
	NULL,
	NULL
};

static int do_bdread(int argc, char **argv)
{
	rtems_status_code sc = RTEMS_SUCCESSFUL;
	dev_t dev = 0;
	rtems_blkdev_bnum block_count = 0;
	rtems_blkdev_bnum block = 0;
	uint32_t block_size = 0;

	get_device_data(&dev, &block_count, &block_size);

	rtems_cpu_usage_reset();

	for (block = 0; block < block_count; ++block) {
		rtems_bdbuf_buffer *bd = NULL;

		sc = rtems_bdbuf_read(dev, block, &bd);
		ASSERT_SC(sc);

		sc = rtems_bdbuf_release(bd);
		ASSERT_SC(sc);
	}

	rtems_cpu_usage_report();

	return 0;
}

static rtems_shell_cmd_t bdread_command = {
	"bdread",
	"bdread",
	"bsp",
	do_bdread,
	NULL,
	NULL
};

static void create_block_data(void *begin, rtems_blkdev_bnum block, uint32_t block_size)
{
	uint32_t *current = (uint32_t *) begin;
	uint32_t *end = current + block_size / 4;
	uint32_t value = block * block_size + 4;

	*current = rtems_clock_get_ticks_since_boot();
	++current;

	while (current != end) {
		*current = value;
		++current;
		value += 4;;
	}
}

static int do_bdwrite(int argc, char **argv)
{
	rtems_status_code sc = RTEMS_SUCCESSFUL;
	dev_t dev = 0;
	rtems_blkdev_bnum block_count = 0;
	rtems_blkdev_bnum block = 0;
	uint32_t block_size = 0;

	get_device_data(&dev, &block_count, &block_size);

	rtems_cpu_usage_reset();

	for (block = 0; block < block_count; ++block) {
		rtems_bdbuf_buffer *bd = NULL;

		sc = rtems_bdbuf_get(dev, block, &bd);
		ASSERT_SC(sc);

		create_block_data(bd->buffer, block, block_size);

		sc = rtems_bdbuf_release_modified(bd);
		ASSERT_SC(sc);
	}

	sc = rtems_bdbuf_syncdev(dev);
	ASSERT_SC(sc);

	rtems_cpu_usage_report();

	return 0;
}

static rtems_shell_cmd_t bdwrite_command = {
	"bdwrite",
	"bdwrite",
	"bsp",
	do_bdwrite,
	NULL,
	NULL
};

static void fstest_task(rtems_task_argument arg)
{
	rtems_status_code sc = RTEMS_SUCCESSFUL;
	char *path = (char *) arg;

	test_file_system(0, path, "/mnt");
	free(path);

	sc = rtems_task_delete(RTEMS_SELF);
	assert(sc == RTEMS_SUCCESSFUL);
}

static int do_fstest(int argc, char **argv)
{
	rtems_status_code sc = RTEMS_SUCCESSFUL;

	if (argc >= 2) {
		rtems_id id = RTEMS_ID_NONE;
		char *path = strdup(argv [1]);

		assert(path != NULL);

		sc = rtems_task_create(
			rtems_build_name('F', 'S', 'T', 'S'),
			150,
			0,
			RTEMS_DEFAULT_MODES,
			RTEMS_DEFAULT_ATTRIBUTES,
			&id
		);
		assert(sc == RTEMS_SUCCESSFUL);

		sc = rtems_task_start(id, fstest_task, (rtems_task_argument) path);
		assert(sc == RTEMS_SUCCESSFUL);
	}

	return 0;
}

static rtems_shell_cmd_t fstest_command = {
	"fstest",
	"fstest",
	"bsp",
	do_fstest,
	NULL,
	NULL
};

void test_block_devices_shell(void)
{
	rtems_shell_add_cmd_struct(&bdls_command);
	rtems_shell_add_cmd_struct(&bddump_command);
	rtems_shell_add_cmd_struct(&bdwrite_command);
	rtems_shell_add_cmd_struct(&bdread_command);
	rtems_shell_add_cmd_struct(&fstest_command);
}
