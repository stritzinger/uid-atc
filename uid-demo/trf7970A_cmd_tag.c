/**
 * @file
 *
 * @brief TRF_tag command.
 *
 * Command handling for making the trf7970A RFID controller read from a tag
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
#include "trf7970A_regs.h"
#include "trf7970A_irq.h"

static int trf7970A_cmd_tag_func( int argc, char **argv )
{
  int                 eno;
  uint8_t             read_buf[3][MULTIIO_READ_BUF_SIZE];
  const unsigned int  WRITE_BUF_SIZE_CHARACTERS = 3 * MULTIIO_WRITE_BUF_SIZE;
  char                write_buf[strlen( "TRF_tag " ) + WRITE_BUF_SIZE_CHARACTERS];
  unsigned int        index;

  (void)argc;
  (void)argv;

  /*  8F = Reset command (clear FIFI and status registers)
     + 91 = Transmission with CRC
     + 3D = Continous write to TX Length registers
     + 00 30 = Content for TX Length registers: 2
     + 26 01 00 = 3 Bytes of FIFO data */
  eno = trf7979A_cmd_raw_string(
    "TRF_tag 8F 91 3D 00 30 26 01 00",
    &read_buf[0][0],
    MULTIIO_READ_BUF_SIZE
  );

  if( eno == 0 ) {
    /* Get IRQ Status */
    eno = trf7979A_cmd_raw_string(
      "TRF_tag 6C 00 00",
      &read_buf[0][0],
      MULTIIO_READ_BUF_SIZE
    );
    eno = trf7970A_irq_print_unexpected(
      TRF7970A_REGS_IRQ_STATUS_TX | TRF7970A_REGS_IRQ_STATUS_SRX,
      read_buf[0][1]
    );
  }
  if( eno == 0 ) {
    if( (read_buf[0][1] & TRF7970A_REGS_IRQ_STATUS_SRX) == 0 ) {
      /* Give the reader some more time, then try again */
      rtems_task_wake_after( rtems_clock_get_ticks_per_second() / 20 );
      if( eno == 0 ) {
        eno = trf7979A_cmd_raw_string(
          "TRF_tag 6C 00 00",
          &read_buf[0][0],
          MULTIIO_READ_BUF_SIZE
        );
        eno = trf7970A_irq_print_unexpected(
          TRF7970A_REGS_IRQ_STATUS_TX | TRF7970A_REGS_IRQ_STATUS_SRX,
          read_buf[0][1]
        );
      }
    }
  }
  if( (read_buf[0][1] & TRF7970A_REGS_IRQ_STATUS_SRX) == 0) {
    /* We did not get a reply */
    eno = EFAULT;
  }
  if( eno == 0 ) {
    /* Read the FIFO status. read_buf[1] will contain number of bytes in FIFO */
    eno = trf7979A_cmd_raw_string(
      "TRF_tag 5C 00",
      &read_buf[0][0],
      MULTIIO_READ_BUF_SIZE
    );
  }
  if( eno == 0 ) {
    /* Read the FIFO content */
    if( (read_buf[0][1] & TRF7970A_REGS_FIFO_STATUS_OVERFLOW) == 0 ) {
      if( read_buf[0][1] > 0 ) {
        sprintf( &write_buf[0], "TRF_tag 7F" );
        for( index = 0; index < read_buf[0][1] && index < MULTIIO_WRITE_BUF_SIZE; ++index ) {
          strcat( &write_buf[10 + index * 3], " 00" );
        }
        eno = trf7979A_cmd_raw_string(
          &write_buf[0],
          &read_buf[1][0],
          MULTIIO_READ_BUF_SIZE
        );
      }
    } else {
      eno = ENOSPC;
    }
  }
  if( eno == 0 ) {
    /* Reset command (clear FIFI and status registers) */
    eno = trf7979A_cmd_raw_string(
      "TRF_tag 8F",
      &read_buf[2][0],
      MULTIIO_READ_BUF_SIZE
    );
  }
  if( eno == 0 ) {
    /* Read RSSI levels and oscillator status */
    eno = trf7979A_cmd_raw_string(
      "TRF_tag 4F 00",
      &read_buf[2][0],
      MULTIIO_READ_BUF_SIZE
    );
  }

  if( eno == 0 ) {
    for( index = 1; index <= read_buf[0][1]; ++index ) {
      printf( "%02"PRIx8" ", read_buf[1][index] );
    }
    printf( "%02"PRIx8"\n", read_buf[2][1] );
  } else {
    printf( "Failure\n" );
  }
  return eno;
}

rtems_shell_cmd_t trf7970A_cmd_tag = {
  "TRF_tag",
  "TRF_tag\n"
  "Read from a tag.\n"
  "Returns the FIFO content plus the RSSI levels and oscillator status upon success or \n"
  "\"Failure\" upon failure.\n",
  "TRF",
  trf7970A_cmd_tag_func,
  NULL,
  NULL
};