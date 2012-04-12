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
#include <rtems/rtems_bsdnet.h>

#define PRIO_HIGH 1

#define PRIO_LOW 2

#define READ_TOKEN 'R'

#define WRITE_TOKEN 'W'

static int read_select_pipe [2];

static char read_token;

static rtems_id init_task_id;

static void suspend_init_task(void)
{
  rtems_status_code sc = rtems_task_suspend(init_task_id);
  assert(sc == RTEMS_SUCCESSFUL);
}

static void resume_init_task(void)
{
  rtems_status_code sc = rtems_task_resume(init_task_id);
  assert(sc == RTEMS_SUCCESSFUL);
}

static void read_select_task(rtems_task_argument arg)
{
  rtems_status_code sc;
  int fd = read_select_pipe [0];
  int nfds = fd + 1;
  struct fd_set read_set;
  struct timeval timeout = {
    .tv_sec = 0,
    .tv_usec = 1
  };
  int rv;
  ssize_t n;

  suspend_init_task();

  FD_ZERO(&read_set);
  FD_SET(fd, &read_set);
  rv = select(nfds, &read_set, NULL, NULL, &timeout);
  assert(rv == 0);

  resume_init_task();

  FD_ZERO(&read_set);
  FD_SET(fd, &read_set);
  rv = select(nfds, &read_set, NULL, NULL, NULL);
  assert(rv == 1);
  assert(FD_ISSET(fd, &read_set));

  n = read(fd, &read_token, sizeof(read_token));
  assert(n == 1);
  assert(read_token == READ_TOKEN);

  sc = rtems_task_delete(RTEMS_SELF);
  assert(sc == RTEMS_SUCCESSFUL);
}

static int write_select_pipe [2];

static char write_buffer [PIPE_BUF];

static char read_buffer [PIPE_BUF];

static void write_select_task(rtems_task_argument arg)
{
  rtems_status_code sc;
  int fd = write_select_pipe [1];
  int nfds = fd + 1;
  struct fd_set write_set;
  struct timeval timeout = {
    .tv_sec = 0,
    .tv_usec = 1
  };
  int rv;
  ssize_t n;
  char token = WRITE_TOKEN;

  suspend_init_task();

  n = write(fd, write_buffer, sizeof(write_buffer));
  assert(n == (ssize_t) sizeof(write_buffer));

  FD_ZERO(&write_set);
  FD_SET(fd, &write_set);
  rv = select(nfds, NULL, &write_set, NULL, &timeout);
  assert(rv == 0);

  resume_init_task();

  FD_ZERO(&write_set);
  FD_SET(fd, &write_set);
  rv = select(nfds, NULL, &write_set, NULL, NULL);
  assert(rv == 1);
  assert(FD_ISSET(fd, &write_set));

  n = write(fd, &token, sizeof(token));
  assert(n == 1);

  sc = rtems_task_delete(RTEMS_SELF);
  assert(sc == RTEMS_SUCCESSFUL);
}

static void create_task(rtems_task_entry entry)
{
  rtems_id id;
  rtems_status_code sc = rtems_task_create(
    rtems_build_name('?', '?', '?', '?'),
    PRIO_HIGH,
    RTEMS_MINIMUM_STACK_SIZE,
    RTEMS_DEFAULT_MODES,
    RTEMS_DEFAULT_ATTRIBUTES,
    &id
  );
  assert(sc == RTEMS_SUCCESSFUL);

  sc = rtems_task_start(id, entry, 0);
  assert(sc == RTEMS_SUCCESSFUL);
}

struct rtems_bsdnet_config rtems_bsdnet_config;

static void Init(rtems_task_argument arg)
{
  int rv;
  ssize_t n;
  char token = READ_TOKEN;

  puts("\n\n*** TEST PIPE SELECT 1 ***");

  init_task_id = rtems_task_self();

  rv = rtems_bsdnet_initialize_network();
  assert(rv == 0);

  rv = pipe(&read_select_pipe [0]);
  assert(rv == 0);
  rv = pipe(&write_select_pipe [0]);
  assert(rv == 0);

  create_task(read_select_task);

  n = write(read_select_pipe [1], &token, sizeof(token));
  assert(n == 1);

  assert(read_token == READ_TOKEN);

  create_task(write_select_task);

  memset(read_buffer, 0xff, sizeof(read_buffer));
  n = read(write_select_pipe [0], read_buffer, sizeof(read_buffer));
  assert(n == (ssize_t) sizeof(read_buffer));
  assert(memcmp(read_buffer, write_buffer, sizeof(read_buffer)) == 0);

  n = read(write_select_pipe [0], &token, sizeof(token));
  assert(n == 1);
  assert(token == WRITE_TOKEN);

  rv = close(read_select_pipe [0]);
  assert(rv == 0);
  rv = close(read_select_pipe [1]);
  assert(rv == 0);
  rv = close(write_select_pipe [0]);
  assert(rv == 0);
  rv = close(write_select_pipe [1]);
  assert(rv == 0);

  puts("*** END OF TEST PIPE SELECT 1 ***");

  exit(0);
}

#define CONFIGURE_MICROSECONDS_PER_TICK 10000

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER

#define CONFIGURE_PIPES_ENABLED
#define CONFIGURE_MAXIMUM_PIPES 2

#define CONFIGURE_USE_IMFS_AS_BASE_FILESYSTEM

#define CONFIGURE_FILESYSTEM_IMFS

#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS 7

#define CONFIGURE_MAXIMUM_TASKS 3

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE

#define CONFIGURE_INIT_TASK_PRIORITY PRIO_LOW
#define CONFIGURE_INIT_TASK_INITIAL_MODES RTEMS_DEFAULT_MODES

#define CONFIGURE_STACK_CHECKER_ENABLED

#define CONFIGURE_INIT

#include <rtems/confdefs.h>
