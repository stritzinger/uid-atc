/**
 * @file
 *
 * @ingroup demo
 *
 * @brief MD5 sum device.
 */

/*
 * Copyright (c) 2011 embedded brains GmbH.  All rights reserved.
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

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <md5.h>

#include <rtems/libio.h>

#include "demo.h"

#ifndef MD5_DIGEST_LENGTH
  #define MD5_DIGEST_LENGTH 16
#endif

rtems_status_code demo_md5sum_initialize(
  rtems_device_major_number major,
  rtems_device_minor_number minor,
  void *arg
)
{
  rtems_status_code sc = RTEMS_SUCCESSFUL;

  sc = rtems_io_register_name(DEMO_MD5SUM_DEVICE_NAME, major, 0);
  assert(sc == RTEMS_SUCCESSFUL);

  return sc;
}

rtems_status_code demo_md5sum_open(
  rtems_device_major_number major,
  rtems_device_minor_number minor,
  void *arg
)
{
  rtems_status_code sc = RTEMS_SUCCESSFUL;
  rtems_libio_open_close_args_t *oc = arg;
  MD5_CTX *ctx = malloc(sizeof(*ctx));

  oc->iop->data1 = ctx;

  if (ctx != NULL) {
    MD5Init(ctx);
  } else {
    sc = RTEMS_NO_MEMORY;
  }

  return sc;
}

rtems_status_code demo_md5sum_close(
  rtems_device_major_number major,
  rtems_device_minor_number minor,
  void *arg
)
{
  rtems_libio_open_close_args_t *oc = arg;
  MD5_CTX *ctx = oc->iop->data1;
  uint8_t digest [MD5_DIGEST_LENGTH];
  size_t i = 0;

  MD5Final(digest, ctx);
  free(ctx);

  for (i = 0; i < sizeof(digest); ++i) {
    printf("%02x", digest [i]);
  }
  printf("\n");

  return RTEMS_SUCCESSFUL;
}

rtems_status_code demo_md5sum_read(
  rtems_device_major_number major,
  rtems_device_minor_number minor,
  void *arg
)
{
  rtems_status_code sc = RTEMS_SUCCESSFUL;
  rtems_libio_rw_args_t *rw = arg;

  rw->bytes_moved = 0;

  return RTEMS_IO_ERROR;
}

rtems_status_code demo_md5sum_write(
  rtems_device_major_number major,
  rtems_device_minor_number minor,
  void *arg
)
{
  rtems_status_code sc = RTEMS_SUCCESSFUL;
  rtems_libio_rw_args_t *rw = arg;
  MD5_CTX *ctx = rw->iop->data1;

  rw->bytes_moved = rw->count;
  MD5Update(ctx, (const uint8_t *) rw->buffer, rw->count);

  return RTEMS_SUCCESSFUL;
}

rtems_status_code demo_md5sum_ioctl(
  rtems_device_major_number major,
  rtems_device_minor_number minor,
  void *arg
)
{
  rtems_libio_ioctl_args_t *ctl = arg;

  ctl->ioctl_return = (uint32_t) -1;

  return RTEMS_IO_ERROR;
}
