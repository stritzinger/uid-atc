/**
 * @file
 *
 * @ingroup uid_demo
 *
 * @brief LED driver.
 *
 * Driver for LEDs on the UID board.
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

#ifndef LED_H
#define LED_H

#define LED_OBJECT_COUNT_DRIVERS 1
#define LED_OBJECT_COUNT_SEMAPHORES 1
#define LED_OBJECT_COUNT_FILE_DESCRIPTORS 8

#define LED_DEV_NAME_PWR_RED    "/dev/led_pwr_red"
#define LED_DEV_NAME_PWR_GREEN  "/dev/led_pwr_green"
#define LED_DEV_NAME_ETH_RED    "/dev/led_eth_red"
#define LED_DEV_NAME_ETH_GREEN  "/dev/led_eth_green"
#define LED_DEV_NAME_DP_RED     "/dev/led_dp_red"
#define LED_DEV_NAME_DP_GREEN   "/dev/led_dp_green"
#define LED_DEV_NAME_RF_RED     "/dev/led_rf_red"
#define LED_DEV_NAME_RF_GREEN   "/dev/led_rf_green"


extern const rtems_driver_address_table led_driver_table;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LED_H */