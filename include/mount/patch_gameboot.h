#ifdef PATCH_GAMEBOOT

#define MIN_RCO_SIZE			300000
#define BASE_PATCH_ADDRESS		0x800000UL

static u32 patched_address1 = BASE_PATCH_ADDRESS;
static u32 patched_address2 = BASE_PATCH_ADDRESS;
static u32 patched_address3 = BASE_PATCH_ADDRESS;
static u32 patched_address4 = BASE_PATCH_ADDRESS;

static bool is_patching_gameboot = false;

static void patch_gameboot(u8 boot_type)
{
	if(is_patching_gameboot) return;

	if(IS_ON_XMB && (file_size(CUSTOM_RENDER_PLUGIN_RCO) >= MIN_RCO_SIZE))
	{
		is_patching_gameboot = true;

		u32 pid = get_current_pid();

		if(pid)
		{
			const char *ids[] = { "non", "ps1", "ps2", "ps3", "psp", "bdv", "dvd", "rom", // 0-7
								  "sns", "nes", "gba", "gen", "neo", "pce", "mam", "fba", // 8-15
								  "ata", "gby", "cmd", "ids"}; // 16-19

			if(boot_type > 19) boot_type = 0;

			const char *id = ids[boot_type];

			u32 address;
			char value[16]; int len;

			len = sprintf(value, "%s_%sboot", "page", "game"); // find "page_gameboot"
			address = (u32)ps3mapi_find_offset(pid, BASE_PATCH_ADDRESS, 0x1800000, 4, value, len, value, patched_address1);

			if(address > BASE_PATCH_ADDRESS)
			{
				len = sprintf(value, "%s_", id); // patch "xxx__gameboot"
				ps3mapi_patch_process(pid, address, value, len, 0); patched_address1 = address;

				len = sprintf(value, "%slogo", "ps3"); // find ps3logo
				address = ps3mapi_find_offset(pid, patched_address1, 0x1800000, 4, value, len, value, patched_address2);

				if(address > patched_address1)
				{
					len = sprintf(value, "%slogo", (boot_type == 3) ? "psx" : id); // patch xxxlogo
					ps3mapi_patch_process(pid, address, value, len, 0); patched_address2 = address;
				}

				bool is_bdv_dvd = (boot_type == 5) || (boot_type == 6);
				const char *anim = is_bdv_dvd ? "other" : "game";

				len = sprintf(value, "%s_%sboot", "anim", "game"); // find anim_gameboot  for ps3/ps2/ps1/psp/rom
				address = ps3mapi_find_offset(pid, patched_address1, 0x1800000, 4, value, len, value, patched_address3);

				if(address > patched_address1)
				{
					len = sprintf(value, "%s__%sboot", id, anim); // patch xxx__otherboot / xxx__gameboot
					ps3mapi_patch_process(pid, address, value, len + 1, 0); patched_address3 = address;
				}

				len = sprintf(value, "%s_%sboot", "anim", "other"); // find anim_otherboot for dvd/bdv
				address = ps3mapi_find_offset(pid, patched_address1, 0x1800000, 4, value, len, value, patched_address4);

				if(address > patched_address1)
				{
					len = sprintf(value, "%s__%sboot", id, anim); // patch xxx__otherboot / xxx__gameboot
					ps3mapi_patch_process(pid, address, value, len + 1, 0); patched_address4 = address;
				}
			}

			char path[48];
			if(isDir(WM_GAMEBOOT_PATH))
			{
				map_patched_modules();

				concat_path2(path, WM_GAMEBOOT_PATH, id, "_boot_stereo.ac3");

				const char *snd = file_exists(path) ? path : NULL;
				sys_map_path(GAMEBOOT_MULTI_AC3,  snd);
				sys_map_path(GAMEBOOT_STEREO_AC3, snd);

				const char *media[6] = {"PIC0.PNG", "PIC1.PNG", "PIC2.PNG", "SND0.AT3", "ICON1.PAM", "ICON0.PNG"};
				for(u8 i = 0; i < 6; i++)
				{
					concat3(path, PKGLAUNCH_DIR, "/PS3_GAME/", media[i]);
					if(not_exists(path))
					{
						char src_path[40];
						sprintf(src_path, "%s/%s_%s", WM_GAMEBOOT_PATH, id, media[i]);
						file_copy(src_path, path);
					}
				}
			}
		}

		is_patching_gameboot = false;
	}
}

static void patch_gameboot_by_type(const char *path) // called only by set_mount_type
{
	// customize gameboot per console emulator using DeViL303's custom_render_plugin.rco
	if(IS_ON_XMB && (file_size(CUSTOM_RENDER_PLUGIN_RCO) >= MIN_RCO_SIZE))
	{
		if(mount_unk == EMU_PSX)
			patch_gameboot(1); // PS1
		else if(mount_unk == EMU_PS2_DVD || mount_unk == EMU_PS2_CD)
			patch_gameboot(2); // PS2
		else if(mount_unk == EMU_PS3)
			patch_gameboot(3); // PS3
		else if(mount_unk == EMU_PSP)
			patch_gameboot(4); // PSP
		else if(mount_unk == EMU_BD)
			patch_gameboot(5); // BDV
		else if(mount_unk == EMU_DVD)
			patch_gameboot(6); // DVD
		else if(mount_unk == EMU_ROMS)
		{
			// "rom", "sns", "nes", "gba", "gen", "neo", "pce", "mam", "fba", "ata", "gby", "cmd", "ids" // 7-19

			if(strcasestr(path, "SNES")) // MSNES, SNES, SNES9X, SNES9X2005, SNES9X2010, SNES9X_NEXT
				patch_gameboot(8); // sns
			else if(strstr(path, "NES") || strcasestr(path, "/NES/") || strcasestr(path, "FCEUMM")) // NES, NESTOPIA, QNES, FCEUMM
				patch_gameboot(9); // nes
			else if(strcasestr(path, "GBA") || strcasestr(path, "VBA") || strstr(path, "DS"))  // GBA, MGBA, VBA, DS
				patch_gameboot(10); // gba
			else if(strcasestr(path, "SEGA") || strstr(path, "GEN") || strcasestr(path, "/GEN/") || strcasestr(path, "GENESIS") || strcasestr(path, "MEGAD") || strcasestr(path, "MASTER") || strcasestr(path, "SG1000") || strcasestr(path, "PICO") || strcasestr(path, "GG") || strcasestr(path, "GEARBOY")) // GEN, GENESIS, MEGADRIVE, GEARBOY, GG, PICO
				patch_gameboot(11); // gen
			else if(strcasestr(path, "NEO") || strcasestr(path, "NGP")) // NEOCD, FBNEO, NEO, NEOGEO, NGP
				patch_gameboot(12); // neo
			else if(strcasestr(path, "PCE") || strcasestr(path, "PCFX") || strcasestr(path, "SGX")) // PCE, PCFX, SGX
				patch_gameboot(13); // pce
			else if(strcasestr(path, "MAME") || strcasestr(path, "MIDWAY"))	// MAME, MAME078, MAME2000, MAME2003, MAMEPLUS
				patch_gameboot(14); // mam
			else if(strcasestr(path, "FBA"))	// FBA, FBA2012
				patch_gameboot(15); // fba
			else if(strcasestr(path, "ATARI") || strcasestr(path, "STELLA") || strcasestr(path, "HANDY") || strcasestr(path, "LYNX") || strcasestr(path, "JAGUAR")) // ATARI, ATARI2600, ATARI5200, ATARI7800, HATARI, HANDY, STELLA
				patch_gameboot(16); // ata
			else if(strcasestr(path, "GB") || strcasestr(path, "GAMBATTE") || strcasestr(path, "VB"))  // GB, GBC, GAMBATTE, VBOY
				patch_gameboot(17); // gby
			else if(strcasestr(path, "AMIGA") || strcasestr(path, "VICE") || strcasestr(path, "VICE") || strcasestr(path, "/X"))  // AMIGA, VICE, X64, X128, XPET, XCBM*
				patch_gameboot(18); // cmd
			else if(strcasestr(path, "DOOM") || strcasestr(path, "QUAKE"))  // DOOM, QUAKE, QUAKE2
				patch_gameboot(19); // ids
			else
				patch_gameboot(7); // rom: 2048, BMSX, BOMBER, CANNONBALL, CAP32, COLECO, DOSBOX, FMSX, FUSE, GW, INTV, JAVAME, LUA, MSX, MSX2, NXENGINE, O2EM, PALM, POKEMINI, SCUMMVM, SGX, TGBDUAL, THEODORE, UZEM, VECX, WSWAM, ZX81
		}
		else
			patch_gameboot(0); // None
	}
}

static void set_mount_type(const char *path)
{
	if(strstr(path, "PSXISO"))
		mount_unk = EMU_PSX; // PS1
	else if(strstr(path, "PS2ISO"))
		mount_unk = EMU_PS2_DVD; // PS2
	else if(strstr(path, "PS3ISO") || strstr(path, "/GAME"))
		mount_unk = EMU_PS3; // PS3
	else if(strstr(path, "PSPISO"))
		mount_unk = EMU_PSP; // PSP
	else if(strstr(path, "BDISO"))
		mount_unk = EMU_BD; // BDV
	else if(strstr(path, "DVDISO"))
		mount_unk = EMU_DVD; // DVD
	else if(strcasestr(path, "/ROMS"))
		mount_unk = EMU_ROMS; // ROMS

	patch_gameboot_by_type(path);
}
#endif //#ifdef PATCH_GAMEBOOT
