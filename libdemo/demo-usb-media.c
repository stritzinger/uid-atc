/**
 * @file
 *
 * @ingroup demo
 *
 * @brief USB media implementation.
 */

/*
 * Copyright (c) 2009-2011 embedded brains GmbH.  All rights reserved.
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

#if defined(LIBBSP_ARM_LPC24XX_BSP_H) || defined(LIBBSP_ARM_LPC32XX_BSP_H)

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <rtems.h>
#include <rtems/media.h>
#include <rtems/diskdevs.h>
#include <rtems/freebsd/bsd.h>

#include "demo.h"

#define ASSERT_SC(sc) assert((sc) == RTEMS_SUCCESSFUL)

typedef struct {
	rtems_chain_node link;
	char *path;
} media_mount_path;

static rtems_id media_semaphore = RTEMS_ID_NONE;

static RTEMS_CHAIN_DEFINE_EMPTY(media_list);

static rtems_status_code media_listener(rtems_media_event event, rtems_media_state state, const char *src, const char *dest, void *arg)
{
  rtems_status_code sc = RTEMS_SUCCESSFUL;

  printf("media listener: event = %s, state = %s, src = %s", rtems_media_event_description(event), rtems_media_state_description(state), src);

  if (dest != NULL) {
    printf(", dest = %s", dest);
  }

  if (arg != NULL) {
    printf(", arg = %p\n", arg);
  }

  printf("\n");

  if (state == RTEMS_MEDIA_STATE_SUCCESS) {
    if (event == RTEMS_MEDIA_EVENT_MOUNT) {
      char *buf = malloc(sizeof(media_mount_path) + strlen(dest) + 1);

      if (buf != NULL) {
        media_mount_path *mp = (media_mount_path *) buf;

        mp->path = buf + sizeof(media_mount_path);
        strcpy(mp->path, dest);

        rtems_chain_append(&media_list, &mp->link);

        sc = rtems_semaphore_release(media_semaphore);
        ASSERT_SC(sc);
      }
    }
  }

  return sc;
}

void demo_usb_media_initialize(void)
{
  rtems_status_code sc = RTEMS_SUCCESSFUL;

  sc = rtems_semaphore_create(
    rtems_build_name('M', 'D', 'I', 'A'),
    0,
    RTEMS_LOCAL | RTEMS_FIFO | RTEMS_COUNTING_SEMAPHORE,
    0,
    &media_semaphore
  );
  ASSERT_SC(sc);

  sc = rtems_disk_io_initialize();
  ASSERT_SC(sc);

  sc = rtems_media_initialize();
  ASSERT_SC(sc);

  sc = rtems_media_listener_add(media_listener, NULL);
  ASSERT_SC(sc);

  sc = rtems_media_server_initialize(200, 32 * 1024, RTEMS_DEFAULT_MODES, RTEMS_DEFAULT_ATTRIBUTES);
  ASSERT_SC(sc);

  sc = rtems_bsd_initialize_with_interrupt_server();
  ASSERT_SC(sc);
}

rtems_status_code demo_usb_media_wait(rtems_interval timeout, char **path)
{
  rtems_status_code sc = RTEMS_SUCCESSFUL;
  media_mount_path *mp = NULL;

  sc = rtems_semaphore_obtain(
    media_semaphore,
    RTEMS_WAIT,
    timeout
  );
  if (sc != RTEMS_SUCCESSFUL) {
    return RTEMS_UNSATISFIED;
  }

  mp = (media_mount_path *) rtems_chain_get(&media_list);
  *path = strdup(mp->path);
  free(mp);
  if (*path == NULL) {
    return RTEMS_NO_MEMORY;
  }

  return RTEMS_SUCCESSFUL;
}

#define USB_SYSINIT_INIT

#include "usb-sysinit.h"

#endif
