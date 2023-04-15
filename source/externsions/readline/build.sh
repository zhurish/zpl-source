#!/bin/bash


INSTALL_DIR=$4
COMPILER=$3
RUNHOST=$2

#./configure --prefix=/home/zhurish/workspace/working/zpl-source/source/externsions/readline-8.1/_install --enable-shared --disable-install-examples --host=arm-gnueabihf-linux CC="/opt/gcc-linaro-7.5.0-arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc"

cd ncurses-6.2

if test "xlib" == "x$1" ;then
    ./configure --prefix=${INSTALL_DIR}/_install --enable-shared --disable-install-examples --without-curses --disable-bracketed-paste-default --disable-largefile \
        --without-ada --without-progs --host=${RUNHOST} CC="${COMPILER}"
    make all
fi

if test "xinstall" == "x$1" ;then
    make install
fi

if test "xclean" == "x$1" ;then
    make clean
fi