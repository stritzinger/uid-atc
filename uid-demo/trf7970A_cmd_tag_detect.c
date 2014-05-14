/**
 * @file
 *
 * @brief TRF_tag_detect command.
 *
 * Command handling disabling AGC of the trf7970A RFID controller
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
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "multiio.h"
#include "led.h"
#include "trf7970A_impl.h"
#include "trf7970A_regs.h"
#include "trf7970A_irq.h"

static int trf7970A_cmd_tag_detect_func( int argc, char **argv )
{
  int     eno = 0;
  uint8_t read_buf[MULTIIO_READ_BUF_SIZE];
  int     fd_led_rf_red    = -1;
  int     fd_led_rf_green  = -1;

  (void)argc;
  (void)argv;

  printf( "Enter 'q' to quit\n" );

  fd_led_rf_red = open( LED_DEV_NAME_RF_RED, O_WRONLY );
  if( fd_led_rf_red == -1 ) {
    eno = errno;
  }
  if( eno == 0 ) {
    fd_led_rf_green = open( LED_DEV_NAME_RF_GREEN, O_WRONLY );
    if( fd_led_rf_green == -1 ) {
      eno = errno;
    }
  }

  while( eno == 0 ) {
    ssize_t bytes_written;
    uint8_t led_status;
    /*  8F = Reset command (clear FIFI and status registers)
      + 91 = Transmission with CRC
      + 3D = Continous write to TX Length registers
      + 00 30 = Content for TX Length registers: 2
      + 26 01 00 = 3 Bytes of FIFO data */
    eno = trf7979A_cmd_raw_string(
      "TRF_tag 8F 91 3D 00 30 26 01 00",
      &read_buf[0],
      MULTIIO_READ_BUF_SIZE
    );

    if( eno == 0 ) {
      /* Get IRQ Status */
      eno = trf7979A_cmd_raw_string(
        "TRF_tag 6C 00 00",
        &read_buf[0],
        MULTIIO_READ_BUF_SIZE
      );
      eno = trf7970A_irq_print_unexpected(
        TRF7970A_REGS_IRQ_STATUS_TX | TRF7970A_REGS_IRQ_STATUS_SRX,
        read_buf[1]
      );
    }
    if( eno == 0 ) {
      if( (read_buf[1] & TRF7970A_REGS_IRQ_STATUS_SRX) == 0 ) {
        /* Give the reader some more time, then try again */
        rtems_task_wake_after( rtems_clock_get_ticks_per_second() / 1000 );
        if( eno == 0 ) {
          eno = trf7979A_cmd_raw_string(
            "TRF_tag 6C 00 00",
            &read_buf[0],
            MULTIIO_READ_BUF_SIZE
          );
          eno = trf7970A_irq_print_unexpected(
            TRF7970A_REGS_IRQ_STATUS_TX | TRF7970A_REGS_IRQ_STATUS_SRX,
            read_buf[1]
          );
        }
      }
    }
    if( (read_buf[1] & TRF7970A_REGS_IRQ_STATUS_SRX) == 0) {
      /* We did not get a reply */
      printf( "Failure\n" );
      led_status = 0;
    } else {
      printf( "\t\tSuccess\n" );
      led_status = 1;
    }
    bytes_written = write( fd_led_rf_green, &led_status, sizeof(led_status) );
    if( bytes_written != sizeof(led_status) ) {
      eno = errno;
    } else {
      bytes_written = write( fd_led_rf_red, &led_status, sizeof(led_status) );
      if( bytes_written != sizeof(led_status) ) {
        eno = errno;
      }
    }
    
    rtems_task_wake_after( ( rtems_clock_get_ticks_per_second() / 1000 ) * 9 );
  }
  if( fd_led_rf_red != -1 ) {
    if( eno == 0 ) {
      eno = close( fd_led_rf_red );
    } else {
      close( fd_led_rf_red );
    }
  }
  if( fd_led_rf_green != -1 ) {
    if( eno == 0 ) {
      eno = close( fd_led_rf_red );
    } else {
      close( fd_led_rf_red );
    }
  }

  return eno;
}

rtems_shell_cmd_t trf7970A_cmd_tag_detect = {
  "TRF_tag_detect",
  "TRF_tag_detect\n"
  "Permanently searches for a tag. Terminate with 'q'\n"
  "Prints out \"Success\" if a tag was found and\n"
  "\"Failure\" each time none was found.\n",
  "TRF",
  trf7970A_cmd_tag_detect_func,
  NULL,
  NULL
};