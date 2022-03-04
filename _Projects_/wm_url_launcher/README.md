# ROMS / PKG Launcher

This application is a complementary tool for webMAN MOD used as intermediary to execute
retail SELF applications, decompress archives or start ROMS with a RetroArch emulator.

The tool serves also as template to create an ISO or PSN package that executes web commands.


## Usage

The tool is installed automatically by webMAN MOD installer or updater.

It can be used as a standalone application if you set the `Home` setting in /setup.ps3
to `/dev_hdd0/game/PKGLAUNCH`. Check the Home option and save the settings.


## Features

- Automatic RetroArch emulation selection based on ROMS path or file extension
- Supports official RetroArch and the unofficial RetroArch Community Edition
- Executes web commands or self applications indicated in `launch.txt`
- Sends web commands directly to local web server using localhost port 80
- Executes retail self applications (non-NPRDM)
- Supported archive file decompression: zip, rar, 7z, tar, gzip


## Requirements

★ app_home/PS3_GAME (aka ★ PlayStation) must be enabled in category_game.xml (aka Game column on XMB menu)

[RetroArch](https://store.brewology.com/ahomebrew.php?brewid=152) or [RetroArch Community Edition](https://github.com/crystalct/RetroArch_PSX_CE/releases) must be installed for ROMS support.


## How it works

ROMS/PKG Launcher reads the command to execute from `launch.txt`

- If the command is an URL or web command a call to the local web server is performed on port 80.
- If the path is an archive (zip, rar, 7z, tar, gzip) the file is decompressed and the ISO is mounted automatically.
- If the path is a ROM, the emulator is selected based on the path or file extension and the emulator is launched.

The tools includes two EBOOT.BIN:
- One signed as retail in /PS3_GAME folder to launch it from the disc icon
- The main EBOOT signed as NPDRM to launch it from XMB

During emulation, the RetroArch's USRDIR is mapped to `/dev_bdvd/PS3_GAME/USRDIR`
The following files are used :
- retroarch.cfg
- retroarch.bak - template configured for the official RetroArch
- retroarch.cce - template configured for RetroArch CE


## Credits

- **aldostools** - Original concept, coding, implementation
- **bucanero** - Ported archive decompression libraries (zip, rar, 7z, tar, gzip)
- **CaptainCPS-X** - Python package creator
- **Estwald** - GUI code snippets
- **RetroArch Team** - RetroArch emulators
- **CrystalCT** - Unofficial RetroArch Community Edition
