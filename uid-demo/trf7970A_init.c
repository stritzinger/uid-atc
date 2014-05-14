/**
 * @file
 *
 * @brief Initialization handling.
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

#include "trf7970A.h"
#include "trf7970A_impl.h"

trf7970A_impl_context trf7970A_impl_the_context = TRF7970A_IMPL_CONTEXT_INITIALIZER();

static void trf7970A_init_irq_handler( void* arg )
{
//  trf7970A_impl_context *the_context = (trf7970A_impl_context*)arg;

  printk( "trf7970A irq\n" );
}

int trf7970A_init(
  const multiio_bus_driver *bus_driver,
  const rtems_event_set     EVENT_ID_REPLY_READY,
  const rtems_event_set     EVENT_ID_DEVICE_IRQ
)
{
  int eno;

  trf7970A_impl_the_context.event_id_reply_ready = EVENT_ID_REPLY_READY;
  
  eno = multiio_init(
    bus_driver
  );

/*  if( eno == 0 ) {
    (bus_driver->register_irq_handler)(
      MULTIIO_ADDR_RFID_READER,
      trf7970A_init_irq_handler,
      &trf7970A_impl_the_context
    );
  }*/
  
  return eno;
}