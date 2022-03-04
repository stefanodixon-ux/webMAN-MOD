# VSH Menu for webMAN

VSH Menu is an alternative UI for accessing your webMAN games on the XMB.

It's based on Simple VSH Menu made by littlebalup and from @3141card PS3 VSH menu PoC.


## Usage

Press [Select] button for 3 seconds from XMB to open/close the Menu.

- L/R Sticks = Navigate
- PAD Arrows = Navigate
- `L1` / `R1`    = Prev / Next page
- `CROSS`      = Load the game title
- `CIRCLE`     = Exit to XMB

Note: Screenshots are saved to /dev_hdd0.


## Features

- Shows system info (CFW version, Cobra version, IP address, CPU/RSX temperatures)
- Shows free space on hdd0, usb devices and dev_blind
- Unmount current game
- Eject disc / Insert disc
- Mount /net0, /net1 or /net2
- Change fan speed & mode
- Shows System Info
- Refresh XML (/dev_hdd0/xmlhost/game_plugin/mygames.xml)
- Toggle gameDATA
- Backup disc to HDD
- XMB screen capture
- Shutdown PS3
- Reboot PS3 (soft* & hard methods). *lpar reboot
<br>

- Rebug menu:
  - Toggle Rebug Mode
  - Toggle XMB Mode
  - Toggle Debug Menu
  - Disable Cobra
  - Disable webMAN MOD
  - Recovery Mode
<br>

- File Manager (file browsing & basic operations)
- Plugins Manager (boot_plugins.txt)


# How it works

The plugins is stored in `/dev_hdd0/tmp/wm_res/wm_vsh_menu.sprx`.
It is loaded dynamically by webMAN MOD and unloaded from memory exits to XMB.

The menu is rendered writting directly to XMB video buffer using the hack
developed to 3141card for VSH menu.

When an option is selected, the menu sends a web command to webMAN MOD.
The request is processed by webMAN MOD and Cobra payload.

Requires webMAN MOD 1.43.07 or later already installed to work.


## Changelog:

v1.21:
	Added menu navigation using Left/Right sticks
	Improved game pad reading

v0.9:
	Added options to open /setup.ps3 and File Manager (req. webMAN MOD 1.43.10 or later)
	Increased pad response when is menu on
	Reduced CPU usage using cached info for cfw version, ip address, temperatures and devices during frame display (~12Hz)

v0.8:
	Added "Unload VSH Menu" option to Rebug Menu and made it the default/first option
	Display of CFW version now uses code provided by Orion
	Updated the Rebug Mode toggler

v0.7
	Added option to disable startup message
	Fixed a bug with left/right on Rebug menu
	New default background image by jriko

v0.6:
	Added options from Rebug VSH Menu made by Orion

v0.5:
	Renamed to VSH Menu for webMAN
	Integrated with webMAN MOD 1.43.07
	Serveral changes since 0.4 :)

v0.4:
	Added "Eject Disc" and "Insert Disc" options (for physical disc, not disc images)
	Scrolling menu with arrows. 
	Changed code of the the storage devices size info:
		- Now displays size of the storage device in add of the available free space
		- Now displays size in MB if less than 1GB (size and/or free space)
		- Added free space and size detection for connected flashcards (/dev_sd, /dev_ms and /dev_cf)
		- Added free space and size detection for /dev_blind (if mounted)
		- Added icons for each device type (hdd, usb, flashcard, internal flash)
	Added thermometer icon for temperatures:
		- Icon is color blue if CPU and RSX below 50°C
		- Icon is color green if hotter of CPU and RSX between 50°C and 65°C
		- Icon is color orange if hotter of CPU and RSX between 65°C and 75°C
		- Icon is color red if CPU or RSX over 75°C
	Some font size slightly reduced.
	Layout modifications (again...).
	
v0.3:
    Added Firmware version info (spoofed if it is). Thanks @Zar.
    New layout, background and icons.
    (Only one english version for the moment...)

v0.2:
    Fixed bug about IP adress not shown in some cases (thanks again to @3141card )
    Added "Network connexion" type info (LAN or WLAN)

v0.1:
    first beta release

## Credits

// 2015 by littlebalup, Mysis, 3141card, Orion, aldostools

- **3141card** - PoC, vsh menu, video rendering functions and memory handling.
- **Littlebalup** - Scrolling support, enhancements and new features.
- **Orion** - Enhancements and additional features.
- **aldostools** - Modification, additional features and optimizations.
- **Mysis** - VSH exports
