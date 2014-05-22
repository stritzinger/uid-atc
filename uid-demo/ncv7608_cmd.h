/**
 * @file
 *
 * @ingroup uid_demo
 *
 * @brief NCV7608 commands.
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

#ifndef NCV7608_CMD_H
#define NCV7608_CMD_H

extern rtems_shell_cmd_t ncv7608_cmd_wr;
extern rtems_shell_cmd_t ncv7608_cmd_write;
extern rtems_shell_cmd_t ncv7608_cmd_read;

#endif /* NCV7608_CMD_H */
