RTEMS_ROOT = $(PWD)/../rtems-4.12
RTEMS_BSP = br_uid

include $(RTEMS_ROOT)/make/custom/$(RTEMS_BSP).mk

LIB = $(BUILDDIR)/libuid.a
LIB_PIECES =
LIB_PIECES += uid-init-flash
LIB_PIECES += uid-init-network
LIB_PIECES += uid-stage-1-update
LIB_PIECES += uid-stage-1-shell
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
