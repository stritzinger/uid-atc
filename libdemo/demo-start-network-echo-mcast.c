/**
 * @file
 *
 * @ingroup demo
 *
 * @brief Network multicast listen start command.
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

#define __BSD_VISIBLE 1

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

#define ASSERT_SC(sc) assert((sc) == RTEMS_SUCCESSFUL)

static rtems_task_priority demo_network_echo_mcast_priority;

static const char *demo_network_echo_mcast_ip_addr;

static uint16_t demo_network_echo_mcast_port;

static int demo_network_echo_mcast_create_socket(void)
{
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  assert(fd >= 0);

  return fd;
}

static void demo_network_echo_mcast_bind_socket(int fd, uint16_t port)
{
  struct sockaddr_in addr;
  int rv = 0;

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(port);

  rv = bind(fd, (const struct sockaddr *) &addr, sizeof(addr));
  assert(rv == 0);
}

static void demo_network_echo_mcast_join_multicast_group(int fd, const char *ip)
{
  struct ip_mreq mreq;
  int rv = 0;

  memset(&mreq, 0, sizeof(mreq));
  mreq.imr_interface.s_addr = htonl(INADDR_ANY);
  rv = inet_aton(ip, &mreq.imr_multiaddr);
  assert(rv != 0);

  rv = setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
  assert(rv == 0);
}

static void demo_network_echo_mcast_task(rtems_task_argument arg)
{
  rtems_status_code sc = RTEMS_SUCCESSFUL;
  int fd = demo_network_echo_mcast_create_socket();
  uint16_t port = htons(demo_network_echo_mcast_port);

  demo_network_echo_mcast_bind_socket(fd, demo_network_echo_mcast_port);
  demo_network_echo_mcast_join_multicast_group(fd, demo_network_echo_mcast_ip_addr);

  while (true) {
    char buf [ETHERMTU];
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    ssize_t n = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *) &addr, &len);

    assert(len == sizeof(addr));

    if (n > 0) {
      addr.sin_port = port;
      sendto(fd, buf, (size_t) n, 0, (const struct sockaddr *) &addr, len);
    }
  }

  sc = rtems_task_delete(RTEMS_SELF);
  ASSERT_SC(sc);
}

rtems_status_code demo_initialize_network_echo_mcast(
  rtems_task_priority priority,
  const char *ip_addr,
  uint16_t port
)
{
  rtems_status_code sc = RTEMS_SUCCESSFUL;
  rtems_id id = RTEMS_ID_NONE;

  /* FIXME */
  demo_network_echo_mcast_priority = priority;
  demo_network_echo_mcast_ip_addr = ip_addr;
  demo_network_echo_mcast_port = port;

  sc = rtems_task_create(
    rtems_build_name('M', 'C', 'E', 'O'),
    priority,
    RTEMS_MINIMUM_STACK_SIZE,
    RTEMS_DEFAULT_MODES,
    RTEMS_DEFAULT_ATTRIBUTES,
    &id
  );
  ASSERT_SC(sc);

  sc = rtems_task_start(id, demo_network_echo_mcast_task, 0);
  ASSERT_SC(sc);

  return RTEMS_SUCCESSFUL;
}

static rtems_status_code demo_start_network_echo_mcast_command(void)
{
  return demo_initialize_network_echo_mcast(
    demo_network_echo_mcast_priority,
    demo_network_echo_mcast_ip_addr,
    demo_network_echo_mcast_port
  );
}

static demo_start_entry demo_start_network_echo_mcast_entry = {
  .next = NULL,
  .name = "network_echo_mcast",
  .start = demo_start_network_echo_mcast_command,
  .status = DEMO_STATUS_AGAIN,
  .dependencies = { "network", NULL }
};

void demo_start_network_echo_mcast(
  rtems_task_priority priority,
  const char *ip_addr,
  uint16_t port
)
{
  demo_network_echo_mcast_priority = priority;
  demo_network_echo_mcast_ip_addr = ip_addr;
  demo_network_echo_mcast_port = port;
  demo_start_add(&demo_start_network_echo_mcast_entry);
}

#endif /* RTEMS_NETWORKING */
