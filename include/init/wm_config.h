////////////////////////////////
typedef struct
{
	u16 version; // [0-1] 0x1337

	u8 padding0[12]; // [2-13] reserved

	u8 artemis;  // [14] 0=disabled, 1=enabled on game
	u8 wm_proxy; // [15] 0=use wm_proxy, 1=use webbrowser_plugin
	u8 lang;     // [16] 0=EN, 1=FR, 2=IT, 3=ES, 4=DE, 5=NL, 6=PT, 7=RU, 8=HU, 9=PL, 10=GR, 11=HR, 12=BG, 13=IN, 14=TR, 15=AR, 16=CN, 17=KR, 18=JP, 19=ZH, 20=DK, 21=CZ, 22=SK, 99=XX

	// scan devices settings

	u8 usb0;    // [17] 0=none, 1=scan /dev_usb000
	u8 usb1;    // [18] 0=none, 1=scan /dev_usb001
	u8 usb2;    // [19] 0=none, 1=scan /dev_usb002
	u8 usb3;    // [20] 0=none, 1=scan /dev_usb003
	u8 usb6;    // [21] 0=none, 1=scan device (find first port available from /dev_usb006 to /dev_usb128)
	u8 usb7;    // [22] 0=none, 1=scan device (find first port available from /dev_usb007 to /dev_usb128)
	u8 dev_sd;  // [23] 0=none, 1=scan /dev_sd
	u8 dev_ms;  // [24] 0=none, 1=scan /dev_ms
	u8 dev_cf;  // [25] 0=none, 1=scan /dev_cf
	u8 ntfs;    // [26] 0=use prepISO for NTFS, 1=enable internal prepNTFS to scan content

	u8 padding1[5]; // [27-31] reserved

	// scan content settings

	u8 refr;  // [32] 1=disable content scan on startup
	u8 foot;  // [33] buffer size during content scanning : 0=896KB,1=320KB,2=1280KB,3=512KB,4 to 7=1280KB
	u8 cmask; // [34] content mask

	u8 nogrp;   // [35] 0=group content on XMB, 1=no group
	u8 nocov;   // [36] 0=show covers, 1=no covers
	u8 nosetup; // [37] 0=show setup, 1=no setup
	u8 rxvid;   // [38] 0=none, 1=show Video sub-folder
	u8 ps2l;    // [39] 0=none, 1=show PS2 Classic Launcher
	u8 pspl;    // [40] 0=none, 1=show PSP Launcher
	u8 tid;     // [41] 0=none, 1=show title ID in the name of the game
	u8 use_filename;  // [42] 0=use title in PARAM.SFO, 1=show filename as name
	u8 launchpad_xml; // [43] 0=none, 1=use launchpad / PhotoGUI
	u8 launchpad_grp; // [44] 0=sort launchpad by type/name, 1=sort launchpad by name (disable grouping)
	u8 gamei;  // [45] 0=none, 1=scan GAMEI folder
	u8 roms;   // [46]  0=none, 1=ROMS group
	u8 noused; // [47]  formerly mc_app
	u8 info;   // [48]  info level: 0=Path, 1=Path + ID, 2=ID, 3=None
	u8 npdrm;  // [49]  0=none, 1=show NP games in /dev_hdd0/game
	u8 vsh_mc; // [50]  allow allocation from vsh memory container (0=none, 1=app, 2=debug, 3=fg, 4=bg)
	u8 ignore; // [51]  0=none, 1=ignore files/title id listed in /dev_hdd0/tmp/wm_res/wm_ignore.txt
	u8 root;   // [52]  0=webMAN folder, 1=root
	u8 reloadxmb; // [53] 0=use logout method, 1=use logout method + focus webMAN Games, 2=use RELOADXMB app

	u8 padding2[10]; // [54-63] reserved

	// start up settings

	u8 wmstart; // [64] 0=show start up message, 1=disable start up message (webMAN Loaded!)
	u8 lastp;   // [65] 0=none, 1=load last-played game on startup
	u8 autob;   // [66] 0=check for AUTOBOOT.ISO, 1=disable check for AUTOBOOT.ISO
	char autoboot_path[256]; // [67-322] autoboot path (default: /dev_hdd0/PS3ISO/AUTOBOOT.ISO)
	u8 delay;   // [323] 0=none, 1=delay loading of AUTOBOOT.ISO/last-game (Disc Auto-start)
	u8 bootd;   // [324] Wait for any USB device to be ready: 0, 5, 9 or 15 seconds
	u8 boots;   // [325] Wait additionally for each selected USB device to be ready: 0, 3, 5, 10, 15 seconds
	u8 nospoof; // [326] 0=spoof fw version < 4.53, 1=don't spoof fw version (default)
	u8 blind;   // [327] 0=none, 1=enable /dev_blind on startup
	u8 spp;     // [328] 0=none, 1=disable syscalls, offline: 2=lock PSN, 4=offline ingame
	u8 noss;    // [329] 0=allow singstar icon, 1=remove singstar icon
	u8 nosnd0;  // [330] 0=allow SND0.AT3/ICON1.PAM, 1=mute SND0.AT3/ICON1.PAM
	u8 dsc;     // [331] 0=none, 1=disable syscalls if physical disc is inserted
	u8 noBD;    // [332] 0=normal, 1=apply noBD patch
	u8 music;   // [333] 0=none, 1=play music on startup
	u8 auto_fixclock;// [334] 0=none, 1=auto fix clock using http://ps3.aldostools.org/date.php

	u8 padding3; // [335] reserved

	// fan control settings

	u8 fanc;      // [336] 1 = enabled, 0 = disabled (syscon)
	u8 man_speed; // [337] manual fan speed (calculated using man_rate)
	u8 dyn_temp;  // [338] max temp for dynamic fan control (0 = disabled)
	u8 man_rate;  // [339] % manual fan speed (0 = dynamic fan control)
	u8 ps2_rate;  // [340] % ps2 fan speed
	u8 nowarn;    // [341] 0=show warning, 1=no warning
	u8 minfan;    // [342] minimum fan speed (25%)
	u8 chart;     // [343] 0=none, 1=log to CPU/RSX/FAN Chart
	u8 maxfan;    // [344] maximum fan speed (80%)
	u8 man_ingame;// [345] in-game increment for manual fan speed

	u8 padding4[6]; // [346-351] reserved

	// combo settings

	u8  nopad;      // [352] unused
	u8  keep_ccapi; // [353] 0=disable syscalls keep CCAPI, 1=disable syscalls removes CCAPI
	u32 combo;      // [354-358] combo  flags
	u32 combo2;     // [359-363] combo2 flags
	u8  sc8mode;    // [364] 0/4=Remove cfw syscall disables syscall8 / PS3MAPI=disabled, 1=Keep syscall8 / PS3MAPI=enabled
	u8  nobeep;     // [365] 0=beep, 1=no beeps

	u8 padding5[20]; // [366-385] reserved

	// ftp server settings

	u8  bind;              // [386] 0=allow remote FTP/WWW services, 1=disable remote access to FTP/WWW services
	u8  ftpd;              // [387] 0=allow ftp service, 1=disable ftp service
	u16 ftp_port;          // [388-389] default: 21
	u8  ftp_timeout;       // [390] 0=20 seconds, 1-255= number of minutes
	char ftp_password[20]; // [391-410]
	char allow_ip[16];     // [411-426] block all remote IP addresses except this

	u8 padding6[7]; // [427-433] reserved

	// net server settings

	u8  netsrvd;  // [434] 0=none, 1=enable local ps3netsrv
	u16 netsrvp;  // [435-436] local ps3netsrv port

	u8 padding7[13]; // [437-450] reserved

	// net client settings

	u8   netd[5];     // [451-455] 0..4
	u16  netp[5];     // [456-465] port 0..65535
	char neth[5][16]; // [466-546] ip 255.255.255.255
	u8 nsd;           // [546] Scan remote sub-directories

	u8 padding8[32];  // [547-578] reserved

	// mount settings

	u8 bus;       // [579] 0=enable reset USB bus, 1=disable reset USB bus
	u8 fixgame;   // [580] 0=Auto, 1=Quick, 2=Forced, 3=Disable auto-fix game
	u8 ps1emu;    // [581] 0=ps1emu, 1=ps1_netemu
	u8 autoplay;  // [582] 0=none, 1=Auto-Play after mount
	u8 ps2emu;    // [583] 0=ps2emu, 1=ps2_netemu
	u8 ps2config; // [584] 0=enable auto lookup for PS2 CONFIG, 1=disable auto lookup for PS2 CONFIG
	u8 minfo;     // [585] Mount info level: 0=Both messages, 1=After mount, 2=Previous to mount, 3=none
	u8 deliso;    // [586] 0=none, 1=delete cached PS2ISO copied from net/ntfs
	u8 auto_install_pkg; // [587] 0=auto install PKG when a .ntfs[BDFILE] is mounted, 1=no auto install PKG
	u8 app_home;  // [588] 0=mount folders in app_home, 1=do not mount app_home
	u8 ps1rom;    // [589] 0=ps1_rom.bin, 1=ps1_bios.bin

	u8 padding9[5]; // [590-594] reserved

	// profile settings

	u8 profile;          // [595] User profile [0-5]
	char uaccount[9];    // [596-604] default  user account (not used)
	u8 admin_mode;       // [605] 0=USER MODE, 1=ADMIN MODE / requires !(webman_config->combo & SYS_ADMIN)
	u8 unlock_savedata;  // [606] 0=none, 1=auto unlock savedata on file operations (copy/ftp/download) // deprecated //

	u8 padding10[4]; // [607-610] reserved

	// misc settings

	u8 default_restart;  // [611] default restart mode set by /restart.ps3?<mode>$
	u8 poll;             // [612] poll all usb drives every 2 minutes to keep them awake

	u32 rec_video_format; // [613-617]
	u32 rec_audio_format; // [618-623]

	u8 auto_power_off; // [624] 0 = prevent auto power off on ftp, 1 = allow auto power off on ftp (also on install.ps3, download.ps3)

	u8 ps3mon;  // [625] 0 = none; 1 = load ps3mon.sprx on startup
	u8 qr_code; // [626] 0 = none; 1 = show qr code
	u8 prodg;   // [627] 0 = remap app_home to /dev_hdd0/packages, 1 = don't remap app_home

	u8 gpu_core; // [628] xmb: 0 = default; mhz / 50
	u8 gpu_vram; // [629] xmb: 0 = default; mhz / 25

	u8 homeb; // [630] 0=none, 1=show home button in original GUI
	char home_url[255]; // [631-885] url for home button, search path for files not found or path for default application in app_home

	u8 sman;     // [886] 0=original GUI, 1=sman GUI
	u8 msg_icon; // [887] 0=VSH notify with icon, 1=vshtask_notify (info)

	u8 gpu2_core; // [888] in-game: 0 = default; mhz / 50
	u8 gpu2_vram; // [889] in-game: 0 = default; mhz / 25

	u8 padding11[28]; // [890-917] reserved

	// spoof console id

	u8 sidps; // [918] 0=none, 1=spoof IDPS
	u8 spsid; // [919] 0=none, 1=spoof PSID
	char vIDPS1[17]; // [920-936]
	char vIDPS2[17]; // [937-953]
	char vPSID1[17]; // [954-970]
	char vPSID2[17]; // [971-987]

	u8 padding13[24]; // [988-1011] reserved

	u8 resource_id[12];
	//u8 wallpaper_id;	// [1012]  [0]: 0=random, 1-255: select specific #.png in /dev_hdd0/tmp/wallpaper on each boot
	//u8 earth_id;		// [1013]  [1]: 0=random, 1-255: select specific #.qrc in /dev_hdd0/tmp/earth on each boot
	//u8 canyon_id;		// [1014]  [2]: 0=random, 1-255: select specific #.qrc in /dev_hdd0/tmp/canyon on each boot
	//u8 lines_id;		// [1015]  [3]: 0=random, 1-255: select specific #.qrc in /dev_hdd0/tmp/lines on each boot
	//u8 coldboot_id;	// [1016]  [4]: 0=random, 1-255: select specific #.ac3 in /dev_hdd0/tmp/coldboot on each boot
	//u8 theme_id;		// [1017]  [5]: 0=random, 1-255: select specific #.p3t in /dev_hdd0/tmp/theme on each boot
	//u8 last_theme_id;	// [1018]  [6]: 0=none, 1-255: last selected theme (used to prevent install the current theme again)
	//u8 impose_id;		// [1019]  [7]: 0=random, 1-255: select specific #.rco in /dev_hdd0/tmp/impose on each boot
	//u8 psn_icons_id	// [1020]  [8]: 0=random, 1-255: select specific #.rco in /dev_hdd0/tmp/psn_icons on each boot
	//u8 clock_id		// [1021]  [9]: 0=random, 1-255: select specific #.rco in /dev_hdd0/tmp/system_plugin on each boot
						// [1022] [10]: reserved
						// [1023] [11]: reserved
} /*__attribute__((packed))*/ WebmanCfg;

static u8 wmconfig[sizeof(WebmanCfg)];
static WebmanCfg *webman_config = (WebmanCfg*) wmconfig;

#ifdef COBRA_ONLY
static u8 cconfig[sizeof(CobraConfig)];
static CobraConfig *cobra_config = (CobraConfig*) cconfig;
#endif

////////////////////////////////

// Use: /setup.ps3@<pos>=<value>|<pos>:<string>|... to modify the settings in /dev_hdd0/tmp/wm_config.bin
