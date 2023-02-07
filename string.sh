#!/bin/bash

FILE="/home/pi-unh-daq/ULF/NMEA_String.txt"
echo " " > /home/pi-unh-daq/ULF/pigs_output.txt
echo " " > /home/pi-unh-daq/ULF/NMEA_String.txt

let flag=0

while [ $flag == 0 ]; do
pigs slro 17 9600 8
sleep 1
pigs -a slr 17 600 >/home/pi-unh-daq/ULF/pigs_output.txt

while read -r line; do
    IFS=","
    read -ra arr <<< "$line"
    if [ "${arr[0]}" == '$GPRMC' ] && [ "${#arr[@]}" == 13 ] ;
    then
        echo "$line" > $FILE
        let flag=1
    fi
done < /home/pi-unh-daq/ULF/pigs_output.txt

pigs slrc 17

done
