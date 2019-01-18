#!/bin/sh

cd /tmp

case $1 in
	"lan")
	ifconfig eth0.2 down
	ifconfig br-lan:1 inet 192.168.2.101
	tftp -r SWP-V0.0.1.bin -g $2
	sync
	;;
	"wan")
	tftp -r SWP-V0.0.1.bin -g $2
	sync
	;;
	**)
esac

chmod +x SWP-V0.0.1.bin
./SWP-V0.0.1.bin -d
