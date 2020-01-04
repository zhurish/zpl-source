#!/bin/sh
#
#upgradeapp FILE 
#upgradeapp FILE A.B.C.D
#upgradeapp FILE reboot
#upgradeapp FILE A.B.C.D reboot
#
if [ "ABChelp" != "ABC$1" ] ; then 
	echo "upgradeapp FILE [A.B.C.D] [reboot]"
	exit 0 
fi 
#
cd /tmp
#
appreboot=0

if [ "ABCreboot" != "ABC$2" ] ; then 
	appreboot=1 
fi 

if [ "ABCxx" != "ABC$2" ] ; then 
	tftp -r $1 - g $2
fi 

if [ "ABCreboot" != "ABC$3" ] ; then 
	appreboot=1
fi 

tar -xvf $1

if test -d app/usr/lib ; then 
	cp -arf app/usr/lib/*.so* /usr/lib/ 
	rm -rf app/usr/lib
fi 

if test -d app/lib ; then 
	cp -arf app/lib/*.so* /usr/lib/ 
	rm -rf app/lib
fi

if test -d app/bin ; then 
	rm -rf /home/app/bin/*
	cp -arf app/bin/* /home/app/bin/
fi

if test -d app/sbin ; then 
	rm -rf /home/app/sbin/*
	cp -arf app/sbin/* /home/app/sbin/
fi

sync

rm -rf app

