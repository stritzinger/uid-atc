/**
 * @file
 *
 * @brief Initialization handling.
 *
 * Initialization handling for MultiIO command driver.
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

#include "mio.h"
#include "mio_impl.h"

mio_impl_context mio_impl_the_context;

int mio_init(
  const multiio_bus_driver *bus_driver,
  const rtems_event_set     EVENT_ID_REPLY_READY
)
{
  int eno;

  mio_impl_the_context.event_id_reply_ready = EVENT_ID_REPLY_READY;
  
  eno = multiio_init(
    bus_driver
  );

  return eno;
}
