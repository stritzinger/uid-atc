/**
 * @file
 *
 * @ingroup uid_demo
 *
 * @brief MultiIO command driver.
 *
 * Command and initialization methods for the MultiIO command driver.
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

#ifndef MIO_H
#define MIO_H

#include <rtems/shell.h>
#include "multiio.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int mio_init(
  const multiio_bus_driver *bus_driver,
  const rtems_event_set     EVENT_ID_REPLY_READY,
  const rtems_event_set     EVENT_ID_DEVICE_IRQ
);

extern rtems_shell_cmd_t mio_cmd_raw;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MIO_H */
