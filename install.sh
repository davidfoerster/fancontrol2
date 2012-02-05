#!/bin/sh
cd "`dirname "$0"`"
nice -n 10 -- make -j 2 -C Release all
sudo install -st /usr/local/bin Release/fancontrol2
