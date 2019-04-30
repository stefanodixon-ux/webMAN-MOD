@echo off
set PS3SDK=/c/PSDK3v2
set WIN_PS3SDK=C:/PSDK3v2
set PATH=%WIN_PS3SDK%/mingw/msys/1.0/bin;%WIN_PS3SDK%/mingw/bin;%WIN_PS3SDK%/ps3dev/bin;%WIN_PS3SDK%/ps3dev/ppu/bin;%WIN_PS3SDK%/ps3dev/spu/bin;%WIN_PS3SDK%/mingw/Python27;%PATH%;
set PSL1GHT=%PS3SDK%/psl1ght
set PS3DEV=%PS3SDK%/ps3dev

copy /y pkgfiles\ICON0.PNG pkgfiles-metalification_theme>>nul
copy /y pkgfiles\PARAM.SFO pkgfiles-metalification_theme>>nul

if not exist pkgfiles-metalification_theme\USRDIR\addons mkdir pkgfiles-metalification_theme\USRDIR\addons
if not exist pkgfiles-metalification_theme\USRDIR\html   mkdir pkgfiles-metalification_theme\USRDIR\html
if not exist pkgfiles-metalification_theme\USRDIR\lang   mkdir pkgfiles-metalification_theme\USRDIR\lang
if not exist pkgfiles-metalification_theme\USRDIR\res    mkdir pkgfiles-metalification_theme\USRDIR\res
if not exist pkgfiles-metalification_theme\USRDIR\xml    mkdir pkgfiles-metalification_theme\USRDIR\xml
if not exist pkgfiles-metalification_theme\USRDIR\xmb    mkdir pkgfiles-metalification_theme\USRDIR\xmb

copy /y pkgfiles\USRDIR\*.txt  pkgfiles-metalification_theme\USRDIR>>nul
copy /y pkgfiles\USRDIR\wm_custom_combo pkgfiles-metalification_theme\USRDIR>>nul
copy /y pkgfiles\USRDIR\*.sprx pkgfiles-metalification_theme\USRDIR>>nul

copy /y pkgfiles\USRDIR\icon_lp_*.png pkgfiles-metalification_theme\USRDIR>>nul
copy /y pkgfiles\USRDIR\addons\*.pkg  pkgfiles-metalification_theme\USRDIR\addons>>nul
copy /y pkgfiles\USRDIR\html\*.*      pkgfiles-metalification_theme\USRDIR\html>>nul
copy /y pkgfiles\USRDIR\icons\icon_lp_*.png pkgfiles-metalification_theme\USRDIR\icons>>nul
copy /y pkgfiles\USRDIR\lang\*.txt    pkgfiles-metalification_theme\USRDIR\lang>>nul
copy /y pkgfiles\USRDIR\res\*.*       pkgfiles-metalification_theme\USRDIR\res>>nul
copy /y pkgfiles\USRDIR\xml\*.xml     pkgfiles-metalification_theme\USRDIR\xml>>nul
copy /y pkgfiles\USRDIR\xmb\*.xml     pkgfiles-metalification_theme\USRDIR\xmb>>nul
copy /y pkgfiles\USRDIR\xmbm\*.sfo    pkgfiles-metalification_theme\USRDIR\xmbm>>nul

mkdir pkgfiles-metalification_theme\USRDIR\images>>nul
copy /y pkgfiles\USRDIR\images\*.png pkgfiles-metalification_theme\USRDIR\images>>nul
copy /y pkgfiles\USRDIR\images\*.jpg pkgfiles-metalification_theme\USRDIR\images>>nul
mkdir pkgfiles-metalification_theme\USRDIR\official>>nul
copy /y pkgfiles\USRDIR\official\*.sprx pkgfiles-metalification_theme\USRDIR\official>>nul

mkdir pkgfiles-metalification_theme\USRDIR\CONFIG\CUSTOM
copy /y pkgfiles\USRDIR\CONFIG\CUSTOM\*.CONFIG pkgfiles-metalification_theme\USRDIR\CONFIG\CUSTOM>>nul
mkdir pkgfiles-metalification_theme\USRDIR\CONFIG\GX
copy /y pkgfiles\USRDIR\CONFIG\GX\*.CONFIG pkgfiles-metalification_theme\USRDIR\CONFIG\GX>>nul
mkdir pkgfiles-metalification_theme\USRDIR\CONFIG\NET
copy /y pkgfiles\USRDIR\CONFIG\NET\*.CONFIG pkgfiles-metalification_theme\USRDIR\CONFIG\NET>>nul
mkdir pkgfiles-metalification_theme\USRDIR\CONFIG\SOFT
copy /y pkgfiles\USRDIR\CONFIG\SOFT\*.CONFIG pkgfiles-metalification_theme\USRDIR\CONFIG\SOFT>>nul

cls

ren pkgfiles pkgfiles-normal_theme
ren pkgfiles-metalification_theme pkgfiles

if exist EP0001-UPDWEBMOD_00-0000000000000000.pkg del EP0001-UPDWEBMOD_00-0000000000000000.pkg>>nul
if exist webMAN_MOD_1.47.xx_Updater_metalification_theme.pkg del webMAN_MOD_1.47.xx_Updater_metalification_theme.pkg>>nul

if exist updater.elf del updater.elf>>nul
if exist updater.self del updater.self>>nul
if exist build del /s/q build\*.*>>nul

make pkg

ren build\pkg EP0001-UPDWEBMOD_00-0000000000000000
param_sfo_editor.exe build\EP0001-UPDWEBMOD_00-0000000000000000\PARAM.SFO "ATTRIBUTE" 133

if exist updater.elf del updater.elf>>nul
if exist updater.self del updater.self>>nul
if exist updater.pkg del updater.pkg>>nul
if exist build del /q build\*.*>>nul
if not exist build goto end

echo ContentID = EP0001-UPDWEBMOD_00-0000000000000000>package.conf
echo Klicensee = 000000000000000000000000000000000000>>package.conf
echo PackageVersion = 01.00>>package.conf
echo DRMType = Free>>package.conf
echo ContentType = GameExec>>package.conf

psn_package_npdrm.exe -n package.conf build\EP0001-UPDWEBMOD_00-0000000000000000

del package.conf>>nul

if exist webMAN_MOD_1.47.xx_Updater_metalification_theme.pkg del webMAN_MOD_1.47.xx_Updater_metalification_theme.pkg>>nul
move /y EP0001-UPDWEBMOD_00-0000000000000000.pkg webMAN_MOD_1.47.xx_Updater_metalification_theme.pkg>>nul

rd /q/s build>>nul

ren pkgfiles pkgfiles-metalification_theme>>nul
ren pkgfiles-normal_theme pkgfiles>>nul

del /s/q pkgfiles-metalification_theme\USRDIR\*.txt>>nul

del /s/q pkgfiles-metalification_theme\USRDIR\*.sprx>>nul
del /s/q pkgfiles-metalification_theme\USRDIR\addons\*.pkg>>nul
del /s/q pkgfiles-metalification_theme\USRDIR\html\*.*>>nul
del /s/q pkgfiles-metalification_theme\USRDIR\icons\icon_lp_*.png>>nul
del /s/q pkgfiles-metalification_theme\USRDIR\lang\*.txt>>nul
del /s/q pkgfiles-metalification_theme\USRDIR\res\*.*>>nul
del /s/q pkgfiles-metalification_theme\USRDIR\xml\*.xml>>nul
del /s/q pkgfiles-metalification_theme\USRDIR\xmb\*.xml>>nul
del /s/q pkgfiles-metalification_theme\USRDIR\xmbm\*.sfo>>nul
del /s/q pkgfiles-metalification_theme\USRDIR\*.xml>>nul
del /s/q pkgfiles-metalification_theme\USRDIR\icon_lp_*.png>>nul
del /s/q pkgfiles-metalification_theme\USRDIR\wm_custom_combo>>nul

del /s/q pkgfiles-metalification_theme\USRDIR\images\*.png>>nul
del /s/q pkgfiles-metalification_theme\USRDIR\images\*.jpg>>nul
del /s/q pkgfiles-metalification_theme\USRDIR\official\*.sprx>>nul
del /s/q pkgfiles-metalification_theme\ICON0.PNG>>nul
del /s/q pkgfiles-metalification_theme\PARAM.SFO>>nul

del /s/q pkgfiles-metalification_theme\USRDIR\CONFIG\*.*>>nul
rmdir /s/q pkgfiles-metalification_theme\USRDIR\CONFIG>>nul

rd pkgfiles-metalification_theme\USRDIR\images>>nul
rd pkgfiles-metalification_theme\USRDIR\official>>nul
rd pkgfiles-metalification_theme\USRDIR\addons>>nul
rd pkgfiles-metalification_theme\USRDIR\html>>nul
rd pkgfiles-metalification_theme\USRDIR\lang>>nul
rd pkgfiles-metalification_theme\USRDIR\res>>nul
rd pkgfiles-metalification_theme\USRDIR\xmb>>nul
rd pkgfiles-metalification_theme\USRDIR\xml>>nul

:end
