#!/bin/sh

export TTROOT=$(pwd)/..

rm -rf obj
mkdir obj
cd obj
MAKE_DIR="../linux"

echo "build TT... ..."
make -f $MAKE_DIR/makefile 

echo " "
echo "build CTP so simu... ..."
make -f $MAKE_DIR/makefile.libUSTPmduserapi
make -f $MAKE_DIR/makefile.libthosttraderapi 

cd ..
rm -rf obj
