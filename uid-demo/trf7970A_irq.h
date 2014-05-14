/**
 * @file
 *
 * @ingroup uid_demo
 *
 * @brief trf7970A Interrupt handling.
 *
 * Methods for handling interrupts.
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

#ifndef TRF7970A_IRQ_H
#define TRF7970A_IRQ_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int trf7970A_irq_print_unexpected(
  const uint8_t expected_irq_mask,
  const uint8_t irq_status
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TRF7970A_IRQ_H */