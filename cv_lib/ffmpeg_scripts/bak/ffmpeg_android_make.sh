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
ADDI_CFLAGS="-marm"

make -j16 && make install || exit 1
rm  libavcodec/reverse.o libavcodec/log2_tab.o libavformat/log2_tab.o libavformat/golomb_tab.o \
    libswresample/log2_tab.o libavfilter/log2_tab.o libswscale/log2_tab.o

$CC -o $PREFIX/libffmpeg.so -shared $LDFLAGS $EXTRA_LDFLAGS --sysroot=$SYSROOT -L $LIBPATH \
    libavutil/*.o  libavcodec/*.o   \
    libavformat/*.o libavfilter/*.o libswresample/*.o  \
    libswscale/*.o  compat/*.o

cp $PREFIX/libffmpeg.so $PREFIX/libffmpeg-debug.so
${STRIP} --strip-unneeded $PREFIX/libffmpeg.so

