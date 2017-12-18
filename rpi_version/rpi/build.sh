#!/bin/bash

rpi_hostname="vs-rpi"
rpi_ip="192.168.1.254"
rpi_user="pi"
rpi_pass="12345"

rpi_mtpt="/tmp/rpi_fs"

if [ "$HOSTNAME" == "$rpi_hostname" ]; then
  echo "remote"
else
  echo "Uploading code to the Raspberry Pi..."
  rm -rf $rpi_mtpt
  mkdir $rpi_mtpt
  echo "yes" > /tmp/bla
  echo "$rpi_pass" >> /tmp/bla
  cat /tmp/bla | sshfs $rpi_user@$rpi_ip: $rpi_mtpt -C -o password_stdin
  cp -r * "$rpi_mtpt/"
  fusermount -u $rpi_mtpt
  rm -rf $rpi_mtpt
fi


