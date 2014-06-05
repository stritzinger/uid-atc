/**
 * @file
 *
 * @ingroup uid_demo
 *
 * @brief DS2408 commands.
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

#include <rtems/io.h>

#ifndef DS2408_CMD_H
#define DS2408_CMD_H

#define DS2408_DEV_NAME "/dev/i2c1.DS2408"

extern rtems_shell_cmd_t ds2408_cmd_write;
extern rtems_shell_cmd_t ds2408_cmd_read;

#endif /* DS2408_CMD_H */
