#!/bin/sh

set echo on

cd "$(dirname "$0")"
# mkdir -p ../bin


lib="libcdk.a"

cFiles=$(find . -type f -name "*.c")

iFlags=-I/storage/Programming/_Engine/CDK/cdk/src
#lFlags="-lX11 -lvulkan"
defines="-D_DEBUG"


echo "building object files ..."
echo gcc -c $cFiles $iFlags
gcc -c $cFiles $iFlags

oFiles=$(find . -type f -name "*.o")

echo "building $out ..."
echo ar -rcs $lib $oFiles
ar -rcs $lib $oFiles
# echo gcc $cFiles $cFlags -o "../bin/$out" $defines $iFlags $lFlags
# gcc $cFiles $cFlags -o "../bin/$out" $defines $iFlags $lFlags
