#!/bin/bash

make clean
autoreconf -vi
./configure --enable-security --enable-runtime-debug
make
sudo make install
