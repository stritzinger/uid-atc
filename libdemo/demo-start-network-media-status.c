/**
 * @file
 *
 * @ingroup demo
 *
 * @brief Network media status start command.
 *
 * This command starts a task which queries each second the media status of the
 * default interface of the BSP.  The PHY address 0 is used.  The media status
 * will be output on the console.
 */

/*
 * Copyright (c) 2012 embedded brains GmbH.  All rights reserved.
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

#include "demo.h"

#include <rtems.h>

#ifdef RTEMS_NETWORKING

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <netdb.h>
#include <net/if.h>
#include <sys/sockio.h>

#include <rtems/rtems_bsdnet.h>
#include <rtems/rtems_mii_ioctl.h>

#include <bsp.h>

#define ASSERT_SC(sc) assert((sc) == RTEMS_SUCCESSFUL)

static rtems_task_priority demo_network_media_status_priority;

static void demo_network_media_status_task(rtems_task_argument arg)
{
  rtems_status_code sc;
  rtems_interval one_second = rtems_clock_get_ticks_per_second();

  while (true) {
    int rv;
    int media = 0;

    rv = rtems_bsdnet_ifconfig(RTEMS_BSP_NETWORK_DRIVER_NAME, SIOCGIFMEDIA, &media);
    if (rv == 0) {
      rtems_ifmedia2str(media, NULL, 0);
      printf("\n");
    } else {
      printf("cannot get media status\n");
    }

    sc = rtems_task_wake_after(one_second);
    ASSERT_SC(sc);
  }

  sc = rtems_task_delete(RTEMS_SELF);
  ASSERT_SC(sc);
}

rtems_status_code demo_initialize_network_media_status(rtems_task_priority priority)
{
  rtems_status_code sc;
  rtems_id id;

  sc = rtems_task_create(
    rtems_build_name('N', 'M', 'D', 'A'),
    priority,
    RTEMS_MINIMUM_STACK_SIZE,
    RTEMS_DEFAULT_MODES,
    RTEMS_DEFAULT_ATTRIBUTES,
    &id
  );
  ASSERT_SC(sc);

  sc = rtems_task_start(id, demo_network_media_status_task, 0);
  ASSERT_SC(sc);

  return sc;
}

static rtems_status_code demo_start_network_media_status_command(void)
{
  return demo_initialize_network_media_status(
    demo_network_media_status_priority
  );
}

static demo_start_entry demo_start_network_media_status_entry = {
  .next = NULL,
  .name = "network_media_status",
  .start = demo_start_network_media_status_command,
  .status = DEMO_STATUS_AGAIN,
  .dependencies = { "network", NULL }
};

void demo_start_network_media_status(rtems_task_priority priority)
{
  demo_network_media_status_priority = priority;
  demo_start_add(&demo_start_network_media_status_entry);
}

#endif /* RTEMS_NETWORKING */
