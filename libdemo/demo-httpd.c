/**
 * @file
 *
 * @ingroup demo
 *
 * @brief Source for HTTP code.
 */

/*
 * Copyright (c) 2008-2011 embedded brains GmbH.  All rights reserved.
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

#if defined(RTEMS_NETWORKING) && defined(RTEMS_POSIX_API)

#include <stdio.h>

#include <rtems/cpuuse.h>

#include <mghttpd/mongoose.h>

#include "demo.h"

static void *cpuuse(
  enum mg_event event,
  struct mg_connection *con,
  const struct mg_request_info *request_info
)
{
  if (event == MG_NEW_REQUEST) {
    static const char uri [] = "/index.html";
    int match = strncmp(request_info->uri, uri, sizeof(uri));

    if (match == 0) {
      static const char begin [] =
        "<html><head><title>CPU Usage</title></head><body><pre>";
      static const char end [] = "</pre></body></html>";

      mg_write(con, begin, sizeof(begin));
      rtems_cpu_usage_report_with_plugin(con, (rtems_printk_plugin_t) mg_printf);
      mg_write(con, end, sizeof(end));

      /* Mark as processed */
      return "";
    }
  }

  return NULL;
}

rtems_status_code demo_initialize_httpd(rtems_task_priority priority)
{
  static const char *options [] = {
    "listening_ports", "80",
    "document_root", "/",
    "num_threads", "1",
    NULL
  };
  struct mg_context *ctx = mg_start(cpuuse, NULL, options);

  if (ctx != NULL) {
    return RTEMS_SUCCESSFUL;
  } else {
    return RTEMS_NO_MEMORY;
  }
}

#endif /* RTEMS_NETWORKING */
