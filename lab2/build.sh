#!/bin/bash
mkdir build
cd build
cmake -S ../ -B ./
make

mv host* ../
mv client* ../

cd ../
rm -r build