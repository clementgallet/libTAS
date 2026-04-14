#!/bin/bash

SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd $SCRIPT_DIR

docker build -f ../Dockerfile --tag libtas_test . --build-arg=RUFFLE_VERSION=nightly-$(date +%Y-%m-%d)
docker run --rm -it -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix -v $HOME/.Xauthority:/root/.Xauthority:rw -v "$SCRIPT_DIR"/test_ruffle_nv14.ltm:/home/test_ruffle_nv14.ltm --net=host libtas_test bash -c 'apt-get -y install wget && \
       cd /home/ && \
       # Download game to test
       wget https://github.com/Perdu/nv14_TAS/raw/refs/heads/main/external/n_v14.swf && \
       # Fix for the sync issue (we need to launch ruffle once to load libopenh264.so)
       timeout 2 xvfb-run -a ruffle n_v14.swf ; \
       # We need to launch libTAS once so it creates a config file
       libTAS -n -r /home/test_ruffle_nv14.ltm /usr/local/bin/ruffle -g gl --no-gui --width 792 /home/n_v14.swf && \
       # Disable "prevent writing to disk" so we can check savefile size later
       sed -i "s/prevent_savefiles=true/prevent_savefiles=false/" ~/.config/libTAS/ruffle/.ini && \
       # Actual game execution test
       libTAS -n -r /home/test_ruffle_nv14.ltm /usr/local/bin/ruffle -g gl --no-gui --width 792 /home/n_v14.swf && \
       # Test that save file has the expected size, indicating the level was finished
       test $(stat -c%s "/root/.local/share/ruffle/SharedObjects/localhost/n_v14b_userdata.sol") -eq 44787 && \
       echo -e "\nRuffle test passed successfully!"'
