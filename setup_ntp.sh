#!/bin/bash

killall -9 -q gpsd ntpd
gpsd -n /dev/serial0
sleep 2
ntpd -gN

