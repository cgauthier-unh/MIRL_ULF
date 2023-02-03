#!/bin/bash

pigs slro 17 9600 8
sleep 1
pigs -a slr 17 600 >/home/pi-unh-daq/ULF/pigs_output.txt

FILE="/home/pi-unh-daq/ULF/NMEA_String.txt"
#echo $FILE
while read -r line; do
    IFS=","
    read -ra arr <<< "$line"
    if [[ ${arr[0]} == '$GPRMC' ]] ; 
    then
        echo "$line" > $FILE
#	echo "$line"
    fi
done < /home/pi-unh-daq/ULF/pigs_output.txt

pigs slrc 17

