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
#include "multiio.h"
#include "ncv7608.h"

#define BUF_SIZE 2

static int ncv7608_cmd_write_func( int argc, char **argv )
{
  int eno = 0;
  FILE *file = NULL;
  char write;

  /* Check argument */
  if( argc != 3 ) {
    printf( "Failure: Need exactly one argument!\n" );
    eno = EINVAL;
  }

  if( eno == 0 ) {
    /* Get values */
    if( 1 != sscanf( argv[1], "%2"SCNx8, &write ) ) {
      printf( "Failure: Invalid argument: %s\n", argv[1] );
      eno = EINVAL;
    }
  }

  if( eno == 0 ) {
    file = fopen( NCV7608_DEV_NAME_BMP, "w" );
  }

  if( file == NULL ) {
    eno = errno;
    printf( "Failure: Can not open device.\n" );
  }

  if( fputc( write, file ) != write ) {
    eno = EIO;
    printf( "Failure: Can not write to device.\n" );
  }

  if( eno == 0 ) {
    printf( "Success\n" );
  } else {
    printf( "Failure\n" );
  }
  return eno;
}

rtems_shell_cmd_t ncv7608_cmd_write = {
  "NCV_write",
  "NCV_write\n"
  "Write data to the output-chip.\n",
  "NCV",
  ncv7608_cmd_write_func,
  NULL,
  NULL
};
