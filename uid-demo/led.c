/**
 * @file
 *
 * @brief LED Driver.
 *
 * Driver for the LEDs on the UID board
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
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <rtems/libio.h>
#include "led.h"

#define LED_DEV_NAME_LED_CONTROLLER "/dev/i2c1.sc620"

typedef struct {
  uint8_t activity;
} led_context;

static led_context led_ctxt;

static rtems_status_code led_errno_to_status_code( const int eno )
{
  rtems_status_code sc = RTEMS_STATUS_CODES_FIRST;
  while( eno != rtems_status_code_to_errno( sc ) && sc != RTEMS_STATUS_CODES_LAST ) {
    ++sc;
  }
  return sc;
}

rtems_device_driver led_init(
  rtems_device_major_number id_major,
  rtems_device_minor_number id_minor,
  void                     *arg
)
{
  rtems_status_code sc               = RTEMS_SUCCESSFUL;
  int               fhdl;
  uint8_t           index;
  int               nbytes;
  char              led_names[LED_OBJECT_COUNT_FILE_DESCRIPTORS][19] = {
    LED_DEV_NAME_PWR_RED,
    LED_DEV_NAME_PWR_GREEN,
    LED_DEV_NAME_ETH_RED,
    LED_DEV_NAME_ETH_GREEN,
    LED_DEV_NAME_DP_RED,
    LED_DEV_NAME_DP_GREEN,
    LED_DEV_NAME_RF_RED,
    LED_DEV_NAME_RF_GREEN
  };

  (void)id_minor;
  (void)arg;

  fhdl = open( LED_DEV_NAME_LED_CONTROLLER, O_RDWR);

  if( fhdl >= 0 ) {
    /* Switch off all LEDs */
    uint8_t buf[2] = {0x00, 0x00};
    if( sc == RTEMS_SUCCESSFUL ) {
      nbytes = write( fhdl, &buf[0], sizeof(buf) );
      if( nbytes < 0 ) {
        sc = led_errno_to_status_code( errno );
      } else {
        led_ctxt.activity = 0;
      }
    }
    if( sc == RTEMS_SUCCESSFUL ) {
      /* Select a gain of 373 micro Amperes. This sould be safe for our LEDs */
      buf[0] = 0x09;
      buf[1] = 0x0b;
      nbytes = write( fhdl, &buf[0], sizeof(buf) );
      if( nbytes < 0 ) {
        sc = led_errno_to_status_code( errno );
      }
    }

    if( sc == RTEMS_SUCCESSFUL ) {
      for( index = 0; index < LED_OBJECT_COUNT_FILE_DESCRIPTORS && sc == RTEMS_SUCCESSFUL; ++index ) {
        if( sc == RTEMS_SUCCESSFUL ) {
          /* Select medium dimming */
          buf[0] = (uint8_t)(index + 1);
          buf[1] = 0x19;
          nbytes = write( fhdl, &buf[0], sizeof(buf) );
          if( nbytes < 0 ) {
            sc = led_errno_to_status_code( errno );
          }
        }
        if( sc == RTEMS_SUCCESSFUL ) {
          sc = rtems_io_register_name(
            &led_names[index][0],
            id_major,
            (rtems_device_minor_number)(1 << index)
          );
        }
      }
    }
    fhdl = close( fhdl );
    if( fhdl != 0 ) {
      if( sc == RTEMS_SUCCESSFUL ) {
        sc = led_errno_to_status_code( errno );
      }
    }
  } else {
    sc = RTEMS_INVALID_NAME;
  }
  
  return sc;
}

rtems_device_driver led_read(
  rtems_device_major_number id_major,
  rtems_device_minor_number id_minor,
  void                     *arg
)
{
  rtems_status_code      sc  = RTEMS_IO_ERROR;
  rtems_libio_rw_args_t *rw  = arg;
  unsigned char         *buf = (unsigned char *) &rw->buffer[0];

  if (rw->count == 1 && id_minor < (1 << LED_OBJECT_COUNT_FILE_DESCRIPTORS)) {
    *buf = ( led_ctxt.activity & id_minor ) != 0;
    sc = RTEMS_SUCCESSFUL;
  }

  rw->bytes_moved = sc == RTEMS_SUCCESSFUL ? 1 : 0;

  return sc;
}

rtems_device_driver led_write(
  rtems_device_major_number id_major,
  rtems_device_minor_number id_minor,
  void                     *arg
)
{
  rtems_status_code      sc  = RTEMS_IO_ERROR;
  rtems_libio_rw_args_t *rw  = arg;
  unsigned char         *buf = (unsigned char *) &rw->buffer[0];

  if (rw->count == 1 && id_minor < (1 << LED_OBJECT_COUNT_FILE_DESCRIPTORS)) {
    int fhdl = open( LED_DEV_NAME_LED_CONTROLLER, O_WRONLY, S_IREAD );
    if( fhdl >= 0 ) {
      uint8_t write_buf[] = {0x00, 0x00};
      int     nbytes;
      if( buf[0] == 0 ) {
        write_buf[1] = led_ctxt.activity & (uint8_t)~id_minor;
      } else {
        write_buf[1] = led_ctxt.activity | (uint8_t)id_minor;
      }
      nbytes = write( fhdl, &write_buf[0], sizeof(write_buf) );
      if( nbytes == sizeof(write_buf) ) {
        sc = RTEMS_SUCCESSFUL;
        led_ctxt.activity = write_buf[1];
      }
      fhdl = close( fhdl );
      if( fhdl != 0 ) {
        if( sc == RTEMS_SUCCESSFUL ) {
          sc = led_errno_to_status_code( errno );
        }
      }
    }
  }
        
  rw->bytes_moved = sc == RTEMS_SUCCESSFUL ? 1 : 0;

  return sc;
}

const rtems_driver_address_table led_driver_table = {
  led_init,     /* initialization_entry */
  NULL,         /* open_entry */
  NULL,         /* close_entry */
  led_read,     /* read_entry */
  led_write,    /* write_entry */
  NULL,         /* control_entry */
};