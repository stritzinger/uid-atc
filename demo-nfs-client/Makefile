RTEMS_MAKEFILE_PATH = /opt/rtems-4.11/powerpc-rtems4.11/mpc8309som

include $(RTEMS_MAKEFILE_PATH)/make/bsp.mk

APP = $(BUILDDIR)/app.exe
APP_PIECES = init
APP_OBJS = $(APP_PIECES:%=$(BUILDDIR)/%.o)
APP_DEPS = $(APP_PIECES:%=$(BUILDDIR)/%.d)

all: $(BUILDDIR) $(APP)

$(BUILDDIR):
	mkdir $(BUILDDIR)

$(APP): $(APP_OBJS)
	$(CCLINK) $^ -ldemo -lnfs -lquicc -o $@

clean:
	rm -rf $(BUILDDIR)

-include $(APP_DEPS)
