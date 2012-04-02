/**
 * @file
 *
 * @ingroup demo
 *
 * @brief Directory iteration implementatin.
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
#include <stdlib.h>
#include <dirent.h>

#include "demo.h"

rtems_status_code demo_directory_iterate(
  const char *dir_path,
  demo_per_directory_routine routine,
  void *routine_arg
)
{
  rtems_status_code rsc = RTEMS_SUCCESSFUL;
  int rv = 0;
  size_t const path_len = strlen(dir_path);
  DIR *const dir = opendir(dir_path);
  struct dirent *de = NULL;
  size_t buf_len = 0;
  char *buf = NULL;

  if (dir == NULL) {
    return RTEMS_INVALID_ID;
  }

  while ((de = readdir(dir)) != NULL) {
    size_t const new_buf_len = path_len + strlen(de->d_name) + 2;
    struct stat st;

    if (new_buf_len > buf_len) {
      free(buf);
      buf = malloc(new_buf_len);
      if (buf == NULL) {
        rsc = RTEMS_NO_MEMORY;

        break;
      }

      strcpy(buf, dir_path);
      buf [path_len] = '/';
    }

    strcpy(buf + path_len + 1, de->d_name);

    rv = lstat(buf, &st);
    if (rv != 0) {
      rsc = RTEMS_IO_ERROR;

      break;
    }

    rsc = (*routine)(buf, &st, routine_arg);
    if (rsc != RTEMS_SUCCESSFUL) {
      break;
    }
  }

  free(buf);
  closedir(dir);

  return rsc;
}
