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

#ifdef LIBBSP_ARM_LPC32XX_BSP_H

#include <assert.h>

#include <rtems.h>

#include <bsp/lpc32xx.h>

#include "usb.h"

#define ASSERT_SC(sc) assert((sc) == RTEMS_SUCCESSFUL)

#define SUPPLY_SWITCH (1U << 1)

#define DATA_SWITCH (1U << 14)

#define MIN_DELAY_IN_US 1000000U

static rtems_interval delay;

static rtems_interval delay_min;

static rtems_interval delay_max;

static rtems_interval delay_delta;

typedef enum {
  ON_SUPPLY,
  ON_SUPPLY_AND_DATA,
  OFF_DATA,
  OFF_DATA_AND_SUPPLY
} usb_break_state;

static rtems_interval usb_break(void)
{
  static usb_break_state state = OFF_DATA;

  rtems_interval const switch_delay = rtems_clock_get_ticks_per_second() / 20;
  rtems_interval const disconnected_delay = rtems_clock_get_ticks_per_second();
  rtems_interval new_delay = 0;

  switch (state) {
    case ON_SUPPLY:
      lpc32xx.gpio.p3_outp_clr = SUPPLY_SWITCH;
      new_delay = switch_delay;
      state = ON_SUPPLY_AND_DATA;
      break;
    case ON_SUPPLY_AND_DATA:
      lpc32xx.gpio.p3_outp_clr = DATA_SWITCH;
      state = OFF_DATA;
      break;
    case OFF_DATA:
      lpc32xx.gpio.p3_outp_set = DATA_SWITCH;
      new_delay = switch_delay;
      state = OFF_DATA_AND_SUPPLY;
      break;
    case OFF_DATA_AND_SUPPLY:
      lpc32xx.gpio.p3_outp_set = SUPPLY_SWITCH;
      new_delay = disconnected_delay;
      state = ON_SUPPLY;
      break;
    default:
      assert(false);
      break;
  }

  return new_delay;
}

static void usb_break_trigger(rtems_id id, void *arg)
{
  rtems_status_code sc = RTEMS_SUCCESSFUL;
  rtems_interval new_delay = usb_break();

  if (new_delay == 0) {
    if (delay < delay_max) {
      delay += delay_delta;
    } else {
      delay = delay_min;
    }

    new_delay = delay;
  }

  sc = rtems_timer_fire_after(id, new_delay, usb_break_trigger, NULL);
  ASSERT_SC(sc);
}

void usb_break_initialize(unsigned unit_in_us, unsigned min_units, unsigned max_units, unsigned delta_units)
{
  rtems_status_code sc = RTEMS_SUCCESSFUL;
  rtems_id id = RTEMS_ID_NONE;
  rtems_interval us_per_tick = rtems_configuration_get_microseconds_per_tick();
  rtems_interval ticks_per_unit = 0;

  assert(unit_in_us > 0 && min_units > 0 && min_units <= max_units);

  if (unit_in_us < us_per_tick) {
    unit_in_us = us_per_tick;
  }

  ticks_per_unit = (unit_in_us + us_per_tick - 1) / us_per_tick;

  if (unit_in_us * min_units < MIN_DELAY_IN_US) {
    min_units = (MIN_DELAY_IN_US + unit_in_us - 1) / unit_in_us;
  }

  if (max_units < min_units) {
    max_units = min_units;
  }

  delay = ((min_units + max_units) / 2) * ticks_per_unit;
  delay_min = min_units * ticks_per_unit;
  delay_max = max_units * ticks_per_unit;
  delay_delta = delta_units * ticks_per_unit;

  lpc32xx.gpio.p3_outp_clr = SUPPLY_SWITCH;
  lpc32xx.gpio.p3_outp_clr = DATA_SWITCH;

  sc = rtems_timer_create(rtems_build_name('U', 'B', 'R', 'K'), &id);
  ASSERT_SC(sc);

  sc = rtems_timer_fire_after(id, delay, usb_break_trigger, NULL);
  ASSERT_SC(sc);
}

#endif /* LIBBSP_ARM_LPC32XX_BSP_H */
