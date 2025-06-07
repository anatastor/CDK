
setlocal EnableDelayedExpansion

:: set lib=libcdk.a
set lib=bin/libcdk.dll

set cFiles=
for /r src\ %%f in (*.c) do (
  set cFiles=!cFiles! %%f
)

set iFlags=-Isrc

:: -lvulkan" TODO link to vulkan on windows
set lFlags="-lgdi32"
set defines=-D_DEBUG

:: echo "building object files ..."
:: gcc -c %cFiles%


gcc -shared -o %lib% %cFiles% %iFlags% %lFlags%

:: set oFiles=
:: for /r %%f in (*.o) do (
  :: set oFiles=!cFiles! %%f
:: )

:: ar -rcs %lib% %oFiles%

:: pause -1
