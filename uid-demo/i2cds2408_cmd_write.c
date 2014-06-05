/**
 * @file
 *
 * @brief Ant_LED command.
 *
 * Switch the LED on the antenna board.
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


static int ds2408_cmd_write_func( int argc, char **argv )
{
  int eno = 0;
  FILE *file = NULL;
  char write;

  /* Check argument */
  if( argc != 2 ) {
    printf( "Failure: Need exactly one argument!\n" );
    eno = EINVAL;
  }

  if( eno == 0 ) {
    if(argv[1][0] == '1') {
      write = DS2408_LED_ON;
    }
    else {
      write = DS2408_LED_OFF;
    }
  }

  if( eno == 0 ) {
    file = fopen( DS2408_DEV_NAME, "w" );
  }

  if( file == NULL ) {
    eno = errno;
    printf( "Failure: Can not open device.\n" );
  }
  if (eno == 0) {
    if( fputc( write, file ) != write ) {
      eno = EIO;
      printf( "Failure: Can not write to device.\n" );
    }
  }
  if( eno == 0 ) {
    printf( "Success\n" );
  } else {
    printf( "Failure\n" );
  }
  return eno;
}

rtems_shell_cmd_t ds2408_cmd_write = {
  "Ant_LED",
  "Ant_LED\n"
  "Switch the LED on the antenna board.\n",
  "ANT",
  ds2408_cmd_write_func,
  NULL,
  NULL
};
