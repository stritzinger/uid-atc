/**
 * @file
 *
 * @brief Multiio Context
 *
 * Context handling for multi IO boards
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
#include <stdint.h>
#include <errno.h>
#include <rtems/irq.h>
#include "multiio.h"
#include "trf7970A_impl.h"

#define MULTIIO_IRQ BSP_IPIC_IRQ_IRQ1

typedef struct {
  const    multiio_bus_driver *bus_driver;
  uint8_t *read_buf[MULTIIO_ADDR_COUNT];
} multiio_context;

#define MULTIIO_CONTEXT_INITIALIZER( \
  bus_driver_addr                    \
)                                    \
{                                    \
  bus_driver_addr,                   \
  { NULL, NULL, NULL, NULL }         \
}

static multiio_context multiio_the_ctxt = MULTIIO_CONTEXT_INITIALIZER(
  NULL
);

int multiio_init(
  const multiio_bus_driver *bus_driver
)
{
  int eno = 0;
  
  assert( bus_driver                       != NULL );
  assert( bus_driver->init                 != NULL );
  assert( bus_driver->data_exchange        != NULL );
  assert( bus_driver->register_irq_handler != NULL );

  multiio_the_ctxt.bus_driver           = bus_driver;

  if( bus_driver != NULL ) {
    if( bus_driver->init != NULL ) {
      eno = (bus_driver->init)();
    } else {
      eno = EINVAL;
    }
  } else {
    eno = EINVAL;
  }

  return eno;
}

const multiio_bus_driver* multiio_get_bus_driver( void )
{
  assert( multiio_the_ctxt.bus_driver != NULL );

  return multiio_the_ctxt.bus_driver;
}


int multiio_reply_ready(
  void* arg
)
{
  rtems_status_code sc;
  multiio_reply *reply_data = (multiio_reply*) arg;
  
  /* We may need to change context */
  multiio_the_ctxt.read_buf[reply_data->addr] = reply_data->read_buf;
  
  sc = rtems_event_send(
    reply_data->id_task,
    reply_data->id_event
  );
  
  return rtems_status_code_to_errno( sc );
}

int multiio_reply_wait_for(
  const rtems_event_set id_event,
  const multiio_addr    addr,
  const rtems_interval  timeout,
  uint8_t**             read_buf
)
{
  int eno;
  rtems_event_set   events_received = 0;
  rtems_status_code sc = rtems_event_receive (
    id_event,
    RTEMS_WAIT | RTEMS_EVENT_ANY,
    timeout,
    &events_received
  );
  eno = rtems_status_code_to_errno( sc );
  
  if( eno == 0) {
    *read_buf                       = multiio_the_ctxt.read_buf[addr];
    multiio_the_ctxt.read_buf[addr] = NULL;
  }
  
  return eno;
}