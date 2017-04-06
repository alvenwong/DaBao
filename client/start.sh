#!/bin/sh

killall -9 client
sed -i -e '7s/.*/kill = 0/' -e "6s/.*/connections = $1/" settings.cfg
./client 

