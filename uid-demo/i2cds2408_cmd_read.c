/**
 * @file
 *
 * @brief Ant_read command.
 *
 * Read the antenna board version.
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
#include <rtems/shell.h>
#include <stdio.h>
#include "i2cds2408_cmd.h"
#include "i2cds2408.h"


static int ds2408_cmd_read_func( int argc, char **argv )
{
  int eno = 0;
  FILE *file = NULL;
  int input;

  file = fopen( DS2408_DEV_NAME, "r" );

  if( file == NULL ) {
    eno = errno;
    printf( "Failure: Can not open device.\n" );
  }

  if( eno == 0 ) {
    input = fgetc( file );
    if( input < 0 ) {
      eno = EIO;
      printf( "Failure: Can not read from device.\n" );
    }
  }
  if( eno == 0 ) {
    printf( "Antenna board type: A%04d\n", (~input) & DS2408_VERSION_MASK);
  }

  if( eno == 0 ) {
    printf( "Success\n" );
  } else {
    printf( "Failure\n" );
  }
  return eno;
}

rtems_shell_cmd_t ds2408_cmd_read = {
  "Ant_read",
  "Ant_read\n"
  "Readout the antenna type.\n",
  "ANT",
  ds2408_cmd_read_func,
  NULL,
  NULL
};
