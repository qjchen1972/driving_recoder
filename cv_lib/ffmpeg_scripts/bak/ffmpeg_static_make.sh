#!/bin/bash

FFMPEG_SRC_PATH=$(cd `dirname $0`; pwd)
SYSROOT=/root/cqj/ffmpeg/work/toolchain/android/linux-x86_64/ndk-r16/android-14/arm/sysroot
LIBPATH=/root/cqj/ffmpeg/work/toolchain/android/linux-x86_64/ndk-r16/android-14/arm/sysroot/usr/lib
TOOLCHAIN=/root/cqj/ffmpeg/work/toolchain/android/linux-x86_64/ndk-r16/android-14/arm


export PATH=$TOOLCHAIN/bin:$PATH
export CROSS_PREFIX=arm-linux-androideabi-
export CC="${CROSS_PREFIX}gcc "
export CXX=${CROSS_PREFIX}g++
export LD=${CROSS_PREFIX}ld
export AR=${CROSS_PREFIX}ar
export STRIP=${CROSS_PREFIX}strip

LDFLAGS="-lm -lz -Wl,-soname=libffmpeg.so,-z,noexecstack -lx264 -L/root/cqj/lib264/lib"

CPU=arm
PREFIX=ffout
ADDI_CFLAGS="-marm"

make -j8 && make install || exit 1

$TOOLCHAIN/bin/arm-linux-androideabi-ar  d ffout/lib/libavfilter.a  log2_tab.o
$TOOLCHAIN/bin/arm-linux-androideabi-ar  d ffout/lib/libavcodec.a  log2_tab.o reverse.o
$TOOLCHAIN/bin/arm-linux-androideabi-ar  d ffout/lib/libswresample.a  log2_tab.o
$TOOLCHAIN/bin/arm-linux-androideabi-ar  d ffout/lib/libavformat.a  log2_tab.o golomb_tab.o
$TOOLCHAIN/bin/arm-linux-androideabi-ar  d ffout/lib/libswscale.a  log2_tab.o
$TOOLCHAIN/bin/arm-linux-androideabi-ar  d ffout/lib/libavdevice.a  reverse.o


$TOOLCHAIN/bin/arm-linux-androideabi-ld \
-rpath-link=$SYSROOT/usr/lib \
-L$SYSROOT/usr/lib \
-L$PREFIX/lib \
-L/root/cqj/lib264/lib \
-soname libffmpeg.so -shared -nostdlib  -Bsymbolic --whole-archive --no-undefined -o \
$PREFIX/libffmpeg.so \
ffout/lib/libavcodec.a \
ffout/lib/libavfilter.a \
ffout/lib/libswresample.a \
ffout/lib/libavformat.a \
ffout/lib/libavutil.a \
ffout/lib/libswscale.a \
ffout/lib/libavdevice.a \
ffout/lib/libpostproc.a \
../lib264/lib/libx264.a \
-lc -lm -lz -ldl -llog --dynamic-linker=/system/bin/linker \
$TOOLCHAIN/lib/gcc/arm-linux-androideabi/4.9.x/libgcc.a

