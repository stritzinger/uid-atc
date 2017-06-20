RTEMS_ROOT = /opt/rtems-4.12
RTEMS_BSP = br_uid

include $(RTEMS_ROOT)/make/custom/$(RTEMS_BSP).mk

APP = $(BUILDDIR)/app
APP_PIECES = stage-1-init
APP_OBJS = $(APP_PIECES:%=$(BUILDDIR)/%.o)
APP_DEPS = $(APP_PIECES:%=$(BUILDDIR)/%.d)

BOOTSTRAP = $(BUILDDIR)/bootstrap
BOOTSTRAP_PIECES = bootstrap
BOOTSTRAP_OBJS = $(BOOTSTRAP_PIECES:%=$(BUILDDIR)/%.o)
BOOTSTRAP_DEPS = $(BOOTSTRAP_PIECES:%=$(BUILDDIR)/%.d)

STAGE_1 = $(BUILDDIR)/stage-1
STAGE_1_PIECES = stage-1-start
STAGE_1_PIECES += stage-1-init
STAGE_1_OBJS = $(STAGE_1_PIECES:%=$(BUILDDIR)/%.o)
STAGE_1_DEPS = $(STAGE_1_PIECES:%=$(BUILDDIR)/%.d)

STAGE_1_LOADER = $(BUILDDIR)/stage-1-loader
STAGE_1_LOADER_PIECES = stage-1-loader
STAGE_1_LOADER_OBJS = $(STAGE_1_LOADER_PIECES:%=$(BUILDDIR)/%.o)
STAGE_1_LOADER_DEPS = $(STAGE_1_LOADER_PIECES:%=$(BUILDDIR)/%.d)

all: $(BUILDDIR) $(BOOTSTRAP).bin $(STAGE_1).bin $(STAGE_1_LOADER).exe $(APP).exe README.html

$(BUILDDIR):
	mkdir $(BUILDDIR)

$(APP).exe: $(APP_OBJS)
	$(CCLINK) $^ -luid -lquicc -ldemo -lbed -lyaffs2 -lftpd -lnfs -lini -o $@

$(BOOTSTRAP).exe: $(BOOTSTRAP_OBJS)
	$(CCLINK) $^ -qnolinkcmds -T linkcmds.bootstrap -o $@

$(STAGE_1).exe: $(STAGE_1_OBJS)
	$(CCLINK) $^ -luid -lquicc -ldemo -lbed -lyaffs2 -lftpd -lnfs -lini -qnolinkcmds -T linkcmds.stage-1 -Wl,--wrap,boot_card -o $@

$(STAGE_1_LOADER).exe: $(STAGE_1_LOADER_OBJS) $(BUILDDIR)/stage-1-payload.o
	$(CCLINK) $^ -luid -lbed -o $@

$(BUILDDIR)/stage-1-payload.o: $(BUILDDIR)/stage-1.bin
	cd $(BUILDDIR) && $(OBJCOPY) -I binary -O elf32-powerpc -B powerpc stage-1.bin stage-1-payload.o

%.bin: %.exe
	$(OBJCOPY) -O binary --gap-fill 0xff $< $@

%.html: %.txt
	asciidoc $<

clean:
	rm -rf $(BUILDDIR)

-include $(APP_DEPS) $(STAGE_1_DEPS) $(STAGE_1_LOADER_DEPS) $(BOOTSTRAP_DEPS)
