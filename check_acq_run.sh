#!/bin/sh

ACQ="main_acq"


while true; do
    if pgrep -x "$ACQ" >/dev/null
    then
        :
    else
        sudo /home/pi-unh-daq/ULF/main_acq &
    fi
    sleep 60
done
