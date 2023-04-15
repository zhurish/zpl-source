#!/bin/bash


INSTALL_DIR=$4
COMPILER=$3
RUNHOST=$2

FILEDIR=openssl-3.1.0-alpha1

#CC=/opt/toolchain/toolchain-mipsel_24kc_gcc-7.3.0_glibc/bin/mipsel-openwrt-linux-gnu-gcc ./config enable-shared --prefix=/home/zhurish/workspace/SWPlatform/externsions/openssl/mipsl

cd $FILEDIR

if test "xlib" == "x$1" ;then
    echo "./config shared --prefix=${INSTALL_DIR}/_install no-threads CC=\"${COMPILER}"\"
    ./config shared --prefix=${INSTALL_DIR}/_install \
        no-threads \
        CC="${COMPILER} "
    make all
fi

if test "xinstall" == "x$1" ;then
    make install
fi

if test "xclean" == "x$1" ;then
    make clean
    make distclean
fi