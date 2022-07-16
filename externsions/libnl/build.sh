#!/bin/bash


INSTALL_DIR=$4
COMPILER=$3
RUNHOST=$2

#./configure --prefix=/home/zhurish/Downloads/libnl-3.6.0/_install --enable-static  --disable-pthreads --enable-cli=no --disable-shared --host=arm-gnueabihf-linux CC="/opt/gcc-linaro-7.5.0-arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc"

cd libnl-3.6.0

if test "xlib" == "x$1" ;then
    ./configure --prefix=${INSTALL_DIR}/_install --enable-static --disable-shared \
        --disable-pthreads --enable-cli=no \
        --host=${RUNHOST} CC="${COMPILER}"
    make all
fi

if test "xinstall" == "x$1" ;then
    make install
fi

if test "xclean" == "x$1" ;then
    make clean
fi