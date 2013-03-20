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

#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include <rtems.h>
#include <rtems/rtems_bsdnet.h>
#include <rtems/rtems_mii_ioctl.h>

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/mbuf.h>

#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>

#include <mpc83xx/mpc83xx.h>

#include <bsp.h>
#include <bsp/irq.h>
#include <bsp/utility.h>

#include "quicc.h"

#define ASSERT_SC(sc) assert((sc) == RTEMS_SUCCESSFUL)

#define UEC_IF_EVENT_TX_START RTEMS_EVENT_1

#define UEC_IF_EVENT_IRQ RTEMS_EVENT_2

#define UEC_IF_WATCHDOG_TIMEOUT 5

#define UEC_IF_RX_UCCE_FLAGS QUICC_UCF_UCCE_UEC_RXB0

#define UEC_IF_TX_UCCE_FLAGS (QUICC_UCF_UCCE_UEC_TXE | QUICC_UCF_UCCE_UEC_TXB0)

typedef struct {
  uint32_t rx_interrupts;
  uint32_t rx_frames;
  uint32_t rx_length_violations;
  uint32_t rx_non_octet_aligned_frames;
  uint32_t rx_short_frames;
  uint32_t rx_crc_errors;
  uint32_t rx_overruns;
  uint32_t tx_interrupts;
  uint32_t tx_fragments;
  uint32_t tx_frames;
  uint32_t tx_defer_indications;
  uint32_t tx_late_collisions;
  uint32_t tx_retries;
  uint32_t tx_compacts;
} uec_if_statistics;

typedef struct {
  struct arpcom arpcom;
  struct rtems_mdio_info mdio;
  rtems_id rx_task_id;
  rtems_id tx_task_id;
  quicc_ucf_context ucf_context;
  quicc_uec_context uec_context;
  quicc_ucf_config ucf_config;
  quicc_uec_config uec_config;
  uec_if_statistics stats;
  uint16_t anlpar;
  bool promisc;
} uec_if_context;

static void uec_if_interrupt_handler(void *arg)
{
  rtems_status_code sc = RTEMS_SUCCESSFUL;
  uec_if_context *self = arg;
  const quicc_ucf_context *ucf_context = &self->ucf_context;
  volatile quicc_ucf *ucf_regs = ucf_context->ucf_regs;
  uint32_t ucce = ucf_regs->ucce;
  uint32_t uccm = ucf_regs->uccm;
  uint32_t rx_flags = UEC_IF_RX_UCCE_FLAGS;
  uint32_t tx_flags = UEC_IF_TX_UCCE_FLAGS;
  rtems_event_set event = UEC_IF_EVENT_IRQ;

  ucf_regs->ucce = ucce & uccm;
  ucf_regs->uccm = uccm & ~ucce;

  if ((ucce & rx_flags) != 0) {
    ++self->stats.rx_interrupts;
    sc = rtems_bsdnet_event_send(self->rx_task_id, event);
    ASSERT_SC(sc);
  }

  if ((ucce & tx_flags) != 0) {
    ++self->stats.tx_interrupts;
    sc = rtems_bsdnet_event_send(self->tx_task_id, event);
    ASSERT_SC(sc);
  }
}

static void uec_if_interrupt_install(uec_if_context *self)
{
  quicc_context *context = quicc_init();
  int ucc_index = self->ucf_config.index;
  quicc_interrupt irq = quicc_ucc_index_to_interrupt(ucc_index);

  quicc_irq_handler_install(context, irq, uec_if_interrupt_handler, self);
}

static struct mbuf *uec_if_get_mbuf(uec_if_context *self)
{
  struct ifnet *ifp = &self->arpcom.ac_if;
  struct mbuf *m = m_gethdr(M_WAIT, MT_DATA);

  MCLGET(m, M_WAIT);
  m->m_pkthdr.rcvif = ifp;

  return m;
}

static void *uec_if_bd_rx_fill(void *arg, quicc_bd *bd, bool last)
{
  uec_if_context *self = arg;
  struct mbuf *m = uec_if_get_mbuf(self);

  bd->status = QUICC_BD_I | QUICC_BD_RX_E;
  bd->buffer = mtod(m, uint32_t);

  if (last) {
    bd->status |= QUICC_BD_W;
  }

  return m;
}

static void *uec_if_bd_done_and_refill(
  uec_if_context *self,
  volatile quicc_bd *bd,
  struct mbuf *m,
  uint32_t status
)
{
  uint32_t error_flags = QUICC_BD_RX_LG | QUICC_BD_RX_NO | QUICC_BD_RX_SH
    | QUICC_BD_RX_CR | QUICC_BD_RX_OV;

  ++self->stats.rx_frames;

  if ((status & error_flags) == 0) {
    struct ifnet *ifp = &self->arpcom.ac_if;
    struct ether_header *eh = mtod(m, struct ether_header *);

    int sz = (int) QUICC_BD_LENGTH_GET(status) - ETHER_HDR_LEN - ETHER_CRC_LEN;

    m->m_len = sz;
    m->m_pkthdr.len = sz;
    m->m_data = mtod(m, char *) + ETHER_HDR_LEN;

    ether_input(ifp, eh, m);

    m = uec_if_get_mbuf(self);
  } else {
    self->stats.rx_length_violations += (status & QUICC_BD_RX_LG) != 0;
    self->stats.rx_non_octet_aligned_frames += (status & QUICC_BD_RX_NO) != 0;
    self->stats.rx_short_frames += (status & QUICC_BD_RX_SH) != 0;
    self->stats.rx_crc_errors += (status & QUICC_BD_RX_CR) != 0;
    self->stats.rx_overruns += (status & QUICC_BD_RX_OV) != 0;
  }

  bd->status = (status & QUICC_BD_W) | QUICC_BD_I | QUICC_BD_RX_E;
  bd->buffer = mtod(m, uint32_t);

  return m;
}

static uint32_t uec_if_bd_wait(
  uec_if_context *self,
  volatile quicc_bd *bd,
  uint32_t status_flags,
  uint32_t ucce_flags
)
{
  rtems_status_code sc = RTEMS_SUCCESSFUL;
  const quicc_ucf_context *ucf_context = &self->ucf_context;
  volatile quicc_ucf *ucf_regs = ucf_context->ucf_regs;
  uint32_t status = bd->status;
  bool done = (status & status_flags) == 0;

  while (!done) {
    ucf_regs->ucce = ucce_flags;
    ucf_regs->ucce;
    status = bd->status;
    done = (status & status_flags) == 0;
    if (!done) {
      rtems_interrupt_level level;
      rtems_event_set events;

      rtems_interrupt_disable(level);
      ucf_regs->uccm |= ucce_flags;
      rtems_interrupt_enable(level);

      sc = rtems_bsdnet_event_receive(
        UEC_IF_EVENT_IRQ,
        RTEMS_EVENT_ANY | RTEMS_WAIT,
        RTEMS_NO_TIMEOUT,
        &events
      );
      ASSERT_SC(sc);
      assert(events == UEC_IF_EVENT_IRQ);
    }
  }

  return status;
}

static void *uec_if_bd_process(
  void *arg,
  volatile quicc_bd *bd,
  void *bd_arg
)
{
  uec_if_context *self = arg;
  uint32_t status_flags = QUICC_BD_RX_E;
  uint32_t ucce_flags = UEC_IF_RX_UCCE_FLAGS;
  uint32_t status = uec_if_bd_wait(self, bd, status_flags, ucce_flags);

  return uec_if_bd_done_and_refill(self, bd, bd_arg, status);
}

static void uec_if_rx_task(void *arg)
{
  uec_if_context *self = arg;
  quicc_uec_context *uec_context = &self->uec_context;

  quicc_bd_rx_process(
    &uec_context->rx_bd_context,
    uec_if_bd_process,
    self
  );
}

static struct mbuf *uec_if_dequeue(struct ifnet *ifp, struct mbuf *m)
{
  struct mbuf *n = NULL;
  int size = 0;

  /* Dequeue */
  while (size <= 0) {
    if (m == NULL) {
      IF_DEQUEUE(&ifp->if_snd, m);

      if (m == NULL) {
        return m;
      }
    }

    size = m->m_len;

    if (size <= 0) {
      m = m_free(m);
    }
  }

  /* Discard empty successive fragments */
  n = m->m_next;
  while (n != NULL && n->m_len <= 0) {
    n = m_free(n);
  }
  m->m_next = n;

  return m;
}

static void uec_if_bd_compact(
  void *arg,
  void *discard_bd_arg,
  uint32_t *compact_bd_status,
  void **compact_bd_buffer,
  void **compact_bd_arg
)
{
  uec_if_context *self = arg;
  struct mbuf *discard_m = discard_bd_arg;
  struct mbuf *compact_m = *compact_bd_arg;

  if (compact_m == NULL) {
    ++self->stats.tx_compacts;

    compact_m = uec_if_get_mbuf(self);
    compact_m->m_len = 0;
  }

  int old_compact_len = compact_m->m_len;
  int discard_len = discard_m->m_len;
  int new_compact_len = old_compact_len + discard_len;
  struct mbuf *next = discard_m->m_next;

  *compact_bd_status = QUICC_BD_LENGTH_SET(*compact_bd_status, new_compact_len)
    | QUICC_BD_I | (next != NULL ? 0 : QUICC_BD_L);
  *compact_bd_buffer = mtod(compact_m, void *);
  *compact_bd_arg = compact_m;

  compact_m->m_len = new_compact_len;
  compact_m->m_next = next;

  memcpy(
    mtod(compact_m, char *) + old_compact_len,
    mtod(discard_m, void *),
    (size_t) discard_len
  );

  m_free(discard_m);
}

static void uec_if_bd_wait_and_free(void *arg, volatile quicc_bd *bd, void *bd_arg)
{
  uec_if_context *self = arg;
  struct mbuf *m = bd_arg;
  uint32_t status_flags = QUICC_BD_TX_R;
  uint32_t ucce_flags = UEC_IF_TX_UCCE_FLAGS;
  uint32_t status = uec_if_bd_wait(self, bd, status_flags, ucce_flags);

  if (m != NULL) {
    ++self->stats.tx_fragments;
    self->stats.tx_frames += (status & QUICC_BD_L) != 0;
    self->stats.tx_defer_indications += (status & QUICC_BD_TX_DEF) != 0;
    self->stats.tx_late_collisions += (status & QUICC_BD_TX_LC_PTP) != 0;
    self->stats.tx_retries += QUICC_BD_TX_RC_VID_GET(status) != 0;

    m_free(m);
  }
}

static void uec_if_tx_task(void *arg)
{
  rtems_status_code sc = RTEMS_SUCCESSFUL;
  uec_if_context *self = arg;
  struct ifnet *ifp = &self->arpcom.ac_if;
  quicc_bd_tx_context *context = &self->uec_context.tx_bd_context;

  while (true) {
    struct mbuf *m = NULL;
    rtems_event_set events;

    sc = rtems_bsdnet_event_receive(
      UEC_IF_EVENT_TX_START,
      RTEMS_EVENT_ANY | RTEMS_WAIT,
      RTEMS_NO_TIMEOUT,
      &events
    );
    ASSERT_SC(sc);
    assert(events == UEC_IF_EVENT_TX_START);

    while ((m = uec_if_dequeue(ifp, m)) != NULL) {
      struct mbuf *current = m;
      struct mbuf *next = current->m_next;
      uint32_t status = QUICC_BD_LENGTH(current->m_len)
        | QUICC_BD_I | (next != NULL ? 0 : QUICC_BD_L);

      m = next;

      quicc_bd_tx_submit_and_wait(
        context,
        status,
        mtod(current, void *),
        current,
        uec_if_bd_wait_and_free,
        uec_if_bd_compact,
        self
      );
    }

    ifp->if_flags &= ~IFF_OACTIVE;
  }
}

static void uec_if_pin_config(void)
{
  #ifdef MPC83XX_BOARD_BR_UID
    /* PHY reset */
    int phy_reset_pin = 36;
    uint32_t phy_reset_bit = BSP_BBIT32(phy_reset_pin % 32);
    volatile m83xxGPIORegisters_t *phy_reset_gpio = &mpc83xx.gpio[phy_reset_pin / 32];

    /* PHY configuration */
    int config0_pin = 16;
    int config1_pin = 17;
    int config2_pin = 19;
    int phyad0_pin = 24;
    int phyad1_pin = 23;
    int phyad2_pin = 22;
    uint32_t phy_cfg_bits = BSP_BBIT32(config0_pin)
      | BSP_BBIT32(config1_pin)
      | BSP_BBIT32(config2_pin)
      | BSP_BBIT32(phyad0_pin)
      | BSP_BBIT32(phyad1_pin)
      | BSP_BBIT32(phyad2_pin);
    volatile m83xxGPIORegisters_t *phy_cfg_gpio = &mpc83xx.gpio[0];

    rtems_interval tsr = (rtems_clock_get_ticks_per_second() + 100 - 1) / 100 + 1;
    rtems_interval tmdio = 2;
    volatile m83xxSysConRegisters_t *syscon = &mpc83xx.syscon;

    /* Disable pull-up for FEC1 block */
    syscon->gpr_1 |= BSP_BBIT32(1);

    /* Set GPIO pin function for FEC1 block */
    syscon->sicrl = BSP_BFLD32SET(syscon->sicrl, 0x2, 28, 29);

    /* Set PHY configuration pins to LOW */
    phy_cfg_gpio->gpimr &= ~phy_cfg_bits;
    phy_cfg_gpio->gpdr &= ~phy_cfg_bits;
    phy_cfg_gpio->gpdat &= ~phy_cfg_bits;
    phy_cfg_gpio->gpdir |= phy_cfg_bits;

    /* Set GPIO pin function for USB_D block */
    syscon->sicrh = BSP_BFLD32SET(syscon->sicrh, 0x2, 12, 13);

    phy_reset_gpio->gpimr &= ~phy_reset_bit;
    phy_reset_gpio->gpdr &= ~phy_reset_bit;
    phy_reset_gpio->gpdat &= ~phy_reset_bit;
    phy_reset_gpio->gpdir |= phy_reset_bit;

    rtems_task_wake_after(tsr);

    phy_reset_gpio->gpdat |= phy_reset_bit;

    rtems_task_wake_after(tmdio);

    /* Set FEC1 pin function for FEC1 block */
    syscon->sicrl = BSP_BFLD32SET(syscon->sicrl, 0x0, 28, 29);
  #endif
}

static int ksz8864rmn_smi_phy_addr(int smi_addr)
{
  return 0x6 | ((smi_addr & 0xc0) >> 3) | ((smi_addr & 0x20) >> 5);
}

static int ksz8864rmn_smi_reg_addr(int smi_addr)
{
  return smi_addr & 0x1f;
}

static uint8_t ksz8864rmn_smi_read(quicc_uec_context *uec_context, int smi_addr)
{
  int phy_addr = ksz8864rmn_smi_phy_addr(smi_addr);
  int reg_addr = ksz8864rmn_smi_reg_addr(smi_addr);

  return (uint8_t) quicc_uec_mii_read(uec_context, phy_addr, reg_addr);
}

static void ksz8864rmn_smi_write(quicc_uec_context *uec_context, int smi_addr, uint8_t value)
{
  int phy_addr = ksz8864rmn_smi_phy_addr(smi_addr);
  int reg_addr = ksz8864rmn_smi_reg_addr(smi_addr);

  quicc_uec_mii_write(uec_context, phy_addr, reg_addr, value);
}

static void ksz8864rmn_enable_switch(quicc_uec_context *uec_context)
{
  uint8_t chip_id = ksz8864rmn_smi_read(uec_context, 0);
  uint8_t revision_id = ksz8864rmn_smi_read(uec_context, 1);

  if (chip_id == 0x95 && revision_id == 0x40) {
    ksz8864rmn_smi_write(uec_context, 1, 0x1);
  }
}

static bool uec_if_is_phy_addr_valid(int phy_addr)
{
  return (phy_addr & 0x1f) == phy_addr;
}

static void uec_if_phy_init(uec_if_context *self)
{
  quicc_uec_context *uec_context = &self->uec_context;
  int phy_addr = self->uec_config.phy_address;

  if (uec_if_is_phy_addr_valid(phy_addr)) {
    uint16_t bmcr = quicc_uec_mii_read(uec_context, phy_addr, MII_BMCR);
    bmcr = (uint16_t) (bmcr & ~(BMCR_PDOWN | BMCR_ISO));
    quicc_uec_mii_write(uec_context, phy_addr, MII_BMCR, bmcr);
  } else {
    ksz8864rmn_enable_switch(uec_context);
  }
}

static void uec_if_interface_init(void *arg)
{
  uec_if_context *self = arg;
  struct ifnet *ifp = &self->arpcom.ac_if;

  uec_if_pin_config();

  quicc_irq_init(quicc_init());
  quicc_ucf_init(&self->ucf_context, &self->ucf_config);
  quicc_uec_init(&self->uec_context, &self->ucf_context, &self->uec_config);

  uec_if_phy_init(self);

  quicc_uec_set_mac_address(&self->uec_context, self->arpcom.ac_enaddr);
  quicc_uec_mac_enable(&self->uec_context, QUICC_DIR_RX_AND_TX);
  quicc_ucf_enable(&self->ucf_context, QUICC_DIR_RX_AND_TX);
  quicc_uec_start(&self->uec_context, QUICC_DIR_RX_AND_TX);

  assert(self->rx_task_id == RTEMS_ID_NONE);
  self->rx_task_id = rtems_bsdnet_newproc(
    "ntrx",
    4096,
    uec_if_rx_task,
    self
  );

  assert(self->tx_task_id == RTEMS_ID_NONE);
  self->tx_task_id = rtems_bsdnet_newproc(
    "nttx",
    4096,
    uec_if_tx_task,
    self
  );

  uec_if_interrupt_install(self);

  ifp->if_timer = 1;
  ifp->if_flags |= IFF_RUNNING;
}

static int uec_if_mdio_read(
  int phy_addr,
  void *arg,
  unsigned reg_addr,
  uint32_t *val
)
{
  uec_if_context *self = arg;
  quicc_uec_context *uec_context = &self->uec_context;

  /* FIXME */
  phy_addr = self->uec_config.phy_address;

  *val = quicc_uec_mii_read(uec_context, phy_addr, (int) reg_addr);

  return 0;
}

static int uec_if_mdio_write(
  int phy_addr,
  void *arg,
  unsigned reg_addr,
  uint32_t data
)
{
  uec_if_context *self = arg;
  quicc_uec_context *uec_context = &self->uec_context;

  /* FIXME */
  phy_addr = self->uec_config.phy_address;

  quicc_uec_mii_write(uec_context, phy_addr, (int) reg_addr, (uint16_t) data);

  return 0;
}

static bool uec_if_media_status(uec_if_context *self, int *media)
{
  struct ifnet *ifp = &self->arpcom.ac_if;

  *media = (int) IFM_MAKEWORD(0, 0, 0, self->uec_config.phy_address);

  return (*ifp->if_ioctl)(ifp, SIOCGIFMEDIA, (caddr_t) media) == 0;
}

static void uec_if_interface_stats(uec_if_context *self)
{
  if (uec_if_is_phy_addr_valid(self->uec_config.phy_address)) {
    int media = 0;
    bool media_ok = uec_if_media_status(self, &media);

    if (media_ok) {
      rtems_ifmedia2str(media, NULL, 0);
      printf("\n");
    } else {
      printf("PHY communication error\n");
    }
  }

  printf(
    "rx interrupt: %" PRIu32 "\n"
    "rx frames: %" PRIu32 "\n"
    "rx length violations: %" PRIu32 "\n"
    "rx non-octet aligned frames: %" PRIu32 "\n"
    "rx short frames: %" PRIu32 "\n"
    "rx CRC errors: %" PRIu32 "\n"
    "rx overruns: %" PRIu32 "\n"
    "tx interrupts: %" PRIu32 "\n"
    "tx fragments: %" PRIu32 "\n"
    "tx frames: %" PRIu32 "\n"
    "tx defer indications: %" PRIu32 "\n"
    "tx late collisions: %" PRIu32 "\n"
    "tx retries: %" PRIu32 "\n"
    "tx compacts: %" PRIu32 "\n",
    self->stats.rx_interrupts,
    self->stats.rx_frames,
    self->stats.rx_length_violations,
    self->stats.rx_non_octet_aligned_frames,
    self->stats.rx_short_frames,
    self->stats.rx_crc_errors,
    self->stats.rx_overruns,
    self->stats.tx_interrupts,
    self->stats.tx_fragments,
    self->stats.tx_frames,
    self->stats.tx_defer_indications,
    self->stats.tx_late_collisions,
    self->stats.tx_retries,
    self->stats.tx_compacts
  );
}

static void uec_if_reset_multicast_filter(uec_if_context *self)
{
  quicc_uec_context *uec_context = &self->uec_context;
  quicc_uec_rx_gparam *rx_global_param = uec_context->rx_global_param;
  quicc_direction dir = QUICC_DIR_RX_AND_TX;

  quicc_uec_config_mode_enter(uec_context, dir);

  rx_global_param->af_iaddr_h = 0;
  rx_global_param->af_iaddr_l = 0;
  rx_global_param->af_gaddr_h = 0;
  rx_global_param->af_gaddr_l = 0;

  quicc_uec_config_mode_leave(uec_context, dir);
}

static uint64_t uec_if_mac_addr_to_uint64(const uint8_t *addr)
{
  return (((uint64_t) addr [0]) << 40)
    | (((uint64_t) addr [1]) << 32)
    | (((uint64_t) addr [2]) << 24)
    | (((uint64_t) addr [3]) << 16)
    | (((uint64_t) addr [4]) << 8)
    | (((uint64_t) addr [5]) << 0);
}

static void uec_if_add_multicast_filter(uec_if_context *self)
{
  struct arpcom *ac = &self->arpcom;
  quicc_uec_context *uec_context = &self->uec_context;
  quicc_uec_rx_gparam *rx_global_param = uec_context->rx_global_param;
  struct ether_multistep step;
  struct ether_multi *enm;

  ETHER_FIRST_MULTI(step, ac, enm);
  while (enm != NULL) {
    uint64_t addrlo = uec_if_mac_addr_to_uint64(enm->enm_addrlo);
    uint64_t addrhi = uec_if_mac_addr_to_uint64(enm->enm_addrhi);

    while (addrlo <= addrhi) {
      uint32_t a0 = (uint8_t) (addrlo >> 40);
      uint32_t a1 = (uint8_t) (addrlo >> 32);
      uint32_t a2 = (uint8_t) (addrlo >> 24);
      uint32_t a3 = (uint8_t) (addrlo >> 16);
      uint32_t a4 = (uint8_t) (addrlo >> 8);
      uint32_t a5 = (uint8_t) (addrlo >> 0);
      rx_global_param->af_taddr_h = (a5 << 8) | (a4 << 0);
      rx_global_param->af_taddr_m_l =
        (a3 << 24) | (a2 << 16) | (a1 << 8) | (a0 << 0);

      quicc_uec_execute_command(
        uec_context,
        QUICC_CMD_FAST_SET_GROUP_ADDRESS,
        0
      );

      ++addrlo;
    }
    ETHER_NEXT_MULTI(step, enm);
  }
}

static int uec_if_multicast_control(
  uec_if_context *self,
  bool add,
  struct ifreq *ifr
)
{
  struct arpcom *ac = &self->arpcom;
  int eno = 0;

  if (add) {
    eno = ether_addmulti(ifr, ac);
  } else {
    eno = ether_delmulti(ifr, ac);
  }

  if (eno == ENETRESET) {
    eno = 0;
    uec_if_reset_multicast_filter(self);
    uec_if_add_multicast_filter(self);
  }

  return eno;
}

static void uec_if_enable_promiscous_mode(
  uec_if_context *self,
  bool enable
)
{
  if (enable != self->promisc) {
    const quicc_uec_context *uec_context = &self->uec_context;
    quicc_direction dir = QUICC_DIR_RX_AND_TX;

    self->promisc = enable;

    quicc_uec_config_mode_enter(uec_context, dir);
    quicc_uec_enable_promiscuous_mode(uec_context, enable);
    quicc_uec_config_mode_leave(uec_context, dir);
  }
}

static int uec_if_interface_ioctl(
  struct ifnet *ifp,
  ioctl_command_t cmd,
  caddr_t data
) {
  uec_if_context *self = ifp->if_softc;
  struct ifreq *ifr = (struct ifreq *) data;
  int eno = 0;

  switch (cmd)  {
    case SIOCGIFMEDIA:
    case SIOCSIFMEDIA:
      rtems_mii_ioctl(&self->mdio, self, cmd, (int *) data);
      break;
    case SIOCGIFADDR:
    case SIOCSIFADDR:
      ether_ioctl(ifp, cmd, data);
      break;
    case SIOCSIFFLAGS:
      uec_if_enable_promiscous_mode(self, (ifp->if_flags & IFF_PROMISC) != 0);

      if ((ifp->if_flags & IFF_RUNNING) == 0) {
        /* TODO: off */
      }

      if ((ifp->if_flags & IFF_UP) != 0) {
        ifp->if_flags |= IFF_RUNNING;
        /* TODO: init */
      }
      break;
    case SIOCADDMULTI:
    case SIOCDELMULTI:
      eno = uec_if_multicast_control(self, cmd == SIOCADDMULTI, ifr);
      break;
    case SIO_RTEMS_SHOW_STATS:
      uec_if_interface_stats(self);
      break;
    default:
      eno = EINVAL;
      break;
  }

  return eno;
}

static void uec_if_interface_start(struct ifnet *ifp)
{
  rtems_status_code sc = RTEMS_SUCCESSFUL;
  uec_if_context *self = ifp->if_softc;

  ifp->if_flags |= IFF_OACTIVE;

  sc = rtems_bsdnet_event_send(self->tx_task_id, UEC_IF_EVENT_TX_START);
  ASSERT_SC(sc);
}

static void uec_if_interface_watchdog(struct ifnet *ifp)
{
  uec_if_context *self = ifp->if_softc;
  const quicc_uec_context *uec_context = &self->uec_context;
  int phy_addr = self->uec_config.phy_address;

  if (uec_if_is_phy_addr_valid(phy_addr)) {
    uint16_t anlpar = quicc_uec_mii_read(uec_context, phy_addr, MII_ANLPAR);

    if (self->anlpar != anlpar) {
      quicc_direction dir = QUICC_DIR_RX_AND_TX;
      quicc_uec_interface_type type = self->uec_config.interface_type;
      quicc_uec_speed speed = QUICC_UEC_SPEED_10;
      bool full_duplex = false;

      self->anlpar = anlpar;

      if ((anlpar & ANLPAR_TX_FD) != 0) {
        full_duplex = true;
        speed = QUICC_UEC_SPEED_100;
      } else if ((anlpar & ANLPAR_T4) != 0) {
        speed = QUICC_UEC_SPEED_100;
      } else if ((anlpar & ANLPAR_TX) != 0) {
        speed = QUICC_UEC_SPEED_100;
      } else if ((anlpar & ANLPAR_10_FD) != 0) {
        full_duplex = true;
      }

      quicc_uec_config_mode_enter(uec_context, dir);
      quicc_uec_set_interface_mode(uec_context, type, speed, full_duplex);
      quicc_uec_config_mode_leave(uec_context, dir);
    }

    ifp->if_timer = UEC_IF_WATCHDOG_TIMEOUT;
  }
}

static uec_if_context uec_if_context_instance;

static void uec_if_attach(struct rtems_bsdnet_ifconfig *config)
{
  uec_if_context *self = &uec_if_context_instance;
  struct ifnet *ifp = &self->arpcom.ac_if;
  char *unit_name = NULL;
  int unit_number = rtems_bsdnet_parse_driver_name(config, &unit_name);

  /* Interrupt number */
  config->irno = 0;

  /* Device control */
  config->drv_ctrl = NULL;

  /* Receive unit number */
  if (config->rbuf_count < 32) {
    config->rbuf_count = 32;
  }

  /* Transmit unit number */
  if (config->xbuf_count < 32) {
    config->xbuf_count = 32;
  }

  /* UCF config */
  self->ucf_config.index = 0;
  self->ucf_config.rx_clk = QUICC_CLK_9;
  self->ucf_config.tx_clk = QUICC_CLK_10;
  self->ucf_config.type = QUICC_UCF_ETHERNET_FAST;

  /* UEC config */
  self->uec_config.speed = QUICC_UEC_SPEED_100;
  self->uec_config.interface_type = QUICC_UEC_INTERFACE_TYPE_MII;
  self->uec_config.rx_thread_count = QUICC_UEC_THREAD_COUNT_1;
  self->uec_config.tx_thread_count = QUICC_UEC_THREAD_COUNT_1;
  self->uec_config.rx_bd_count = (size_t) config->rbuf_count;
  self->uec_config.tx_bd_count = (size_t) config->xbuf_count;
  self->uec_config.max_rx_buf_len = MCLBYTES;
  self->uec_config.bd_arg = self;
  self->uec_config.fill_rx_bd = uec_if_bd_rx_fill;
  self->uec_config.phy_address = MPC83XX_NETWORK_INTERFACE_0_PHY_ADDR;

  /* Copy MAC address */
  memcpy(self->arpcom.ac_enaddr, config->hardware_address, ETHER_ADDR_LEN);

  /* Set interface data */
  ifp->if_softc = self;
  ifp->if_unit = (short) unit_number;
  ifp->if_name = unit_name;
  ifp->if_mtu = config->mtu > 0 ? (u_long) config->mtu : ETHERMTU;
  ifp->if_init = uec_if_interface_init;
  ifp->if_ioctl = uec_if_interface_ioctl;
  ifp->if_start = uec_if_interface_start;
  ifp->if_output = ether_output;
  ifp->if_watchdog = uec_if_interface_watchdog;
  ifp->if_flags = IFF_MULTICAST | IFF_BROADCAST | IFF_SIMPLEX;
  ifp->if_snd.ifq_maxlen = ifqmaxlen;
  ifp->if_timer = 0;

  /* MDIO */
  self->mdio.mdio_r = uec_if_mdio_read;
  self->mdio.mdio_w = uec_if_mdio_write;

  /* Attach the interface */
  if_attach(ifp);
  ether_ifattach(ifp);
}

/* FIXME */
#define quicc_uec_attach_detach BSP_tsec_attach

int quicc_uec_attach_detach(
  struct rtems_bsdnet_ifconfig *config,
  int attaching
) {
  if (attaching) {
    uec_if_attach(config);
  } else {
    /* TODO */
  }

  /* FIXME: Return value */
  return 0;
}
