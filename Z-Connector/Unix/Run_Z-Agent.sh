#!/bin/bash

cd ${0%/*}

if [ "`uname`" == "Darwin" ]
then
	export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:.
fi

while [ 1 ]
do
	./z-agent -s z-cloud.z-wave.me --cacert Certificates/cacert.pem --cert Certificates/cert.pem --key Certificates/cert.key -d /dev/ttyUSB0 "${@}"
	# substitute /dev/ttyUSB0 by /dev/ttyACM0 or another device depending on the hardware you have
	# on Mac OS X use /dev/cu.usbserial or another cu.* device

	sleep 1 # spare the CPU 
done
