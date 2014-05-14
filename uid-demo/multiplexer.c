/**
 * @file
 *
 * @brief Multiplexer Handling
 *
 * Implements the handling for the multiplexer which selects between SPI
 * and UCC. By default UCC will be selected.
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

#include <mpc83xx/mpc83xx.h>
#include "multiplexer.h"

#define MUX_CTRL_MASK_GPIO (1 << (31 - 3))

void multiplexer_init( void )
{
  /* GPIO 3 */
  /* Disable intrrupting */
  mpc83xx.gpio[0].gpimr &= (uint32_t)~MUX_CTRL_MASK_GPIO;
  /* Clear interrupt (if any) */
  mpc83xx.gpio[0].gpier = MUX_CTRL_MASK_GPIO;
  /* Configure as output */
  mpc83xx.gpio[0].gpdir |= MUX_CTRL_MASK_GPIO;
  /* Confige as non-open drain output */
  mpc83xx.gpio[0].gpdr  &= (uint32_t)~MUX_CTRL_MASK_GPIO;
}

void multiplexer_select_ucc( void )
{
  mpc83xx.gpio[0].gpdat |= MUX_CTRL_MASK_GPIO;
}

void multiplexer_select_spi( void )
{
  mpc83xx.gpio[0].gpdat &= (uint32_t)~MUX_CTRL_MASK_GPIO;
}