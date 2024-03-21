# prepISO

prepISO (formerly prepNTFS) is a tool used to scan the contents stored on USB devices
formatted in NTFS, exFAT and ext2/3/4.

It's based on prepNTFS tool released by DeanK.

## Usage

Execute `prepISO` from XMB to scan the contents, refresh XML and reload XMB.

Hold `(X)` when the program is launched to keep existing cached files (*.ntfs).
By default all the cached files from previous scans are removed from `/dev_hdd0/tmp/wmtmp`

During the content scanning, the screen will turn black for a moment and return to XMB.

Although it's not required, it is recommended to disable the option `/dev_ntfs` in `/setup.ps3`
prepISO is equivalent to that option, but has additional features.


## Features

- Scan all attached USB devices formatted in NTFS, exFAT and ext2/3/4
- Scans the following folders in root directory or /PS3 directory:
  - /PS3ISO - *Scans for PS3 games in ISO format*
  - /PSXISO - *Scans for PS1 games in CUE/BIN, CUE/ISO, CCD/BIN format.*
  - /PS2ISO - *Scans for PS2 games in ISO format*
  - /PSPISO - *Scans for PSP games in ISO format*
  - /BDISO  - *Scans for Bluray discs in ISO format*
  - /DVDISO - *Scans for DVD discs in ISO format*
  - /PKG or /Packages or /packages - *Scans for pkg files and creates an ISO per file*
  - /VIDEO or /MOVIES - *Scans for video files and creates an ISO per file*
  - /MUSIC - *Scans for audio files and creates an ISO per file*
  - /BDFILE - *Scans for any files and creates an ISO per file*
  - /THEME - *Scans for theme files and creates an ISO per file*
  - /UPDATE - *Scans for PUP files and creates an ISO per file*
  - /ROMS - *Scans for ROM files and creates an ISO per file*
- Single ISO and split ISO support in NTFS
- Extracts the ICON0/PIC1/PIC2/PARAM.SFO for usage during mount process
- Auto-fix games requiring a FW version higher than current FW version (obsolete)
- Supports ISO, BIN, IMG, MDF disc images
- Supports CUE and CCD cuesheet files for PSX games.
- Automatic unmount of current mounted disc when program starts
- Automatic refresh XML after scan

NOTE: The fake ISO files created will appear under Blu-ray® and DVD section of webMAN Game menu.


# How it works

The tool scan the attached devices formatted in NTFS, exFAT and ext2/3/4 
and create a cache file .ntfs[] in `/dev_hdd0/tmp/wmtmp`

Other multimedia files used during the file mounting are extracted to that folder too.

The cache file .ntfs[] contains a list of the sectors used by the ISO file on the USB.
Two internal lists are generated: one with the initial sector of each segment (contiguous sectors)
and the other list for the size of the segment.

For PS1 games, a third list includes the CD track information (address control, track number & LBA).

The cache file for ISO files stored in subfolders are stored as: "[subfolder] filename.ntfs[folder]"
Example: `/dev_hdd0/tmp/wmtmp/[Racing] my game.ntfs[PS3ISO]`

When an exFAT/NTFS game is mounted, the information in the cached file is passed to the `rawseciso plugin` (aka raw_iso.sprx)
The rawseciso plugin acts as a proxy between Cobra payload and the storage device.

The plugin receives the SCSI read requests from Cobra, maps the LBA access to the physical sectors in the disk,
access the raw sectors and returns the data to Cobra which redirect it to the SCSI driver / Bluray drive.

The fake ISO files or .ntfs[BDFILE] use 2 files:
- One .iso file for the ISO header containing a single file.
- The cache file contains the raw sectors referenced by the header.


## Credits

- **DeanK** - Original concept, coding, implementation.
- **aldostools** - Modification, additional features and optimizations.
- **Estwald** - Port of library for NTFS/ext2/3/4, fake ISO files concept.
- **lmirel** - Port of library for exFAT
- **Evilnat** - Support for encrypted 3k3y/Redump ISOs
- **Cobra Team**
