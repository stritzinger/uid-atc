/**
 * @file
 *
 * @brief NCV_wr command.
 *
 * Write and read the NCV7608.
 *
 */

/*
 * Copyright (c) 2014 embedded brains GmbH.  All rights reserved.
 *
 *  embedded brains GmbH
 *  Dornierstr. 4
 *  82178 Puchheim
 *  Germany
 *  <rtems@embedded-brains.de>
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rtems.org/license/LICENSE.
 */

#include <errno.h>
#include <inttypes.h>
#include <rtems/shell.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "multiio.h"
#include "ncv7608.h"

#define BUF_SIZE 2

static int ncv7608_cmd_read_func( int argc, char **argv )
{
  int eno = 0;
  int fdes = -1;
  char input[BUF_SIZE];

  fdes = open( NCV7608_DEV_NAME_BMP, O_RDONLY );
  if( fdes == -1 ) {
    eno = errno;
    printf( "Failure: Can not open device.\n" );
  }

  if( read( fdes, input, BUF_SIZE ) != BUF_SIZE ) {
    eno = EIO;
    printf( "Failure: Can not read from device.\n" );
  }

  if( eno == 0 ) {
    printf( "returns: %2x %2x\n", input[0], input[1] );
  }

  if( fdes != -1 ) {
    if( eno == 0 ) {
      eno = close( fdes );
    } else {
      close( fdes );
    }
  }

  if( eno == 0 ) {
    printf( "Success\n" );
  } else {
    printf( "Failure\n" );
  }
  return eno;
}

rtems_shell_cmd_t ncv7608_cmd_read = {
  "NCV_read",
  "NCV_read\n"
  "Read data from the output-chip.\n",
  "NCV",
  ncv7608_cmd_read_func,
  NULL,
  NULL
};
