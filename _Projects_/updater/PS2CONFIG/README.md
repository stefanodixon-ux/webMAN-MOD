Here's some basic info on the "useful" PS2 config commands documented by @mrjaredbeta:
```
0x07/0x08 - can fix SPS/graphical glitches
0x09-0x0B - memory/disc image patching
0x0C - disc read speed adjustment, slows down disc read speed (slower loading, could fix loading screen freeze or improve performance on games that rely on constant stream of data)
0x0D/0x3E - can fix freeze/black screen (Piposaru 2001 for example)
0x0E-0x10 and 0x26/0x27 - FPU accuracy, fixes bugs like frozen characters
0x15 - can fix freezes
0x1A/0x1B - can fix black screens, related to IPU (FMV stuff)
0x1C-0x1E - multitap stuff
0x20 - fixes screen flickering/black screen when game/sound is still running
0x21 set to 0 - great performance fix for FMVs and sometimes ingame
0x2A - can fix black screen before/after FMV
0x2F set to 2 - sound fix for some games (Square, Kengo 3, Everybody's Golf)

Anything 0x35 and above is netemu only, so you will not see them in gx configs:
0x35 - fixes black screen in some games
0x40 - fixes fullscreen graphical glitches in some games
0x41 - fixes freezes in some games
0x42 - another method for memory patching with a large hook
0x46 - performance enhancement in games using GS download
0x47 - fixes blocky image
0x50 - enable pressure sensitive controls
```
More detailed information and examples can be found in these links:

https://www.psdevwiki.com/ps3/PS2_Emulation/PS2_Config_Commands

https://www.psdevwiki.com/ps3/PS2_Emulation#Config_Commands

https://www.psdevwiki.com/ps3/PS2_Official_Configs

https://www.psdevwiki.com/ps3/PS2_Custom_Configs

https://www.psdevwiki.com/ps3/PS2_Classics_Emulator_Compatibility_List

https://ps3.aldostools.org/ps2config.html
