@echo off

mkdir ..\build
pushd ..\build
cl  -Zi ../code/Win32_SharunSheros.cpp user32.lib Gdi32.lib
popd