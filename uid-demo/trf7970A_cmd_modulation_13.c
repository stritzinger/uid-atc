/**
 * @file
 *
 * @brief TRF_13 command.
 *
 * Command handling for setting modulation of the trf7970A RFID controller to
 * 13%.
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

#define TRF7970A_CMD_MODULATION_MASK_30 0x07
#define TRF7970A_CMD_MODULATION_MASK_13 0x04

static int trf7970A_cmd_modulation_13_func( int argc, char **argv )
{
  int eno;
  uint8_t read_buf[MULTIIO_READ_BUF_SIZE];
  char    write_buf[strlen("TRF_13 ") + 3 * MULTIIO_WRITE_BUF_SIZE];

  (void)argc;
  (void)argv;

  /* Read register 0x9  (Modulator and SYS_CLK Control Register) */
  eno = trf7979A_cmd_raw_string(
    "TRF_13 49 00",
    &read_buf[0],
    MULTIIO_READ_BUF_SIZE
  );

  if( eno == 0 ) {
    read_buf[1] &= (uint8_t)~TRF7970A_CMD_MODULATION_MASK_30;
    read_buf[1] |= TRF7970A_CMD_MODULATION_MASK_13;
    snprintf( &write_buf[0], sizeof(write_buf), "TRF_13 09 %02"PRIx8, read_buf[1] );
    /* Write to register 0x9 (Modulator and SYS_CLK Control Register)
       Use perameters as previously read, but modulation set to 13% */
    eno = trf7979A_cmd_raw_string(
      &write_buf[0],
      &read_buf[0],
      MULTIIO_READ_BUF_SIZE
    );
  }

  if( eno == 0 ) {
    /* Continous read on register 0xC (IRQ Status Register) */
    eno = trf7979A_cmd_raw_string(
      "TRF_13 6C 00 00",
      &read_buf[0],
      MULTIIO_READ_BUF_SIZE
    );
    if( eno == 0 ) {
      /* No IRQ must be pending */
      if( read_buf[1] != 0x00 ) {
        eno = EPROTO;
      }
    }
  }

  if( eno == 0 ) {
    printf( "Success\n" );
  } else {
    printf( "Failure\n" );
  }
  return eno;
}

rtems_shell_cmd_t trf7970A_cmd_modulation_13 = {
  "TRF_13",
  "TRF_13\n"
  "Sets the modulation of the TRF RFID reader to 13%.\n"
  "Returns \"Success\" upon success or \n"
  "\"Failure\" upon failure.\n",
  "TRF",
  trf7970A_cmd_modulation_13_func,
  NULL,
  NULL
};