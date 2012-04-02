/**
 * @file
 *
 * @ingroup demo
 *
 * @brief Source for load ELF code.
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

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "demo.h"

#include <rtems/status-checks.h>

rtems_status_code demo_copy_file(const char *src, const char *dest)
{
  int in = 0;
  int out = 0;
  ssize_t n_in = 0;
  ssize_t n_out = 0;
  char buf [DEMO_STACK_BUFFER_SIZE];

  in = open(src, O_RDONLY);
  RTEMS_CHECK_RV_SC(in, "open source");

  out = open(dest, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
  RTEMS_CLEANUP_RV(out, cleanup_input, "open destination");

  while (true) {
    /* Read */
    n_in = read(in, buf, sizeof(buf));
    if (n_in > 0) {
      /* Write */
      n_out = write(out, buf, (size_t) n_in);
      if (n_out != n_in) {
        RTEMS_DO_CLEANUP(cleanup_all, "write error");
      }
    } else if (n_in == 0) {
      /* End of file */
      break;
    } else {
      RTEMS_DO_CLEANUP(cleanup_all, "read error");
    }
  }

  close(out);

  close(in);

  return RTEMS_SUCCESSFUL;

cleanup_all:

  close(out);

cleanup_input:

  close(in);

  return RTEMS_IO_ERROR;
}
