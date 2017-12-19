#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

base_directory=""
obj_files_list=""

function compile {
  # Path to the current directory relative to the base directory.
  local path=$1
  
  # Handle the subdirectories first.
  for d in $base_directory/$path/*/ ; do
    local base=$(basename "$d")
    if [[ -d $d ]]; then
      compile "$path/$base"
    fi
  done
 
  # 1) Handle cc files.
  for f in $base_directory/$path/*.cc ; do
    local base=$(basename "$f")
    if [[ -f $f ]]; then
      local base_noext=$(echo $base | sed 's/\.[^.]*$//')
      if [[ ! -d $base_directory/objs/$path ]]; then
        mkdir -p $base_directory/objs/$path
      fi
      cd $base_directory/$path/
      # g++ -I$base_directory/ -O3 -c $f -o $base_directory/objs/$path/$base_noext.o
      g++ --std=c++0x -I$base_directory/ -g -c $f -o $base_directory/objs/$path/$base_noext.o
      if [[ $? -ne 0 ]]; then
        echo -e "${RED}^ ${base}${NC}"
        cd - > /dev/null
        exit -1
      else
        echo -e "${GREEN}^ ${base}${NC}"
        cd - > /dev/null
        obj_files_list="$obj_files_list $base_directory/objs/$path/$base_noext.o"
      fi
    fi
  done
}

rpi_hostname="vs-rpi"
rpi_ip="192.168.1.254"
rpi_user="pi"
rpi_pass="12345"

rpi_mtpt="/tmp/rpi_fs"
rpi_code_dir="BB"

if [ "$HOSTNAME" == "$rpi_hostname" ]; then
  echo "Running a remote build..."
  mkdir $rpi_code_dir 2> /dev/null
  cd $rpi_code_dir
  base_directory=$PWD
  compile "."
  g++ --std=c++0x -g $obj_files_list -o $base_directory/run -lwiringPi -lpthread
else
  echo "Uploading code to the Raspberry Pi..."
  fusermount -u $rpi_mtpt 2> /dev/null
  rm -rf $rpi_mtpt
  mkdir $rpi_mtpt
  echo "$rpi_pass" | sshfs $rpi_user@$rpi_ip: $rpi_mtpt -C -o password_stdin -o workaround=rename
  if [ $? -eq 0 ]; then
    rm -rf "$rpi_mtpt/$rpi_code_dir"
    mkdir "$rpi_mtpt/$rpi_code_dir"
    cp -r * "$rpi_mtpt/$rpi_code_dir/"
    fusermount -u $rpi_mtpt
    rm -rf $rpi_mtpt
    echo "Sucessfully uploaded."
  else
    echo "Failed to upload!"
    exit 1
  fi
  sshpass -p "$rpi_pass" ssh $rpi_user@$rpi_ip 'bash -s' < build.sh
fi
