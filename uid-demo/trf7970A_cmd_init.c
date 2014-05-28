/**
 * @file
 *
 * @brief TRF_init command.
 *
 * Initialization handling for trf7970A RFID controller.
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
#include "config_uid.h"

static int trf7970A_cmd_init_func( int argc, char **argv )
{
  int eno;
  uint8_t read_buf[MULTIIO_READ_BUF_SIZE];

  (void)argc;
  (void)argv;

  /* Software Reset */
  eno = trf7979A_cmd_raw_string(
    "TRF_init 83",
    &read_buf[0],
    MULTIIO_READ_BUF_SIZE
  );
  
  if( eno == 0 ) {
    /* Idle command */
    eno = trf7979A_cmd_raw_string(
      "TRF_init 80",
      &read_buf[0],
      MULTIIO_READ_BUF_SIZE
    );
  }
  
  if( eno == 0 ) {
    /* Write register 0x0 (Chip Status Control Register):
       - 3V operation
       - Automatic enable
       - AGG off
       - Select Main RX input
       - full output power
       - RF output active
       - Direct mode 0
       - Active mode */
    eno = trf7979A_cmd_raw_string(
#ifdef USE_MULTIIO
      "TRF_init 00 21",
#else /* USE_MULTIIO */
      "TRF_init 00 20",
#endif /* USE_MULTIIO */
      &read_buf[0],
      MULTIIO_READ_BUF_SIZE
    );
  }
  
  if( eno == 0 ) {
    /* Write register 0x1 (ISO Control Register):
       - 212 kbps
       - NFC Normal Modes
       - passive mode
       - target
       - RFID Mode
       - Direct Mode 0
       - RX CRC (CRC is present in the response) */
    eno = trf7979A_cmd_raw_string(
      "TRF_init 01 02",
      &read_buf[0],
      MULTIIO_READ_BUF_SIZE
    );
  }

  if( eno == 0 ) {
    /* Write register 0x9 (Modulator and SYS_CLK Control Register)
       - ASK 30%
       - en_ana = Default
       - 6.78 MHz respectively 13.56 MHz
       - Default operation as defined in B0 to B2 (0x09)
       - 27.12-MHz crystal disabled */
    eno = trf7979A_cmd_raw_string(
      "TRF_init 09 27",
      &read_buf[0],
      MULTIIO_READ_BUF_SIZE
    );
  }
  
  if( eno == 0 ) {
    /* Continous read on register 0xC (IRQ Status Register) */
    eno = trf7979A_cmd_raw_string(
      "TRF_init 6C 00 00",
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

rtems_shell_cmd_t trf7970A_cmd_init = {
  "TRF_init",
  "TRF_init\n"
  "Initializes the TRF RFID reader.\n"
  "Returns \"Success\" upon success or \n"
  "\"Failure\" upon failure.\n",
  "TRF",
  trf7970A_cmd_init_func,
  NULL,
  NULL
};
