RTEMS_MAKEFILE_PATH = /opt/rtems-4.11/sparc-rtems4.11/sis
#RTEMS_MAKEFILE_PATH = /opt/rtems-4.11/nios2-rtems4.11/nios2_alt
#RTEMS_MAKEFILE_PATH = /opt/rtems-4.11/nios2-rtems4.11/BES3_FPGA_DEV_2AGX125_Q-1312-2.2
#RTEMS_MAKEFILE_PATH = /opt/rtems-4.11/nios2-rtems4.11/BES3_FPGA_DEV_2AGX125_Q-1312-2.4
#RTEMS_MAKEFILE_PATH = /opt/rtems-4.11/nios2-rtems4.11/BES3_FPGA_DEV_2AGX125_Q-1312-2.5
#RTEMS_MAKEFILE_PATH = /opt/rtems-4.11/nios2-rtems4.11/BES3_FPGA_DEV_2AGX125_Q-1312-2.6
#RTEMS_MAKEFILE_PATH = /opt/rtems-4.11/arm-rtemseabi4.11/lpc2362
#RTEMS_MAKEFILE_PATH = /opt/rtems-4.11/arm-rtemseabi4.11/lpc24xx_ea
#RTEMS_MAKEFILE_PATH = /opt/rtems-4.11/arm-rtemseabi4.11/lpc24xx_ncs_ram
#RTEMS_MAKEFILE_PATH = /opt/rtems-4.11/arm-rtemseabi4.11/lpc24xx_ncs_rom_ext
#RTEMS_MAKEFILE_PATH = /opt/rtems-4.11/arm-rtemseabi4.11/lpc24xx_ncs_rom_int
#RTEMS_MAKEFILE_PATH = /opt/rtems-4.11/arm-rtemseabi4.11/lpc32xx_mzx
#RTEMS_MAKEFILE_PATH = /opt/rtems-4.11/arm-rtemseabi4.11/lpc32xx_mzx
#RTEMS_MAKEFILE_PATH = /opt/rtems-4.11/arm-rtemseabi4.11/lpc32xx_mzx_stage_2
#RTEMS_MAKEFILE_PATH = /opt/rtems-4.11/arm-rtemseabi4.11/lpc32xx_phycore
#RTEMS_MAKEFILE_PATH = /opt/rtems-4.11/powerpc-rtems4.11/brs5l
#RTEMS_MAKEFILE_PATH = /opt/rtems-4.10/powerpc-rtems4.10/brs5l
#RTEMS_MAKEFILE_PATH = /opt/rtems-4.11/powerpc-rtems4.11/dp2
#RTEMS_MAKEFILE_PATH = /opt/rtems-4.11/powerpc-rtems4.11/mpc5674fevb
#RTEMS_MAKEFILE_PATH = /opt/rtems-4.11/powerpc-rtems4.11/mpc5566evb
#RTEMS_MAKEFILE_PATH = /opt/rtems-4.10/powerpc-rtems4.10/mpc5566evb
#RTEMS_MAKEFILE_PATH = /opt/rtems-4.11/powerpc-rtems4.11/gwlcfm
#RTEMS_MAKEFILE_PATH = /opt/rtems-4.11/powerpc-rtems4.11/mpc8313erdb
#RTEMS_MAKEFILE_PATH = /opt/rtems-4.11/powerpc-rtems4.11/mpc8309som
#RTEMS_MAKEFILE_PATH = /opt/rtems-4.11/powerpc-rtems4.11/pm520_ze30
#RTEMS_MAKEFILE_PATH = /opt/rtems-4.11/powerpc-rtems4.11/qoriq_p1020rdb
#RTEMS_MAKEFILE_PATH = /opt/rtems-4.11/powerpc-rtems4.11/qoriq_core_0
#RTEMS_MAKEFILE_PATH = /opt/rtems-4.11/powerpc-rtems4.11/qoriq_core_1

include $(RTEMS_MAKEFILE_PATH)/make/bsp.mk

CPPFLAGS += -Iinclude

LIB = $(BUILDDIR)/libdemo.a
LIB_PIECES = \
	demo-dir-iterate \
	demo-file-copy \
	demo-find-file \
	demo-ftpd \
	demo-ftpfs \
	demo-httpd \
	demo-loadelf \
	demo-network \
	demo-shell \
	demo-connect \
	demo-md5sum \
	demo-start \
	demo-start-ftpd \
	demo-start-ftpfs \
	demo-start-httpd \
	demo-start-loadelf \
	demo-start-network \
	demo-start-telnetd \
	demo-start-tftpfs \
	demo-start-netperf \
	demo-start-network-echo \
	demo-start-network-flood \
	demo-start-network-echo-mcast \
	demo-telnetd \
	demo-tftpfs \
	demo-usb-media \
	demo-wait \
	demo-mii-dump \
	load-elf \
	load-elf-desc \
	test-file-system \
	test-file-system-fat \
	test-block-devices-shell \
	usb-break-lpc24xx \
	usb-break-lpc32xx
LIB_OBJS = $(LIB_PIECES:%=$(BUILDDIR)/%.o)
LIB_DEPS = $(LIB_PIECES:%=$(BUILDDIR)/%.d)

all: $(BUILDDIR) $(LIB)

$(BUILDDIR):
	mkdir $(BUILDDIR)

$(LIB): $(LIB_OBJS)
	$(AR) rcu $@ $^
	$(RANLIB) $@

install:  all
	mkdir -p $(PROJECT_INCLUDE)/local
	install -m 644 $(LIB) $(PROJECT_LIB)
	install -m 644 *.h $(PROJECT_INCLUDE)/local
	install -m 644 include/*.h $(PROJECT_INCLUDE)
	install -m 644 include/sys/*.h $(PROJECT_INCLUDE)/sys

clean:
	rm -rf $(BUILDDIR)

-include $(LIB_DEPS)
