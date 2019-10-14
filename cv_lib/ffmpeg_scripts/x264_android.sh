#!/bin/bash

FFMPEG_SRC_PATH=$(cd `dirname $0`; pwd)
SYSROOT=/root/cqj/ffmpeg/work/toolchain/android/linux-x86_64/ndk-r16/android-14/arm/sysroot
LIBPATH=/root/cqj/ffmpeg/work/toolchain/android/linux-x86_64/ndk-r16/android-14/arm/sysroot/usr/lib
TOOLCHAIN=/root/cqj/ffmpeg/work/toolchain/android/linux-x86_64/ndk-r16/android-14/arm


export PATH=$TOOLCHAIN/bin:$PATH
echo $PATH
export CROSS_PREFIX=arm-linux-androideabi-
export CC="${CROSS_PREFIX}gcc "
export CXX=${CROSS_PREFIX}g++
export LD=${CROSS_PREFIX}ld
export AR=${CROSS_PREFIX}ar
export STRIP=${CROSS_PREFIX}strip

LDFLAGS="-lm -lz -Wl,-z,noexecstack"

CPU=arm
PREFIX=../lib264
ADDI_CFLAGS="-marm"

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
--enable-shared \
--enable-static \
--enable-pic \
--disable-asm \
--disable-cli \
--disable-debug \
--cross-prefix=$TOOLCHAIN/bin/arm-linux-androideabi- \
--host=arm-linux-androideabi \
--sysroot=$SYSROOT \
--extra-cflags="-fPIC -D__ANDROID_API__=14 -D__thumb__ $ADDI_CFLAGS " \
--extra-ldflags="$LDFLAGS "

sed -i   's/HAVE_LOG2F 1/HAVE_LOG2F 0/g' config.h

make 
make install
