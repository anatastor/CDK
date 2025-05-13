
setlocal EnableDelayedExpansion

set lib=libcdk.a

set cFiles=
for /r %%f in (*.c) do (
  set cFiles=!cFiles! %%f
)

set defines=-D_DEBUG

echo "building object files ..."
gcc -c %cFiles%

set oFiles=
for /r %%f in (*.o) do (
  set oFiles=!cFiles! %%f
)

ar -rcs %lib% %oFiles%

pause -1
