#!/bin/bash

sudo ifconfig wlan0 down

if pgrep pigpiod >/dev/null
then
    :
else
    sudo pigpiod
fi

sleep 120

if pgrep check_ntpq >/dev/null
then
    :
else
    sudo /home/pi-unh-daq/ULF/check_ntpq.sh &
fi

sleep 10

if pgrep check_acq >/dev/null
then
    :
else
    sudo /home/pi-unh-daq/ULF/check_acq_run.sh &
fi

sleep 10

if pgrep push_data > /dev/null
then
    :
else
    /home/pi-unh-daq/ULF/push_data.sh &
fi
