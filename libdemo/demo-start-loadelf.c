/**
 * @file
 *
 * @ingroup demo
 *
 * @brief Loadelf start command.
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

#include <stdio.h>

#include <rtems.h>

#include "demo.h"

#include <rtems/status-checks.h>

static const char *demo_loadelf_remote;

static const char *demo_loadelf_local;

static demo_jump demo_loadelf_jump;

static rtems_status_code demo_start_loadelf_command(void)
{
  rtems_status_code sc = RTEMS_SUCCESSFUL;

  sc = demo_loadelf(
    demo_loadelf_remote,
    demo_loadelf_local,
    demo_loadelf_jump
  );
  RTEMS_CHECK_SC(sc, "loadelf");

  return DEMO_STATUS_AGAIN;
}

static demo_start_entry demo_start_loadelf_entry = {
  .next = NULL,
  .name = "loadelf",
  .start = demo_start_loadelf_command,
  .status = DEMO_STATUS_AGAIN,
  .dependencies = { "ftpfs", NULL }
};

void demo_start_loadelf(const char *remote, const char *local, demo_jump jump)
{
  demo_loadelf_remote = remote;
  demo_loadelf_local = local;
  demo_loadelf_jump = jump;
  demo_start_add(&demo_start_loadelf_entry);
}
