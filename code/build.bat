@echo off

mkdir ..\build
pushd ..\build
cl -DSHARUN_DEBUG_SLOW -DSHARUN_INTERNAL -Zi ../code/Win32_SharunSheros.cpp user32.lib Gdi32.lib
popd