#!/bin/sh

#
# ./start-boot.sh /tmp/app SWP-V0.0.0.1.tar.bz2 SWP-V0.0.0.1.bin
#
echo "loading $2 to $1 $3"

mkdir $1 -p

cp /mnt/$2 /$1 -arf

cd $1

tar -jxvf $2

cd bin

./$3 -d
