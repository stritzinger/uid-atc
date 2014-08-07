/**
 * @file
 *
 * @brief LED_write command.
 *
 * Switch the LEDs on or off.
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
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include "led.h"

#define NO_LEDS 8

static int led_cmd_write_func( int argc, char **argv )
{
  int eno = 0;
  char *led_names[NO_LEDS] = {
    LED_DEV_NAME_PWR_RED,
    LED_DEV_NAME_PWR_GREEN,
    LED_DEV_NAME_ETH_RED,
    LED_DEV_NAME_ETH_GREEN,
    LED_DEV_NAME_DP_RED,
    LED_DEV_NAME_DP_GREEN,
    LED_DEV_NAME_RF_RED,
    LED_DEV_NAME_RF_GREEN
  };
  int fd_leds[NO_LEDS] = {-1, -1, -1, -1, -1, -1, -1, -1};
  int i;
  char value;

  /* Check argument */
  if( argc != 2 ) {
    printf( "Failure: Need exactly one argument!\n" );
    eno = EINVAL;
  }

  if( eno == 0 ) {
    /* Get value */
    if( 1 != sscanf( argv[1], "%2"SCNx8, &value ) ) {
      printf( "Failure: Invalid argument: %s\n", argv[1] );
      eno = EINVAL;
    }
  }

  for( i = 0; i < NO_LEDS; ++i ) {
    if( eno == 0 ) {
      fd_leds[i] = open( led_names[i], O_WRONLY );
      if( fd_leds[i] == -1 ) {
        eno = errno;
      }
    }
  }

  if( eno == 0 ) {
    for( i = 0; i < NO_LEDS; ++i ) {
      uint8_t status = (value >> i) & 0x1;
      int bytes_written;
      bytes_written = write( fd_leds[i], &status, sizeof(status) );
      if( bytes_written != sizeof(status) ) {
        eno = errno;
      }
    }
  }

  for( i = 0; i < NO_LEDS; ++i ) {
    if( fd_leds[i] != -1 ) {
      if( eno == 0 ) {
        eno = close( fd_leds[i] );
      } else {
        close( fd_leds[i] );
      }
    }
  }

  if( eno == 0 ) {
    printf( "Success\n" );
  } else {
    printf( "Failure\n" );
  }
  return eno;
}

rtems_shell_cmd_t led_cmd_write = {
  "LED_write",
  "LED_write\n"
  "Switch the LEDs on or off.\n",
  "LED",
  led_cmd_write_func,
  NULL,
  NULL
};
