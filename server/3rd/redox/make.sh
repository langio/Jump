#!/bin/bash

cd ../package/redox &&
chmod +x make.sh &&
dos2unix make.sh &&
./make.sh &&
cd build &&
sudo make install