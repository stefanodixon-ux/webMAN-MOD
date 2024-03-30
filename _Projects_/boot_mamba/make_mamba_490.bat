@ECHO OFF
title make_package_custom
if not exist mamba md mamba_490

:: ----------------------------------------------
:: Simple script to build a PKG (by CaptainCPS-X)
:: ----------------------------------------------

:: Change these for your application / manual...
set CID=CUSTOM-INSTALLER_00-0000000000000000
set PKG_DIR=./mamba_490/
set PKG_NAME=boot_mamba_490.pkg

..\updater\pkg_custom.exe --contentid %CID% %PKG_DIR% %PKG_NAME%

pause
