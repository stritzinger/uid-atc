/*
 * Copyright (c) 2013-2014 embedded brains GmbH.  All rights reserved.
 *
 *  embedded brains GmbH
 *  Dornierstr. 4
 *  82178 Puchheim
 *  Germany
 *  <rtems@embedded-brains.de>
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

#include <assert.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <rtems.h>
#include <rtems/shell.h>
#include <rtems/console.h>
#include <rtems/malloc.h>
#include <rtems/rtems_bsdnet.h>

#include <bsp.h>

#include <local/network-config.h>

#include <ini/ini.h>

#include <quicc/quicc.h>

static char mac_address [6] = NETWORK_MAC_ADDRESS;

static char ip_self[16] = NETWORK_IP_SELF;

static char ip_netmask[16] = NETWORK_IP_NETMASK;

static char ip_gateway[16] = NETWORK_IP_GATEWAY;

static struct rtems_bsdnet_ifconfig ifconfig = {
	.name = RTEMS_BSP_NETWORK_DRIVER_NAME,
	.attach = RTEMS_BSP_NETWORK_DRIVER_ATTACH,
	.next = NULL,
	.ip_address = &ip_self[0],
	.ip_netmask = &ip_netmask[0],
	.hardware_address = &mac_address[0],
	.ignore_broadcast = 0,
	.mtu = 0,
	.rbuf_count = 0,
	.xbuf_count = 0,
	.port = 0,
	.irno = 0,
	.bpar = 0
};

struct rtems_bsdnet_config rtems_bsdnet_config = {
	.ifconfig = &ifconfig,
	.bootp = NULL,
	.network_task_priority = 80,
	.mbuf_bytecount = 0,
	.mbuf_cluster_bytecount = 0,
	.hostname = NULL,
	.domainname = NULL,
	.gateway = &ip_gateway[0],
	.log_host = &ip_gateway[0],
	.name_server = { &ip_gateway[0], NULL, NULL },
	.ntp_server = { &ip_gateway[0], NULL, NULL },
	.sb_efficiency = 0,
	.udp_tx_buf_size = 0,
	.udp_rx_buf_size = 0,
	.tcp_tx_buf_size = 0,
	.tcp_rx_buf_size = 0
};

static int ini_value_copy(void *dst, size_t dst_size, const char *value)
{
	int ok = 1;
	size_t value_size = strlen(value) + 1;

	if (value_size <= dst_size) {
		memcpy(dst, value, value_size);
	} else {
		ok = 0;
	}

	return ok;
}

static int ini_file_handler(void *arg, const char *section, const char *name, const char *value)
{
	int ok = 0;

	if (strcmp(section, "network") == 0) {
		if (strcmp(name, "mac_address") == 0) {
			int v [6];
			int count = sscanf(
				value,
				"%02x:%02x:%02x:%02x:%02x:%02x",
				&v [0], &v [1], &v [2], &v [3], &v [4], &v [5]
			);
			ok = count == 6;
			if (ok) {
				int i;
				for (i = 0; i < 6; ++i) {
					mac_address [i] = (uint8_t) v [i];
				}
			}
		} else if (strcmp(name, "ip_self") == 0) {
			ok = ini_value_copy(&ip_self[0], sizeof(ip_self), value);
		} else if (strcmp(name, "ip_gateway") == 0) {
			ok = ini_value_copy(&ip_gateway[0], sizeof(ip_gateway), value);
		} else if (strcmp(name, "ip_netmask") == 0) {
			ok = ini_value_copy(&ip_netmask[0], sizeof(ip_netmask), value);
		}
	}

	return ok;
}

void uid_init_network(const char *ini_file)
{
	ini_parse(ini_file, ini_file_handler, NULL);

	quicc_reset(quicc_init());

	int rv = rtems_bsdnet_initialize_network();
	assert(rv == 0);
}
