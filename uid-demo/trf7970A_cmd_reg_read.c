/**
 * @file
 *
 * @brief TRF_regr command.
 *
 * Command handling for reading a single register of the trf7970A RFID controller
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

#define TRF7979A_CMD_REG_READ_MASK_ADDRESS     0x1F
#define TRF7979A_CMD_REG_READ_MASK_SINGLE_READ 0x40

static int trf7970A_cmd_reg_read_func( int argc, char **argv )
{
  int     eno = 0;
  uint8_t reg_no = 0;
  uint8_t read_buf[2][MULTIIO_READ_BUF_SIZE];
  
  if( argc == 2 ) {
    char    write_buf[strlen("TRF_regr ") + 3 * MULTIIO_WRITE_BUF_SIZE];
    int     count;
    
    count = sscanf( argv[1], "%02"SCNx8, &reg_no );
    if( count != 1 ) {
      printf( "Invalid parameter: %s\n", argv[1] );
      eno = EINVAL;
    }
    if( eno == 0 ) {
      if( reg_no > 0x1f ) {
        printf( "Invalid register number: %s\n", argv[1] );
        eno = EINVAL;
      }
    }
    if( eno == 0 ) {
      /* Read the register */
      count = snprintf(
        &write_buf[0],
        sizeof(write_buf),
        "TRF_regr %02"PRIx8" 00",
          TRF7979A_CMD_REG_READ_MASK_SINGLE_READ
        | (reg_no & TRF7979A_CMD_REG_READ_MASK_ADDRESS)
      );
      if( count < 0 || (unsigned int)count >= sizeof(write_buf) ) {
        eno = EFAULT;
      }
    }
    if( eno == 0 ) {
      eno = trf7979A_cmd_raw_string(
        &write_buf[0],
        &read_buf[0][0],
        MULTIIO_READ_BUF_SIZE
      );
    }

    if( eno == 0 ) {
      /* Continous read on register 0xC (IRQ Status Register) */
      eno = trf7979A_cmd_raw_string(
        "TRF_regr 6C 00 00",
        &read_buf[1][0],
        MULTIIO_READ_BUF_SIZE
      );
    }
    if( eno == 0 ) {
      /* No IRQ must be pending */
      if( read_buf[1][1] != 0x00 ) {
        printf( "Unexpected IRQ: %02"PRIx8"\n", read_buf[1][1] );
        eno = EPROTO;
      }
    }
  } else {
    printf( "Invalid number of parameters\n" );
    eno = EINVAL;
  }
  
  if( eno == 0 ) {
    printf( "%02"PRIx8"\n", read_buf[0][1] );
  } else {
    printf( "Failure\n" );
  }
  return eno;
}

rtems_shell_cmd_t trf7970A_cmd_reg_read = {
  "TRF_regr",
  "TRF_regr <REGISTER_NUMBER>\n"
  "Reads the content of the single register identified by\n"
  "REGISTER_NUMBER. REGISTER_NUMBER must be a hexadecimal\n"
  "number without leading 0x or trailing h.\n"
  "Returns the content of the register as a hexadecimal number in\n"
  "the same format as REGISTER_NUMBER upon success or \n"
  "\"Failure\" upon failure.\n",
  "TRF",
  trf7970A_cmd_reg_read_func,
  NULL,
  NULL
};