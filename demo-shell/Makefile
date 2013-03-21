RTEMS_MAKEFILE_PATH = /opt/rtems-4.11/powerpc-rtems4.11/br_uid

include $(RTEMS_MAKEFILE_PATH)/make/bsp.mk

APP = $(BUILDDIR)/app
APP_PIECES = init
APP_OBJS = $(APP_PIECES:%=$(BUILDDIR)/%.o)
APP_DEPS = $(APP_PIECES:%=$(BUILDDIR)/%.d)

all: $(BUILDDIR) /scratch/nfs/br_uid.bin

$(BUILDDIR):
	mkdir $(BUILDDIR)

$(APP).exe: $(APP_OBJS)
	$(CCLINK) $^ -o $@

%.bin: %.exe
	$(OBJCOPY) -O binary $< $@

/scratch/nfs/br_uid.bin: $(APP).bin
	cp $< $@

clean:
	rm -rf $(BUILDDIR)

-include $(APP_DEPS)
