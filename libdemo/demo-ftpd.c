/**
 * @file
 *
 * @ingroup demo
 *
 * @brief Source for FTP code.
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

#include <rtems/ftpd.h>

#include "demo.h"

#include <rtems/status-checks.h>

struct rtems_ftpd_configuration rtems_ftpd_configuration = {
  /* FTPD task priority */
  .priority = 100,

  /* Maximum buffersize for hooks */
  .max_hook_filesize = 0,

  /* Well-known port */
  .port = 21,

  /* List of hooks */
  .hooks = NULL,

  /* Root for FTPD or NULL for "/" */
  .root = NULL,

  /* Max. connections */
  .tasks_count = 4,

  /* Idle timeout in seconds  or 0 for no (infinite) timeout */
  .idle = 0,

  /* Access: 0 - r/w, 1 - read-only, 2 - write-only, 3 - browse-only */
  .access = 0
};

rtems_status_code demo_initialize_ftpd(rtems_task_priority priority)
{
  int rv = 0;

  rtems_ftpd_configuration.priority = priority;

  rv = rtems_initialize_ftpd();
  RTEMS_CHECK_RV_SC(rv, "initialize ftpd");

  return RTEMS_SUCCESSFUL;
}

#endif /* RTEMS_NETWORKING */
