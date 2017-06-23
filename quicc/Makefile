RTEMS_ROOT = $(PWD)/../rtems-4.12
RTEMS_BSP = br_uid

include $(RTEMS_ROOT)/make/custom/$(RTEMS_BSP).mk

CPPFLAGS += -DNEW_NETWORK_STACK -DMPC83XX_BOARD_BR_UID
LIBS = -lbsd -lm

IMAGE = /tftpboot/app.img

LIB = $(BUILDDIR)/libquicc
LIB_PIECES =
LIB_PIECES += quicc
LIB_PIECES += quicc-irq
LIB_PIECES += quicc-ucc
LIB_PIECES += quicc-ucf
LIB_PIECES += quicc-uec
LIB_PIECES += quicc-bd
LIB_PIECES += quicc-uec-if
LIB_OBJS = $(LIB_PIECES:%=$(BUILDDIR)/%.o)
LIB_DEPS = $(LIB_PIECES:%=$(BUILDDIR)/%.d)

APP = $(BUILDDIR)/app
APP_PIECES = init
APP_OBJS = $(APP_PIECES:%=$(BUILDDIR)/%.o)
APP_DEPS = $(APP_PIECES:%=$(BUILDDIR)/%.d)

TEST = $(BUILDDIR)/test
TEST_PIECES = test-init test-quicc
TEST_OBJS = $(TEST_PIECES:%=$(BUILDDIR)/%.o)
TEST_DEPS = $(TEST_PIECES:%=$(BUILDDIR)/%.d)

all: $(BUILDDIR) $(LIB).a $(APP).exe $(TEST).exe

$(BUILDDIR):
	mkdir $(BUILDDIR)

$(LIB).a: $(LIB_OBJS)
	$(AR) rcu $@ $^
	$(RANLIB) $@

$(APP).exe: $(APP_OBJS) $(LIB).a
	$(CCLINK) -Wl,--start-group $^ $(LIBS) -Wl,--end-group -o $@

$(TEST).exe: $(TEST_OBJS) $(LIB).a
	$(CXXLINK) $^ -lgtest -o $@

%.bin: %.exe
	$(OBJCOPY) -O binary $^ $@

$(BUILDDIR)/app.bin.gz: $(BUILDDIR)/app.bin
	gzip -9 -f -c $^ > $@

$(BUILDDIR)/app.img: $(BUILDDIR)/app.bin.gz
	mkimage -A ppc -O rtems -T kernel -a 0x10000 -e 0x10000 -n RTEMS -d $^ $@ 1>/dev/null

$(IMAGE): $(BUILDDIR)/app.img
	test -d $(dir $(IMAGE)) && cp $^ $@ || true

quicc-regs.h: quicc-regs.txt convert.py
	python convert.py < $< > $@

install: $(BUILDDIR) $(LIB).a
	mkdir -p $(PROJECT_INCLUDE)/quicc
	install -m 644 $(LIB).a $(PROJECT_LIB)
	install -m 644 *.h $(PROJECT_INCLUDE)/quicc

clean:
	rm -rf $(BUILDDIR)

-include $(LIB_DEPS) $(APP_DEPS) $(TEST_DEPS)
