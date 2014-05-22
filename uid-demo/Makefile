#RTEMS_MAKEFILE_PATH = /scratch/install-gcc-git/powerpc-rtems4.11/br_uid
RTEMS_MAKEFILE_PATH = /opt/rtems-4.11/powerpc-rtems4.11/br_uid

include $(RTEMS_MAKEFILE_PATH)/make/bsp.mk

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
  mio_init                      \
  mio_cmd_raw                   \
  mio_cmd_input                 \
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
