#!/bin/bash

rpi_hostname="vs-rpi"
rpi_ip="192.168.1.254"
rpi_user="pi"
rpi_pass="12345"

rpi_mtpt="/tmp/rpi_fs"
rpi_code_dir="BB"

if [ "$HOSTNAME" == "$rpi_hostname" ]; then
  echo "Running remote on Raspberry Pi..."
  cd $rpi_code_dir
  ./run $@
else
  sshpass -p "$rpi_pass" ssh $rpi_user@$rpi_ip 'bash -s' < run.sh
fi