#!/usr/bin/env bash
set -x

prefix="$PWD/rtems-4.12"
git submodule update --init
cd rtems-source-builder/rtems
../source-builder/sb-set-builder --prefix="$prefix" 4.12/rtems-powerpc
cd ../..
export PATH="$prefix/bin:$PATH"
cd rtems
./bootstrap
cd ..
cd build
./build-br_uid.sh "$prefix"
cd ..
cd libbsd
git submodule update --init rtems_waf
../build/waf configure --prefix="$prefix" --rtems-bsps=powerpc/br_uid
../build/waf build
../build/waf install
cd ..
RTEMS_MAKEFILE_PATH="$prefix/powerpc-rtems4.12/br_uid" make clean install -f Makefile.rtems -C libyaffs2
cp build/br_uid.mk "$prefix/make/custom"
RTEMS_ROOT="$prefix" make clean install -C libini
RTEMS_ROOT="$prefix" make clean install -C libdemo
RTEMS_ROOT="$prefix" make clean install -C libbed
RTEMS_ROOT="$prefix" make clean install -C quicc
RTEMS_ROOT="$prefix" make clean install -C libuid
