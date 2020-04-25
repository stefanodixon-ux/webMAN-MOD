@echo off
title Compiling prepISO
taskkill /IM cmd.exe /T /FI "WINDOWTITLE eq Administrator:*">NUL
cls

set PS3SDK=/c/PSDK3v2
set WIN_PS3SDK=C:/PSDK3v2
set PATH=%WIN_PS3SDK%/mingw/msys/1.0/bin;%WIN_PS3SDK%/mingw/bin;%WIN_PS3SDK%/ps3dev/bin;%WIN_PS3SDK%/ps3dev/ppu/bin;%WIN_PS3SDK%/ps3dev/spu/bin;%WIN_PS3SDK%/mingw/Python27;%PATH%;
set PSL1GHT=%PS3SDK%/psl1ght
set PS3DEV=%PS3SDK%/ps3dev

rem make npdrm
make pkg

del EBOOT.BIN>nul
del *.elf>nul
del /s/q build>nul
rd /s/q build>nul

if exist prepNTFS.self del prepNTFS.self>nul
ren prepISO.self prepNTFS.self

:quit
pause
