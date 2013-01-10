/*
 * Copyright (c) 2012 embedded brains GmbH.  All rights reserved.
 *
 *  embedded brains GmbH
 *  Obere Lagerstr. 30
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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>

#include <rtems.h>
#include <rtems/libio.h>
#include <rtems/malloc.h>

#include <bsp.h>

#include <mpc83xx/mpc83xx.h>

#include <local/demo.h>
#include <local/network-config.h>

#include <quicc/quicc.h>

#define NFS_SERVER_PATH "1000.100@192.168.96.64:/scratch/nfs"

#define APP_FILE_PATH "/nfs/br_uid.bin"

static const char mac_address [6] = NETWORK_MAC_ADDRESS;

static rtems_id led_timer_id;

static void led_progress(bool success)
{
  static int fd = -1;
  static uint8_t on_off[2] = { 0x00, 0xff };
  static uint8_t success_on_off[2] = { 0x00, 0xaa };
  static bool forward = false;

  int val;

  if (fd == -1) {
    fd = open("/dev/i2c1.sc620", O_RDWR);
    assert(fd >= 0);
  }

  if (success) {
    write(fd, success_on_off, sizeof(success_on_off));
  } else {
    write(fd, on_off, sizeof(on_off));

    val = on_off[1];
    if (val >= 0x80) {
      forward = false;
      val = 0x40;
    } else if (val == 0x01) {
      forward = true;
      val = 0x02;
    } else if (forward) {
      val <<= 1;
    } else {
      val >>= 1;
    }
    on_off[1] = (uint8_t) val;
  }
}

static void load_via_file(const char *file)
{
  rtems_status_code sc;
  size_t buffer_size = 64 * 1024 * 1024;
  uint8_t *buffer = rtems_heap_allocate_aligned_with_boundary(buffer_size, 64 * 1024 * 1024, 0);
  int fd;
  ssize_t in;
  int rv;

  assert(buffer != NULL);

  printf("boot: open file \"%s\"... ", file);
  fd = open(file, O_RDONLY);
  assert(fd >= 0);
  printf("done\n");

  printf("boot: read file \"%s\"... ", file);
  in = read(fd, buffer, buffer_size);
  assert(in > 0);
  printf("received %zi bytes\n", in);

  rv = close(fd);
  assert(rv == 0);

  sc = rtems_timer_cancel(led_timer_id);
  assert(sc == RTEMS_SUCCESSFUL);

  led_progress(true);

  quicc_reset(quicc_init());

  sleep(1);

  bsp_restart(buffer + 0x10000);
}

static void print_generation_counter(void)
{
  volatile m83xxSysConRegisters_t *syscon = &mpc83xx.syscon;
  uint32_t generation = syscon->sgprh;

  ++generation;

  syscon->sgprh = generation;

  printf("boot: generation %" PRIu32 "\n", generation);
}

static void led_timer(rtems_id timer, void *arg)
{
  rtems_status_code sc;

  sc = rtems_timer_reset(timer);
  assert(sc == RTEMS_SUCCESSFUL);

  led_progress(false);
}

static void Init(rtems_task_argument arg)
{
  rtems_status_code sc;
  int rv;

  printf("boot: register I2C... ");
  sc = bsp_register_i2c();
  if (sc == RTEMS_SUCCESSFUL) {
    printf("done\n");
  } else {
    printf("failed\n");
  }

  sc =  rtems_timer_initiate_server(
    250,
    RTEMS_MINIMUM_STACK_SIZE,
    RTEMS_DEFAULT_ATTRIBUTES
  );
  assert(sc == RTEMS_SUCCESSFUL);

  sc = rtems_timer_create(rtems_build_name('L', 'E', 'D', ' '), &led_timer_id);
  assert(sc == RTEMS_SUCCESSFUL);

  sc = rtems_timer_server_fire_after(
    led_timer_id,
    rtems_clock_get_ticks_per_second() / 10,
    led_timer,
    NULL
  );
  assert(sc == RTEMS_SUCCESSFUL);

  print_generation_counter();

  printf("boot: initialize network... ");
  sc = demo_initialize_network(
    80,
    mac_address,
    NETWORK_IP_SELF,
    NETWORK_IP_GATEWAY,
    NETWORK_IP_NETMASK
  );
  assert(sc == RTEMS_SUCCESSFUL);
  printf("done\n");

  printf("boot: mount NFS...\n");
  rv = mount_and_make_target_path(
    NFS_SERVER_PATH,
    "/nfs",
    RTEMS_FILESYSTEM_TYPE_NFS,
    RTEMS_FILESYSTEM_READ_WRITE,
    NULL
  );
  assert(rv == 0);
  printf("boot: ...mount NFS done\n");

  load_via_file(APP_FILE_PATH);
}

#define CONFIGURE_MAXIMUM_DRIVERS 8

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER

#define CONFIGURE_USE_IMFS_AS_BASE_FILESYSTEM

#define CONFIGURE_FILESYSTEM_NFS

#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS 32

#define CONFIGURE_UNLIMITED_OBJECTS

#define CONFIGURE_UNIFIED_WORK_AREAS

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE

#define CONFIGURE_STACK_CHECKER_ENABLED

#define CONFIGURE_INIT

#include <rtems/confdefs.h>
