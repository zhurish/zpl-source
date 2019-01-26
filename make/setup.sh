#!/bin/sh

#
# make and setup startup shell
#
BOOT_FILE=start-boot.sh

#VAL=`grep BINFILE $BOOT_FILE | awk '{print $3}'` 

if test "X" == "$1X" ;then
	echo "wrong argement, setup.sh FILE-NAME"
	exit 0
fi

sed -i 's/BINFILE=TAGET.*$/BINFILE='$1'/g' $BOOT_FILE

BOOT_FILE=../../default/x5b-app.sh
sed -i 's/TAGET/'$1'/g' $BOOT_FILE

