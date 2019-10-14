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

LDFLAGS="-lm -lz -Wl,-soname=libffmpeg.so,-z,noexecstack"

CPU=arm
PREFIX=ffout
ADDI_CFLAGS=" -marm "
ADDI_LDFLAGS=
echo " "
echo "please wait..."
echo " "

#cd $FFMPEG_SRC_PATH
rm ./$PREFIX -rf
make clean

echo " "
echo "preparing to configure..."
echo " "

./configure \
--prefix=$PREFIX \
--target-os=android \
--enable-shared \
--disable-static \
--disable-doc \
--disable-asm \
--enable-debug \
--enable-small \
--disable-ffmpeg \
--disable-ffplay \
--disable-ffprobe \
--enable-gpl \
--enable-libx264 \
--cross-prefix=$TOOLCHAIN/bin/arm-linux-androideabi- \
--arch=arm \
--enable-cross-compile \
--sysroot=$SYSROOT \
--extra-cflags="-Os -fpic  -I/root/cqj/lib264/include $ADDI_CFLAGS" \
--extra-ldflags=" -L/root/cqj/lib264/lib  $ADDI_LDFLAGS " \
$ADDITIONAL_CONFIGURE_FLAG

sed -i   's/HAVE_STRUCT_IP_MREQ_SOURCE 1/HAVE_STRUCT_IP_MREQ_SOURCE 0/g' config.h
#sed -i   's/HAVE_LOG2F 1/HAVE_LOG2F 0/g' config.h
