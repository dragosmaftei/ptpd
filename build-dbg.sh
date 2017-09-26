#!/bin/bash

make clean
autoreconf -vi
./configure --enable-runtime-debug
make
sudo make install
