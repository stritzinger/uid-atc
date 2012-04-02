/*
 * Copyright (c) 2011-2012 embedded brains GmbH.  All rights reserved.
 *
 *  embedded brains GmbH
 *  Obere Lagerstr. 30
 *  82178 Puchheim
 *  Germany
 *  <rtems@embedded-brains.de>
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rtems.com/license/LICENSE.
 */

#define __INSIDE_RTEMS_BSD_TCPIP_STACK__ 1
#define __BSD_VISIBLE 1

#include <stdio.h>
#include <inttypes.h>

#include <rtems.h>
#include <rtems/rtems_bsdnet.h>
#include <rtems/rtems_mii_ioctl.h>

#define TBLSZ(tbl) (sizeof(tbl) / sizeof(tbl [0]))

typedef struct {
  const char *name;
  uint32_t mask;
} reg_field;

typedef struct {
  const char *name;
  unsigned reg;
  size_t field_count;
  const reg_field *field_table;
} reg_desc;

static const reg_field bmcr_fields [] = {
  { .name = "RESET", .mask = BMCR_RESET },
  { .name = "LOOP", .mask = BMCR_LOOP },
  { .name = "SPEED0", .mask = BMCR_SPEED0 },
  { .name = "AUTOEN", .mask = BMCR_AUTOEN },
  { .name = "PDOWN", .mask = BMCR_PDOWN },
  { .name = "ISO", .mask = BMCR_ISO },
  { .name = "STARTNEG", .mask = BMCR_STARTNEG },
  { .name = "FDX", .mask = BMCR_FDX },
  { .name = "CTEST", .mask = BMCR_CTEST },
  { .name = "SPEED1", .mask = BMCR_SPEED1 }
};

static const reg_field bmsr_fields [] = {
  { .name = "100T4", .mask = BMSR_100T4 },
  { .name = "100TXFDX", .mask = BMSR_100TXFDX },
  { .name = "100TXHDX", .mask = BMSR_100TXHDX },
  { .name = "10TFDX", .mask = BMSR_10TFDX },
  { .name = "10THDX", .mask = BMSR_10THDX },
  { .name = "100T2FDX", .mask = BMSR_100T2FDX },
  { .name = "100T2HDX", .mask = BMSR_100T2HDX },
  { .name = "EXTSTAT", .mask = BMSR_EXTSTAT },
  { .name = "MFPS", .mask = BMSR_MFPS },
  { .name = "ACOMP", .mask = BMSR_ACOMP },
  { .name = "RFAULT", .mask = BMSR_RFAULT },
  { .name = "ANEG", .mask = BMSR_ANEG },
  { .name = "LINK", .mask = BMSR_LINK },
  { .name = "JABBER", .mask = BMSR_JABBER },
  { .name = "EXTCAP", .mask = BMSR_EXTCAP }
};

static const reg_field phyidr1_fields [] = {
  { .name = "OUI", .mask = 0xffffffff }
};

static const reg_field phyidr2_fields [] = {
  { .name = "OUI", .mask = IDR2_OUILSB },
  { .name = "MODEL", .mask = IDR2_MODEL },
  { .name = "REV", .mask = IDR2_REV }
};

static const reg_field anar_fields [] = {
  { .name = "NP", .mask = ANAR_NP },
  { .name = "ACK", .mask = ANAR_ACK },
  { .name = "RF", .mask = ANAR_RF },
  { .name = "PAUSE", .mask = ANAR_X_PAUSE_TOWARDS },
  { .name = "T4", .mask = ANAR_T4 },
  { .name = "TX_FD", .mask = ANAR_TX_FD },
  { .name = "TX", .mask = ANAR_TX },
  { .name = "10_FD", .mask = ANAR_10_FD },
  { .name = "10", .mask = ANAR_10 },
  { .name = "SELECTOR", .mask = 0x1f }
};

static const reg_field aner_fields [] = {
  { .name = "MLF", .mask = ANER_MLF },
  { .name = "LPNP", .mask = ANER_LPNP },
  { .name = "NP", .mask = ANER_NP },
  { .name = "PAGE_RX", .mask = ANER_PAGE_RX },
  { .name = "LPAN", .mask = ANER_LPAN }
};

static const reg_desc reg_desc_table [] = {
  {
    .name = "BMCR",
    .reg = MII_BMCR,
    .field_count = TBLSZ(bmcr_fields),
    .field_table = &bmcr_fields [0]
  }, {
    .name = "BMSR",
    .reg = MII_BMSR,
    .field_count = TBLSZ(bmsr_fields),
    .field_table = &bmsr_fields [0]
  }, {
    .name = "PHYIDR1",
    .reg = MII_PHYIDR1,
    .field_count = TBLSZ(phyidr1_fields),
    .field_table = &phyidr1_fields [0]
  }, {
    .name = "PHYIDR2",
    .reg = MII_PHYIDR2,
    .field_count = TBLSZ(phyidr2_fields),
    .field_table = &phyidr2_fields [0]
  }, {
    .name = "ANAR",
    .reg = MII_ANAR,
    .field_count = TBLSZ(anar_fields),
    .field_table = &anar_fields [0]
  }, {
    .name = "ANLPAR",
    .reg = MII_ANLPAR,
    .field_count = TBLSZ(anar_fields),
    .field_table = &anar_fields [0]
  }, {
    .name = "ANER",
    .reg = MII_ANER,
    .field_count = TBLSZ(aner_fields),
    .field_table = &aner_fields [0]
  }
};

static int get_shift(uint32_t mask)
{
  int i = 0;

  while ((mask & (1U << i)) == 0) {
    ++i;
  }

  return i;
}

static int get_last(uint32_t mask)
{
  int i = 31;

  while ((mask & (1U << i)) == 0) {
    --i;
  }

  return i;
}

static void dump_reg(
  const struct rtems_mdio_info *mdio,
  int phy,
  void *arg,
  const reg_desc *desc
)
{
  int rv = 0;
  uint32_t val = 0;

  rv = (*mdio->mdio_r)(phy, arg, desc->reg, &val);
  if (rv == 0) {
    size_t i = 0;

    printf(
      "-------------------------------------------------------------------------------\n"
      " %s\n"
      "------------------------------------------------------------------+------------\n"
      "                                                                * | %#010" PRIx32 "\n",
      desc->name,
      val
    );
    for (i = 0; i < desc->field_count; ++i) {
      const reg_field *field = &desc->field_table [i];
      int shift = get_shift(field->mask);
      int last = get_last(field->mask);
      if (shift == last) {
        printf(
          " %64s |   %8" PRIx32 "\n",
          field->name,
          (val & field->mask) >> shift
        );
      } else {
        printf(
          " %64s | %#10" PRIx32 "\n",
          field->name,
          (val & field->mask) >> shift
        );
      }
    }
    printf(
      "------------------------------------------------------------------+------------\n"
    );
  }
}

void mii_dump(const struct rtems_mdio_info *mdio, int phy, void *arg)
{
  size_t i = 0;

  for (i = 0; i < TBLSZ(reg_desc_table); ++i) {
    dump_reg(mdio, phy, arg, &reg_desc_table [i]);
  }
}
