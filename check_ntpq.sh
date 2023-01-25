#!/bin/bash

killall -9 -q ntpd gpsd
gpsd -n /dev/serial0
sleep 2
ntpd -gN
sleep 10

while true; do
let flag=0

while [ $flag == 0 ]; do
output=$(ntpq -p)
#echo $output
echo $output > ntp_output.txt

while read -r line; do
    IFS=' '
    read -ra arr <<< "$line"

    for val in "${arr[@]}";
    do
        if [[ "$val" == "*SHM(0)" ]]
        then
             let flag=1
	fi
    done
done < ntp_output.txt

if [ $flag == 0 ]
then
    killall -9 -q ntpd gpsd
    gpsd -n /dev/serial0
    sleep 2
    ntpd -gN
fi
done

sleep 4h
done
