# PKG Custom / Python PKG 

PKG Custom is a modified version of the Python scripts created by `CaptainCPS-X` 
to create PS3 packages from a directory and extract them.

PKG Custom was modified by `lmn7` to allow installation of files on any path of the PS3 file system
based on the research of `aldostools` that proposed the use of path traversal attack adding `..\` 
to the default installation path. 

`lmn7` made important improvents like:
- Adding `..\..\` to reach system's root.
- Changing the content type of the PKG to 0x09 THEME to avoid the addition of entries on XMB
- Modified the Python scripts for more control of the PKG creation.
- Improved the Python script to support large files
- Improved the performance of the Python script using a pyc script for the files encryption

`aldostools` made additional improvements over PKG Custom script:
- Improved the performance of the script
- Added some GUI improvements
- Support for custom and standard PKG creation
- Repacked the PKG Custom script an executable eliminating the pre-requisite of having Python installed

## Usage

[based on revision 20220805]

    pkg_custom [package-file-to-extract]

    pkg_custom [target-directory] [output-file]

    pkg_custom [options] [target-directory]
        -c | --contentid        make package using content id

    pkg_custom [options] npdrm-package
        -l | --list             list packaged files
        -x | --extract          extract package

    pkg_custom [options]
        --version               print revision
        --help                  print this message

Drag & drop a folder named with the content id for automatic PKG creation.
If the folder does not a content id, `CUSTOM-INSTALLER_00-0000000000000000` will be used as this content id. 

Example:
   CUSTOM-INSTALLER_00-PACKAGEINSTALLER
     |_ dev_blind\
     |  |_ vsh\
     |     |_ resource\
     |        |_ coldboot.raf
     |_ dev_hdd0\
        |_ boot_plugins.txt

If the folder contains a PARAM.SFO, the PKG is created based on the content type indicated in the file.

   EP0000-BLES12345_00-GAME000000000000
     |_ ICON0.PNG
     |_ PARAM.SFO  <= content id = HG
     |_ USRDIR\
        |_ EBOOT.BIN

Drag & drop a PKG over pkg_custom.exe or pkg_custom.py to extract the package.


## Requirements

- Python 2.7.18 (32-bit) [Download](https://python.org/downloads/release/python-2718)
- pywin32: python -m pip install pywin32
- pyinstaller: python -m pip install pyinstaller

## Credits

- **CaptainCPS-X** - Python package creator
- **lmn7** - Performance improvements, path traversal, large package support
- **aldostools** - Additional performance improvements, GUI improvements, support custom & standard PKG, executable
