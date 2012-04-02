/**
 * @file
 *
 * @ingroup demo
 *
 * @brief Start shell command.
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

#include <string.h>
#include <stdio.h>

#include <rtems.h>
#include <rtems/shell.h>

#include "demo.h"

#include <rtems/status-checks.h>

static demo_start_entry *demo_start_list_head = NULL;

void demo_start_add(demo_start_entry *e)
{
  rtems_interrupt_level level;

  rtems_interrupt_disable(level);
  e->next = demo_start_list_head;
  demo_start_list_head = e;
  rtems_interrupt_enable(level);
}

static demo_start_entry *demo_query_start_entry(const char *name)
{
  if (name != NULL) {
    demo_start_entry *e = demo_start_list_head;

    while (e != NULL) {
      if (strcmp(name, e->name) == 0) {
        return e;
      }

      e = e->next;
    }
  }

  return NULL;
}

static rtems_status_code demo_start_execute(demo_start_entry *e)
{
  rtems_status_code sc = e->status;

  if (sc != RTEMS_SUCCESSFUL) {
    printf("start: %s\n", e->name);

    sc = e->start();
    if (sc != DEMO_STATUS_AGAIN) {
      RTEMS_CHECK_SC(sc, "start");
    }

    e->status = sc;
  } else {
    printf("already started: %s\n", e->name);
  }

  return sc;
}

static rtems_status_code demo_start_with_dependencies(const char *name)
{
  demo_start_entry *e = demo_query_start_entry(name);

  if (e != NULL) {
    rtems_status_code sc = RTEMS_SUCCESSFUL;
    const char **dep = e->dependencies;

    while (*dep != NULL) {
      sc = demo_start_with_dependencies(*dep);
      if (sc != RTEMS_SUCCESSFUL || sc != DEMO_STATUS_AGAIN) {
        RTEMS_CHECK_SC(sc, "start dependency");
      }
      ++dep;
    }

    return demo_start_execute(e);
  }

  return RTEMS_SUCCESSFUL;
}

static int demo_start(int argc, char **argv)
{
  if (argc > 1 && strcmp(argv [1], "list") == 0) {
    demo_start_entry *e = demo_start_list_head;

    while (e != NULL) {
      const char **dep = e->dependencies;
      printf("%s", e->name);

      if (*dep != NULL) {
        printf(" -> %s", *dep);
        ++dep;
      }

      while (*dep != NULL) {
        printf(", %s", *dep);
        ++dep;
      }

      printf("\n");

      e = e->next;
    }
  } else {
    int i = 0;

    /* Start with the second argument, the first argument is the command name */
    for (i = 1; i < argc; ++i) {
      rtems_status_code sc = demo_start_with_dependencies(argv [i]);
      if (sc != RTEMS_SUCCESSFUL) {
        return 1;
      }
    }
  }

  return 0;
}

struct rtems_shell_cmd_tt demo_start_command = {
  .name = "start",
  .usage = "starts system services, "
    "type 'start list' to list available services",
  .topic = "bsp",
  .command = demo_start,
  .alias = NULL,
  .next = NULL
};
