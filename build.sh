#!/bin/sh

set echo on
# mkdir -p ../bin


lib="libcdk.a"

cFiles=$(find . -type f -name "*.c")

#iFlags=
#lFlags="-lX11 -lvulkan"
defines="-D_DEBUG"


echo "building object files ..."
gcc -c $cFiles

oFiles=$(find . -type f -name "*.o")

echo "building $out ..."
echo ar -rcs $lib $cFiles
ar -rcs $lib $oFiles
# echo gcc $cFiles $cFlags -o "../bin/$out" $defines $iFlags $lFlags
# gcc $cFiles $cFlags -o "../bin/$out" $defines $iFlags $lFlags
