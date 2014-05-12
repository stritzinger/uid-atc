/*
 * Copyright (c) 2013-2014 embedded brains GmbH.  All rights reserved.
 *
 *  embedded brains GmbH
 *  Obere Lagerstr. 30
 *  82178 Puchheim
 *  Germany
 *  <rtems@embedded-brains.de>
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

#ifndef NAND_CHIP_H
#define NAND_CHIP_H

#include <bed/bed-elbc.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define BANK 0

#define BASE_ADDRESS 0xfe000000

#define BLOCK_COUNT 2048

#define BLOCK_SIZE (128 * 1024)

#define PAGE_SIZE 2048

#define OOB_SIZE 64

#define PAGES_PER_BLOCK (BLOCK_SIZE / PAGE_SIZE)

#define HAS_LARGE_PAGES (PAGE_SIZE > 512)

#define NEEDS_3_PAGE_CYCLES 1

#define STAGE_1_BLOCK_BEGIN 0

#define STAGE_1_BLOCK_END 32

#define STAGE_1_BLOCK_COUNT (STAGE_1_BLOCK_END - STAGE_1_BLOCK_BEGIN)

#define STAGE_1_PAGES (STAGE_1_BLOCK_COUNT * PAGES_PER_BLOCK)

#define FS_BLOCK_BEGIN STAGE_1_BLOCK_END

#define FS_BLOCK_END (BLOCK_COUNT - 32)

#define FS_BLOCK_COUNT (FS_BLOCK_END - FS_BLOCK_BEGIN)

#define FS_PAGES (FS_BLOCK_COUNT * PAGES_PER_BLOCK)

#define ELBC ((volatile bed_elbc *) 0xe0005000)

/*
 * Micron MT29F2G08ABAEAH4 3.3V
 *
 * tADL =  70 ns
 * tALH =   5 ns
 * tALS =  10 ns
 * tCEA =  25 ns
 * tCH  =   5 ns
 * tCLH =   5 ns
 * tCLS =  10 ns
 * tCS  =  15 ns
 * tRC  =  20 ns
 * tREA =  16 ns
 * tREH =   7 ns
 * tRHZ = 100 ns
 * tRP  =  10 ns
 * tWB  = 100 ns
 * tWC  =  20 ns
 * tWH  =   7 ns
 * tWP  =  10 ns
 */

/*
 * Micron MT29F2G08ABAEA 3.3V timing for tLCLK = 13.8ns
 *
 * tCSCT = 27.6ns
 * tEHTR = 110ns
 * tCST = 6.80ns
 * tCHT = 20.ns
 * tWS = 27.6ns (write)
 * tWP = 13.8ns
 * tWC = 41.4ns
 * tADL = 221ns
 * tRP = 13.8ns
 * tRHT = 27.6ns
 * tWS = 0ns (read)
 * tRC = 41.4ns
 * tWRT = 221ns
 */
#define MT29F2G08ABAEA_LCLK_72_5_MHZ_OR \
	(ELBC_OR_FCM_SCY(0) \
		| ELBC_OR_TRLX \
		| ELBC_OR_EHTR \
		| ELBC_OR_FCM_RST)

/*
 * SPANSION S34ML02G1 3.3V timing for tLCLK = 13.8ns
 *
 * tCSCT = ?ns
 * tEHTR = ?ns
 * tCST = ?ns
 * tCHT = ?ns
 * tWS = ?ns (write)
 * tWP = ?ns
 * tWC = ?ns
 * tADL = ?ns
 * tRP = ?ns
 * tRHT = ?ns
 * tWS = ?ns (read)
 * tRC = ?ns
 * tWRT = ?ns
 */
#define S34ML02G1_LCLK_72_5_MHZ_OR \
	(ELBC_OR_FCM_SCY(0) \
		| ELBC_OR_TRLX \
		| ELBC_OR_EHTR \
		| ELBC_OR_FCM_RST)

#undef USE_MICRON_WITH_INTERNAL_ECC

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NAND_CHIP_H */
