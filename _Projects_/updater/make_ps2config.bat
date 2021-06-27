@ECHO OFF
title make_package_custom
if not exist custom md custom

:: ----------------------------------------------
:: Simple script to build a PKG (by CaptainCPS-X)
:: ----------------------------------------------

:: Change these for your application / manual...
set CID=CUSTOM-PS2CONFIG_00-0000000000000000
set PKG_DIR=./PS2CONFIG/
set PKG_NAME=PS2CONFIG.pkg

pkg_custom.exe --contentid %CID% %PKG_DIR% %PKG_NAME%

ren PS2CONFIG.pkg PS2CONFIG_%date:~10,4%%date:~4,2%%date:~7,2%.pkg

pause
