RTEMS_MAKEFILE_PATH = /opt/rtems-4.11/powerpc-rtems4.11/br_uid

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
	$(CCLINK) $^ -o $@ -ldemo -lftpd -ltelnetd -lquicc -lnfs

$(BUILDDIR)/app.bin: $(APP)
	$(OBJCOPY) -O binary $^ $@

$(BUILDDIR)/app.bin.gz: $(BUILDDIR)/app.bin
	gzip -9 -f -c $^ > $@

$(BUILDDIR)/app.img: $(BUILDDIR)/app.bin.gz
	mkimage -A ppc -O rtems -T kernel -a 0x10000 -e 0x10000 -n RTEMS -d $^ $@ 1>/dev/null
#mkimage -A ppc -O rtems -T kernel -a 0x0 -e 0x4000 -n RTEMS -d $^ $@ 1>/dev/null

$(IMAGE): $(BUILDDIR)/app.img
	test -d $(dir $(IMAGE)) && cp $^ $@ || true

clean:
	rm -rf $(BUILDDIR)

-include $(APP_DEPS)
