/*
 * Copyright (c) 2011 embedded brains GmbH.  All rights reserved.
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

#include <errno.h>
#include <assert.h>
#include <stdlib.h>

#include <rtems.h>
#include <rtems/shell.h>
#include <rtems/console.h>

#include <bsp.h>
#include "led.h"
#include "spi.h"
#include "trf7970A.h"
#include "chip_select.h"
#include "multiplexer.h"

#define SHELL_STACK_SIZE (8 * 1024)

#define INIT_OBJECT_COUNT_DRIVERS_CONSOLE   2
#define INIT_OBJECT_COUNT_DRIVERS_I2C_BUS   1
#define INIT_OBJECT_COUNT_SEMAPHORE_I2C_BUS 2

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

static void Init(rtems_task_argument arg)
{
  rtems_status_code  sc = RTEMS_SUCCESSFUL;
  int                eno;
  rtems_shell_cmd_t *cmd;
  rtems_device_major_number id_major_led = 0;

  printf( "\nuid-demo Version 0.3\n" );

  sc = bsp_register_i2c();
  assert( sc == RTEMS_SUCCESSFUL );
  eno = rtems_status_code_to_errno( sc );

  if( eno == 0 ) {
    sc = rtems_io_register_driver(
      0,
      &led_driver_table,
      &id_major_led
    );
    assert( sc == RTEMS_SUCCESSFUL );
    eno = rtems_status_code_to_errno( sc );
  }

  multiplexer_init();
  chip_select_init();

  if( eno == 0 ) {
    eno = trf7970A_init(
      &spi_bus_driver,
      RTEMS_EVENT_0,
      RTEMS_EVENT_1
    );
    assert( eno == 0 );
  }

  if( eno == 0 ) {
    cmd = rtems_shell_add_cmd_struct(&exception_command);
    assert( cmd == &exception_command );
    if( cmd != &exception_command ) {
      eno = EFAULT;
    }
  }
  if( eno == 0 ) {
    cmd = rtems_shell_add_cmd_struct(&trf7970A_cmd_raw);
    assert( cmd == &trf7970A_cmd_raw );
    if( cmd != &trf7970A_cmd_raw ) {
      eno = EFAULT;
    }
  }
  if( eno == 0 ) {
    cmd = rtems_shell_add_cmd_struct(&trf7970A_cmd_init);
    assert( cmd == &trf7970A_cmd_init );
    if( cmd != &trf7970A_cmd_init ) {
      eno = EFAULT;
    }
  }
  if( eno == 0 ) {
    cmd = rtems_shell_add_cmd_struct(&trf7970A_cmd_modulation_13);
    assert( cmd == &trf7970A_cmd_modulation_13 );
    if( cmd != &trf7970A_cmd_modulation_13 ) {
      eno = EFAULT;
    }
  }
  if( eno == 0 ) {
    cmd = rtems_shell_add_cmd_struct(&trf7970A_cmd_modulation_30);
    assert( cmd == &trf7970A_cmd_modulation_30 );
    if( cmd != &trf7970A_cmd_modulation_30 ) {
      eno = EFAULT;
    }
  }
  if( eno == 0 ) {
    cmd = rtems_shell_add_cmd_struct(&trf7970A_cmd_modulation_100);
    assert( cmd == &trf7970A_cmd_modulation_100 );
    if( cmd != &trf7970A_cmd_modulation_100 ) {
      eno = EFAULT;
    }
  }
  if( eno == 0 ) {
    cmd = rtems_shell_add_cmd_struct(&trf7970A_cmd_readall);
    assert( cmd == &trf7970A_cmd_readall );
    if( cmd != &trf7970A_cmd_readall ) {
      eno = EFAULT;
    }
  }
  if( eno == 0 ) {
    cmd = rtems_shell_add_cmd_struct(&trf7970A_cmd_reg_read);
    assert( cmd == &trf7970A_cmd_reg_read );
    if( cmd != &trf7970A_cmd_reg_read ) {
      eno = EFAULT;
    }
  }
  if( eno == 0 ) {
    cmd = rtems_shell_add_cmd_struct(&trf7970A_cmd_reg_write);
    assert( cmd == &trf7970A_cmd_reg_write );
    if( cmd != &trf7970A_cmd_reg_write ) {
      eno = EFAULT;
    }
  }
  if( eno == 0 ) {
    cmd = rtems_shell_add_cmd_struct(&trf7970A_cmd_agc_off);
    assert( cmd == &trf7970A_cmd_agc_off );
    if( cmd != &trf7970A_cmd_agc_off ) {
      eno = EFAULT;
    }
  }
  if( eno == 0 ) {
    cmd = rtems_shell_add_cmd_struct(&trf7970A_cmd_agc_on);
    assert( cmd == &trf7970A_cmd_agc_on );
    if( cmd != &trf7970A_cmd_agc_on ) {
      eno = EFAULT;
    }
  }
  if( eno == 0 ) {
    cmd = rtems_shell_add_cmd_struct(&trf7970A_cmd_am);
    assert( cmd == &trf7970A_cmd_am );
    if( cmd != &trf7970A_cmd_am ) {
      eno = EFAULT;
    }
  }
  if( eno == 0 ) {
    cmd = rtems_shell_add_cmd_struct(&trf7970A_cmd_pm);
    assert( cmd == &trf7970A_cmd_pm );
    if( cmd != &trf7970A_cmd_pm ) {
      eno = EFAULT;
    }
  }
  if( eno == 0 ) {
    cmd = rtems_shell_add_cmd_struct(&trf7970A_cmd_tag_detect);
    assert( cmd == &trf7970A_cmd_tag_detect );
    if( cmd != &trf7970A_cmd_tag_detect ) {
      eno = EFAULT;
    }
  }
  if( eno == 0 ) {
    cmd = rtems_shell_add_cmd_struct(&trf7970A_cmd_tag);
    assert( cmd == &trf7970A_cmd_tag );
    if( cmd != &trf7970A_cmd_tag ) {
      eno = EFAULT;
    }
  }
  if( eno == 0 ) {
    sc = rtems_shell_init(
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

  exit(0);
}

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER

#define CONFIGURE_USE_IMFS_AS_BASE_FILESYSTEM

#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS \
(                                                \
  4                                              \
  + LED_OBJECT_COUNT_FILE_DESCRIPTORS            \
)

#define CONFIGURE_EXTRA_TASK_STACKS \
  (SHELL_STACK_SIZE - RTEMS_MINIMUM_STACK_SIZE)

#define CONFIGURE_MAXIMUM_TASKS 2

#define CONFIGURE_MAXIMUM_SEMAPHORES    \
(                                       \
    SPI_OBJECT_COUNT_SEMAPHORE          \
  + INIT_OBJECT_COUNT_SEMAPHORE_I2C_BUS \
  + LED_OBJECT_COUNT_SEMAPHORES         \
)

#define CONFIGURE_MAXIMUM_DRIVERS        \
(                                        \
    INIT_OBJECT_COUNT_DRIVERS_CONSOLE    \
  + INIT_OBJECT_COUNT_DRIVERS_I2C_BUS    \
  + LED_OBJECT_COUNT_DRIVERS             \
)                                        \

#define CONFIGURE_MAXIMUM_POSIX_KEYS 16
#define CONFIGURE_MAXIMUM_POSIX_KEY_VALUE_PAIRS 16

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE

#define CONFIGURE_STACK_CHECKER_ENABLED

#define CONFIGURE_INIT

#include <rtems/confdefs.h>

#define CONFIGURE_SHELL_COMMANDS_INIT

#define CONFIGURE_SHELL_COMMAND_CPUUSE
#define CONFIGURE_SHELL_COMMAND_PERIODUSE
#define CONFIGURE_SHELL_COMMAND_STACKUSE

#include <rtems/shellconfig.h>
