RTEMS_ROOT = $(PWD)/../rtems-4.12
RTEMS_BSP = br_uid

include $(RTEMS_ROOT)/make/custom/$(RTEMS_BSP).mk

APP = $(BUILDDIR)/app
APP_PIECES =                    \
  chip_select                   \
  init                          \
  led                           \
  multiio                       \
  multiplexer                   \
  ncv7608                       \
  ncv7608_cmd_wr                \
  ncv7608_cmd_write             \
  ncv7608_cmd_read              \
  spi                           \
  spi_addressable               \
  i2cds2408                     \
  i2cds2408_cmd_read            \
  i2cds2408_cmd_write           \
  led_cmd_write                 \
  mio_init                      \
  mio_cmd_raw                   \
  mio_cmd_input                 \
  mio_cmd_irq                   \
  trf7970A_cmd_agc_off          \
  trf7970A_cmd_agc_on           \
  trf7970A_cmd_am               \
  trf7970A_cmd_init             \
  trf7970A_cmd_modulation_13    \
  trf7970A_cmd_modulation_30    \
  trf7970A_cmd_modulation_100   \
  trf7970A_cmd_pm               \
  trf7970A_cmd_raw              \
  trf7970A_cmd_readall          \
  trf7970A_cmd_reg_read         \
  trf7970A_cmd_reg_write        \
  trf7970A_cmd_tag              \
  trf7970A_cmd_tag_detect       \
  trf7970A_init                 \
  trf7970A_irq_print_unexpected

APP_OBJS = $(APP_PIECES:%=$(BUILDDIR)/%.o)
APP_DEPS = $(APP_PIECES:%=$(BUILDDIR)/%.d)

CFLAGS += $(DEPFLAGS) $(GCCFLAGS) -Wno-unused -O0

CXXFLAGS += $(CFLAGS)

all: $(BUILDDIR) $(APP).exe $(APP).bin

$(BUILDDIR):
	mkdir $(BUILDDIR)

$(APP).exe: $(APP_OBJS)
	$(CCLINK) $^ -o $@

$(APP).bin: $(APP).exe
	$(OBJCOPY) -O binary $^ $@

bin: all $(APP).bin

clean:
	rm -rf $(BUILDDIR)

-include $(APP_DEPS)
