/*
 * Copyright (c) 2010-2014 embedded brains GmbH.  All rights reserved.
 *
 *  embedded brains GmbH
 *  Dornierstr. 4
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

#include <assert.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>

#include <rtems.h>
#include <rtems/bspIo.h>
#include <rtems/shell.h>
#include <rtems/termiostypes.h>

#include <local/demo.h>

#include <uid/uid.h>

static void
change_serial_settings(int fd, const struct termios *current, bool icanon)
{
	struct termios term = *current;

	term.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR |
	    ICRNL | IXON);
	term.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL | ECHOPRT | ECHOCTL |
	    ECHOKE | ICANON | ISIG | IEXTEN);
	term.c_cflag &= ~(CSIZE | PARENB);
	term.c_cflag |= CS8;
	term.c_oflag &= ~(OPOST | ONLRET | ONLCR | OCRNL | ONLRET | TABDLY |
	    OLCUC);

	term.c_cc[VMIN] = 0;
	term.c_cc[VTIME] = 10;

	if (icanon) {
		term.c_iflag |= ICRNL;
		term.c_lflag |= ICANON;
	}

	tcsetattr(fd, TCSANOW, &term);
}

static void
do_read_select(int fd)
{
	int nfds = fd + 1;
	struct fd_set read_set;
	struct timeval timeout = {
		.tv_sec = 10,
		.tv_usec = 0
	};
	int rv;

	FD_ZERO(&read_set);
	FD_SET(fd, &read_set);

	rv = select(nfds, &read_set, NULL, NULL, &timeout);
	if (rv == 0) {
		printf("timeout\n");
	} else if (rv > 0) {
		if (FD_ISSET(fd, &read_set)) {
			char buf[512];
			ssize_t n = read(fd, buf, sizeof(buf));
			printf("read returned %zi\n", n);
		}
	} else {
		perror("select failed");
	}
}

static void
do_write_select(int fd)
{
	int nfds = fd + 1;
	struct fd_set write_set;
	struct timeval to = {
		.tv_sec = 0,
		.tv_usec = 1
	};
	struct timeval *timeout = &to;
	char buf[512];
	int rv;
	size_t i;

	memset(buf, 'a', sizeof(buf));

	for (i = 0; i < sizeof(buf); i += 24) {
		buf[i] = '\r';
		buf[i + 1] = '\n';
	}

	for (i = 0; i < 10; ++i) {
		write(fd, buf, sizeof(buf));

		FD_ZERO(&write_set);
		FD_SET(fd, &write_set);
		rv = select(nfds, NULL, &write_set, NULL, timeout);
		if (rv == 0) {
			printf("timeout\n");
		} else {
			printf("write set: %i\n", FD_ISSET(fd, &write_set));
		}

		timeout = NULL;
	}
}

static int
termiosselect_main(int argc, char *argv[])
{
	bool icanon = argc > 1 && strcmp(argv[1], "icanon") == 0;
	int fd = STDIN_FILENO;
	struct termios term;
	int rv = tcgetattr(fd, &term);
	assert(rv == 0);

	change_serial_settings(fd, &term, icanon);
	do_read_select(fd);
	do_write_select(STDOUT_FILENO);
	tcsetattr(fd, TCSANOW, &term);
	return (0);
}

static rtems_shell_cmd_t termiosselect = {
	.name = "termiosselect",
	.usage = "termiosselect [icanon]",
	.topic = "net",
	.command = termiosselect_main
};

static void Init(rtems_task_argument arg)
{
  rtems_status_code sc = RTEMS_SUCCESSFUL;

  printk("Init\n");

  uid_init_flash_file_system("/ffs");
  uid_init_network("/ffs/br_uid.ini");

  demo_initialize_ftpfs(true, 10);

  demo_initialize_telnetd(70, 32 * 1024);

  demo_initialize_ftpd(70);

  rtems_shell_add_cmd_struct(&uid_shell_stage_1_update);
  rtems_shell_add_cmd_struct(&termiosselect);

  sc = demo_initialize_shell(10, 32 * 1024);
  assert(sc == RTEMS_SUCCESSFUL);

  exit(0);
}

#define CONFIGURE_MICROSECONDS_PER_TICK 10000

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_STUB_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_ZERO_DRIVER

#define CONFIGURE_MAXIMUM_DRIVERS 32
#define CONFIGURE_MAXIMUM_USER_EXTENSIONS 1

#define CONFIGURE_FILESYSTEM_IMFS
#define CONFIGURE_FILESYSTEM_FTPFS
#define CONFIGURE_FILESYSTEM_NFS

#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS 32

#define CONFIGURE_UNLIMITED_OBJECTS
#define CONFIGURE_UNIFIED_WORK_AREAS

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE

#define CONFIGURE_INIT_TASK_STACK_SIZE (32 * 1024)

#define CONFIGURE_STACK_CHECKER_ENABLED

#define CONFIGURE_INIT

#include <rtems/confdefs.h>

#define CONFIGURE_SHELL_COMMANDS_INIT

#define CONFIGURE_SHELL_COMMAND_CHMOD

#include <local/demo-shell.h>
#include <local/demo-shell-network.h>
#include <rtems/shellconfig.h>
