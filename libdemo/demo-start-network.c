/**
 * @file
 *
 * @ingroup demo
 *
 * @brief Network start commands.
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

#include "demo.h"

static rtems_task_priority demo_start_network_priority;

static const char *demo_start_network_mac_address;

static const char *demo_start_network_ip_self;

static const char *demo_start_network_ip_server;

static const char *demo_start_network_netmask;

static void (*demo_start_network_bootp_param)(void);

static rtems_status_code demo_start_network_command(void)
{
  return demo_initialize_network(
    demo_start_network_priority,
    demo_start_network_mac_address,
    demo_start_network_ip_self,
    demo_start_network_ip_server,
    demo_start_network_netmask
  );
}

static rtems_status_code demo_start_network_bootp_command(void)
{
  return demo_initialize_network_bootp(
    demo_start_network_priority,
    demo_start_network_mac_address,
    demo_start_network_bootp_param
  );
}

static demo_start_entry demo_start_network_entry = {
  .next = NULL,
  .name = "network",
  .start = demo_start_network_command,
  .status = DEMO_STATUS_AGAIN,
  .dependencies = { NULL }
};

static demo_start_entry demo_start_network_bootp_entry = {
  .next = NULL,
  .name = "network",
  .start = demo_start_network_bootp_command,
  .status = DEMO_STATUS_AGAIN,
  .dependencies = { NULL }
};

void demo_start_network(rtems_task_priority priority, const char mac_address [6], const char *ip_self, const char *ip_server, const char *netmask)
{
  demo_start_network_priority = priority;
  demo_start_network_mac_address = mac_address;
  demo_start_network_ip_self = ip_self;
  demo_start_network_ip_server = ip_server;
  demo_start_network_netmask = netmask;
  demo_start_add(&demo_start_network_entry);
}

void demo_start_network_bootp(rtems_task_priority priority, const char mac_address [6], void (*bootp)(void))
{
  demo_start_network_priority = priority;
  demo_start_network_mac_address = mac_address;
  demo_start_network_bootp_param = bootp;
  demo_start_add(&demo_start_network_bootp_entry);
}
