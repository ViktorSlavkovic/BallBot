#!/bin/bash

rm -rf run
g++ pwr_main.cc -o run -lboost_system -lpthread
