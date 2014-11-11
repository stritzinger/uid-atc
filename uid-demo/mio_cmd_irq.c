/**
 * @file
 *
 * @brief MIO_irq command.
 *
 * Command for reading the IRQ pin.
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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <inttypes.h>
#include <rtems/shell.h>
#include "mio_impl.h"
#include "mio.h"
#include "multiio.h"
#include <assert.h>

#include <bsp.h>
#include <bsp/irq.h>
#include <mpc83xx/mpc83xx.h>

#define IPIC_SECNR_EDI1 0x00004000
#define IPIC_SEPNR_IRQ1 0x40000000
#define IPIC_SEMSR_IRQ1 0x40000000

#define NO_IRQ_MESSAGES 1
#define MSG_SIZE sizeof(int)

static rtems_id mq_int_received = RTEMS_ID_NONE;

static void mio_irq_handler(void *arg)
{
  mpc83xx.ipic.sepnr = IPIC_SEPNR_IRQ1;
  if ((mpc83xx.ipic.secnr & IPIC_SECNR_EDI1) == 0) {
    printk("WARNING: IRQ was level triggered. Switch to edge triggered.\n");
    mpc83xx.ipic.secnr |= IPIC_SECNR_EDI1;
  }

  rtems_message_queue_send(mq_int_received, (void *)1, MSG_SIZE);
}

int mio_irq_init( void )
{
  int eno = 0;
  rtems_status_code sc = RTEMS_SUCCESSFUL;

  sc = rtems_message_queue_create(
    rtems_build_name('M', 'I', 'O', 'I'),
    NO_IRQ_MESSAGES,
    MSG_SIZE,
    RTEMS_DEFAULT_ATTRIBUTES,
    &mq_int_received
  );
  assert(sc == RTEMS_SUCCESSFUL);

  sc = rtems_interrupt_handler_install(
    BSP_IPIC_IRQ_IRQ1,
    "MIO IRQ",
    RTEMS_INTERRUPT_UNIQUE,
    mio_irq_handler,
    NULL
  );
  assert(sc == RTEMS_SUCCESSFUL);

  /* Set to edge sensitive, clear pending and unmask irq */
  mpc83xx.ipic.secnr |= IPIC_SECNR_EDI1;
  mpc83xx.ipic.sepnr = IPIC_SEPNR_IRQ1;
  mpc83xx.ipic.semsr |= IPIC_SEMSR_IRQ1;

  return eno;
}

static int mio_cmd_irq_func( int argc, char **argv )
{
  int eno = 0;
  bool status = false;
  rtems_status_code sc = RTEMS_SUCCESSFUL;
  int msg = 0;
  size_t size = 0;

  sc = rtems_message_queue_receive(
    mq_int_received,
    &msg,
    &size,
    RTEMS_NO_WAIT,
    0
  );

  if (sc != RTEMS_SUCCESSFUL) {
    status = false;
  } else {
    status = true;
  }

  /* Print the results */
  printf( "Interrupt since last call: %s\n", status ? "true" : "false" );

  return eno;
}

rtems_shell_cmd_t mio_cmd_irq = {
  "MIO_irq",
  "MIO_irq\n"
  "Print out if there has been an Interrupt (falling\n"
  "Edge on IRQ Pin) since the last call.\n",
  "MIO",
  mio_cmd_irq_func,
  NULL,
  NULL
};
