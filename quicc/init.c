/*
 * Copyright (c) 2011-2012 embedded brains GmbH.  All rights reserved.
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

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <rtems.h>

#include <libcpu/powerpc-utility.h>

#include <bsp.h>

#include <local/demo.h>

static const char mac_address [6] = { 0x00, 0x1a, 0xf1, 0x00, 0x07, 0xa4 };

static void Init(rtems_task_argument arg)
{
  rtems_status_code sc = RTEMS_SUCCESSFUL;

  demo_network_set_buffer_space(512 * 1024, 512 * 1024);

  sc = demo_initialize_network(80, mac_address, "192.168.96.93", "192.168.96.1", "255.255.255.0");
  assert(sc == RTEMS_SUCCESSFUL);

  sc = demo_initialize_ftpfs(true, 10);
  assert(sc == RTEMS_SUCCESSFUL);

  sc = demo_initialize_ftpd(70);
  assert(sc == RTEMS_SUCCESSFUL);

  sc = demo_initialize_network_echo_mcast(90, DEMO_MCAST_IP_ADDR, DEMO_PORT);
  assert(sc == RTEMS_SUCCESSFUL);

  demo_start_telnetd(70, 32 * 1024);

  sc = demo_initialize_shell(10, 32 * 1024);
  assert(sc == RTEMS_SUCCESSFUL);

  exit(0);
}

#define CONFIGURE_MICROSECONDS_PER_TICK 1000

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_STUB_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_ZERO_DRIVER

#define CONFIGURE_USE_IMFS_AS_BASE_FILESYSTEM

#define CONFIGURE_FILESYSTEM_IMFS
#define CONFIGURE_FILESYSTEM_FTPFS

#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS 32

#define CONFIGURE_MAXIMUM_TASKS 32
#define CONFIGURE_MAXIMUM_DRIVERS 8
#define CONFIGURE_MAXIMUM_SEMAPHORES 32
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES 4
#define CONFIGURE_MAXIMUM_TIMERS 4
#define CONFIGURE_MAXIMUM_PERIODS 4

#define CONFIGURE_MAXIMUM_POSIX_THREADS 4
#define CONFIGURE_MAXIMUM_POSIX_MUTEXES 16

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE

#define CONFIGURE_INIT_TASK_STACK_SIZE (32 * 1024)

#define CONFIGURE_STACK_CHECKER_ENABLED

#define CONFIGURE_INIT

#include <rtems/confdefs.h>

#define CONFIGURE_SHELL_COMMANDS_INIT

#include <local/demo-shell.h>
#include <local/demo-shell-network.h>
#include <rtems/shellconfig.h>
