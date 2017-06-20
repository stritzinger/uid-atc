/*
 * Copyright (c) 2013, 2017 embedded brains GmbH.  All rights reserved.
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

#include <sys/stat.h>
#include <assert.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sysexits.h>

#include <rtems.h>
#include <rtems/shell.h>
#include <rtems/console.h>
#include <rtems/malloc.h>
#include <rtems/bsd/bsd.h>

#include <machine/rtems-bsd-commands.h>

#include <bsp.h>

#include <local/network-config.h>

#include <ini/ini.h>

#include <quicc/quicc.h>

RTEMS_BSD_DEFINE_NEXUS_DEVICE(qe, 0, 0, NULL);

SYSINIT_DRIVER_REFERENCE(qe, nexus);

static char mac_address[6] = NETWORK_MAC_ADDRESS;

static char ip_self[16] = NETWORK_IP_SELF;

static char ip_netmask[16] = NETWORK_IP_NETMASK;

static char ip_gateway[16] = NETWORK_IP_GATEWAY;

void
rtems_bsd_get_mac_address(const char *name, int unit, uint8_t mac_addr[6])
{

	assert(strcmp(name, "qe") == 0);
	assert(unit == 0);
	memcpy(mac_addr, mac_address, sizeof(mac_address));
}

static int
ini_value_copy(void *dst, size_t dst_size, const char *value)
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

static int
ini_file_handler(void *arg, const char *section, const char *name,
    const char *value)
{
	int ok = 0;

	(void)arg;

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

static void
default_network_ifconfig_lo0(void)
{
	int exit_code;
	char *lo0[] = {
	    "ifconfig",
	    "lo0",
	    "inet",
	    "127.0.0.1",
	    "netmask",
	    "255.255.255.0",
	    NULL };
	char *lo0_inet6[] = {
	    "ifconfig",
	    "lo0",
	    "inet6",
	    "::1",
	    "prefixlen",
	    "128",
	    NULL };

	exit_code = rtems_bsd_command_ifconfig(RTEMS_BSD_ARGC(lo0), lo0);
	assert(exit_code == EX_OK);

	exit_code = rtems_bsd_command_ifconfig(RTEMS_BSD_ARGC(lo0_inet6),
	    lo0_inet6);
	assert(exit_code == EX_OK);
}

static void
default_network_ifconfig_hwif0(char *ifname)
{
	int exit_code;
	char *ifcfg[] = {
	    "ifconfig",
	    ifname,
	    "inet",
	    ip_self,
	    "netmask",
	    ip_netmask,
	    NULL };

	exit_code = rtems_bsd_command_ifconfig(RTEMS_BSD_ARGC(ifcfg), ifcfg);
	assert(exit_code == EX_OK);
}

static void
default_network_route_hwif0(char *ifname)
{
	int exit_code;
	char *dflt_route[] = {
	    "route",
	    "add",
	    "-host",
	    ip_gateway,
	    "-iface",
	    ifname,
	    NULL };
	char *dflt_route2[] = {
	    "route",
	    "add",
	    "default",
	    ip_gateway,
	    NULL };

	exit_code = rtems_bsd_command_route(RTEMS_BSD_ARGC(dflt_route),
	    dflt_route);
	assert(exit_code == EXIT_SUCCESS);

	exit_code = rtems_bsd_command_route(RTEMS_BSD_ARGC(dflt_route2),
	    dflt_route2);
	assert(exit_code == EXIT_SUCCESS);
}

void
uid_init_network(const char *ini_file)
{
	char *ifname = "qe0";
	rtems_status_code sc;

	ini_parse(ini_file, ini_file_handler, NULL);

	quicc_reset(quicc_init());

	sc = rtems_bsd_initialize();
	assert(sc == RTEMS_SUCCESSFUL);

	default_network_ifconfig_lo0();
	default_network_ifconfig_hwif0(ifname);
	default_network_route_hwif0(ifname);
}
