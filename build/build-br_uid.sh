#!/usr/bin/env bash
set -x

# Variables
rtems_version="4.12"
bsp_name="br_uid"
source_dir="$PWD/../rtems"
build_dir="b-$bsp_name"
target="powerpc-rtems$rtems_version"
prefix="${1:-"$PWD/../rtems-$rtems_version"}"

# Create empty build directory
rm -rf "$build_dir"
mkdir -p "$build_dir"
cd "$build_dir"

# Configure
"$source_dir/configure" \
	"--target=$target" \
	"--prefix=$prefix" \
	"--enable-rtemsbsp=$bsp_name" \
	--enable-maintainer-mode \
	--disable-tests \
	--enable-posix \
	--disable-networking \
	"BSP_PRINT_EXCEPTION_CONTEXT=1"

# Make
make
make install
