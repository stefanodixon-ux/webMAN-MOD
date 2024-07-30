@echo off
mode con: cols=132 lines=45

set CELL_SDK=/c/msys/1.0/cell_4.50
set PS3SDK=/c/PSDK3v2
set PS3DEV=/c/PSDK3v2/ps3dev2
set WIN_PS3SDK=C:/PSDK3v2
set PATH=%WIN_PS3SDK%/mingw/msys/1.0/bin;%WIN_PS3SDK%/mingw/bin;%PS3DEV%/ppu/bin;%path%;

set CHERE_INVOKING=1

cls

make

pause