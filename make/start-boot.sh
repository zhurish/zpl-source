#!/bin/sh

#
#
#
if test "X" == "$1X" ;then
	TTYDEV=/dev/ttyS0
else
	TTYDEV=$1
fi


BINFILE=TAGET

mkdir /tmp/tmp -p

cd /app

if test "TAGET" == "${BINFILE}X" ;then
	echo "Can not execule '${BINFILE}' and startup Application."
fi

chmod +x $BINFILE
sync

echo "boot '$BINFILE -t $TTYDEV -d', Please waitting ... "

./$BINFILE -t /dev/ttyS1 -d

#exit 0

