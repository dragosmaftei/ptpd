#!/bin/bash

make clean
autoreconf -vi
./configure
make
sudo make install
