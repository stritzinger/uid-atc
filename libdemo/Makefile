RTEMS_ROOT = $(PWD)/../rtems-4.12
RTEMS_BSP = br_uid

include $(RTEMS_ROOT)/make/custom/$(RTEMS_BSP).mk

CPPFLAGS += -Iinclude

LIB = $(BUILDDIR)/libdemo.a
LIB_PIECES = \
	demo-dir-iterate \
	demo-file-copy \
	demo-find-file \
	demo-ftpd \
	demo-ftpfs \
	demo-loadelf \
	demo-network \
	demo-shell \
	demo-connect \
	demo-md5sum \
	demo-start \
	demo-start-ftpd \
	demo-start-ftpfs \
	demo-start-loadelf \
	demo-start-network \
	demo-start-telnetd \
	demo-start-tftpfs \
	demo-start-netperf \
	demo-start-network-echo \
	demo-start-network-flood \
	demo-start-network-echo-mcast \
	demo-start-network-media-status \
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
# FIXME:
# demo-httpd and demo-start-httpd not working due to API changes in mongoose
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
