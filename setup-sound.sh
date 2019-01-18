#!/bin/sh

mkdir /tmp/tmp
cd /app

tftp -r Alarm05.wav -g 192.168.1.118

#tftp -r s16le-8000-pcm.raw -g 192.168.1.118

#chmod +x X5-B

#tftp -r SWP-V0.0.1.bin -g 192.168.1.118

chmod +x SWP-V0.0.1.bin

#cd /app/etc
#./volume_setup.sh

cd /app
sleep 8
./SWP-V0.0.1.bin -t /dev/ttyS1 -d

