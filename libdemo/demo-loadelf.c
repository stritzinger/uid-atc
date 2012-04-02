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

#include "demo.h"
#include "load-elf.h"

#include <stdio.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static load_elf_segment segments [32];

static load_elf_segment *allocate_segments(void *arg, size_t count)
{
  return (count <= sizeof(segments) / sizeof(segments [0])) ?  segments : NULL;
}

static bool check_segments(
  void *arg,
  const load_elf_segment *segments,
  size_t count
)
{
  size_t i = 0;

  for (i = 0; i < count; ++i) {
    const load_elf_segment *seg = &segments [i];

    if (seg->dest_begin != seg->dest_end) {
      printf(
        "load -> [0x%08" PRIxPTR ", 0x%08" PRIxPTR ")\n",
        seg->dest_begin,
        seg->dest_end
      );
    }
  }

  return true;
}

static void start(void *arg, void *entry)
{
  demo_jump jump = (demo_jump) arg;
  printf("jump 0x%08" PRIxPTR "...\n", (uintptr_t) entry);
  rtems_task_wake_after(rtems_clock_get_ticks_per_second() / 10);
  (*jump)(entry);
}

rtems_status_code demo_loadelf(const char *filename, const char *local, demo_jump jump)
{
  int fd = open(filename, O_RDONLY);

  if (fd >= 0) {
    const load_elf_config config = {
      .arg = jump,
      .copy = load_elf_copy_with_memcpy,
      .allocate_segments = allocate_segments,
      .check_segments = check_segments,
      .start = start
    };
    load_elf_context context;
    char buf [4096];
    ssize_t in = 0;

    load_elf_init(&context, &config);
    while ((in = read(fd, buf, sizeof(buf))) > 0) {
      load_elf_process(&context, buf, (size_t) in);
    }
    load_elf_final(&context);

    close(fd);
  }

  return RTEMS_IO_ERROR;
}
