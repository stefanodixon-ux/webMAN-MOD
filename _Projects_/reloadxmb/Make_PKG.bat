@echo off
set PS3SDK=/c/PSDK3v2
set WIN_PS3SDK=C:/PSDK3v2
set PATH=%WIN_PS3SDK%/mingw/msys/1.0/bin;%WIN_PS3SDK%/mingw/bin;%WIN_PS3SDK%/ps3dev/bin;%WIN_PS3SDK%/ps3dev/ppu/bin;%WIN_PS3SDK%/ps3dev/spu/bin;%WIN_PS3SDK%/mingw/Python27;%PATH%;
set PSL1GHT=%PS3SDK%/psl1ght
set PS3DEV=%PS3SDK%/ps3dev

if exist EP0001-RELOADXMB_00-0000000000000000.pkg del EP0001-RELOADXMB_00-0000000000000000.pkg>>nul

if exist reloadxmb.elf del reloadxmb.elf>>nul
if exist reloadxmb.self del reloadxmb.self>>nul
if exist build del /s/q build\*.*>>nul

make pkg

del reloadxmb.elf>>nul
del reloadxmb.self>>nul
del /s/q build\*.*>>nul
rd /q/s build>>nul

:end
