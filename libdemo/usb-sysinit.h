/**
 * @file
 *
 * @ingroup demo
 *
 * @brief USB system initialization.
 */

/*
 * Copyright (c) 2009-2011 embedded brains GmbH.  All rights reserved.
 *
 *  embedded brains GmbH
 *  Obere Lagerstr. 30
 *  82178 Puchheim
 *  Germany
 *  <info@embedded-brains.de>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <rtems/freebsd/machine/rtems-bsd-sysinit.h>

#include <bsp.h>

#ifdef USB_SYSINIT_INIT

#if defined(LIBBSP_ARM_LPC24XX_BSP_H) || defined(LIBBSP_ARM_LPC32XX_BSP_H)
	#define NEED_USB_OHCI
#elif defined(__GEN83xx_BSP_h) || defined(LIBBSP_POWERPC_QORIQ_BSP_H)
	#define NEED_USB_EHCI
#endif

#if defined(LIBBSP_POWERPC_QORIQ_BSP_H)
	#define NEED_SDHC
#endif

SYSINIT_NEED_FREEBSD_CORE;
SYSINIT_NEED_USB_CORE;
#ifdef NEED_USB_OHCI
	SYSINIT_NEED_USB_OHCI;
#endif
#ifdef NEED_USB_EHCI
	SYSINIT_NEED_USB_EHCI;
#endif
SYSINIT_NEED_USB_MASS_STORAGE;
#ifdef NEED_SDHC
	SYSINIT_NEED_SDHC;
#endif

const char *const _bsd_nexus_devices [] = {
	#ifdef NEED_USB_OHCI
		"ohci",
	#endif
	#ifdef NEED_USB_EHCI
		"ehci",
	#endif
	#ifdef NEED_SDHC
		"sdhci",
	#endif
	NULL
};

#endif /* USB_SYSINIT_INIT */
