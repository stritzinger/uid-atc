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

#ifndef DS2408_H
#define DS2408_H

#include <rtems/libi2c.h>


/* Function Commands */
#define DS2482_CMD_DEVICE_RESET           0xF0
#define DS2482_CMD_WRITE_CONFIG           0xD2
#define DS2482_CMD_SET_READ_POINTER       0xE1
#define DS2482_CMD_1WIRE_RESET            0xB4
#define DS2482_CMD_1WIRE_WRITE_BYTE       0xA5
#define DS2482_CMD_1WIRE_READ_BYTE        0x96

/* Pointer Addresses */
#define DS2482_PIONTER_STATUS_REGISTER    0xF0
#define DS2482_PIONTER_READ_DATA_REGISTER 0xE1

/* DS2408 Addresses */
#define DS2408_TGD_ADDR_8D                0x8D
#define DS2408_TGD_ADDR_00                0x00
#define DS2408_TGD_ADDR_8B                0x8B

/*DS2408 Commands */
#define DS2408_SKIP_ROM                   0xCC
#define DS2408_ISSUE_WRITE_CMD_SEARCH_REG 0xCC
#define DS2408_ISSUE_READ_POI_REG_CMD     0xF0
#define DS2408_READ_ROM_FAMILY_CODE       0x33
#define DS2408_ISSUE_CHAN_WRITE           0x5A
#define DS2408_ISSUE_CHAN_READ            0xF5
#define DS2408_RESET_ACTIVE_LATCH         0xC3


/* Change RFID LED */
#define DS2408_LED_ON                     0x7F
#define DS2408_LED_OFF                    0xFF

/* Masks */
#define DS2482_1WB_MASK                   0x01
#define DS2408_WRITE_CON_STAT_MASK        0x01
#define DS2408_READ_CON_STAT_MASK         0X04
#define DS2408_VERSION_MASK               0x7F
#define DS2408_READBACK_MASK              0x84

/* 1-Wire Configuration Reg Value */
#define DS2482_1WIRE_CONFIG_REG_VALUE     0xF0

extern rtems_libi2c_drv_t *i2c_ds2408_driver_descriptor;

#endif /*DS2408_H */
