/**
 * @file
 *
 * @ingroup demo
 *
 * @brief FTP file system initialization.
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

#include <rtems/ftpfs.h>

#include "demo.h"

#include <rtems/status-checks.h>

rtems_status_code demo_initialize_ftpfs(bool verbose, time_t timeout_seconds)
{
  rtems_status_code sc = RTEMS_SUCCESSFUL;
  int rv = 0;
  struct timeval to = {
    .tv_sec = timeout_seconds,
    .tv_usec = 0
  };
  const char *target = RTEMS_FTPFS_MOUNT_POINT_DEFAULT;

  rv = mount_and_make_target_path(
    NULL,
    target,
    RTEMS_FILESYSTEM_TYPE_FTPFS,
    RTEMS_FILESYSTEM_READ_WRITE,
    NULL
  );
  RTEMS_CHECK_RV_SC(rv, "mount FTP file system");

  sc = rtems_ftpfs_set_verbose(target, verbose);
  RTEMS_CHECK_SC(sc, "set FTP file system verbose mode");

  sc = rtems_ftpfs_set_timeout(target, &to);
  RTEMS_CHECK_SC(sc, "set FTP file system timeout");

  return RTEMS_SUCCESSFUL;
}
