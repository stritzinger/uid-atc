RTEMS_ROOT ?= $(PWD)/../rtems-4.12
RTEMS_BSP ?= br_uid

include $(RTEMS_ROOT)/make/custom/$(RTEMS_BSP).mk

# inih uses a slightly odd cast. The way it is used, it should be no problem.
# Therefore make an exception for this error.
CFLAGS += -Wno-error=cast-qual

# to support long lines:
CFLAGS += -DINI_USE_STACK=0
CFLAGS += -DINI_ALLOW_REALLOC=1
CFLAGS += -DINI_MAX_LINE=1024*32

LIB = $(BUILDDIR)/libinih.a
LIB_PIECES = $(wildcard inih/*.c)
LIB_OBJS = $(LIB_PIECES:%.c=$(BUILDDIR)/%.o)
LIB_DEPS = $(LIB_PIECES:%.c=$(BUILDDIR)/%.d)

all: $(BUILDDIR) $(LIB)

$(BUILDDIR):
	mkdir $(BUILDDIR)
	mkdir $(BUILDDIR)/inih

$(LIB): $(LIB_OBJS)
	$(AR) rcu $@ $^
	$(RANLIB) $@

install:  all
	mkdir -p $(PROJECT_INCLUDE)/inih
	install -m 644 $(LIB) $(PROJECT_LIB)
	install -m 644 inih/*.h $(PROJECT_INCLUDE)/inih

clean:
	rm -rf $(BUILDDIR)

-include $(LIB_DEPS)
