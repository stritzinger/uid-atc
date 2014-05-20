/**
 * @file
 *
 * @ingroup uid_demo
 *
 * @brief MultiIO Bus Driver
 *
 * Definitions for abstract bus drivers for the MultiIO board
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

#ifndef MULTIIO_H
#define MULTIIO_H

#include <stdint.h>
#include <stddef.h>
#include <bsp.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define MULTIIO_READ_BUF_SIZE  32
#define MULTIIO_WRITE_BUF_SIZE 32

#define MULTIIO_ADDR_DIG_IP 0x01       /* the input control of the CPLD */
#define MULTIIO_ADDR_IRQ_CTRL 0x03     /* the interrupt control of the CPLD */
#define MULTIIO_ADDR_RFID_READER 0x04  /* the trf7990A RFID reader */
#define MULTIIO_ADDR_DIG_OP 0x05       /* the nvc7608 digital output control */

#define MULTIIO_ADDR_INVALID 0xFF      /* invalid address */

#define MULTIIO_ADDR_COUNT 4        /* Numuber of addressable SPI devices
                                       on the multiio board */
typedef uint8_t multiio_addr;

typedef void (*multiio_irq_handler)(void *arg);
typedef int (*multiio_on_reply_ready_handler)(void* arg);

typedef union {
  uint8_t raw[  sizeof(uint8_t)
              + sizeof(size_t)
              + sizeof(void*)
              + sizeof(multiio_on_reply_ready_handler)
              + sizeof(multiio_addr)
              + MULTIIO_WRITE_BUF_SIZE];
  struct {
    uint8_t                       *read_buf;
    size_t                         num_chars;
    void                          *arg;
    multiio_on_reply_ready_handler on_reply_ready;
    multiio_addr                   address;
    const uint8_t                 *write_buf;
  };
} multiio_exchange_data;

typedef struct {
  int (*init) (void);
  int (*data_exchange)(
    multiio_exchange_data *data
  );
  void (*register_irq_handler) (
    const multiio_addr        addr,
    const multiio_irq_handler handler,
    void                     *handler_arg
  );
} multiio_bus_driver;

#define MULTIIO_BUS_DRIVER_INITIALIZER( \
  init,                                 \
  data_exchange,                        \
  register_irq_handler                  \
)                                       \
{                                       \
  init,                                 \
  data_exchange,                        \
  register_irq_handler                  \
}

typedef struct {
  uint8_t        *read_buf;
  rtems_id        id_task;
  rtems_event_set id_event;
  multiio_addr    addr;
} multiio_reply;
#define MULTIIO_REPLY_INITIALIZER( \
  read_buf_addr,                   \
  id_task,                         \
  id_event,                        \
  addr                             \
)                                  \
{                                  \
  read_buf_addr,                   \
  id_task,                         \
  id_event,                        \
  addr                             \
}

int multiio_init(
  const multiio_bus_driver *bus_driver
);

int multiio_reply_ready(
  void* arg
);

int multiio_reply_wait_for(
  const rtems_event_set id_event,
  const multiio_addr    addr,
  const rtems_interval  timeout,
  uint8_t**             read_buf
);

const multiio_bus_driver* multiio_get_bus_driver( void );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MULTIIO_H */
