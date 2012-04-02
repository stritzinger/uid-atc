/*
 * Copyright (c) 2011-2012 embedded brains GmbH.  All rights reserved.
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

#include <gtest/gtest.h>

#include <rtems.h>

#include "quicc.h"

class Quicc : public ::testing::Test {
	protected:
		enum State {
			NOT_INITIALIZED,
			INITIALIZED,
			FATAL_ERROR
		};

		static const size_t RX_BUFFER_SIZE = 2048;

		static const size_t RX_BD_COUNT = 32;

		static const size_t TX_BD_COUNT = 32;

		static const quicc_ucf_config UCF_CONFIG;

		static const quicc_uec_config UEC_CONFIG;

		static State sState;

		static uint8_t sRxBuf [RX_BD_COUNT] [RX_BUFFER_SIZE];

		static uint8_t *sRxBufCurrent;

		static quicc_context *sQuicc;

		static quicc_ucf_context sUcf;

		static quicc_uec_context sUec;

		void SetUp(void) {
			if (sState == NOT_INITIALIZED) {
				sState = FATAL_ERROR;

				sQuicc = quicc_init();
				ASSERT_TRUE(sQuicc != NULL);

				quicc_ucf_init(&sUcf, &UCF_CONFIG);
				EXPECT_EQ(sUcf.context, sQuicc);
				EXPECT_EQ(sUcf.ucf_regs, &quicc_ucc_index_to_regs(sUcf.context, UCF_CONFIG.index)->fast);
				EXPECT_EQ(sUcf.config, &UCF_CONFIG);

				quicc_ucf_enable(&sUcf, QUICC_DIR_NONE);
				quicc_ucf_disable(&sUcf, QUICC_DIR_RX_AND_TX);

				quicc_uec_init(&sUec, &sUcf, &UEC_CONFIG);

				sState = INITIALIZED;
			}
			ASSERT_EQ(INITIALIZED, sState);
		};

	private:
		static void *fillReceiveBD(void *arg, quicc_bd *bd, bool last)
		{
			uint8_t *buf = sRxBufCurrent;

			bd->status = QUICC_BD_RX_E | QUICC_BD_LENGTH(RX_BUFFER_SIZE);
			bd->buffer = reinterpret_cast<uint32_t>(buf);

			if (!last) {
				sRxBufCurrent += RX_BUFFER_SIZE;
			} else {
				bd->status |= QUICC_BD_W;
				sRxBufCurrent = &sRxBuf [0] [0];
			}

			return buf;
		}
};

const quicc_ucf_config Quicc::UCF_CONFIG = {
	0,
	QUICC_CLK_9,
	QUICC_CLK_10,
	QUICC_UCF_ETHERNET_FAST
};

const quicc_uec_config Quicc::UEC_CONFIG = {
	QUICC_UEC_SPEED_100,
	QUICC_UEC_INTERFACE_TYPE_MII,
	QUICC_UEC_THREAD_COUNT_1,
	QUICC_UEC_THREAD_COUNT_1,
	RX_BD_COUNT,
	TX_BD_COUNT,
	RX_BUFFER_SIZE,
	NULL,
	fillReceiveBD,
	17
};

Quicc::State Quicc::sState = Quicc::NOT_INITIALIZED;

uint8_t Quicc::sRxBuf [RX_BD_COUNT] [RX_BUFFER_SIZE];

uint8_t *Quicc::sRxBufCurrent = &sRxBuf [0] [0];

quicc_context *Quicc::sQuicc;

quicc_ucf_context Quicc::sUcf;

quicc_uec_context Quicc::sUec;

TEST_F(Quicc, MicrocodeRevision)
{
	EXPECT_EQ(0xb0600001U, sQuicc->regs->engine.ceurnr);
}

TEST_F(Quicc, Timestamp)
{
	rtems_status_code sc = rtems_task_wake_after(1);
	EXPECT_EQ(RTEMS_SUCCESSFUL, sc);
	volatile quicc *q = sQuicc->regs;
	uint32_t a0 = q->engine.cetsr1;
	uint32_t b0 = q->engine.cetsr2;
	sc = rtems_task_wake_after(rtems_clock_get_ticks_per_second());
	EXPECT_EQ(RTEMS_SUCCESSFUL, sc);
	uint32_t a1 = q->engine.cetsr1;
	uint32_t b1 = q->engine.cetsr2;
	uint32_t da = a1 - a0;
	uint32_t db = b1 - b0;
	uint32_t us = 1000000;
	uint32_t ea = da > us ? da - us : us - da;
	uint32_t eb = db > us ? db - us : us - db;
	EXPECT_GE(100, ea);
	EXPECT_GE(100, ea);
}

TEST_F(Quicc, MuramOffset)
{
	ssize_t currentOffset = sQuicc->muram_offset;
	ssize_t offset = quicc_muram_allocate_offset(sQuicc, 0, 1);
	EXPECT_EQ(currentOffset, offset);
	void *address = quicc_muram_offset_to_address(sQuicc, offset);
	EXPECT_EQ(reinterpret_cast<void *>(0xe0110000 + currentOffset), address);
	EXPECT_EQ(offset, quicc_muram_address_to_offset(sQuicc, address));
	offset = quicc_muram_allocate_offset(sQuicc, 0, 1);
	EXPECT_EQ(currentOffset, offset);
	EXPECT_EQ(-1, quicc_muram_address_to_offset(sQuicc, NULL));
	EXPECT_EQ(NULL, quicc_muram_offset_to_address(sQuicc, -1));
}

TEST_F(Quicc, SnumIndex)
{
	int snumIndexA = quicc_snum_allocate_index(sQuicc);
	EXPECT_LE(0, snumIndexA);
	int snumIndexB = quicc_snum_allocate_index(sQuicc);
	EXPECT_LE(0, snumIndexB);
	quicc_snum_free_index(sQuicc, snumIndexA);
	int snumIndexC = quicc_snum_allocate_index(sQuicc);
	EXPECT_EQ(snumIndexC, snumIndexA);
	quicc_snum_free_index(sQuicc, snumIndexB);
	quicc_snum_free_index(sQuicc, snumIndexA);
}

TEST_F(Quicc, UCCRegs)
{
	EXPECT_EQ(&QUICC_UCC_1, quicc_ucc_index_to_regs(sQuicc, 0));
	EXPECT_EQ(&QUICC_UCC_2, quicc_ucc_index_to_regs(sQuicc, 1));
	EXPECT_EQ(&QUICC_UCC_3, quicc_ucc_index_to_regs(sQuicc, 2));
	EXPECT_EQ(&QUICC_UCC_4, quicc_ucc_index_to_regs(sQuicc, 3));
	EXPECT_EQ(&QUICC_UCC_5, quicc_ucc_index_to_regs(sQuicc, 4));
	EXPECT_EQ(&QUICC_UCC_6, quicc_ucc_index_to_regs(sQuicc, 5));
	EXPECT_EQ(&QUICC_UCC_7, quicc_ucc_index_to_regs(sQuicc, 6));
	EXPECT_EQ(&QUICC_UCC_8, quicc_ucc_index_to_regs(sQuicc, 7));
	EXPECT_EQ(NULL, quicc_ucc_index_to_regs(sQuicc, 8));
}

TEST_F(Quicc, UCCFastSubblock)
{
	EXPECT_EQ(QUICC_CMD_SUBBLOCK_UCCFAST_1, quicc_ucc_index_to_fast_subblock(0));
	EXPECT_EQ(QUICC_CMD_SUBBLOCK_UCCFAST_2, quicc_ucc_index_to_fast_subblock(1));
	EXPECT_EQ(QUICC_CMD_SUBBLOCK_UCCFAST_3, quicc_ucc_index_to_fast_subblock(2));
	EXPECT_EQ(QUICC_CMD_SUBBLOCK_UCCFAST_4, quicc_ucc_index_to_fast_subblock(3));
	EXPECT_EQ(QUICC_CMD_SUBBLOCK_UCCFAST_5, quicc_ucc_index_to_fast_subblock(4));
	EXPECT_EQ(QUICC_CMD_SUBBLOCK_UCCFAST_6, quicc_ucc_index_to_fast_subblock(5));
	EXPECT_EQ(QUICC_CMD_SUBBLOCK_UCCFAST_7, quicc_ucc_index_to_fast_subblock(6));
	EXPECT_EQ(QUICC_CMD_SUBBLOCK_UCCFAST_8, quicc_ucc_index_to_fast_subblock(7));
	EXPECT_EQ(QUICC_CMD_SUBBLOCK_INVALID, quicc_ucc_index_to_fast_subblock(8));
}

TEST_F(Quicc, UCCClockSource)
{
	EXPECT_TRUE(quicc_ucc_set_clock_source(sQuicc, UCF_CONFIG.index, UCF_CONFIG.rx_clk, UCF_CONFIG.tx_clk));
	EXPECT_EQ(0x00560000, sQuicc->regs->cmx.cmxucr [0] & 0xffff0000);
}
