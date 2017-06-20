/**
 * @file
 *
 * @ingroup demo
 *
 * @brief Telnet demon initialization.
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

#include <rtems.h>

#include <rtems/shell.h>
#include <rtems/telnetd.h>

static void demo_telnet_shell(char *name, void *arg)
{
  rtems_shell_env_t env = rtems_global_shell_env;

  env.devname = name;
  env.taskname = "TLNT";
  env.login_check = NULL;
  env.forever = false;

  rtems_shell_main_loop(&env); 
}

rtems_telnetd_config_table rtems_telnetd_config = {
  .command = demo_telnet_shell,
  .arg = NULL,
  .priority = 0,
  .stack_size = 0,
  .login_check = NULL,
  .keep_stdio = false
};

rtems_status_code demo_initialize_telnetd(rtems_task_priority priority, size_t stack_size)
{
  rtems_telnetd_config.priority = priority;
  rtems_telnetd_config.stack_size = stack_size;

  return rtems_telnetd_initialize() ? RTEMS_IO_ERROR : RTEMS_SUCCESSFUL; 
}
