/**
 * @file
 *
 * @ingroup uid_demo
 *
 * @brief Chip Select Line Handling
 *
 * API for the SPI driver
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

#ifndef CHIP_SELECT_H
#define CHIP_SELECT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void chip_select_init( void );
void chip_select( void );
void chip_deselect( void );


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CHIP_SELECT_H */