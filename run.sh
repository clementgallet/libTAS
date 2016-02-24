#!/bin/sh

cd /home/clement/supermeatboy
LD_PRELOAD=$OLDPWD/bin/libTAS.so amd64/SuperMeatBoy &
cd - > /dev/null
sleep 1
bin/linTAS -d ./test2.avi

