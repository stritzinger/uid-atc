/**
 * @file
 *
 * @ingroup uid_demo
 *
 * @brief NCV7608 driver.
 *
 * Driver for for the NCV7608 digital output controller on the UID board.
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

#include <rtems/io.h>

#ifndef NCV7608_H
#define NCV7608_H

#define NCV7608_OBJECT_COUNT_DRIVERS 1
#define NCV7608_OBJECT_COUNT_FILE_DESCRIPTORS 1

#define NCV7608_DEV_NAME_BMP    "/dev/dig_op_bmp"

typedef struct {
  rtems_event_set id_event_reply_ready;
} ncv7608_init_args;

extern const rtems_driver_address_table ncv7608_driver_table;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NCV7608_H */