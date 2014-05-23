/**
 * @file
 *
 * @brief Digital Output Driver for NCV7608
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

#include <rtems/libio.h>
#include "ncv7608.h"
#include "multiio.h"

#define NCV7608_ID_MINOR 0

typedef struct {
  uint8_t driver_en;
  uint8_t diag_en;
  rtems_event_set id_event_reply_ready;
} ncv7608_context;

#define NCV7608_CONTEXT_INITIALIZER() \
{                                     \
  0,                                  \
  0,                                  \
  RTEMS_EVENT_0                       \
}

#define BUF_SIZE 2

static ncv7608_context ncv7608_the_context = NCV7608_CONTEXT_INITIALIZER();

rtems_device_driver ncv7608_init(
  rtems_device_major_number id_major,
  rtems_device_minor_number id_minor,
  void                     *arg
)
{
  rtems_status_code  sc   = RTEMS_SUCCESSFUL;
  ncv7608_init_args *args = (ncv7608_init_args*)arg;
  
  (void)id_minor;

  sc = rtems_io_register_name(
    NCV7608_DEV_NAME_BMP,
    id_major,
    NCV7608_ID_MINOR
  );
  if( sc == RTEMS_SUCCESSFUL ) {
    ncv7608_the_context.id_event_reply_ready = args->id_event_reply_ready;
  }

  return sc;
}

static rtems_status_code rw_from_context( uint8_t read_buf[BUF_SIZE] )
{
  /* write data from context and return result in read[] */
  rtems_status_code      sc  = RTEMS_IO_ERROR;

  uint8_t write_buf[]                  = { 0x00, 0x00 };
  const multiio_bus_driver *BUS_DRIVER = multiio_get_bus_driver();
  multiio_reply reply_data             = MULTIIO_REPLY_INITIALIZER(
    read_buf,
    rtems_task_self(),
    ncv7608_the_context.id_event_reply_ready,
    MULTIIO_ADDR_DIG_OP
  );
  multiio_exchange_data exchange_data;
  int eno;

  write_buf[0] = ncv7608_the_context.driver_en;
  write_buf[1] = ncv7608_the_context.diag_en;

  exchange_data.address        = MULTIIO_ADDR_DIG_OP;
  exchange_data.write_buf      = &write_buf[0];
  exchange_data.read_buf       = &read_buf[0];
  exchange_data.num_chars      = sizeof(write_buf);
  exchange_data.arg            = &reply_data;
  exchange_data.on_reply_ready = multiio_reply_ready;
  eno = (BUS_DRIVER->data_exchange)( &exchange_data );
  if( eno == 0 ) {
    uint8_t* buf_read;
    /* Wait for the result */
    eno = multiio_reply_wait_for(
      reply_data.id_event,
      MULTIIO_ADDR_DIG_OP,
      rtems_clock_get_ticks_per_second() / 1000,
      &buf_read
    );
    if( eno == 0 ) {
      read_buf[0] = buf_read[0];
      read_buf[1] = buf_read[1];
      sc = RTEMS_SUCCESSFUL;
    }
  }

  return sc;
}

rtems_device_driver ncv7608_read(
  rtems_device_major_number id_major,
  rtems_device_minor_number id_minor,
  void                     *arg
)
{
  rtems_status_code      sc  = RTEMS_IO_ERROR;
  rtems_libio_rw_args_t *rw  = arg;
  unsigned char         *buf = (unsigned char *) &rw->buffer[0];

  (void)id_major;

  if (rw->count >= 2 && id_minor == NCV7608_ID_MINOR ) {
    sc = rw_from_context( buf );
  }
  
  rw->bytes_moved = sc == RTEMS_SUCCESSFUL ? 2 : 0;
  
  return sc;
}

rtems_device_driver ncv7608_write(
  rtems_device_major_number id_major,
  rtems_device_minor_number id_minor,
  void                     *arg
)
{
  rtems_status_code      sc  = RTEMS_IO_ERROR;
  rtems_libio_rw_args_t *rw  = arg;
  unsigned char         *buf = (unsigned char *) &rw->buffer[0];

  (void)id_major;

  if (rw->count == 1 && id_minor == NCV7608_ID_MINOR) {
    uint8_t read_buf[BUF_SIZE] = { 0x00, 0x00 };
    ncv7608_the_context.driver_en = buf[0];

    sc = rw_from_context( read_buf );
  }
  
  rw->bytes_moved = sc == RTEMS_SUCCESSFUL ? 1 : 0;
  
  return sc;
}

const rtems_driver_address_table ncv7608_driver_table = {
  ncv7608_init,  /* initialization_entry */
  NULL,          /* open_entry */
  NULL,          /* close_entry */
  ncv7608_read,  /* read_entry */
  ncv7608_write, /* write_entry */
  NULL,          /* control_entry */
};
