#!/bin/sh

echo "build..."

cd make

chmod +x version.sh
VER=`./version.sh`

VER_FILE=board.cfg

VAL=`grep VERSION $VER_FILE | awk '{print $3}'` 

if test "X" == "$1X" ;then
	NVER=${VER}
else
	NVER=$1
fi


if test ! "X$VAL" == "X$NVER" ;then
	sed -i 's/VERSION.*$/VERSION = '$NVER'/g' $VER_FILE
fi
  
TAGET=SWP-$NVER

cd ../
              
make app ARCH_TYPE=MIPS

mkdir -p .tmp/lib

cp -arf debug/etc .tmp/
cp -arf debug/bin .tmp/
cp -arf debug/sbin .tmp/
cp -arf debug/lib/*.so .tmp/lib/
cp -arf debug/lib/*.so* .tmp/lib/

#cp -arf ${TAGET}* .tmp/sbin/

tar -jcf ${TAGET}.tar.bz2	.tmp

rm -rf .tmp