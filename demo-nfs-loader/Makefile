RTEMS_ROOT = $(PWD)/../rtems-4.12
RTEMS_BSP = br_uid

include $(RTEMS_ROOT)/make/custom/$(RTEMS_BSP).mk

APP = $(BUILDDIR)/app
APP_PIECES = init
APP_OBJS = $(APP_PIECES:%=$(BUILDDIR)/%.o)
APP_DEPS = $(APP_PIECES:%=$(BUILDDIR)/%.d)

all: $(BUILDDIR) /scratch/nfs/br_uid.bin

$(BUILDDIR):
	mkdir $(BUILDDIR)

$(APP).exe: $(APP_OBJS)
	$(CCLINK) $^ -ldemo -lnfs -lquicc -o $@

%.bin: %.exe
	$(OBJCOPY) -O binary $< $@

/scratch/nfs/br_uid.bin: $(APP).bin
	cp $< $@

clean:
	rm -rf $(BUILDDIR)

-include $(APP_DEPS)
