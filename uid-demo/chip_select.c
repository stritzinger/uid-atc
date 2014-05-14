/**
 * @file
 *
 * @brief Chip Select Handling
 *
 *Implements the SPI variant of a mutiio bus driver
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
#include "chip_select.h"

#define SPI_CSEL_MASK_GPIO (1 << (31 - 22))
#define SPI_CSEL_MASK_GPR_1_GPIO_16_22_SEL (1 << (31 - 25))
#define SPI_CSEL_MASK_SICR_2_HDLC2_A (3 << (31 - 9))
#define SPI_CSEL_FUNC_SICR_2_HDLC2_A (1 << (31 - 9))
#define SPI_CSEL_FUNC_GPR_1_GPIO_16_22_SEL_HDLC2 SPI_CSEL_MASK_GPR_1_GPIO_16_22_SEL

void chip_select_init( void )
{
  /* GPIO 22 */
  /* Select GPIO 16 - 22 multiplexing to be HDLC2_A */
  mpc83xx.syscon.gpr_1 &= (uint32_t)~SPI_CSEL_MASK_GPR_1_GPIO_16_22_SEL;
  mpc83xx.syscon.gpr_1 |= SPI_CSEL_FUNC_GPR_1_GPIO_16_22_SEL_HDLC2;
  /* Select gpio functionality for GPIO 16 - 22 */
  mpc83xx.syscon.sicrh &= (uint32_t)~SPI_CSEL_MASK_SICR_2_HDLC2_A;
  mpc83xx.syscon.sicrh |= SPI_CSEL_FUNC_SICR_2_HDLC2_A;
  /* Disable intrrupting */
  mpc83xx.gpio[0].gpimr &= (uint32_t)~SPI_CSEL_MASK_GPIO;
  /* Clear interrupt (if any) */
  mpc83xx.gpio[0].gpier = SPI_CSEL_MASK_GPIO;
  /* The output is active low, thus apply high level for now */
  mpc83xx.gpio[0].gpdat |= SPI_CSEL_MASK_GPIO;
  /* Configure as output */
  mpc83xx.gpio[0].gpdir |= SPI_CSEL_MASK_GPIO;
  /* Confige as non-open drain output */
  mpc83xx.gpio[0].gpdr  &= (uint32_t)~SPI_CSEL_MASK_GPIO;
}

void chip_select( void )
{
  mpc83xx.gpio[0].gpdat &= (uint32_t)~SPI_CSEL_MASK_GPIO;
  /* The lead time for the trf7970A RFID chip is specified to 200ns.
   * The measured lead time is about 1,5 microseconds. Because this
   * driver is intended for the trf7970A only, we don't need to do any
   * waiting at all. */
}

void chip_deselect( void )
{
  mpc83xx.gpio[0].gpdat |= SPI_CSEL_MASK_GPIO;
  /* The lag time for the trf7970A RFID chip is specified to 300ns.
   * The actual lag time is much longer. Because this
   * driver is intended for the trf7970A only, we don't need to do any
   * waiting at all. */
}