/**
 * @file
 *
 * @brief SPI Driver
 *
 *Implements the SPI variant of a mutiio bus driver
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
#include <bsp.h>
#include <bsp/irq.h>
#include "spi.h"
#include "multiio.h"
#include "chip_select.h"
#include "multiplexer.h"

#define  MPC83XX_SPIE_LT     (1 << (31-17))

spi_context spi_the_ctxt;

int spi_init( void )
{
  rtems_status_code    sc;
  int                  eno;
  rtems_libi2c_bus_t  *bus_handle = (rtems_libi2c_bus_t*)(&spi_the_ctxt.spi_desc);
  mpc83xx_spi_softc_t *spi_softc  = &spi_the_ctxt.spi_desc.softc;

  memset( &spi_the_ctxt, 0, sizeof(spi_the_ctxt) );

  spi_softc->reg_ptr        = &mpc83xx.spi;
  spi_softc->initialized    = false;
  spi_softc->irq_number     = BSP_IPIC_IRQ_SPI;
  spi_softc->base_frq       = BSP_bus_frequency;
  spi_softc->irq_sema_id    = RTEMS_ID_NONE;
  spi_softc->curr_addr      = 0;
  spi_softc->idle_char      = 0x00;
  spi_softc->bytes_per_char = 1;
  spi_softc->bit_shift      = 16;

  multiplexer_select_spi();
  sc = mpc83xx_spi_init( bus_handle );
  eno = rtems_status_code_to_errno( sc );

  if( eno == 0 ) {
    const rtems_libi2c_tfr_mode_t TRF_MODE = {
      1000000, /* maximum bits per second */
      8,       /* how many bits per byte/word/longword? */
      false,   /* true: send LSB first                  */
      false,   /* true: inverted clock (high active)    */
      true,    /* true: clock starts toggling at start of data tfr */
      0x00     /* This character will be continuously transmitted in read only functions */
    };
    sc = mpc83xx_spi_set_tfr_mode(
      bus_handle, /* bus specifier structure        */
      &TRF_MODE   /* transfer mode info             */
    );
    eno = rtems_status_code_to_errno( sc );
  }  
  
  return eno;
}

int spi_data_exchange(
  multiio_exchange_data *data
)
{
  int                 eno = 0;
  int                 bytes_received;
  rtems_libi2c_bus_t *bus_handle = (rtems_libi2c_bus_t*)(&spi_the_ctxt.spi_desc);

  assert( data                 != NULL );
  assert( data->on_reply_ready != NULL );

  /* For the non-adressable SPI driver only the RFID reader is supported */
  assert( data->address == MULTIIO_ADDR_RFID_READER );

  chip_select();
  bytes_received = mpc83xx_spi_read_write_bytes(
    bus_handle,
    &data->read_buf[0],
    &data->write_buf[0],
    (const int)data->num_chars
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

static void spi_irq_on_off(
 const rtems_irq_connect_data *irq_conn_data
)
{
  (void)irq_conn_data;
}


static int spi_irq_is_on(
 const rtems_irq_connect_data *irq_conn_data
)
{
  (void)irq_conn_data;
  return true;
}

void spi_register_irq_handler (
  const multiio_addr        addr,
  const multiio_irq_handler handler,
  void                     *handler_arg
)
{
  bool retval = true;

  (void)addr;

  rtems_irq_connect_data irq_conn_data = {
    BSP_IPIC_IRQ_IRQ1,
    spi_the_ctxt.irq_data.handler, /* rtems_irq_hdl           */
    spi_the_ctxt.irq_data.arg,     /* (rtems_irq_hdl_param)   */
    spi_irq_on_off,                /* (rtems_irq_enable)      */
    spi_irq_on_off,                /* (rtems_irq_disable)     */
    spi_irq_is_on                   /* (rtems_irq_is_enabled)  */
  };

  if( spi_the_ctxt.irq_data.handler != NULL ) {
    /* un-install handler for SPI device */
    retval = BSP_remove_rtems_irq_handler( &irq_conn_data );
    assert( retval == true );
    if( retval == true ) {
      spi_the_ctxt.irq_data.handler = NULL;
      spi_the_ctxt.irq_data.arg     = NULL;
    }
  }
  if( retval != false ) {
    /* Insall new handler */
    irq_conn_data.hdl    = handler;
    irq_conn_data.handle = handler_arg;
    retval = BSP_install_rtems_irq_handler( &irq_conn_data );
    if( retval != false ) {
      spi_the_ctxt.irq_data.handler = handler;
      spi_the_ctxt.irq_data.arg     = handler_arg;
    }
  }
}

multiio_bus_driver spi_bus_driver = MULTIIO_BUS_DRIVER_INITIALIZER(
  spi_init,
  spi_data_exchange,
  spi_register_irq_handler
);