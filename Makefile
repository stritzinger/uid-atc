PREFIX = $(PWD)/rtems-4.12

.PHONY: libbsd libyaffs2 libini libdemo libbed quicc libuid

all: git rsb bsp libbsd libyaffs2 libini libdemo libbed quicc libuid

git:
	git submodule update --init

rsb:
	cd rtems-source-builder/rtems && ../source-builder/sb-set-builder --prefix=$(PREFIX) 4.12/rtems-powerpc

bsp:
	cd rtems && PATH="$(PREFIX)/bin:$$PATH" ./bootstrap
	cd build && PATH="$(PREFIX)/bin:$$PATH" ./build-br_uid.sh $(PREFIX)

libbsd:
	cd libbsd && git submodule update --init rtems_waf
	cd libbsd && ../build/waf configure --prefix=$(PREFIX) --rtems-bsps=powerpc/br_uid
	cd libbsd && ../build/waf build
	cd libbsd && ../build/waf install

$(PREFIX)/make/custom/br_uid.mk: build/br_uid.mk
	cp $^ $@

libyaffs2:
	PATH="$(PREFIX)/bin:$$PATH" RTEMS_MAKEFILE_PATH=$(PREFIX)/powerpc-rtems4.12/br_uid make clean install -f Makefile.rtems -C libyaffs2

libini: $(PREFIX)/make/custom/br_uid.mk
	PATH="$(PREFIX)/bin:$$PATH" make clean install -C $@ RTEMS_ROOT=$(PREFIX)

libdemo: $(PREFIX)/make/custom/br_uid.mk
	PATH="$(PREFIX)/bin:$$PATH" make clean install -C $@ RTEMS_ROOT=$(PREFIX)

libbed: $(PREFIX)/make/custom/br_uid.mk
	PATH="$(PREFIX)/bin:$$PATH" make clean install -C $@ RTEMS_ROOT=$(PREFIX)

quicc: $(PREFIX)/make/custom/br_uid.mk
	PATH="$(PREFIX)/bin:$$PATH" make clean install -C $@ RTEMS_ROOT=$(PREFIX)

libuid: $(PREFIX)/make/custom/br_uid.mk
	PATH="$(PREFIX)/bin:$$PATH" make clean install -C $@ RTEMS_ROOT=$(PREFIX)
