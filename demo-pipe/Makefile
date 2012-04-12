#RTEMS_MAKEFILE_PATH = /opt/rtems-4.11/powerpc-rtems4.11/mpc8309som
RTEMS_MAKEFILE_PATH = /opt/rtems-4.11/sparc-rtems4.11/sis

include $(RTEMS_MAKEFILE_PATH)/make/bsp.mk

APP = $(BUILDDIR)/app.exe
APP_PIECES = init
APP_OBJS = $(APP_PIECES:%=$(BUILDDIR)/%.o)
APP_DEPS = $(APP_PIECES:%=$(BUILDDIR)/%.d)

all: $(BUILDDIR) $(APP)

$(BUILDDIR):
	mkdir $(BUILDDIR)

$(APP): $(APP_OBJS)
	$(CCLINK) $^ -ldemo -o $@

clean:
	rm -rf $(BUILDDIR)

-include $(APP_DEPS)
