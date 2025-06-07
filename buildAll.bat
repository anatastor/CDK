
pushd cdk
call build.bat

pushd tests
call build.bat
popd

pause -1
