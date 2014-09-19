/*
 * Copyright (c) 2011-2014 embedded brains GmbH.  All rights reserved.
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

#ifdef NEW_NETWORK_STACK

#include <machine/rtems-bsd-commands.h>

#include <net/if.h>

#include <sysexits.h>

#include <rtems/console.h>
#include <rtems/shell.h>
#include <rtems/ftpd.h>
#include <rtems/bsd/bsd.h>

static void
default_network_ifconfig_lo0(void)
{
  int exit_code;
  char *lo0[] = {
    "ifconfig",
    "lo0",
    "inet",
    "127.0.0.1",
    "netmask",
    "255.255.255.0",
    NULL
  };
  char *lo0_inet6[] = {
    "ifconfig",
    "lo0",
    "inet6",
    "::1",
    "prefixlen",
    "128",
    NULL
  };

  exit_code = rtems_bsd_command_ifconfig(RTEMS_BSD_ARGC(lo0), lo0);
  assert(exit_code == EX_OK);

  exit_code = rtems_bsd_command_ifconfig(RTEMS_BSD_ARGC(lo0_inet6), lo0_inet6);
  assert(exit_code == EX_OK);
}

static void
default_network_ifconfig_hwif0(char *ifname)
{
  int exit_code;
  char *ifcfg[] = {
    "ifconfig",
    ifname,
    "inet",
    "192.168.100.70",
    "netmask",
    "255.255.255.0",
    NULL
  };

  exit_code = rtems_bsd_command_ifconfig(RTEMS_BSD_ARGC(ifcfg), ifcfg);
  assert(exit_code == EX_OK);
}

static void
default_network_route_hwif0(char *ifname)
{
  int exit_code;
  char *dflt_route[] = {
    "route",
    "add",
    "-host",
    "192.168.100.254",
    "-iface",
    ifname,
    NULL
  };
  char *dflt_route2[] = {
    "route",
    "add",
    "default",
    "192.168.100.254",
    NULL
  };

  exit_code = rtems_bsd_command_route(RTEMS_BSD_ARGC(dflt_route), dflt_route);
  assert(exit_code == EXIT_SUCCESS);

  exit_code = rtems_bsd_command_route(RTEMS_BSD_ARGC(dflt_route2), dflt_route2);
  assert(exit_code == EXIT_SUCCESS);
}

struct rtems_ftpd_configuration rtems_ftpd_configuration = {
  /* FTPD task priority */
  .priority = 100,

  /* Maximum buffersize for hooks */
  .max_hook_filesize = 0,

  /* Well-known port */
  .port = 21,

  /* List of hooks */
  .hooks = NULL,

  /* Root for FTPD or NULL for "/" */
  .root = NULL,

  /* Max. connections */
  .tasks_count = 4,

  /* Idle timeout in seconds  or 0 for no (infinite) timeout */
  .idle = 5 * 60,

  /* Access: 0 - r/w, 1 - read-only, 2 - write-only, 3 - browse-only */
  .access = 0
};

#else /* NEW_NETWORK_STACK */

#include <local/demo.h>
#include <local/network-config.h>

static const char mac_address [6] = NETWORK_MAC_ADDRESS;

#endif /* NEW_NETWORK_STACK */

static void Init(rtems_task_argument arg)
{
#ifdef NEW_NETWORK_STACK
  rtems_status_code sc;
  int rv;
  char *ifname = "qe0";

  sc = rtems_shell_init(
    "SHLL",
    32 * 1024,
    1,
    CONSOLE_DEVICE_NAME,
    false,
    false,
    NULL
  );
  assert(sc == RTEMS_SUCCESSFUL);

  rtems_bsd_initialize();

  default_network_ifconfig_lo0();
  default_network_ifconfig_hwif0(ifname);
  default_network_route_hwif0(ifname);

  rv = rtems_initialize_ftpd();
  assert(rv == 0);

  rtems_task_delete(RTEMS_SELF);
  assert(0);
#else /* NEW_NETWORK_STACK */
  rtems_status_code sc = RTEMS_SUCCESSFUL;

  demo_network_set_buffer_space(512 * 1024, 512 * 1024);

  sc = demo_initialize_network(
    80,
    mac_address,
    NETWORK_IP_SELF,
    NETWORK_IP_GATEWAY,
    NETWORK_IP_NETMASK
  );
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
#endif /* NEW_NETWORK_STACK */
}

#define CONFIGURE_MICROSECONDS_PER_TICK 1000

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_STUB_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_ZERO_DRIVER

#define CONFIGURE_USE_IMFS_AS_BASE_FILESYSTEM

#ifdef NEW_NETWORK_STACK

#define CONFIGURE_MAXIMUM_USER_EXTENSIONS 1

#else /* NEW_NETWORK_STACK */

#define CONFIGURE_FILESYSTEM_FTPFS

#endif /* NEW_NETWORK_STACK */

#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS 32

#define CONFIGURE_MAXIMUM_DRIVERS 32

#define CONFIGURE_UNIFIED_WORK_AREAS

#define CONFIGURE_UNLIMITED_OBJECTS

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE

#define CONFIGURE_INIT_TASK_STACK_SIZE (32 * 1024)

//#define CONFIGURE_STACK_CHECKER_ENABLED

#define CONFIGURE_INIT

#include <rtems/confdefs.h>

#define CONFIGURE_SHELL_COMMANDS_INIT

#ifdef NEW_NETWORK_STACK

#include <bsp/irq-info.h>

#include <rtems/netcmds-config.h>

#define CONFIGURE_SHELL_USER_COMMANDS \
  &bsp_interrupt_shell_command, \
  &rtems_shell_PING_Command, \
  &rtems_shell_ROUTE_Command, \
  &rtems_shell_NETSTAT_Command, \
  &rtems_shell_IFCONFIG_Command

#define CONFIGURE_SHELL_COMMAND_CPUUSE
#define CONFIGURE_SHELL_COMMAND_PERIODUSE
#define CONFIGURE_SHELL_COMMAND_STACKUSE

#define CONFIGURE_SHELL_COMMAND_CP
#define CONFIGURE_SHELL_COMMAND_PWD
#define CONFIGURE_SHELL_COMMAND_LS
#define CONFIGURE_SHELL_COMMAND_LN
#define CONFIGURE_SHELL_COMMAND_LSOF
#define CONFIGURE_SHELL_COMMAND_CHDIR
#define CONFIGURE_SHELL_COMMAND_CD
#define CONFIGURE_SHELL_COMMAND_MKDIR
#define CONFIGURE_SHELL_COMMAND_RMDIR
#define CONFIGURE_SHELL_COMMAND_CAT
#define CONFIGURE_SHELL_COMMAND_MV
#define CONFIGURE_SHELL_COMMAND_RM
#define CONFIGURE_SHELL_COMMAND_MALLOC_INFO

#else /* NEW_NETWORK_STACK */

#include <local/demo-shell.h>
#include <local/demo-shell-network.h>

#endif /* NEW_NETWORK_STACK */

#include <rtems/shellconfig.h>
