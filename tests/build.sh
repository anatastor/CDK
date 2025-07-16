#!/bin/sh

set echo on
mkdir -p ../bin


out="test"

cFiles=$(find . -type f -name "*.c")

iFlags="-Isrc -I../src/"
lFlags="-L../bin -lcdk -Wl,-rpath,. " 
defines="-D_DEBUG"


echo "building $out"
echo gcc $cFiles $cFlags -o "../bin/$out" $defines $iFlags $lFlags
gcc $cFiles $cFlags -o "../bin/$out" $defines $iFlags $lFlags
