// #define UID_ERL_LOAD_NFS

/*
 * %CopyrightBegin%
 * 
 * Copyright Ericsson AB 2000-2009. All Rights Reserved.
 * 
 * The contents of this file are subject to the Erlang Public License,
 * Version 1.1, (the "License"); you may not use this file except in
 * compliance with the License. You should have received a copy of the
 * Erlang Public License along with this software. If not, it can be
 * retrieved online at http://www.erlang.org/.
 * 
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 * 
 * %CopyrightEnd%
 */
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "sys.h"
#include "erl_vm.h"
#include "global.h"

#ifdef __rtems__
#include <rtems.h>
#include <rtems/shell.h>
#include <rtems/console.h>
#include <bsp.h>
#include <assert.h>
#include <rtems/libio.h>
#ifdef TELNETD_IN_APPLICATION
#include <rtems/telnetd.h>
#endif /* TELNETD_IN_APPLICATION */
#include <local/demo.h>
#include <rtems/termiostypes.h>

#include <local/demo.h>

#include <ini/ini.h>
#include <uid/uid.h>

static char hostname[256] = "defaulthost";

#define SHELL_STACK_SIZE (8 * 1024)

#ifdef TELNETD_IN_APPLICATION
static void sh_wrap( char *dev, void *arg)
{
     (void)rtems_shell_main_loop(arg);
}

rtems_telnetd_config_table rtems_telnetd_config = {
     sh_wrap,                /* Command */
     NULL,                   /* Argument */
     0,                      /* Priority */
     RTEMS_MINIMUM_STACK_SIZE * 16, /* task stack size */
     0,                      /* Login check */
     0                       /* Keep stdio of the caller  */
};
#endif /* TELNETD_IN_APPLICATION */


static int ini_value_copy(void *dst, size_t dst_size, const char *value)
{
  int ok = 1;
  size_t value_size = strlen(value) + 1;

  if (value_size <= dst_size) 
    memcpy(dst, value, value_size);
  else 
    ok = 0;

  return ok;
}

static int ini_file_handler(void *arg, const char *section, const char *name, 
			    const char *value)
{
  int ok = 0;

  if (strcmp(section, "network") == 0) 
    {
      if (strcmp(name, "hostname") == 0) 
	ok = ini_value_copy(hostname, sizeof(hostname), value);
    }
  else
    ok = 1;

  if (!ok) 
    {
      printf ("erl_main: error in configuration file: "
	      "section \"%s\", name \"%s\", value \"%s\"\n",
	      section, name, value);
      ok = 1;
    }

  return ok;
}

static void evaluate_ini_file(const char *ini_file)
{
  ini_parse(ini_file, ini_file_handler, NULL);
}

#if 0
static int
ignore_input(int c, struct rtems_termios_tty *tp)
{
  return 0;
}

static void
ignore_tty_input(void)
{
  rtems_termios_linesw[0].l_rint = &ignore_input;
}
#endif

static void Init(rtems_task_argument arg)
{
  char *argv[] = { "erl.rtems", "--", "-root", "/mnt/otp",
		   "-home", "/mnt/home", "-boot", "uid", 
		   "-noshell", "-noinput",
		   "-config", "/mnt/uid",
		   "-internal_epmd", "epmd_sup", "-sname", "uid" 
		   /* , "-init_debug", "-loader_debug" */
  };
  int argc = sizeof(argv)/sizeof(*argv);

  rtems_status_code sc = RTEMS_SUCCESSFUL;
  int rv = 0;

  sc = rtems_shell_init("SHLL", SHELL_STACK_SIZE, 10,
			CONSOLE_DEVICE_NAME, true, false, NULL);
  assert(sc == RTEMS_SUCCESSFUL);

  printf("erl_main: register I2C... ");
  sc = bsp_register_i2c();
  if (sc == RTEMS_SUCCESSFUL) {
    printf("done\n");
  } else {
    printf("failed\n");
  }

  uid_init_flash_file_system("/mnt");
  uid_init_network("/mnt/br_uid.ini");

  evaluate_ini_file("/mnt/br_uid.ini");

  sethostname(hostname, strlen(hostname));

#ifdef TELNETD_IN_APPLICATION
  rtems_telnetd_initialize();
#endif /* TELNETD_IN_APPLICATION */

  printf("mkdir /tmp/log\n");
  rv = mkdir("/tmp", 0755);
  assert(rv == 0);
  rv = mkdir("/tmp/log", 0755);
  assert(rv == 0);

  setenv("BINDIR", "/mnt/otp/lib/erlang/bin", 1);
  setenv("ROOTDIR", "/mnt/otp", 1);
  setenv("PROGNAME", "erl.rtems", 1);
  setenv("ERL_INETRC", "/mnt/home/erl_inetrc", 1);
  setenv("ERL_LIBS", "/mnt/apps", 1);
  setenv("HOME", "/mnt/home", 1);

  printf("Starting Erlang runtime\n");
  erl_start(argc, argv);
  exit(0);
}

#define CONFIGURE_MICROSECONDS_PER_TICK 10000

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_STUB_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_ZERO_DRIVER

#define CONFIGURE_APPLICATION_NEEDS_LIBBLOCK

/* increase max file size in IMFS to 64MB */
#define CONFIGURE_IMFS_MEMFILE_BYTES_PER_BLOCK 256 

#define CONFIGURE_FILESYSTEM_NFS

#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS 64

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE

#define CONFIGURE_INIT_TASK_STACK_SIZE (512 * 1024)

#define CONFIGURE_STACK_CHECKER_ENABLED

#define CONFIGURE_UNIFIED_WORK_AREAS
#define CONFIGURE_UNLIMITED_OJEBCTS
#define CONFIGURE_UNLIMITED_ALLOCATION_SIZE 8

#define CONFIGURE_BDBUF_BUFFER_MAX_SIZE (16 * 1024)
#define CONFIGURE_BDBUF_MAX_READ_AHEAD_BLOCKS 4
#define CONFIGURE_BDBUF_CACHE_MEMORY_SIZE (1 * 1024 * 1024)

#define CONFIGURE_INIT

#define CONFIGURE_MALLOC_DIRTY

#include <rtems/confdefs.h>

#include <bsp/irq-info.h>
#include <rtems/netcmds-config.h>

#define CONFIGURE_SHELL_COMMANDS_INIT
#define CONFIGURE_SHELL_COMMANDS_ALL

#define CONFIGURE_SHELL_USER_COMMANDS \
  &bsp_interrupt_shell_command, \
  &rtems_shell_ARP_Command, \
  &rtems_shell_HOSTNAME_Command, \
  &rtems_shell_PING_Command, \
  &rtems_shell_ROUTE_Command, \
  &rtems_shell_NETSTAT_Command, \
  &rtems_shell_IFCONFIG_Command, \
  &rtems_shell_TCPDUMP_Command, \
  &rtems_shell_SYSCTL_Command, \
  &rtems_shell_VMSTAT_Command

#include <rtems/shellconfig.h>

#else
  int
    main(int argc, char **argv)
  {
    erl_start(argc, argv);
    return 0;
  }
#endif
