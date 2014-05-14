/**
 * @file
 *
 * @brief TRF_raw command.
 *
 * Command for writing uninterpreted bytes to the trf7970A RFID controller.
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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <inttypes.h>
#include <rtems/shell.h>
#include "trf7970A_impl.h"
#include "multiio.h"

typedef enum {
  TRF_7979A_CMD_RAW_MODE_SILENT,
  TRF_7979A_CMD_RAW_MODE_VERBOSE
} trf_7970A_cmd_raw_mode;

static int trf7970A_cmd_raw_hdlr(
  int                          argc,
  char                       **argv,
  const trf_7970A_cmd_raw_mode mode,
  uint8_t                     *read_buf,
  const size_t                 read_buf_size
)
{
  int                       eno             = 0;
  int                       index_arg       = 1;
  int                       index_buf       = 0;
  const multiio_bus_driver *BUS_DRIVER      = multiio_get_bus_driver();
  uint8_t                   write_buf[MULTIIO_WRITE_BUF_SIZE];
  uint8_t                  *buf_read;
  size_t                    bytes_written;
  multiio_exchange_data     exchange_data;
  multiio_reply             reply_data      = MULTIIO_REPLY_INITIALIZER(
    &read_buf[0],
    rtems_task_self(),
    trf7970A_impl_the_context.event_id_reply_ready,
    MULTIIO_ADDR_RFID_READER
  );

  memset( read_buf, 0, read_buf_size );
  memset( &write_buf[0], 0, sizeof(write_buf) );

  /* Will the arguments fit into the write buffer? */
  if( argc > (MULTIIO_WRITE_BUF_SIZE + 1) ) {
    if( mode == TRF_7979A_CMD_RAW_MODE_VERBOSE ) {
      printf( "Failure: Too many arguments: %d > %u!\n",
              argc, MULTIIO_WRITE_BUF_SIZE + 1
      );
    }
    eno = E2BIG;
  }

  if( eno == 0 ) {
    /* Get the SPI characters from argv */
    while( index_arg < argc && eno == 0 ) {
      if( 1 == sscanf( argv[index_arg], "%2"SCNx8, &(write_buf[index_buf]) ) ) {
        ++index_arg;
        ++index_buf;
      } else {
        if( mode == TRF_7979A_CMD_RAW_MODE_VERBOSE ) {
          printf( "Failure: Invalid argument: %s\n", argv[index_arg] );
        }
        eno = EINVAL;
      }
    }
  }
  bytes_written = (size_t)index_buf;

  if( bytes_written <= MULTIIO_WRITE_BUF_SIZE ) {
    if( bytes_written <= read_buf_size ) {
      /* Write to the interface */
      exchange_data.address        = MULTIIO_ADDR_RFID_READER;
      exchange_data.write_buf      = &write_buf[0];
      exchange_data.read_buf       = &read_buf[0];
      exchange_data.num_chars      = bytes_written;
      exchange_data.arg            = &reply_data;
      exchange_data.on_reply_ready = multiio_reply_ready;
      eno = (BUS_DRIVER->data_exchange)( &exchange_data );
      if( eno == 0 ) {
        /* Wait for the result */
        eno = multiio_reply_wait_for(
          reply_data.id_event,
          MULTIIO_ADDR_RFID_READER,
          rtems_clock_get_ticks_per_second() / 1000,
          &buf_read
        );
        if( eno == 0 ) {
          read_buf = buf_read;
        } else {
          if( mode == TRF_7979A_CMD_RAW_MODE_VERBOSE ) {
            printf( "Failure: Waiting for reply failed with error %d\n", eno );
          }
        }
      } else {
        if( mode == TRF_7979A_CMD_RAW_MODE_VERBOSE ) {
          printf( "Failure: data_echange method failed with error %d\n", eno );
        }
      }
    } else {
      /* read_buf is too small */
      eno = EINVAL;
    }
  } else {
    /* String is too long */
    eno = EINVAL;
  }
  return eno;
}


static int trf7970A_cmd_raw_func( int argc, char **argv )
{
  int     eno;
  uint8_t read_buf[MULTIIO_READ_BUF_SIZE];
  int     index_buf;

  /* Execute the handling for raw commands verbose */
  eno = trf7970A_cmd_raw_hdlr(
    argc,
    argv,
    TRF_7979A_CMD_RAW_MODE_VERBOSE,
    &read_buf[0],
    MULTIIO_READ_BUF_SIZE
 );

  if( eno == 0 ) {
    /* Print the results */
    for( index_buf = 0; index_buf < (argc - 1); ++index_buf ) {
      printf( "%02X ", read_buf[index_buf] );
    }
    printf( "\n" );
  }
  return eno;
}

rtems_shell_cmd_t trf7970A_cmd_raw = {
  "TRF_raw",
  "TRF_raw <param1 [param2 [param3 [...]]]>\n"
  "Forwards parameters to the TRF RFID reader.\n"
  "Arguments are hexadecimal bytes with leading zeros and without\n"
  "leading 0x or trailing h\n"
  "Returns bytes returned by the TRF RFID reader in the same format.\n",
  "TRF",
  trf7970A_cmd_raw_func,
  NULL,
  NULL
};

int trf7979A_cmd_raw_string( const char* cmd_string, uint8_t *read_buf, const size_t read_buf_size )
{
  int eno;
  char **cmd;
  int index = 0;
  char *cmd_cpy;

  /* Get a writeable copy of the cmd string */
  cmd_cpy = malloc( strlen(cmd_string) + 1 );

  if( NULL != cmd_cpy ) {
    strncpy( cmd_cpy, cmd_string, strlen(cmd_string) );
    
    cmd = calloc( sizeof(char*), MULTIIO_WRITE_BUF_SIZE + 1 );

    if( NULL != cmd ) {
      int index_read;

      /* Split the cmd string into tokens.
         Assign the addres of each token to an entry in array cmd.
         This way cmd will have the format of an argv and index will correspond to an argc. */
      cmd[index] = strtok(cmd_cpy, " " );

      while( cmd[index] != NULL ) {
        ++index;
        cmd[index] = strtok( NULL, " " );
      }

      /* Execute the handling for raw commands silently */
      eno = trf7970A_cmd_raw_hdlr( index, cmd, TRF_7979A_CMD_RAW_MODE_SILENT, read_buf, read_buf_size );
      if( index > 0 ) {
        /* Print the executed commands and their results */
        printf( "> %s\n", &cmd_string[cmd[1] - cmd[0]] );
        printf( "< " );
        for( index_read = 0; index_read < index -1; ++index_read ) {
          printf( "%02"PRIx8" ", read_buf[index_read] );
        }
        printf( "\n" );
      }

      free( cmd );
    } else {
      eno = ENOMEM;
    }
    free( cmd_cpy );
  } else {
    eno = ENOMEM;
  }

  return eno;
}