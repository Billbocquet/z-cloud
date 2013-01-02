#!/bin/sh
NAME=z-connector
mkdir config
ln ../etc/$NAME.conf config
mkdir shared
ln ../etc/init.d/$NAME.sh shared
ln ../usr/local/bin/$NAME shared
