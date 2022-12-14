#!/bin/sh

cd ../

VER_FILE=include/gitversion.h

if test -d ".git"; then

	VER=`git log -1 --format=%s | awk '{print $1}'`
	COMMIT=`git log -1 --format=\"%H\"`
	RELEASE=`cat .git/HEAD | awk -F / '{print $3}'`
	DATE=`date '+%Y%m%d%H%M%S'`
	#RELEASE=`expr substr $RELEASE 1 4`
	
	if test -f $VER_FILE; then
		touch $VER_FILE
	fi
	
	if grep GIT_VER $VER_FILE >/dev/null 2>&1; then
		# VER change
        VAL=`grep GIT_VERSION $VER_FILE | awk '{print $3}'` 
        if test ! $VAL == \"$VER\" ;then
                sed -i 's/GIT_VERSION.*$/GIT_VERSION	"'$VER'"/g' $VER_FILE
        fi
		# RELEASE change
        VAL=`grep GIT_RELEASE $VER_FILE | awk '{print $3}'`
        if test ! $VAL == \"$RELEASE\" ;then
                sed -i 's/GIT_RELEASE.*$/GIT_RELEASE	"'$RELEASE'"/g' $VER_FILE
        fi
		# COMMIT change
        VAL=`grep GIT_COMMIT $VER_FILE | awk '{print $3}'`
        if test ! $VAL == $COMMIT ;then
                sed -i 's/GIT_COMMIT.*$/GIT_COMMIT	'$COMMIT'/g' $VER_FILE
        fi

		# MAKE DATE change
        VAL=`grep MAKE_DATE $VER_FILE | awk '{print $3}'`
        if test ! $VAL == \"$DATE\" ;then
                sed -i 's/MAKE_DATE.*$/MAKE_DATE	"'$DATE'"/g' $VER_FILE
        fi

	else # GIT_VER
       echo "#define GIT_VERSION 	\"$VER\"" >> $VER_FILE
       echo "#define GIT_RELEASE 	\"$RELEASE\"" >> $VER_FILE
       echo "#define GIT_COMMIT 	\"$COMMIT\"" >> $VER_FILE
       echo "#define MAKE_DATE 		\"$DATE\"" >> $VER_FILE
	fi # GIT_VER

else # .git
	if test -f $VER_FILE; then
		touch $VER_FILE
	fi
	echo "Warning:no git version infomation, you should ne setup version string in $VER_FILE"
	echo " Example:
	#define GIT_VERSION 	\"V0.0.0.9\"
	#define GIT_RELEASE 	\"master\"
	#define GIT_COMMIT 		\"00000000\" 
	#define MAKE_DATE		\"$DATE\" >> $VER_FILE"
fi # .git


BOARD_FILE=make/board.cfg

VAL=`grep VERSION $BOARD_FILE | awk '{print $3}'` 

if test "X" == "$1X" ;then
	NVER=${VER}
else
	NVER=$1
fi

if test ! "X$VAL" == "X$NVER" ;then
	sed -i 's/VERSION.*$/VERSION = '$NVER'/g' $BOARD_FILE
fi


#
# make and setup startup shell
#
#START_APPSH=./startup/etc/x5b-app.sh
#echo "#!/bin/sh" > $START_APPSH
#echo "mkdir /tmp/tmp -p" >> $START_APPSH
#echo "cd /app" >> $START_APPSH
#echo "chmod +x SWP-${VER}.bin" >> $START_APPSH
#echo "sleep 5" >> $START_APPSH
#echo "./TimerMgr &" >> $START_APPSH
#echo "sleep 1" >> $START_APPSH
#echo "./SWP-${VER}.bin -t /dev/ttyS1 -d" >> $START_APPSH
#echo "sleep 2" >> $START_APPSH
#echo "./VmrMgr &" >> $START_APPSH

echo "$VER"

