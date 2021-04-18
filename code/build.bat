@echo off

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build
cl -MT -nologo -GR- -EHa- -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4211 -DSHARUN_DEBUG_SLOW=1 -DSHARUN_INTERNAL=1  -FC -Z7 -Fmwin32_SharunSheros.map ../code/Win32_SharunSheros.cpp /link -subsystem:windows user32.lib Gdi32.lib
popd