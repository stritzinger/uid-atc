/**
 * @file
 *
 * @ingroup demo
 *
 * @brief Network performance start command.
 */

/*
 * Copyright (c) 2011 embedded brains GmbH.  All rights reserved.
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
#include <netdb.h>

#include <rtems/rtems_bsdnet.h>

#define ASSERT_SC(sc) assert((sc) == RTEMS_SUCCESSFUL)

static int create_socket(void)
{
  int rv = 0;
  struct sockaddr_in port = {
    .sin_family = AF_INET,
    .sin_port = htons(DEMO_PORT),
    .sin_addr = {
      .s_addr = INADDR_ANY
    }
  };
  int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  assert(fd >= 0);

  rv = bind(fd, (struct sockaddr *) &port, sizeof(port));
  assert(rv == 0);

  return fd;
}

static void receive_task(rtems_task_argument arg)
{
  rtems_status_code sc = RTEMS_SUCCESSFUL;
  int fd = create_socket();

  while (true) {
    char buf [512];
    struct sockaddr_in src;
    socklen_t len = sizeof(src);
    ssize_t n = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *) &src, &len);
    assert(n > 0);
  }

  sc = rtems_task_delete(RTEMS_SELF);
  ASSERT_SC(sc);
}

rtems_status_code demo_initialize_netperf(void)
{
  rtems_status_code sc = RTEMS_SUCCESSFUL;
  rtems_id id = RTEMS_ID_NONE;

  sc = rtems_task_create(
    rtems_build_name('N', 'P', 'R', 'F'),
    rtems_bsdnet_config.network_task_priority - 1,
    RTEMS_MINIMUM_STACK_SIZE,
    RTEMS_DEFAULT_MODES,
    RTEMS_DEFAULT_ATTRIBUTES,
    &id
  );
  ASSERT_SC(sc);

  sc = rtems_task_start(id, receive_task, 0);
  ASSERT_SC(sc);

  return RTEMS_SUCCESSFUL;
}

static rtems_status_code demo_start_netperf_command(void)
{
  return demo_initialize_netperf();
}

static demo_start_entry demo_start_netperf_entry = {
  .next = NULL,
  .name = "netperf",
  .start = demo_start_netperf_command,
  .status = DEMO_STATUS_AGAIN,
  .dependencies = { "network", NULL }
};

void demo_start_netperf(void)
{
  demo_start_add(&demo_start_netperf_entry);
}

#endif /* RTEMS_NETWORKING */
