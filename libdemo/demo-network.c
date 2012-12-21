/**
 * @file
 *
 * @ingroup demo
 *
 * @brief Network configuration and initialization..
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

#include <rtems.h>

#ifdef RTEMS_NETWORKING

#include <rtems/rtems_bsdnet.h>

#include <bsp.h>

#include "demo.h"

#include <rtems/status-checks.h>

static struct rtems_bsdnet_ifconfig demo_ifconfig = {
  .name = RTEMS_BSP_NETWORK_DRIVER_NAME,
  .attach = RTEMS_BSP_NETWORK_DRIVER_ATTACH,
  .next = NULL,
  .ip_address = NULL,
  .ip_netmask = NULL,
  .hardware_address = NULL,
  .ignore_broadcast = 0,
  .mtu = 0,
  .rbuf_count = 0,
  .xbuf_count = 0,
  .port = 0,
  .irno = 0,
  .bpar = 0
};

struct rtems_bsdnet_config rtems_bsdnet_config = {
  .ifconfig = &demo_ifconfig,
  .bootp = NULL,
  .network_task_priority = 80,
  .mbuf_bytecount = 0,
  .mbuf_cluster_bytecount = 0,
  .hostname = NULL,
  .domainname = NULL,
  .gateway = NULL,
  .log_host = NULL,
  .name_server = { NULL, NULL, NULL },
  .ntp_server = { NULL, NULL, NULL },
  .sb_efficiency = 0,
  .udp_tx_buf_size = 0,
  .udp_rx_buf_size = 0,
  .tcp_tx_buf_size = 0,
  .tcp_rx_buf_size = 0
};

rtems_status_code demo_initialize_network(
  rtems_task_priority priority,
  const char mac_address [6],
  const char *ip_self,
  const char *ip_gateway,
  const char *ip_netmask
)
{
  int rv = 0;

  rtems_bsdnet_config.network_task_priority = priority;
  rtems_bsdnet_config.gateway = ip_gateway;
  rtems_bsdnet_config.log_host = ip_gateway;
  rtems_bsdnet_config.name_server [0] = ip_gateway;
  rtems_bsdnet_config.ntp_server [0] = ip_gateway;
  rtems_bsdnet_config.ifconfig->hardware_address = mac_address;
  rtems_bsdnet_config.ifconfig->ip_address = ip_self;
  rtems_bsdnet_config.ifconfig->ip_netmask = ip_netmask;

  rv = rtems_bsdnet_initialize_network();
  RTEMS_CHECK_RV_SC(rv, "initialize network");

  return RTEMS_SUCCESSFUL;
}

rtems_status_code demo_initialize_network_bootp(rtems_task_priority priority, const char mac_address [6], void (*bootp)(void))
{
  int rv = 0;

  rtems_bsdnet_config.network_task_priority = priority;
  rtems_bsdnet_config.bootp = bootp;
  rtems_bsdnet_config.ifconfig->hardware_address = mac_address;

  rv = rtems_bsdnet_initialize_network();
  RTEMS_CHECK_RV_SC(rv, "initialize network");

  return RTEMS_SUCCESSFUL;
}

void demo_network_set_buffer_space(size_t mbuf_bytes, size_t mbuf_cluster_bytes)
{
  rtems_bsdnet_config.mbuf_bytecount = mbuf_bytes;
  rtems_bsdnet_config.mbuf_cluster_bytecount = mbuf_cluster_bytes;
}

#endif /* RTEMS_NETWORKING */
