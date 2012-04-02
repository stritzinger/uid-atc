RTEMS_MAKEFILE_PATH = /opt/rtems-4.11/powerpc-rtems4.11/mpc8309som

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
