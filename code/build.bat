@echo off

set CommonCompilerFlags=-MTd -nologo -GR- -EHa- -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4211 -wd4244 -wd4505  -DSHARUN_DEBUG_SLOW=1 -DSHARUN_INTERNAL=1 -DSHARUN_WIN32 -FC -Z7
set CommonLinkerFlags=-incremental:no -opt:ref user32.lib Gdi32.lib winmm.lib
REM set CommonLinkerFlags=-subsystem:windows user32.lib Gdi32.lib winmm.lib

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

REM 32-bit build
REM cl %CommonCompilerFlags%  ../code/Win32_SharunSheros.cpp /link -subsystem:windows,5.1 %CommonLinkerFlags%

REM 64-bit build

del *.pdb > NUL 2> NUL
cl %CommonCompilerFlags%  ..\code\SharunMade.cpp -FmSharunMade.map -LD /link -incremental:no -opt:ref /PDB:SharunMade_%random%.pdb  /EXPORT:GameGetSoundSamples /EXPORT:GameUpdateAndRenderer 
cl %CommonCompilerFlags%  ..\code\Win32_SharunSheros.cpp -Fmwin32_SharunSheros.map /link %CommonLinkerFlags%
popd