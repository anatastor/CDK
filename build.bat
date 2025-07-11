
setlocal EnableDelayedExpansion

:: set lib=libcdk.a
set lib=bin/libcdk.dll

set cFiles=
for /r src\ %%f in (*.c) do (
  set cFiles=!cFiles! %%f
)

set iFlags=-Isrc -I%VULKAN_SDK%/Include
set lFlags=-lgdi32 -lvulkan-1 -L%VULKAN_SDK%/Lib

set defines=-D_DEBUG

gcc -shared -o %lib% %defines% %cFiles% %iFlags% %lFlags%

:: in case of static linking
:: set oFiles=
:: for /r %%f in (*.o) do (
  :: set oFiles=!cFiles! %%f
:: )

:: ar -rcs %lib% %oFiles%

:: pause -1
