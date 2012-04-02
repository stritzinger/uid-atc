/**
 * @file
 *
 * @brief Breaks USB connection from time to time.
 */

/*
 * Copyright (c) 2010-2011 embedded brains GmbH.  All rights reserved.
 *
 *  embedded brains GmbH
 *  Obere Lagerstr. 30
 *  82178 Puchheim
 *  Germany
 *  <info@embedded-brains.de>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <bsp.h>

#ifdef LIBBSP_ARM_LPC24XX_BSP_H

#include <assert.h>

#include <rtems.h>

#include <bsp/irq.h>
#include <bsp/io.h>
#include <bsp/lpc24xx.h>
#include <bsp/lpc-timer.h>

#include "usb.h"

#define ASSERT_SC(sc) assert((sc) == RTEMS_SUCCESSFUL)

static uint32_t timer_ticks_per_unit;

static unsigned delay;

static unsigned delay_min;

static unsigned delay_max;

static unsigned delay_delta;

static void usb_break(void)
{
  static const lpc24xx_pin_range pins [] = {
    LPC24XX_PIN_USB_D_PLUS_1,
    LPC24XX_PIN_USB_D_MINUS_1,
    LPC24XX_PIN_USB_PPWR_1,
    LPC24XX_PIN_TERMINAL
  };

  rtems_status_code sc = RTEMS_SUCCESSFUL;

  sc = lpc24xx_pin_config(&pins [0], LPC24XX_PIN_SET_INPUT);
  ASSERT_SC(sc);

  sc = lpc24xx_pin_config(&pins [0], LPC24XX_PIN_SET_FUNCTION);
  ASSERT_SC(sc);
}

static void timer_handler(void *arg)
{
  volatile lpc_timer *timer = arg;

  timer->ir = LPC_TIMER_IR_MR0;

  usb_break();

  if (delay < delay_max) {
    delay += delay_delta;
  } else {
    delay = delay_min;
  }

  timer->mr0 = delay * timer_ticks_per_unit;
}

static void initialize_timer(volatile lpc_timer *timer)
{
  /* Reset timer */
  timer->tcr = LPC_TIMER_TCR_RST;

  /* Clear interrupt flags */
  timer->ir = LPC_TIMER_IR_ALL;

  /* Set timer mode */
  timer->ccr = 0;

  /* Timer is incremented every PERIPH_CLK tick */
  timer->pr = 0;

  /* Set match registers */
  timer->mr0 = delay * timer_ticks_per_unit;

  /* Generate interrupt and reset counter on match with MR0 */
  timer->mcr = LPC_TIMER_MCR_MR0_INTR | LPC_TIMER_MCR_MR0_RST;

  /* No external match */
  timer->emr = 0;

  /* Enable timer */
  timer->tcr = LPC_TIMER_TCR_EN;
}

void usb_break_initialize(unsigned unit_in_us, unsigned min_units, unsigned max_units, unsigned delta_units)
{
  rtems_status_code sc = RTEMS_SUCCESSFUL;

  if (unit_in_us > 0) {
    volatile lpc_timer *timer = (volatile lpc_timer *) TMR2_BASE_ADDR;

    timer_ticks_per_unit = unit_in_us * (LPC24XX_CCLK / 1000000);
    delay = (min_units + max_units) / 2;
    delay_min = min_units;
    delay_max = max_units;
    delay_delta = delta_units;

    sc = lpc24xx_module_enable(LPC24XX_MODULE_TIMER_2, LPC24XX_MODULE_CCLK);
    ASSERT_SC(sc);

    initialize_timer(timer);

    sc = rtems_interrupt_handler_install(
      LPC24XX_IRQ_TIMER_2,
      "USB Break Timer",
      RTEMS_INTERRUPT_UNIQUE,
      timer_handler,
      timer
    );
    ASSERT_SC(sc);
  }
}

#endif /* LIBBSP_ARM_LPC24XX_BSP_H */
