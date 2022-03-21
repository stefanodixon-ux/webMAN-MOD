@ECHO OFF
title Building webMAN_MOD_latest.pkg
if not exist update md update

:: ----------------------------------------------
:: Simple script to build a PKG (by CaptainCPS-X)
:: ----------------------------------------------

:: Change these for your application / manual...
set CID=EP0001-UPDWEBMOD_00-0000000000000000
set PKG_DIR=./update/
set PKG_NAME=webMAN_MOD_latest.pkg

pkg_custom.exe --contentid %CID% %PKG_DIR% %PKG_NAME%
