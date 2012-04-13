/*
 * Copyright (c) 2012 embedded brains GmbH.  All rights reserved.
 *
 *  embedded brains GmbH
 *  Obere Lagerstr. 30
 *  82178 Puchheim
 *  Germany
 *  <rtems@embedded-brains.de>
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rtems.com/license/LICENSE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <rtems.h>
#include <rtems/libio.h>

#include <local/demo.h>

static const char mac_address [6] = { 0x00, 0x1a, 0xf1, 0x00, 0x07, 0xa4 };

static void Init(rtems_task_argument arg)
{
  rtems_status_code sc = RTEMS_SUCCESSFUL;
  int rv = 0;

  sc = demo_initialize_network(80, mac_address, "192.168.96.93", "192.168.96.1", "255.255.255.0");

  rv = mount_and_make_target_path(
    "1000.100@192.168.96.31:/srv/nfs",
    //"1000.100@www.rtems.com:/srv/nfs",
    "/nfs",
    RTEMS_FILESYSTEM_TYPE_NFS,
    RTEMS_FILESYSTEM_READ_WRITE,
    NULL
  );
  assert(rv == 0);

  sc = demo_initialize_shell(10, 32 * 1024);
  assert(sc == RTEMS_SUCCESSFUL);

  exit(0);
}

#define CONFIGURE_MAXIMUM_DRIVERS 8

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_STUB_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_ZERO_DRIVER

#define CONFIGURE_USE_IMFS_AS_BASE_FILESYSTEM

#define CONFIGURE_FILESYSTEM_NFS
#define CONFIGURE_FILESYSTEM_TFTPFS

#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS 32

#define CONFIGURE_MAXIMUM_TASKS rtems_resource_unlimited(16)
#define CONFIGURE_MAXIMUM_SEMAPHORES rtems_resource_unlimited(16)
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES rtems_resource_unlimited(16)
#define CONFIGURE_MAXIMUM_TIMERS rtems_resource_unlimited(16)
#define CONFIGURE_MAXIMUM_PERIODS rtems_resource_unlimited(16)

#define CONFIGURE_MAXIMUM_POSIX_THREADS rtems_resource_unlimited(16)
#define CONFIGURE_MAXIMUM_POSIX_MUTEXES rtems_resource_unlimited(16)
#define CONFIGURE_MAXIMUM_POSIX_CONDITION_VARIABLES rtems_resource_unlimited(16)

#define CONFIGURE_EXTRA_TASK_STACKS (512 * 1024)

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE

#define CONFIGURE_INIT_TASK_STACK_SIZE (32 * 1024)

#define CONFIGURE_STACK_CHECKER_ENABLED

#define CONFIGURE_INIT

#include <rtems/confdefs.h>

#define CONFIGURE_SHELL_COMMANDS_INIT
#define CONFIGURE_SHELL_COMMAND_LN
#define CONFIGURE_SHELL_COMMAND_LSOF
#define CONFIGURE_SHELL_COMMAND_MOUNT
#define CONFIGURE_SHELL_COMMAND_UNMOUNT

#include <local/demo-shell.h>
#include <local/demo-shell-network.h>
#include <rtems/shellconfig.h>
