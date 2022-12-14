#!/bin/sh



TAGET=$1

#TAGET=`echo ${TAGET%.bin*}`

echo "packet... $1"

cd ../

if test ! -d app ; then 
	mkdir -p app/lib ;
	mkdir -p app/usr/lib ; 
fi 
    
if test -d debug/etc ; then 
	cp -arf debug/etc app/
	cp -arf make/upgradeapp.sh app/
fi 

if test -d debug/bin ; then 
	cp -arf debug/bin app/ 
fi 

if test -d debug/sbin ; then 
	cp -arf debug/sbin app/ 
fi 

if test -d debug/www ; then 
	cp -arf debug/www app/ 
fi 

if test -d debug/usr/lib ; then 
	cp -arf debug/usr/lib/*.so* app/usr/lib/ 
fi      

if test -d debug/usr/bin ; then 
	cp -arf debug/usr/bin app/usr/ 
fi 

if test -d debug/usr/sbin ; then 
	cp -arf debug/usr/sbin app/usr/ 
fi 

LIBSO=`ls debug/lib/*.so*`

if [ "ABCx" != "ABC$LIBSO" ] ; then 
	cp -arf debug/lib/*.so* app/lib/ 
fi 

tar -zcf ${TAGET}.tar.gz	app

rm -rf app