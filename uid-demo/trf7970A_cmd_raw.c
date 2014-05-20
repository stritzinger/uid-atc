/**
 * @file
 *
 * @brief TRF_raw command.
 *
 * Command for writing uninterpreted bytes to the trf7970A RFID controller.
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

static int trf7970A_cmd_raw_func( int argc, char **argv )
{
  int     eno;
  uint8_t read_buf[MULTIIO_READ_BUF_SIZE];
  int     index_buf;

  /* Execute the handling for raw commands verbose */
  eno = mio_cmd_raw_hdlr(
    argc,
    argv,
    true,
    MULTIIO_ADDR_RFID_READER,
    &read_buf[0],
    MULTIIO_READ_BUF_SIZE
 );

  if( eno == 0 ) {
    /* Print the results */
    for( index_buf = 0; index_buf < (argc - 1); ++index_buf ) {
      printf( "%02X ", read_buf[index_buf] );
    }
    printf( "\n" );
  }
  return eno;
}

rtems_shell_cmd_t trf7970A_cmd_raw = {
  "TRF_raw",
  "TRF_raw <param1 [param2 [param3 [...]]]>\n"
  "Forwards parameters to the TRF RFID reader.\n"
  "Arguments are hexadecimal bytes with leading zeros and without\n"
  "leading 0x or trailing h\n"
  "Returns bytes returned by the TRF RFID reader in the same format.\n",
  "TRF",
  trf7970A_cmd_raw_func,
  NULL,
  NULL
};

int trf7979A_cmd_raw_string( const char* cmd_string, uint8_t *read_buf, const size_t read_buf_size )
{
  return mio_cmd_raw_string( cmd_string, read_buf, read_buf_size, MULTIIO_ADDR_RFID_READER );
}
