# sLaunch GUI / Game Menu

sLaunch is an alternative UI for accessing your webMAN games on the XMB.
It's based on the new GUI for sMAN released by DeanK.

## Usage

L/R Sticks = Navigate
PAD Arrows = Navigate
L1 / R1    = Prev / Next page

TRIANGLE   = Side menu
CROSS      = Load the game title
CIRCLE     = Exit to XMB
L3         = Toggle between 2x5 / 4x10 items per page

SELECT     = Toggle between Favorite / Normal list
START      = Add / Remove game from Favorite list

## Features

- Menu access from XMB pressing L2+R2 or START for a few seconds
- On screen temperature display (CPU/RSX)
- Fast menu navigation in grid format (2x5 or 4x10)
- Side menu
  - Unmount - Unload mounted game (ISO / folder)
  - Refresh - Scan for content / instanst list without reload XMB
  - gameDATA - Redirects /dev_hdd0/game to USB device
  - Disable CFW Syscalls (press LEFT/RIGHT over gameDATA)
  - Restart - Restarts the PS3
  - Shutdown - Turn off the PS3
  - Unload webMAN
  - Setup - Open configuration page
  - File Manager (press LEFT/RIGHT over Setup)

# How it works

The file `/dev_hdd0/tmp/wmtmp/slist.bin` containing the list of games
is built by webMAN MOD when the XML content list is scanned.

A second `/dev_hdd0/tmp/wmtmp/slist1.bin` is used for favorite games.

NOTE: The internal data structure is very different to sLaunch menu in sMAN.

The menu is rendered writting directly to XMB video buffer using the hack
developed to 3141card for VSH menu and extended by DeanK to display on full screen.

When a game is selected, the menu sends a web command to webMAN MOD to mount the game.
The rest of the game loading process is perfomed by webMAN MOD and Cobra payload.

## Compilation Notes

It project is compiled with cc/gcc toolchain version 4.6.2 on Windows.
4.1.1 Linux compiler might produce buggy code with optimizations (even with -O1 actually).

## Credits

- DeanK - Original sLaunch GUI concept and coding.
- aldostools - Modification, additional features and optimizations.
- 3141card - Video rendering functions and memory handling.
- DarjanKrijan - ARGB support and testing
- Mysis - VSH exports
