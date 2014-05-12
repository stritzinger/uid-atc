/*
 * Copyright (c) 2010 embedded brains GmbH.  All rights reserved.
 *
 *  embedded brains GmbH
 *  Obere Lagerstr. 30
 *  82178 Puchheim
 *  Germany
 *  <info@embedded-brains.de>
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rtems.com/license/LICENSE.
 */

#include <assert.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>

#include <rtems.h>
#include <rtems/shell.h>

#include <local/demo.h>
#include <local/network-config.h>

static const char mac_address [6] = NETWORK_MAC_ADDRESS;

static void Init(rtems_task_argument arg)
{
  rtems_status_code sc = RTEMS_SUCCESSFUL;

  printk("Init\n");

  demo_network_set_buffer_space(512 * 1024, 512 * 1024);
  demo_initialize_network(
    80,
    mac_address,
    NETWORK_IP_SELF,
    NETWORK_IP_GATEWAY,
    NETWORK_IP_NETMASK
  );

  demo_initialize_ftpfs(true, 10);

  demo_initialize_telnetd(70, 32 * 1024);

  demo_initialize_ftpd(70);

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

#define CONFIGURE_USE_IMFS_AS_BASE_FILESYSTEM

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

#include <local/demo-shell.h>
#include <local/demo-shell-network.h>
#include <rtems/shellconfig.h>
