@echo off

set CYGWIN=C:\cygwin\bin
set CHERE_INVOKING=1

if not exist %CYGWIN%\bash.exe set CYGWIN=C:\msys\1.0\bin

del /q objs\cobra\*.d>nul
del /q objs\cobra\*.o>nul
rd objs\cobra>nul

attrib -r objs\*.*>nul

del /q objs\*.d>nul
del /q objs\*.o>nul
rd objs>nul

if exist *.sprx del *.sprx>nul
if exist *.elf  del *.elf>nul
if exist *.prx  del *.prx>nul
if exist *.sym  del *.sym>nul
if exist *.pkg  del *.pkg>nul

%CYGWIN%\bash --login -i -c 'make clean'>nul
