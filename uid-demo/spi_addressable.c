/**
 * @file
 *
 * @brief Addressable SPI Driver
 *
 * Implements the addressable SPI variant of a mutiio bus driver.
 * Only the data exchange method needs to be different. The reset of the
 * handling can be taken from the SPI driver.
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

#include <assert.h>
#include <mpc83xx/mpc83xx_spidrv.h>
#include "multiio.h"
#include "spi.h"
#include "chip_select.h"

int spi_addressable_data_exchange(
  multiio_exchange_data *data
)
{
  int                 eno = 0;
  int                 bytes_received;
  rtems_libi2c_bus_t *bus_handle = (rtems_libi2c_bus_t*)(&spi_the_ctxt.spi_desc);
  
  assert( data                 != NULL );
  assert( data->on_reply_ready != NULL );
  
  chip_select();
  bytes_received = mpc83xx_spi_read_write_bytes(
    bus_handle,
    &data->read_buf[0],
    (uint8_t*)&data->address,
    (const int)data->num_chars + 1
  );
  chip_deselect();
  
  if( bytes_received < 0 ) {
    eno = bytes_received * -1;
  } else {
    if( data->on_reply_ready != NULL ) {
      eno = (data->on_reply_ready)(
        data->arg
      );
    }
  }
  
  return eno;
}

multiio_bus_driver spi_addressable_bus_driver = MULTIIO_BUS_DRIVER_INITIALIZER(
  spi_init,
  spi_addressable_data_exchange,
  spi_register_irq_handler
);
