/**
 * @file
 *
 * @ingroup uid_demo
 *
 * @brief trf7970A RFID controller internal.
 *
 * Internal methods, types and data for the trf7970A RFID controller.
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

#ifndef TRF7970A_IMPL_H
#define TRF7970A_IMPL_H

#include <stdint.h>
#include <rtems.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


typedef struct {
  rtems_event_set event_id_device_irq;
  rtems_event_set event_id_reply_ready;
} trf7970A_impl_context;

#define TRF7970A_IMPL_CONTEXT_INITIALIZER( \
)                                          \
{                                          \
  0,                                       \
  0                                        \
}

extern trf7970A_impl_context trf7970A_impl_the_context;

int trf7979A_cmd_raw_string( const char* cmd_string, uint8_t *read_buf, const size_t buf_size );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TRF7970A_IMPL_H */
