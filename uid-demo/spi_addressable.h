/**
 * @file
 *
 * @ingroup uid_demo
 *
 * @brief Addressable SPI Driver
 *
 * API for the addressable SPI driver
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

#ifndef SPI_ADDRESSABLE_H
#define SPI_ADDRESSABLE_H

#include "multiio.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SPI_OBJECT_COUNT_SEMAPHORE 1

extern multiio_bus_driver spi_addressable_bus_driver;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SPI_ADDRESSABLE_H */
