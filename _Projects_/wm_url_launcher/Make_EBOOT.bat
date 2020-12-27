@echo off
set PS3SDK=/c/PSDK3v2
set WIN_PS3SDK=C:/PSDK3v2
set PATH=%WIN_PS3SDK%/mingw/msys/1.0/bin;%WIN_PS3SDK%/mingw/bin;%WIN_PS3SDK%/ps3dev/bin;%WIN_PS3SDK%/ps3dev/ppu/bin;%WIN_PS3SDK%/ps3dev/spu/bin;%WIN_PS3SDK%/mingw/Python27;%PATH%;
set PSL1GHT=%PS3SDK%/psl1ght
set PS3DEV=%PS3SDK%/ps3dev

make npdrm

del /S/Q build>NUL
rd build
if exist EBOOT.elf  del EBOOT.elf>NUL
:if exist wm_url_launcher.elf  del wm_url_launcher.elf>NUL
if exist wm_url_launcher.self del wm_url_launcher.self>NUL
if exist UP0001-PKGLAUNCH_00-0000000000000000.pkg del UP0001-PKGLAUNCH_00-0000000000000000.pkg>NUL
move /Y EBOOT.BIN pkg\USRDIR\EBOOT.BIN
move /Y wm_url_launcher.elf pkg\PS3_GAME\USRDIR\EBOOT.ELF
pause
