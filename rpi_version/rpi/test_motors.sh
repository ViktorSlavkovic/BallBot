#!/bin/bash

step0=24
dir0=25
step1=22
dir1=23
step2=1
dir2=21

ser=7
rclk=0
srclk=2


gpio mode $step0 output
gpio mode $dir0 output
gpio mode $step1 output
gpio mode $dir1 output
gpio mode $step2 output
gpio mode $dir2 output
gpio mode $ser output
gpio mode $rclk output
gpio mode $srclk output


gpio write $dir0 0
gpio write $step1 0
gpio write $dir1 0
gpio write $step2 0
gpio write $dir2 0
gpio write $ser 0
gpio write $rclk 0
gpio write $srclk 0

function shift_one_bit {
  local b=$1
  gpio write $ser $b
  gpio write $srclk 1
  gpio write $srclk 0
}

function refresh {
  gpio write $rclk 0
  gpio write $rclk 1
}


function enable_motors {
  shift_one_bit 0
  shift_one_bit 0
  shift_one_bit 0
  shift_one_bit 0
  shift_one_bit 0 # M2(0)
  shift_one_bit 0 # M2(1)
  shift_one_bit 0 # M2(2)
  shift_one_bit 0 # M1(0)
  shift_one_bit 0 # M1(1)
  shift_one_bit 0 # M1(2)
  shift_one_bit 0 # M0(0)
  shift_one_bit 0 # M0(1)
  shift_one_bit 0 # M0(2)
  shift_one_bit 0 # ~EN0
  shift_one_bit 0 # ~EN1
  shift_one_bit 0 # ~EN2
  refresh
}


function step {
  gpio write $step0 1
  gpio write $step1 1
  gpio write $step2 1
  sleep 0.001s
  gpio write $step0 0
  gpio write $step1 0
  gpio write $step2 0
  sleep 0.001s
}

enable_motors

for ((i=0;i<100;i++)); do
  echo $i
  step
  sleep 0.01s
done
