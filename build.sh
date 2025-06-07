#!/bin/sh

set echo on

cd "$(dirname "$0")"
# mkdir -p ../bin


lib="../bin/libcdk.so"

cFiles=$(find . -type f -name "*.c")

iFlags=-I/storage/Programming/_Engine/CDK/cdk/src
lFlags+="-lX11 -lvulkan"
defines="-D_DEBUG"


# echo "building object files ..."
# echo gcc -c $cFiles $iFlags
# gcc -c $cFiles $iFlags

# oFiles=$(find . -type f -name "*.o")

echo "building $lib ..."

echo gcc -shared -o $lib $cFiles $iFlags $lFlags
gcc -shared -o $lib $defines $cFiles $iFlags $lFlags
