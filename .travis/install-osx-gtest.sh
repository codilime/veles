#!/bin/sh

git clone https://github.com/google/googletest
cd googletest
mkdir build
cd build
cmake ..
make
make install
