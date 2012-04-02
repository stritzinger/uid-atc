RTEMS_MAKEFILE_PATH = /opt/rtems-4.11/sparc-rtems4.11/sis

include $(RTEMS_MAKEFILE_PATH)/make/bsp.mk

IMAGE = /tftpboot/app.img

APP = $(BUILDDIR)/app.exe
APP_PIECES = init
APP_OBJS = $(APP_PIECES:%=$(BUILDDIR)/%.o)
APP_DEPS = $(APP_PIECES:%=$(BUILDDIR)/%.d)

all: $(BUILDDIR) $(APP)

$(BUILDDIR):
	mkdir $(BUILDDIR)

$(APP): $(APP_OBJS)
	$(CCLINK) $^ -o $@ -ldemo -lftpd -ltelnetd -lnfs

$(BUILDDIR)/app.bin: $(APP)
	$(OBJCOPY) -O binary $^ $@

$(BUILDDIR)/app.bin.gz: $(BUILDDIR)/app.bin
	gzip -9 -f -c $^ > $@

$(BUILDDIR)/app.img: $(BUILDDIR)/app.bin.gz
	mkimage -A ppc -O rtems -T kernel -a 0x10000 -e 0x10000 -n RTEMS -d $^ $@ 1>/dev/null

$(IMAGE): $(BUILDDIR)/app.img
	test -d $(dir $(IMAGE)) && cp $^ $@ || true

clean:
	rm -rf $(BUILDDIR)

-include $(APP_DEPS)
