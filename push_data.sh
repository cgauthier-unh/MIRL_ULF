#!/bin/bash

username="mirl-ulf"
MIRL_IP="132.177.207.87"
key="/home/pi-unh-daq/ULF/mirl_pass"

Data_DIR="/home/pi-unh-daq/ULF/Data_Files"
Dest_DIR="/home/mirl-ulf/ULF/Data_Files/UNH_incoming"

#sleep 3600 &
while true; do
	#sshpass -p "mirlmirl" rsync -aq $Data_DIR $username@$MIRL_IP:$Dest_DIR &
	if nc -z 132.177.207.87 22 2>/dev/null;
	then
	    sshpass -p "mirlmirl" rsync -aq /home/pi-unh-daq/ULF/Data_Files/ mirl-ulf@132.177.207.87:'/home/mirl-ulf/ULF/Data_Files/UNH_incoming/'
	fi
	sleep 1d
done
