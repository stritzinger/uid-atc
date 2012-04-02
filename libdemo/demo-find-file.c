/**
 * @file
 *
 * @ingroup demo
 *
 * @brief Source for find file code.
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

#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

#include "demo.h"

rtems_status_code demo_find_file(
  const char *dir_path,
  const char *start_pattern,
  char **filepath
)
{
  size_t const start_pattern_len = strlen(start_pattern);
  size_t const path_len = strlen(dir_path) + 2;
  DIR *const dir = opendir(dir_path);

  if (dir != NULL) {
    struct dirent *de = NULL;

    while ((de = readdir(dir)) != NULL) {
      if (strncmp(de->d_name, start_pattern, start_pattern_len) == 0) {
        size_t const len = path_len + strlen(de->d_name);
	char *const fp = malloc(len);

        if (fp != NULL) {
          int rv = 0;
          struct stat mode;

          strcpy(fp, dir_path);
          strcat(fp, "/");
          strcat(fp, de->d_name);

          rv = stat(fp, &mode);
          if (rv == 0 && (mode.st_mode & S_IFREG) != 0) {
            *filepath = fp;

            closedir(dir);

            return RTEMS_SUCCESSFUL;
          } else {
            free(fp);
          }
        }
      }
    }

    closedir(dir);
  }

  return RTEMS_INVALID_ID;
}
