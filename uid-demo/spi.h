/**
 * @file
 *
 * @ingroup uid_demo
 *
 * @brief SPI Driver
 *
 * API for the SPI driver
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

#ifndef SPI_H
#define SPI_H

#include <mpc83xx/mpc83xx_spidrv.h>
#include "multiio.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SPI_OBJECT_COUNT_SEMAPHORE 1

typedef struct {
  multiio_irq_handler handler;
  void               *arg;
} spi_irq_data;

typedef struct {
  mpc83xx_spi_desc_t spi_desc;
  spi_irq_data       irq_data;
} spi_context;

extern spi_context spi_the_ctxt;

int spi_init( void );
int spi_data_exchange(
  multiio_exchange_data *data
);
void spi_register_irq_handler (
  const multiio_addr        addr,
  const multiio_irq_handler handler,
  void                     *handler_arg
);

extern multiio_bus_driver spi_bus_driver;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SPI_H */