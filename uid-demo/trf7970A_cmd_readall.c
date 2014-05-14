/**
 * @file
 *
 * @brief TRF_readall command.
 *
 * Command for reading all reagisters of the trf7970A RFID controller.
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

#include <errno.h>
#include <inttypes.h>
#include <rtems/shell.h>
#include "multiio.h"
#include "trf7970A_impl.h"

static int trf7970A_cmd_readall_func( int argc, char **argv )
{
  int     eno;
  uint8_t read_buf[2][MULTIIO_READ_BUF_SIZE];
  int     index;

  (void)argc;
  (void)argv;

  /* Read register all registers */
  eno = trf7979A_cmd_raw_string(
    "TRF_readall 60"
    " 00 00 00 00 00 00 00 00"
    " 00 00 00 00 00 00 00 00"
    " 00 00 00 00 00 00 00 00"
    " 00 00 00 00 00 00 00 00",
    &read_buf[0][0],
    MULTIIO_READ_BUF_SIZE
  );

  if( eno == 0 ) {
    /* Continous read on register 0xC (IRQ Status Register) */
    eno = trf7979A_cmd_raw_string(
      "TRF_readall 6C 00 00",
      &read_buf[1][0],
      MULTIIO_READ_BUF_SIZE
    );
    if( eno == 0 ) {
      /* No IRQ must be pending */
      if( read_buf[1][1] != 0x00 ) {
        eno = EPROTO;
      }
    }
  }

  if( eno == 0 ) {
    for( index = 1; index < MULTIIO_READ_BUF_SIZE; ++index ) {
      printf( "%02"PRIx8" ", read_buf[0][index] );
    }
    printf( "\n" );
  } else {
    printf( "Failure\n" );
  }
  return eno;
}

rtems_shell_cmd_t trf7970A_cmd_readall = {
  "TRF_readall",
  "TRF_readall\n"
  "Reads out all registers of the RFID reader.\n"
  "Returns the register contents as list of space seperated\n"
  "hexadecimal values without leading 0x or trailing h upon success\n"
  "or \"Failure\" upon failure.\n",
  "TRF",
  trf7970A_cmd_readall_func,
  NULL,
  NULL
};