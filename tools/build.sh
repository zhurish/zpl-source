#!/bin/sh

echo "build..."

cd ../

mkdir -p .tmp/lib

cp -arf debug/etc .tmp/
cp -arf debug/bin .tmp/
cp -arf debug/sbin .tmp/

case $1 in
	X86)
	cp -arf debug/lib/*.so .tmp/lib/
	cp -arf debug/lib/*.so* .tmp/lib/
	;;
	*)
	cp -arf debug/armlib/*.so .tmp/lib/
	cp -arf debug/armlib/*.so* .tmp/lib/
	;;
esac

cp -arf $2 .tmp/bin/

tar -jcf $2.tar.bz2	.tmp

rm -rf .tmp