zpl_video_hal.c         对INPUT,VPSS,ENCODE管理层的调度，供media层使用
zpl_video_input.c       INPUT输入层管理
zpl_video_vpss.c        VPSS 管理
zpl_video_encode.c      编码层管理

zpl_vidhal.c            图像处理层，涉及硬件的图像处理
                        图像缩放，叠加OSD等相关中间处理
                        INPUT输入回调，输出回调，VPSS输入输出回调集中处理


zpl_media_buffer.c        编码层使用数据缓冲区，编码后输出
zpl_media_channel.c       通道管理
zpl_media_client.c        对接编码层，接收编码后数据的客户端
                       



bootargs "mem=512M console=ttyAMA0,115200 root=/dev/mmcblk0p3 rootfstype=ext4 rw rootwait blkdevparts=mmcblk0:5M(boot),10M(kernel),500M(rootfs)"
bootcmd=mmc read 0 0x81000000 0x2800 0x4000;bootm 0x81000000

bootargs "mem=512M console=ttyAMA0,115200 clk_ignore_unused rwrootwait root=/dev/mmcblk0p3 rootfstype=ext4 rw blkdevparts=mmcblk0:5M(uboot.bin),10M(kernel),500M(rootfs.ext4),200M(userdata.ext4),200M(user)"

setenv serverip 192.168.10.100
setenv ipaddr 192.168.10.1
setenv gatewayip 192.168.10.100
setenv netmask 255.255.255.0



bootargs=mem=256M console=ttyAMA0,115200 init=/linuxrc root=/dev/nfs rw nfsroot=192.168.10.100:/home/zhurish/Downloads/tftpboot/r
ootfs,nolock,proto=tcp,nfsvers=3 ip=192.168.10.1:192.168.10.100:192.168.10.100:255.255.255.0


setenv rootpath "/home/zhurish/Downloads/tftpboot/rootfs"
setenv bootargs "mem=256M console=ttyAMA0,115200 init=/linuxrc root=/dev/nfs rw nfsroot=${serverip}:${rootpath},nolock,proto=tcp,nfsvers=3 ip=${ipaddr}:${serverip}:${gatewayip}:${netmask} blkdevparts=mmcblk0:5M(uboot.bin),10M(kernel),500M(rootfs.ext4),200M(userdata.ext4),200M(user)"
setenv bootcmd "tftp 0x81000000 uImage_hi3516dv300_smp;bootm 0x81000000"

make ARCH=arm CROSS_COMPILE=arm-himix200-linux- all

make ARCH=arm CROSS_COMPILE=arm-himix200-linux- modules_install INSTALL_MOD_PATH=/home/zhurish/Downloads/tftpboot/rootfs/lib/modules