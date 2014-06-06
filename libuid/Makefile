RTEMS_MAKEFILE_PATH = /opt/rtems-4.11/powerpc-rtems4.11/br_uid

include $(RTEMS_MAKEFILE_PATH)/make/bsp.mk

LIB = $(BUILDDIR)/libuid.a
LIB_PIECES =
LIB_PIECES += uid-init-flash
LIB_PIECES += uid-init-network
LIB_PIECES += uid-stage-1-update
LIB_OBJS = $(LIB_PIECES:%=$(BUILDDIR)/%.o)
LIB_DEPS = $(LIB_PIECES:%=$(BUILDDIR)/%.d)

all: $(BUILDDIR) $(LIB)

$(BUILDDIR):
	mkdir $(BUILDDIR)

$(LIB): $(LIB_OBJS)
	$(AR) rcu $@ $^
	$(RANLIB) $@

install:  all
	mkdir -p $(PROJECT_INCLUDE)/uid
	install -m 644 $(LIB) $(PROJECT_LIB)
	install -m 644 *.h $(PROJECT_INCLUDE)/uid

clean:
	rm -rf $(BUILDDIR)

-include $(LIB_DEPS)
