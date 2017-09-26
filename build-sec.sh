#!/bin/bash

make clean
autoreconf -vi
./configure --enable-security
make
sudo make install
