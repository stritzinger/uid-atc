/**
 * @file
 *
 * @brief DS2408 driver.
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

#include <rtems.h>
#include <rtems/libi2c.h>

#include "i2cds2408.h"

#include <rtems/libio.h>

#include <unistd.h>

#define INVALID_VALUE 0xde

static rtems_status_code ds2408_read (rtems_device_major_number,
                                      rtems_device_minor_number,
                                      void *arg);

static rtems_status_code ds2408_write (rtems_device_minor_number,
                                       rtems_device_minor_number,
                                      void *arg);

static int ds2408_1wire_read (rtems_device_minor_number,
                              unsigned char *read_readbuff);



static int ds2408_1wire_busy (rtems_device_minor_number minor)
{
  int sc;
  unsigned char busy_readbuff = INVALID_VALUE;
  do {
    sc = rtems_libi2c_start_read_bytes (minor, &busy_readbuff, 1);
    busy_readbuff &= DS2482_1WB_MASK;
  } while ((busy_readbuff == DS2482_1WB_MASK) && (sc >= 0));
/* FIXME add timeout */

  return sc;
}



static int ds2408_1wire_reset (rtems_device_minor_number minor)
{
  int sc;
  unsigned char reset_writebuff;
  unsigned char reset_readbuff = INVALID_VALUE;

/* 1-Wire Reset */
  reset_writebuff = DS2482_CMD_1WIRE_RESET;
  sc = rtems_libi2c_start_write_bytes (minor, &reset_writebuff, 1);
  if (sc >= 0) {
    sc = ds2408_1wire_busy(minor);
  }
  if (sc >= 0) {
    sc = rtems_libi2c_send_stop(minor);
  }

  return sc;
}



static int ds2408_write_1wire_byte (rtems_device_minor_number minor,
                          unsigned char one_wire_write_byte)
{
  int sc;
  unsigned char one_wire_write_writebuff[2] = {DS2482_CMD_1WIRE_WRITE_BYTE,
                                               one_wire_write_byte};

  sc = rtems_libi2c_start_write_bytes (minor, one_wire_write_writebuff, 2);
  if (sc >= 0) {
    sc = ds2408_1wire_busy(minor);
  }
  if (sc >= 0) {
    sc = rtems_libi2c_send_stop(minor);
  }

  return sc;
}



static int ds2408_1wire_address_verify (rtems_device_minor_number minor,
                                        const unsigned char data_value)
{
  int sc, data_counter;
  unsigned char write_readbuff = INVALID_VALUE;
  unsigned char addr_buff;

/* 1-Wire Write */
/* 1-Wire Reset */
  sc = ds2408_1wire_reset(minor);
/* Skip Rom */
  if (sc >= 0) {
    sc = ds2408_write_1wire_byte (minor, DS2408_SKIP_ROM);
  }
/* Issue Control */
  if (sc >= 0) {
    sc = ds2408_write_1wire_byte (minor, DS2408_ISSUE_WRITE_CMD_SEARCH_REG);
  }
/* Target Address */
  if (sc >= 0) {
    sc = ds2408_write_1wire_byte (minor, DS2408_TGD_ADDR_8D);
  }
  if (sc >= 0) {
    sc = ds2408_write_1wire_byte (minor, DS2408_TGD_ADDR_00);
  }
/* Write Byte to Contol/Status Register */
/* Configers RSTZ as STRB output */
  if (sc >= 0) {
    sc = ds2408_write_1wire_byte (minor, data_value);
  }
/* 1-Wire Reset */
  if (sc >= 0) {
    sc = ds2408_1wire_reset(minor);
  }
/* Skip Rom */
  if (sc >= 0) {
    sc = ds2408_write_1wire_byte (minor, DS2408_SKIP_ROM);
  }
  else {
/* Write DS2408 */
    if (sc >= 0) {
      sc = ds2408_write_1wire_byte (minor, DS2408_SKIP_ROM);
    }
  }
/* Issue Control */
  if (sc >= 0) {
    sc = ds2408_write_1wire_byte (minor, DS2408_ISSUE_READ_POI_REG_CMD);
  }
/* Target Address */
  if (sc >= 0) {
    sc = ds2408_write_1wire_byte (minor, DS2408_TGD_ADDR_8D);
  }
  if (sc >= 0) {
    sc = ds2408_write_1wire_byte (minor, DS2408_TGD_ADDR_00);
  }
  if (sc >= 0) {
    sc = rtems_libi2c_send_stop(minor);
  }
/* Read Control/Status Register of DS2408 */
  if (sc >= 0) {
    sc = ds2408_1wire_read(minor, &write_readbuff);
    if (write_readbuff != DS2408_READBACK_MASK) {
      printk ("Verify FAIL\nResult of Reading over 1-Wire %X\n",write_readbuff);
      sc = -RTEMS_IO_ERROR;
    }
  }

  return sc;
}



static int ds2408_1wire_read_inputs (rtems_device_minor_number minor,
                                     unsigned char *input_readbuff)
{
  int sc, counter;

/* Read Inputs */
/* 1-Wire Reset */
  sc = ds2408_1wire_reset(minor);
/* Skip Rom */
  if (sc >= 0) {
    sc = ds2408_write_1wire_byte (minor, DS2408_SKIP_ROM);
  }
/* Issue Control */
  if (sc >= 0) {
    sc = ds2408_write_1wire_byte (minor, DS2408_ISSUE_CHAN_READ);
  }
/* Read PIO Pin Status of DS2408 */
  if (sc >= 0){
    sc = ds2408_1wire_read (minor, input_readbuff);
  }
/* 1-Wire Reset */
  sc = ds2408_1wire_reset(minor);

  return sc;
}



static int ds2408_1wire_read (rtems_device_minor_number minor,
                              unsigned char *read_readbuff)
{
  int sc;
  unsigned char read_writebuff[2] = {DS2482_CMD_1WIRE_READ_BYTE, 0};

/* Read for 1-Wire */
  sc = rtems_libi2c_start_write_bytes (minor, read_writebuff, 1);
  if (sc >= 0) {
    sc = ds2408_1wire_busy(minor);
  }
/* Set DS2482 to Read 1-Wire Device */
  if (sc >= 0) {
    read_writebuff[0] = DS2482_CMD_SET_READ_POINTER;
    read_writebuff[1] = DS2482_PIONTER_READ_DATA_REGISTER;
    sc = rtems_libi2c_start_write_bytes (minor, read_writebuff, 2);
  }
/* Read the DS2482 and Read the Data from DS2408 */
  if (sc >= 0) {
    sc = rtems_libi2c_start_read_bytes (minor, read_readbuff, 1);
  }
  if (sc >= 0) {
    sc = rtems_libi2c_send_stop(minor);
  }
  return sc;
}



static int ds2408_1wire_write_output (rtems_device_minor_number minor,
                                      unsigned char led_action)
{
  int sc;
  unsigned char not_led_action = (unsigned char) ~led_action;

/* Write Output */
/* 1-Wire Reset */
  sc = ds2408_1wire_reset(minor);
/* Skip Rom */
  if (sc >= 0) {
    sc = ds2408_write_1wire_byte (minor, DS2408_SKIP_ROM);
  }
/* Issue Channel Write */
  if (sc >= 0) {
    sc = ds2408_write_1wire_byte (minor, DS2408_ISSUE_CHAN_WRITE);
  }
/* Write to 1-Wire PIO's */
  if (sc >= 0) {
    sc = ds2408_write_1wire_byte (minor, led_action);
  }
/* Write invertet to 1-Wire PIO's */
  if (sc >= 0) {
    sc = ds2408_write_1wire_byte (minor, not_led_action);
  }
/* 1-Wire Reset */
  if (sc >= 0) {
    sc = ds2408_1wire_reset(minor);
  }

  return sc;
}



static rtems_status_code
ds2408_init (rtems_device_major_number major, rtems_device_minor_number minor,
             void *arg)
{
  int sc;
  unsigned char writebuff[2] = { DS2482_CMD_SET_READ_POINTER,
                                 DS2482_PIONTER_STATUS_REGISTER};
  unsigned char cmd;
  unsigned char readbuff = INVALID_VALUE;

  unsigned char familycode[2] = { DS2482_CMD_1WIRE_WRITE_BYTE,
                                  DS2408_READ_ROM_FAMILY_CODE };

  sc = rtems_libi2c_start_write_bytes (minor, writebuff, 2);
  if (sc >= 0) {
    sc = rtems_libi2c_start_read_bytes (minor, &readbuff, 1);
  }
  if (sc >= 0) {
    sc = rtems_libi2c_send_stop(minor);
  }

/* Reset DS2482-100 */
  writebuff[0] = DS2482_CMD_DEVICE_RESET;
  if (sc >= 0) {
    sc = rtems_libi2c_start_write_bytes (minor, writebuff, 1);
  }
  if (sc >= 0) {
    sc = rtems_libi2c_start_read_bytes (minor, &readbuff, 1);
  }
  if (sc >= 0) {
    sc = rtems_libi2c_send_stop(minor);
  }
  if (sc >= 0) {
    sc = ds2408_1wire_busy(minor);
  }
/* Write Configuration Befor Starting 1-Wire Activity */
  if (sc >= 0) {
    writebuff[0] = DS2482_CMD_WRITE_CONFIG;
    writebuff[1] = DS2482_1WIRE_CONFIG_REG_VALUE;
    sc = rtems_libi2c_start_write_bytes (minor, writebuff, 2);
  }

  if (sc >= 0) {
    sc = rtems_libi2c_send_stop(minor);
  }

  return -sc;
}



static rtems_status_code
ds2408_read (rtems_device_major_number major, rtems_device_minor_number minor,
             void *arg)
{

  rtems_libio_rw_args_t *rwargs = arg;
  int sc;
  unsigned char readbuff = INVALID_VALUE;
  unsigned char data_register_value = DS2408_READ_CON_STAT_MASK;

  rwargs->bytes_moved = 0;
  if (rwargs->count == 0) {
    return RTEMS_SUCCESSFUL;
  }

/* Call 1-Wire Write for Address DS2408 an Verify Connection */
/* Reset and Preset not done */
  sc = ds2408_1wire_address_verify (minor, data_register_value);
/* Read Inputs from DS2408 */
  if (sc >= 0) {
    sc = ds2408_1wire_read_inputs (minor, &readbuff);
  }
  if (sc >= 0) {
    rwargs->buffer[0] = (readbuff & DS2408_VERSION_MASK);
    rwargs->bytes_moved = 1;
  }

  return -sc;
}



static rtems_status_code
ds2408_write (rtems_device_major_number major,  rtems_device_minor_number minor,
             void *arg)
{

  rtems_libio_rw_args_t *rwargs = arg;

  int sc;
  unsigned char data_register_value = DS2408_READ_CON_STAT_MASK;

  rwargs->bytes_moved = 0;
  if (rwargs->count == 0) {
    return RTEMS_SUCCESSFUL;
  }

/* Call 1-Wire Write for Address DS2408 an Verfiy Connection */
/* Reset and Preset not done */
  sc = ds2408_1wire_address_verify (minor, data_register_value);
/* Write Output bit 8 of DS2408 */
  if (sc >= 0){
    sc = ds2408_1wire_write_output (minor, rwargs->buffer[0]);
  }
  if (sc >= 0) {
    rwargs->bytes_moved = 1;
  }

  return -sc;
}



static rtems_driver_address_table myops = {
  .initialization_entry = ds2408_init,
  .read_entry = ds2408_read,
  .write_entry = ds2408_write
};

static rtems_libi2c_drv_t my_drv_tbl = {
  .ops =                  &myops,
  .size =                 sizeof (my_drv_tbl),
};

rtems_libi2c_drv_t *i2c_ds2408_driver_descriptor = &my_drv_tbl;
