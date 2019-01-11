#!/bin/sh



TAGET=$1

TAGET=`echo ${TAGET%.bin*}`

echo "packet... $1"

cd ../

if test ! -d .tmp ; then 
	mkdir -p .tmp ; 
fi 
        
cp -arf debug/etc .tmp/
cp -arf debug/bin .tmp/
cp -arf debug/sbin .tmp/
cp -arf debug/lib/*.so .tmp/lib/
cp -arf debug/lib/*.so* .tmp/lib/

tar -jcf ${TAGET}.tar.bz2	.tmp

rm -rf .tmp