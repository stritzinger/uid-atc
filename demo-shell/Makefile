RTEMS_MAKEFILE_PATH = /opt/rtems-4.11/sparc-rtems4.11/sis

include $(RTEMS_MAKEFILE_PATH)/make/bsp.mk

APP = $(BUILDDIR)/app
APP_PIECES = init
APP_OBJS = $(APP_PIECES:%=$(BUILDDIR)/%.o)
APP_DEPS = $(APP_PIECES:%=$(BUILDDIR)/%.d)

all: $(BUILDDIR) $(APP).exe

$(BUILDDIR):
	mkdir $(BUILDDIR)

$(APP).exe: $(APP_OBJS)
	$(CCLINK) $^ -o $@

clean:
	rm -rf $(BUILDDIR)

-include $(APP_DEPS)
