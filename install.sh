#!/usr/bin/env bash
set -x

cd rtems-source-builder/rtems
../source-builder/sb-set-builder --prefix=/opt/rtems-4.12 4.12/rtems-powerpc
cd ../..
RTEMS_MAKEFILE_PATH=/opt/rtems-4.12/powerpc-rtems4.12/br_uid make clean install -f Makefile.rtems -C libyaffs2
cp br_uid.mk /opt/rtems-4.12/make/custom
make clean install -C libini
make clean install -C libdemo
make clean install -C libbed
make clean install -C libuid
