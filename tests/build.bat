
setlocal EnableDelayedExpansion :: neccessary for loop to search all .c files

:: mkdir \a ../bin

set out=test.exe

set cFiles=
for /r %%f in (*.c) do (
  set cFiles=!cFiles! %%f
)


set iFlags=-I../src/
set lFlags=-L../bin -lcdk -Wl,-rpath,.

set defines=-D_DEBUG

echo "FILES: %cFiles%"


gcc %cFiles% %cFlags% -o ../bin/%out% %defines% %iFlags% %lFlags%

:: pause -1
