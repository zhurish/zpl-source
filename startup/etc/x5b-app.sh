#!/bin/sh

mkdir /tmp/tmp
cd /app
chmod +x SWP-V0.0.1.bin

sleep 5

./TimerMgr &
sleep 1
./SWP-V0.0.1.bin -t /dev/ttyS1 -d
sleep 2
./VmrMgr &

