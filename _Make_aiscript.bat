@echo off

set CYGWIN=C:\cygwin\bin

if not exist %CYGWIN%\bash.exe set CYGWIN=C:\msys\1.0\bin

set CHERE_INVOKING=1
make -f makefile_aiscript
del aiscript.prx
del aiscript.sym
del _Projects_\aiscript\pkgfiles\USRDIR\aiscript.sprx
move aiscript.sprx _Projects_\aiscript\pkgfiles\USRDIR\aiscript.sprx
cd _Projects_\aiscript
call Make_PKG.bat
move aiscript.pkg ..\..\UTB_Installer.pkg
