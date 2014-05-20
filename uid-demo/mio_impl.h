/**
 * @file
 *
 * @ingroup uid_demo
 *
 * @brief MultiIO command driver.
 *
 * Internal methods, types and data for the MultiIO command driver.
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

#ifndef MIO_IMPL_H
#define MIO_IMPL_H

#include <stdint.h>
#include <rtems.h>
#include "multiio.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


typedef struct {
  rtems_event_set event_id_device_irq;
  rtems_event_set event_id_reply_ready;
} mio_impl_context;

#define MIO_IMPL_CONTEXT_INITIALIZER( \
)                                          \
{                                          \
  0,                                       \
  0                                        \
}

extern mio_impl_context mio_impl_the_context;

int mio_cmd_raw_string( const char* cmd_string, uint8_t *read_buf, const size_t buf_size, multiio_addr address );

int mio_cmd_raw_hdlr(
  int                           argc,
  char                        **argv,
  bool                          verbose,
  multiio_addr                  address,
  uint8_t                      *read_buf,
  const size_t                  read_buf_size
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MIO_IMPL_H */
