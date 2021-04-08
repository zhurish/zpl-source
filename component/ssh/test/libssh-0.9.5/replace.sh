#! /bin/bash


OLDTXT=$2
NEWTXT=$3
IGENO="multimedia"

function scan_dir() {
    for file in `ls $1` #注意此处这是两个反引号，表示运行系统命令
    do
        if test $IGENO != $file
        then
            if [ -d $1"/"$file ] #注意此处之间一定要加上空格，否则会报错
            then
                scan_dir $1"/"$file
            else
                #echo $1"/"$file #在此处处理文件即可
                sed -i 's/'$OLDTXT'/'$NEWTXT'/g' $1"/"$file
            fi
        fi
    done
}

scan_dir $1