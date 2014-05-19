/**
 * @file
 *
 * @ingroup uid_demo
 *
 * @brief trf7970A RFID controller.
 *
 * Command and initialization methods for the trf7970A RFID controller.
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

#ifndef TRF7970A_H
#define TRF7970A_H

#include <rtems/shell.h>
#include "multiio.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int trf7970A_init(
  const multiio_bus_driver *bus_driver,
  const rtems_event_set     EVENT_ID_REPLY_READY,
  const rtems_event_set     EVENT_ID_DEVICE_IRQ
);

extern rtems_shell_cmd_t trf7970A_cmd_raw;
extern rtems_shell_cmd_t trf7970A_cmd_spi_raw;
extern rtems_shell_cmd_t trf7970A_cmd_init;
extern rtems_shell_cmd_t trf7970A_cmd_modulation_13;
extern rtems_shell_cmd_t trf7970A_cmd_modulation_30;
extern rtems_shell_cmd_t trf7970A_cmd_modulation_100;
extern rtems_shell_cmd_t trf7970A_cmd_readall;
extern rtems_shell_cmd_t trf7970A_cmd_reg_read;
extern rtems_shell_cmd_t trf7970A_cmd_reg_write;
extern rtems_shell_cmd_t trf7970A_cmd_agc_off;
extern rtems_shell_cmd_t trf7970A_cmd_agc_on;
extern rtems_shell_cmd_t trf7970A_cmd_am;
extern rtems_shell_cmd_t trf7970A_cmd_pm;
extern rtems_shell_cmd_t trf7970A_cmd_tag_detect;
extern rtems_shell_cmd_t trf7970A_cmd_tag;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TRF7970A_H */
