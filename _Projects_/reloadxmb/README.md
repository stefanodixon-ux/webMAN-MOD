# Reload XMB

This is a simple application used to refresh the cached XML contents on XMB.


## Usage

It can be used as a standalone application if you set the `Home` setting in /setup.ps3
to `/dev_hdd0/game/RELOADXMB`. Check the Home option and save the settings.

The tool is installed automatically by webMAN MOD installer or updater.

RELOADXMB is called when the option Reload XML & XMB is selected from webMAN Setup menu on XMB.

The web command /reloadxmb.ps3 mounts the application and executes it from app_home/PS3_GAME (aka ★ PlayStation)


## Requirements

★ app_home/PS3_GAME (aka ★ PlayStation) must be enabled in category_game.xml (aka Game column on XMB menu)


## How it works

RELOADXMB is a standard game application that simply returns to XMB when it is loaded.

In the process, VSH is unloaded from memory and reloaded when the application exits.

All the previously cached XML menus and images are unloaded from memory and reloaded on demand.

Alternatives to Reload XMB applcation are:
- Execute `prepISO` to scan the contents on USB devices, refresh XML and reload XMB.
- Load any other homebrew, game or application and return to XMB.
- Start `PSP Remote Play` and press `O` to exit to XMB.
- Restart the PS3 system.


## Credits

- **aldostools** - Coding, implementation.
- **DeViL303** - Concept, XMBML research.
