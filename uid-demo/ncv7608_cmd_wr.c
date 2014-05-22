/**
 * @file
 *
 * @brief NCV_wr command.
 *
 * Write and read the NCV7608.
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

#define BUF_SIZE 2
#define EVENT RTEMS_EVENT_0

static int ncv7608_cmd_wr_func( int argc, char **argv )
{
  int eno = 0;
  uint8_t write_buf[BUF_SIZE] = {0, 0};
  uint8_t read_buf[BUF_SIZE] = {0xff, 0xff};
  multiio_exchange_data exchange_data;
  const multiio_bus_driver *BUS_DRIVER = multiio_get_bus_driver();
  multiio_reply reply_data = MULTIIO_REPLY_INITIALIZER(
    read_buf,
    rtems_task_self(),
    EVENT,
    MULTIIO_ADDR_DIG_OP
  );

  /* Check argument */
  if( argc != 3 ) {
    printf( "Failure: Need exactly one argument!\n" );
    eno = E2BIG;
  }

  if( eno == 0 ) {
    /* Get value */
    if( 1 != sscanf( argv[1], "%2"SCNx8, &write_buf[0] ) ) {
      printf( "Failure: Invalid argument: %s\n", argv[1] );
      eno = EINVAL;
    }
    if( 1 != sscanf( argv[2], "%2"SCNx8, &write_buf[1] ) ) {
      printf( "Failure: Invalid argument: %s\n", argv[2] );
      eno = EINVAL;
    }
  }

  if( eno == 0 ) {
    /* Write to the interface */
    exchange_data.address        = MULTIIO_ADDR_DIG_OP;
    exchange_data.write_buf      = write_buf;
    exchange_data.read_buf       = read_buf;
    exchange_data.num_chars      = BUF_SIZE;
    exchange_data.arg            = &reply_data;
    exchange_data.on_reply_ready = multiio_reply_ready;
    eno = (BUS_DRIVER->data_exchange)( &exchange_data );
    if( eno == 0 ) {
      uint8_t *buf_read;
      /* Wait for the result */
      eno = multiio_reply_wait_for(
        reply_data.id_event,
        MULTIIO_ADDR_DIG_OP,
        rtems_clock_get_ticks_per_second() / 1000,
        &buf_read
      );
      if( eno == 0 ) {
      } else {
        printf( "Failure: Waiting for reply failed with error %d\n", eno );
      }
    } else {
      printf( "Failure: data_echange method failed with error %d\n", eno );
    }
  }

  if( eno == 0 ) {
    printf( "return: %02x %02x\n", read_buf[0], read_buf[1] );
  }

  if( eno == 0 ) {
    printf( "Success\n" );
  } else {
    printf( "Failure\n" );
  }
  return eno;
}

rtems_shell_cmd_t ncv7608_cmd_wr = {
  "NCV_wr",
  "NCV_wr <driver> <open_diagnostic>\n"
  "Write raw data to the output-chip and\n"
  "print response.\n"
  "Returns \"Success\" upon success or \n"
  "\"Failure\" upon failure.\n",
  "NCV",
  ncv7608_cmd_wr_func,
  NULL,
  NULL
};
