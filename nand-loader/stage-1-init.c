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
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <bed/bed.h>
#include <bed/bed-yaffs.h>

#include <yaffs/rtems_yaffs.h>

#include <rtems.h>
#include <rtems/shell.h>
#include <rtems/console.h>
#include <rtems/malloc.h>
#include <rtems/rtems_bsdnet.h>
#include <rtems/stringto.h>

#include <ini/ini.h>

#include <local/demo.h>
#include <local/test.h>
#include <local/network-config.h>

#include <quicc/quicc.h>

#include <uid/uid.h>

#include <bsp.h>

#define SHELL_STACK_SIZE (8 * 1024)

#define NETWORK_TASK_PRIORITY 80

static rtems_id led_timer_id;

static const char nfs_dir[] = "/nfs";

static const char ffs_dir[] = "/ffs";

static const char ini_file[] = "/ffs/br_uid.ini";

static char mac_address [6] = NETWORK_MAC_ADDRESS;

static char ip_self[16] = NETWORK_IP_SELF;

static char ip_netmask[16] = NETWORK_IP_NETMASK;

static char ip_gateway[16] = NETWORK_IP_GATEWAY;

static char nfs_server_path[PATH_MAX + 1];

static char nfs_image_path[PATH_MAX + 1];

static char image_path[PATH_MAX + 1] = "/ffs/br_uid.bin";

static int timeout_in_seconds = 3;

static struct rtems_bsdnet_ifconfig ifconfig = {
	.name = RTEMS_BSP_NETWORK_DRIVER_NAME,
	.attach = RTEMS_BSP_NETWORK_DRIVER_ATTACH,
	.next = NULL,
	.ip_address = &ip_self[0],
	.ip_netmask = &ip_netmask[0],
	.hardware_address = &mac_address[0],
	.ignore_broadcast = 0,
	.mtu = 0,
	.rbuf_count = 0,
	.xbuf_count = 0,
	.port = 0,
	.irno = 0,
	.bpar = 0
};

struct rtems_bsdnet_config rtems_bsdnet_config = {
	.ifconfig = &ifconfig,
	.bootp = NULL,
	.network_task_priority = NETWORK_TASK_PRIORITY,
	.mbuf_bytecount = 0,
	.mbuf_cluster_bytecount = 0,
	.hostname = NULL,
	.domainname = NULL,
	.gateway = &ip_gateway[0],
	.log_host = &ip_gateway[0],
	.name_server = { &ip_gateway[0], NULL, NULL },
	.ntp_server = { &ip_gateway[0], NULL, NULL },
	.sb_efficiency = 0,
	.udp_tx_buf_size = 0,
	.udp_rx_buf_size = 0,
	.tcp_tx_buf_size = 0,
	.tcp_rx_buf_size = 0
};

static void print_status(bool ok)
{
	if (ok) {
		printf("done\n");
	} else {
		printf("failed\n");
	}
}

static void led_progress(bool success)
{
	static int fd = -1;
	static uint8_t on_off[2] = { 0x00, 0xff };
	static uint8_t success_on_off[2] = { 0x00, 0xaa };
	static bool forward = false;

	int val;

	if (fd == -1) {
		fd = open("/dev/i2c1.sc620", O_RDWR);
		assert(fd >= 0);
	}

	if (success) {
		write(fd, success_on_off, sizeof(success_on_off));
	} else {
		write(fd, on_off, sizeof(on_off));

		val = on_off[1];
		if (val >= 0x80) {
			forward = false;
			val = 0x40;
		} else if (val == 0x01) {
			forward = true;
			val = 0x02;
		} else if (forward) {
			val <<= 1;
		} else {
			val >>= 1;
		}
		on_off[1] = (uint8_t) val;
	}
}

static void load_via_file(const char *file)
{
	static uint8_t *buffer;

	size_t buffer_size = 127 * 1024 * 1024;
	if (buffer == NULL) {
		buffer = rtems_heap_allocate_aligned_with_boundary(buffer_size, 64 * 1024 * 1024, 0);
		assert(buffer != NULL);
	}

	printf("boot: open file \"%s\"... ", file);
	int fd = open(file, O_RDONLY);
	bool ok = fd >= 0;
	print_status(ok);
	if (ok) {
		printf("boot: read file \"%s\"... ", file);
		ssize_t in = read(fd, buffer, buffer_size);
		printf("received %zi bytes\n", in);

		int rv = close(fd);
		assert(rv == 0);

		ssize_t entry = 0x10000;
		if (in > entry) {
			rtems_status_code sc = rtems_timer_cancel(led_timer_id);
			assert(sc == RTEMS_SUCCESSFUL);

			led_progress(true);

			quicc_reset(quicc_init());

			sleep(1);

			bsp_restart(buffer + entry);
		}
	}
}

static void led_timer(rtems_id timer, void *arg)
{
	rtems_status_code sc;

	sc = rtems_timer_reset(timer);
	assert(sc == RTEMS_SUCCESSFUL);

	led_progress(false);
}

static int exception(int argc, char **argv)
{
	volatile int *p = (volatile int *) 0xffffffff;

	*p;

	return 0;
}

static rtems_shell_cmd_t exception_command = {
	"exception",
	"exception",
	"app",
	exception,
	NULL,
	NULL
};

static int test_yaffs_mount_handler(
	const char *disk_path,
	const char *mount_path,
	void *arg
)
{
	return mount_and_make_target_path(
		disk_path,
		mount_path,
		RTEMS_FILESYSTEM_TYPE_YAFFS,
		RTEMS_FILESYSTEM_READ_WRITE,
		NULL
	);
}

static int test_yaffs_format_handler(
	const char *disk_path,
	void *arg
)
{
	assert(0);

	return -1;
}

static const test_file_system_handler test_yaffs_handler = {
	.mount = test_yaffs_mount_handler,
	.format = test_yaffs_format_handler
};

static rtems_id test_yaffs_task_id = RTEMS_ID_NONE;

static void test_yaffs_task(rtems_task_argument arg)
{
	rtems_status_code sc = RTEMS_SUCCESSFUL;

	test_file_system_with_handler(
		0,
		RTEMS_FILESYSTEM_TYPE_YAFFS,
		&ffs_dir[0],
		&test_yaffs_handler,
		NULL
	);

	test_yaffs_task_id = RTEMS_ID_NONE;
	sc = rtems_task_delete(RTEMS_SELF);
	assert(sc == RTEMS_SUCCESSFUL);
}

static int test_yaffs(int argc, char **argv)
{
	rtems_status_code sc = RTEMS_SUCCESSFUL;

	if (test_yaffs_task_id == RTEMS_ID_NONE) {
		rtems_id id = RTEMS_ID_NONE;

		sc = rtems_task_create(
			rtems_build_name('Y', 'T', 'S', 'T'),
			60,
			RTEMS_MINIMUM_STACK_SIZE,
			RTEMS_DEFAULT_MODES,
			RTEMS_DEFAULT_ATTRIBUTES,
			&id
		);
		assert(sc == RTEMS_SUCCESSFUL);

		sc = rtems_task_start(id, test_yaffs_task, 0);
		assert(sc == RTEMS_SUCCESSFUL);

		test_yaffs_task_id = id;
	}

	return 0;
}

static rtems_shell_cmd_t test_yaffs_command = {
	"test_yaffs",
	"test_yaffs",
	"app",
	test_yaffs,
	NULL,
	NULL
};

static int format_yaffs(int argc, char **argv)
{
	uid_format_flash_file_system();

	return 0;
}

static rtems_shell_cmd_t format_yaffs_command = {
	"format_yaffs",
	"format_yaffs",
	"app",
	format_yaffs,
	NULL,
	NULL
};

static int bad_blocks(int argc, char **argv)
{
	uid_print_bad_blocks();

	return 0;
}

static rtems_shell_cmd_t bad_blocks_command = {
	"bad_blocks",
	"bad_blocks",
	"app",
	bad_blocks,
	NULL,
	NULL
};

static void print_message(int fd, int seconds_remaining, void *arg)
{
	printf("boot: press key to enter service mode\n");
}

static bool wait_for_user_input(void)
{
	int fd = open(CONSOLE_DEVICE_NAME, O_RDWR);
	assert(fd >= 0);

	rtems_status_code sc = rtems_shell_wait_for_input(fd, timeout_in_seconds, print_message, NULL);
	bool service_mode_requested = sc == RTEMS_SUCCESSFUL;

	return service_mode_requested;
}

static int ini_value_copy(void *dst, size_t dst_size, const char *value)
{
	int ok = 1;
	size_t value_size = strlen(value) + 1;

	if (value_size <= dst_size) {
		memcpy(dst, value, value_size);
	} else {
		ok = 0;
	}

	return ok;
}

static int ini_file_handler(void *arg, const char *section, const char *name, const char *value)
{
	int ok = 0;

	if (strcmp(section, "file") == 0) {
		if (strcmp(name, "image_path") == 0) {
			ok = ini_value_copy(&image_path[0], sizeof(image_path), value);
		}
	} else if (strcmp(section, "network") == 0) {
		if (strcmp(name, "mac_address") == 0) {
			int v [6];
			int count = sscanf(
				value,
				"%02x:%02x:%02x:%02x:%02x:%02x",
				&v [0], &v [1], &v [2], &v [3], &v [4], &v [5]
			);
			ok = count == 6;
			if (ok) {
				int i;
				for (i = 0; i < 6; ++i) {
					mac_address [i] = (uint8_t) v [i];
				}
			}
		} else if (strcmp(name, "ip_self") == 0) {
			ok = ini_value_copy(&ip_self[0], sizeof(ip_self), value);
		} else if (strcmp(name, "ip_gateway") == 0) {
			ok = ini_value_copy(&ip_gateway[0], sizeof(ip_gateway), value);
		} else if (strcmp(name, "ip_netmask") == 0) {
			ok = ini_value_copy(&ip_netmask[0], sizeof(ip_netmask), value);
		}
	} else if (strcmp(section, "nfs") == 0) {
		if (strcmp(name, "server_path") == 0) {
			ok = ini_value_copy(&nfs_server_path[0], sizeof(nfs_server_path), value);
		} else if (strcmp(name, "image_path") == 0) {
			ok = ini_value_copy(&nfs_image_path[0], sizeof(nfs_image_path), value);
		}
	} else if (strcmp(section, "boot") == 0) {
		if (strcmp(name, "timeout_in_seconds") == 0) {
			rtems_status_code sc = rtems_string_to_int(value, &timeout_in_seconds, NULL, 10);
			ok = sc == RTEMS_SUCCESSFUL;
		}
	}

	if (!ok) {
		printf(
			"boot: error in configuration file: section \"%s\", name \"%s\", value \"%s\"\n",
			section,
			name,
			value
		);
		ok = 1;
	}

	return ok;
}

static void evaluate_ini_file(const char *ini_file)
{
	ini_parse(ini_file, ini_file_handler, NULL);
}

static bool init_network(void)
{
	static bool initialized = false;

	if (!initialized) {
		quicc_reset(quicc_init());

		printf("boot: initialize network... ");
		int rv = rtems_bsdnet_initialize_network();
		bool ok = rv == 0;
		print_status(ok);

		if (ok) {
			initialized = true;
			printf("boot: start FTP server...\n");
			demo_initialize_ftpd(NETWORK_TASK_PRIORITY);
		}
	}

	return initialized;
}

static void start_shell(void)
{
	rtems_shell_add_cmd_struct(&exception_command);
	rtems_shell_add_cmd_struct(&test_yaffs_command);
	rtems_shell_add_cmd_struct(&format_yaffs_command);
	rtems_shell_add_cmd_struct(&bad_blocks_command);

	rtems_status_code sc = rtems_shell_init(
		"SHLL",
		SHELL_STACK_SIZE,
		10,
		CONSOLE_DEVICE_NAME,
		false,
		true,
		NULL
	);
	assert(sc == RTEMS_SUCCESSFUL);
}

static void service_mode(void)
{
	init_network();
	start_shell();
}

static void init_flash_file_system(void)
{
	printf("boot: mount flash file system to \"%s\"... ", &ffs_dir[0]);
	bool ok = uid_init_flash_file_system(&ffs_dir[0]);
	print_status(ok);
}

static void init_led(void)
{
	rtems_status_code sc = bsp_register_i2c();
	assert(sc == RTEMS_SUCCESSFUL);

	sc = rtems_timer_initiate_server(
		250,
		RTEMS_MINIMUM_STACK_SIZE,
		RTEMS_DEFAULT_ATTRIBUTES
	);
	assert(sc == RTEMS_SUCCESSFUL);

	sc = rtems_timer_create(rtems_build_name('L', 'E', 'D', ' '), &led_timer_id);
	assert(sc == RTEMS_SUCCESSFUL);

	sc = rtems_timer_server_fire_after(
		led_timer_id,
		rtems_clock_get_ticks_per_second() / 10,
		led_timer,
		NULL
	);
	assert(sc == RTEMS_SUCCESSFUL);

}

static void load_via_nfs(const char *server, const char *dir, const char *image)
{
	bool ok = init_network();
	if (ok) {
		printf(
			"boot: mount network file system \"%s\" to \"%s\"... ",
			server,
			dir
		);
		int rv = mount_and_make_target_path(
			server,
			dir,
			RTEMS_FILESYSTEM_TYPE_NFS,
			RTEMS_FILESYSTEM_READ_WRITE,
			NULL
		);
		ok = rv == 0;
		print_status(ok);

		if (ok && strlen(image) > 0) {
			load_via_file(image);
		}
	}
}

static void Init(rtems_task_argument arg)
{
	printf("\n");
	init_led();
	init_flash_file_system();
	evaluate_ini_file(&ini_file[0]);

	bool service_mode_requested = timeout_in_seconds == 0 ?
		false : wait_for_user_input();
	if (!service_mode_requested) {
		const char *image = &image_path[0];
		if (strlen(image) > 0) {
			load_via_file(image);
		}

		const char *server = &nfs_server_path[0];
		if (strlen(server) > 0) {
			load_via_nfs(server, &nfs_dir[0], &nfs_image_path[0]);
		}
	}

	service_mode();

	exit(0);
}

#define CONFIGURE_MAXIMUM_DRIVERS 32

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_STUB_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_ZERO_DRIVER

#define CONFIGURE_USE_IMFS_AS_BASE_FILESYSTEM

#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS 32

#define CONFIGURE_FILESYSTEM_IMFS
#define CONFIGURE_FILESYSTEM_NFS

#define CONFIGURE_UNLIMITED_OBJECTS

#define CONFIGURE_UNIFIED_WORK_AREAS

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE

#define CONFIGURE_STACK_CHECKER_ENABLED

#define CONFIGURE_INIT

#include <rtems/confdefs.h>

#define CONFIGURE_SHELL_COMMANDS_INIT

#include <local/demo-shell.h>
#include <local/demo-shell-network.h>

#include <rtems/shellconfig.h>
