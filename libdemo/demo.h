/**
 * @file
 *
 * @ingroup demo
 *
 * @brief Application samples.
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

#ifndef DEMO_DEMO_H
#define DEMO_DEMO_H

#include <sys/stat.h>

#include <rtems.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @defgroup demo Application Samples
 *
 * @ingroup LocalPackages
 *
 * @brief Application sample and support functions.
 *
 * @{
 */

#define DEMO_STATUS_AGAIN (RTEMS_STATUS_CODES_LAST + 1)

#define DEMO_STACK_BUFFER_SIZE ((size_t) (4 * 1024))

#define DEMO_LOCAL_ELF_FILE "/file.elf"

#define DEMO_PORT 13161

#define DEMO_MCAST_IP_ADDR "239.9.8.0"

typedef struct demo_start_entry {
  struct demo_start_entry *next;
  const char *name;
  rtems_status_code (*start)(void);
  rtems_status_code status;
  const char *dependencies [];
} demo_start_entry;

typedef void (*demo_jump)(void *address);

extern struct rtems_shell_cmd_tt demo_start_command;

void demo_start_add(demo_start_entry *entry);

rtems_status_code demo_initialize_shell(rtems_task_priority priority, size_t stack_size);

rtems_status_code demo_initialize_network(rtems_task_priority priority, const char mac_address [6], const char *ip_self, const char *ip_server, const char *netmask);

rtems_status_code demo_initialize_network_bootp(rtems_task_priority priority, const char mac_address [6], void (*bootp)(void));

rtems_status_code demo_initialize_ftpfs(bool verbose, time_t timeout_seconds);

rtems_status_code demo_initialize_tftpfs(void);

rtems_status_code demo_initialize_httpd(rtems_task_priority priority);

rtems_status_code demo_initialize_telnetd(rtems_task_priority priority, size_t stack_size);

rtems_status_code demo_initialize_ftpd(rtems_task_priority priority);

rtems_status_code demo_initialize_netperf(void);

rtems_status_code demo_initialize_network_echo(rtems_task_priority priority, uint16_t port);

rtems_status_code demo_initialize_network_flood(rtems_task_priority priority, const char *ip, uint16_t port);

rtems_status_code demo_initialize_network_echo_mcast(rtems_task_priority priority, const char *ip, uint16_t port);

rtems_status_code demo_copy_file(const char *src, const char *dest);

rtems_status_code demo_find_file(const char *dirpath, const char *start_pattern, char **filepath);

typedef rtems_status_code (*demo_per_directory_routine)(const char *entry_path, struct stat *st, void *arg);

rtems_status_code demo_directory_iterate(const char *dir_path, demo_per_directory_routine routine, void *routine_arg);

rtems_status_code demo_loadelf(const char *remote, const char *local, demo_jump jump);

void demo_start_network(rtems_task_priority priority, const char mac_address [6], const char *ip_self, const char *ip_server, const char *netmask);

void demo_start_network_bootp(rtems_task_priority priority, const char mac_address [6], void (*bootp)(void));

void demo_start_ftpfs(bool verbose, time_t timeout_seconds);

void demo_start_tftpfs(void);

void demo_start_httpd(rtems_task_priority priority);

void demo_start_telnetd(rtems_task_priority priority, size_t stack_size);

void demo_start_ftpd(rtems_task_priority priority);

void demo_start_netperf(void);

void demo_start_network_echo(rtems_task_priority priority, uint16_t port);

void demo_start_network_flood(rtems_task_priority priority, const char *ip, uint16_t port);

void demo_start_network_echo_mcast(rtems_task_priority priority, const char *ip, uint16_t port);

void demo_start_loadelf(const char *remote, const char *local, demo_jump jump);

void demo_wait(void);

void demo_usb_media_initialize(void);

void demo_network_set_buffer_space(size_t mbuf_bytes, size_t mbuf_cluster_bytes);

rtems_status_code demo_usb_media_wait(rtems_interval timeout, char **path);

struct rtems_mdio_info;
void demo_mii_dump(const struct rtems_mdio_info *mdio, int phy, void *arg);

int demo_connect(const char *ip_addr, uint16_t port);

rtems_status_code demo_md5sum_initialize(
  rtems_device_major_number major,
  rtems_device_minor_number minor,
  void *arg
);

rtems_status_code demo_md5sum_open(
  rtems_device_major_number major,
  rtems_device_minor_number minor,
  void *arg
);

rtems_status_code demo_md5sum_close(
  rtems_device_major_number major,
  rtems_device_minor_number minor,
  void *arg
);

rtems_status_code demo_md5sum_read(
  rtems_device_major_number major,
  rtems_device_minor_number minor,
  void *arg
);

rtems_status_code demo_md5sum_write(
  rtems_device_major_number major,
  rtems_device_minor_number minor,
  void *arg
);

rtems_status_code demo_md5sum_ioctl(
  rtems_device_major_number major,
  rtems_device_minor_number minor,
  void *arg
);

#define DEMO_MD5SUM_DEVICE_NAME "/dev/md5sum"

#define DEMO_MD5SUM_DRIVER \
  { \
    demo_md5sum_initialize, \
    demo_md5sum_open, \
    demo_md5sum_close, \
    demo_md5sum_read, \
    demo_md5sum_write, \
    demo_md5sum_ioctl \
  }

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DEMO_DEMO_H */
