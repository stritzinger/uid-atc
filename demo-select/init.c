/*
 * Copyright (c) 2012 embedded brains GmbH.  All rights reserved.
 *
 *  embedded brains GmbH
 *  Obere Lagerstr. 30
 *  82178 Puchheim
 *  Germany
 *  <info@embedded-brains.de>
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rtems.com/license/LICENSE.
 */

#include <assert.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <fcntl.h>
#include <stdio.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>

#include <rtems.h>
#include <rtems/console.h>

#include <local/demo.h>

static const char mac_address [6] = { 0x00, 0x1a, 0xf1, 0x00, 0x07, 0xa4 };

static void change_serial_settings(int fd)
{
  struct termios term;
  int rv = tcgetattr(fd, &term);
  assert(rv == 0);

  term.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
  term.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
  term.c_cflag &= ~(CSIZE | PARENB);
  term.c_cflag |= CS8;

  term.c_cc [VMIN] = 0;
  term.c_cc [VTIME] = 10;

  rv = tcsetattr(fd, TCSANOW, &term);
  assert(rv == 0);
}

static void test_termios_read_select(int fd)
{
  while (true) {
    int nfds = fd + 1;
    struct fd_set read_set;
    struct timeval timeout = {
      .tv_sec = 10,
      .tv_usec = 0
    };

    FD_ZERO(&read_set);
    FD_SET(fd, &read_set);

    int rv = select(nfds, &read_set, NULL, NULL, &timeout);
    if (rv == 0) {
      printf("timeout\n");
    } else if (rv > 0) {
      int i;

      for (i = 0; i < nfds; ++i) {
        int set = FD_ISSET(i, &read_set);

        if (set) {
          char buf [512];
          ssize_t n = read(i, buf, sizeof(buf));
          printf("read %i returned %zi\n", i, n);
        }
      }
    } else {
      perror("select failed");
    }
  }
}

static void test_termios_write_select(int fd)
{
  char buf [512];

  memset(buf, '?', sizeof(buf));
  buf [sizeof(buf) - 1] = '\n';

  while (true) {
    int nfds = fd + 1;
    struct fd_set write_set;
    struct timeval timeout = {
      .tv_sec = 10,
      .tv_usec = 0
    };

    FD_ZERO(&write_set);
    FD_SET(fd, &write_set);

    int rv = select(nfds, NULL, &write_set, NULL, &timeout);
    if (rv == 0) {
      printf("timeout\n");
    } else if (rv > 0) {
      int i;

      for (i = 0; i < nfds; ++i) {
        int set = FD_ISSET(i, &write_set);

        if (set) {
          ssize_t n = write(i, buf, sizeof(buf));
          printf("write %i returned %i\n", i, n);
        }
      }
    } else {
      perror("select failed");
    }
  }
}

static void Init(rtems_task_argument arg)
{
  demo_network_set_buffer_space(512 * 1024, 512 * 1024);
  demo_initialize_network(80, mac_address, "192.168.96.93", "192.168.96.1", "255.255.255.0");

  int fd = open(CONSOLE_DEVICE_NAME, O_RDWR);
  assert(fd >= 0);

  change_serial_settings(fd);
  test_termios_read_select(fd);
  test_termios_write_select(fd);

  int rv = close(fd);
  assert(rv == 0);

  exit(0);
}

#define CONFIGURE_MICROSECONDS_PER_TICK 10000

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER

#define CONFIGURE_USE_IMFS_AS_BASE_FILESYSTEM

#define CONFIGURE_FILESYSTEM_IMFS

#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS 32

#define CONFIGURE_MAXIMUM_TASKS 32
#define CONFIGURE_MAXIMUM_DRIVERS 8
#define CONFIGURE_MAXIMUM_SEMAPHORES 32
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES 4
#define CONFIGURE_MAXIMUM_TIMERS 4
#define CONFIGURE_MAXIMUM_PERIODS 4

#define CONFIGURE_MAXIMUM_POSIX_THREADS 4
#define CONFIGURE_MAXIMUM_POSIX_MUTEXES 16

#define CONFIGURE_EXTRA_TASK_STACKS (512 * 1024)

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE

#define CONFIGURE_INIT_TASK_STACK_SIZE (32 * 1024)

#define CONFIGURE_STACK_CHECKER_ENABLED

#define CONFIGURE_INIT

#include <rtems/confdefs.h>
