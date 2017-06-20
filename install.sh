#!/usr/bin/env bash
set -x

cp br_uid.mk /opt/rtems-4.12/make/custom
RTEMS_MAKEFILE_PATH=/opt/rtems-4.12/powerpc-rtems4.12/br_uid make clean install -f Makefile.rtems -C libyaffs2
make clean install -C libini
make clean install -C libdemo
make clean install -C libuid
