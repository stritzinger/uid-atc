/**
 * @file
 *
 * @brief Print unexpected IRQ.
 *
 * Method for printing messages for unexpected IRQs from the trf7970A RFID controller
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
#include <errno.h>
#include "trf7970A_irq.h"
#include "trf7970A_regs.h"

int trf7970A_irq_print_unexpected(
  const uint8_t expected_irq_mask,
  const uint8_t irq_status
)
{
  int bytes_written;
  int eno = 0;
  uint8_t unexpected_irqs = irq_status & (uint8_t)~expected_irq_mask;

  if( (unexpected_irqs & TRF7970A_REGS_IRQ_STATUS_TX) != 0 ) {
    bytes_written = printf( "Unexpected IRQ: IRQ set due to end of TX\n" );
    if( bytes_written < 0 ) {
      if( errno != 0 ) {
        eno = errno;
      } else {
        eno = ferror( stdout );
      }
    }
  }
  if( eno == 0 ) {
    if( (unexpected_irqs & TRF7970A_REGS_IRQ_STATUS_SRX) != 0 ) {
      bytes_written = printf( "Unexpected IRQ: IRQ set due to RX start\n" );
      if( bytes_written < 0 ) {
        if( errno != 0 ) {
          eno = errno;
        } else {
          eno = ferror( stdout );
        }
      }
    }
  }
  if( eno == 0 ) {
    if( (unexpected_irqs & TRF7970A_REGS_IRQ_STATUS_FIFO) != 0 ) {
      bytes_written = printf( "Unexpected IRQ: Signals the FIFO level\n" );
      if( bytes_written < 0 ) {
        if( errno != 0 ) {
          eno = errno;
        } else {
          eno = ferror( stdout );
        }
      }
    }
  }
  if( eno == 0 ) {
    if( (unexpected_irqs & TRF7970A_REGS_IRQ_STATUS_ERR1) != 0 ) {
      bytes_written = printf( "Unexpected IRQ: CRC error\n" );
      if( bytes_written < 0 ) {
        if( errno != 0 ) {
          eno = errno;
        } else {
          eno = ferror( stdout );
        }
      }
    }
  }
  if( eno == 0 ) {
    if( (unexpected_irqs & TRF7970A_REGS_IRQ_STATUS_ERR2) != 0 ) {
      bytes_written = printf( "Unexpected IRQ: Parity error\n" );
      if( bytes_written < 0 ) {
        if( errno != 0 ) {
          eno = errno;
        } else {
          eno = ferror( stdout );
        }
      }
    }
  }
  if( eno == 0 ) {
    if( (unexpected_irqs & TRF7970A_REGS_IRQ_STATUS_ERR3) != 0 ) {
      bytes_written = printf( "Unexpected IRQ: Byte framing or EOF error\n" );
      if( bytes_written < 0 ) {
        if( errno != 0 ) {
          eno = errno;
        } else {
          eno = ferror( stdout );
        }
      }
    }
  }
  if( eno == 0 ) {
    if( (unexpected_irqs & TRF7970A_REGS_IRQ_STATUS_COL) != 0 ) {
      bytes_written = printf( "Unexpected IRQ: Collision error\n" );
      if( bytes_written < 0 ) {
        if( errno != 0 ) {
          eno = errno;
        } else {
          eno = ferror( stdout );
        }
      }
    }
  }
  if( eno == 0 ) {
    if( (unexpected_irqs & TRF7970A_REGS_IRQ_STATUS_NORESP) != 0 ) {
      bytes_written = printf( "Unexpected IRQ: No-response timeinterrupt\n" );
      if( bytes_written < 0 ) {
        if( errno != 0 ) {
          eno = errno;
        } else {
          eno = ferror( stdout );
        }
      }
    }
  }
  if( eno == 0 ) {
    if( unexpected_irqs != 0 ) {
      eno = EFAULT;
    }
  }
  return eno;
}