#!/bin/sh

set echo on
mkdir -p ../bin


out="test"

cFiles=$(find . -type f -name "*.c")

iFlags="-Isrc -I../src/"
lFlags="-L../bin -lcdk -Wl,-rpath,. " # -lX11 -lvulkan"
# lFlags+="-lX11 -lvulkan"
# lFlags="-L../cdk/ -lcdk -Wl,-rpath,." # -lX11"
# lFlags="-lX11 -lvulkan"
defines="-D_DEBUG"


echo "building $out"
echo gcc $cFiles $cFlags -o "../bin/$out" $defines $iFlags $lFlags
gcc $cFiles $cFlags -o "../bin/$out" $defines $iFlags $lFlags
