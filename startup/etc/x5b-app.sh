#!/bin/sh
mkdir /tmp/tmp -p
cd /app
chmod +x SWP-V0.0.0.11.bin
sleep 5
./TimerMgr &
sleep 1
./SWP-V0.0.0.11.bin -t /dev/ttyS1 -d
sleep 2
./VmrMgr &
