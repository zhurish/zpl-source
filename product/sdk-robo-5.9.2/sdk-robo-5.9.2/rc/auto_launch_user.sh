#!/bin/sh
cd /broadcom

grep -q BCM9562 /proc/cpuinfo
if [ $? == 0 ] ; then
    mount -t vfat /dev/mtdblock0 /mnt
else
    grep -q BCM5300 /proc/cpuinfo
    if [ $? == 0 ] ; then
        mount -t vfat /dev/mtdblock2 /mnt
    else
        grep -q BCM98548 /proc/cpuinfo
        if [ $? == 0 ] ; then
            mount -t vfat /dev/mtdblock1 /mnt
        fi
    fi
fi

if [ -f /mnt/config.bcm ] ; then
    export BCM_CONFIG_FILE=/mnt/config.bcm
elif [ -r  /proc/bus/pci/00/14.0 ] ; then
    export BCM_CONFIG_FILE=/broadcom/config-sbx-polaris.bcm
elif [ -d /proc/bus/pci/02 ] ; then
    export BCM_CONFIG_FILE=/broadcom/config-sbx-sirius.bcm
elif [ -d /proc/bus/pci/11.0 ] ; then
    export BCM_CONFIG_FILE=/broadcom/config-sbx-fe2kxt.bcm
fi

./bcm.user

