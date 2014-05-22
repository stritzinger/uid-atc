/**
 * @file
 *
 * @brief MIO_raw command.
 *
 * Command for writing uninterpreted bytes to the Multi-IO layer.
 *
 */

/*
 * Copyright (c) 2014 embedded brains GmbH.  All rights reserved.
 *
 *  embedded brains GmbH
 *  Dornierstr. 4
 *  82178 Puchheim
 *  Germany
 *  <rtems@embedded-brains.de>
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rtems.org/license/LICENSE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <inttypes.h>
#include <rtems/shell.h>
#include "mio_impl.h"
#include "multiio.h"

#define BUF_SIZE 1

static int mio_cmd_input_func( int argc, char **argv )
{
  int     eno;
  uint8_t read_buf[BUF_SIZE];
  int     index_buf;

  eno = mio_cmd_raw_string(
    "MIO_input 00",
    read_buf,
    BUF_SIZE,
    MULTIIO_ADDR_DIG_IP
  );

  if( eno == 0 ) {
    /* Print the results */
    printf( "%02X\n", read_buf[0] );
  }
  return eno;
}

rtems_shell_cmd_t mio_cmd_input = {
  "MIO_input",
  "MIO_input\n"
  "Read MultiIO inputs\n",
  "MIO",
  mio_cmd_input_func,
  NULL,
  NULL
};
