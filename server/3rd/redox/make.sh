#!/bin/bash

INSTALL_PATH=`pwd`
echo $INSTALL_PATH
cd ../package/redox &&
mkdir -p build

cd build &&
cmake -DCMAKE_INSTALL_PREFIX=$INSTALL_PATH .. &&
time make &&
make install &&

if [ -x "$INSTALL_PATH/lib64" ]; then
	cp -v libredox_static.a $INSTALL_PATH/lib64
fi

if [ -x "$INSTALL_PATH/lib32" ]; then
	cp -v libredox_static.a $INSTALL_PATH/lib32
fi


#chmod +x make.sh &&
#dos2unix make.sh &&
#./make.sh &&
#cd build &&
#sudo make install