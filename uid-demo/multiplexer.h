/**
 * @file
 *
 * @ingroup uid_demo
 *
 * @brief Multiplexer Control
 *
 * Control handling for the multiplexer which select between UCC and SPI.
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

#ifndef MULTIPLEXER_H
#define MULTIPLEXER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void multiplexer_init( void );
void multiplexer_select_spi( void );
void multiplexer_select_ucc( void );


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MULTIPLEXER_H */