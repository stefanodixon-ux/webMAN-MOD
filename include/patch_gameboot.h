#ifdef PATCH_GAMEBOOT

#define MIN_RCO_SIZE			300000
#define BASE_PATCH_ADDRESS		0x800000UL

static u32 patched_address1 = BASE_PATCH_ADDRESS;
static u32 patched_address2 = BASE_PATCH_ADDRESS;
static u32 patched_address3 = BASE_PATCH_ADDRESS;
static u32 patched_address4 = BASE_PATCH_ADDRESS;

static bool is_patching_gameboot = false;

static void set_mount_type(const char *path)
{
	if(strstr(path, "PSXISO"))
		mount_unk = EMU_PSX; // PS1
	else if(strstr(path, "PS2ISO"))
		mount_unk = EMU_PS2_DVD; // PS2
	else if(strstr(path, "PS3ISO"))
		mount_unk = EMU_PS3; // PS3
	else if(strstr(path, "PSPISO"))
		mount_unk = EMU_PSP; // PSP
	else if(strstr(path, "BDISO"))
		mount_unk = EMU_BD; // BDV
	else if(strstr(path, "DVDISO"))
		mount_unk = EMU_DVD; // DVD
	else if(strstr(path, "/ROMS"))
		mount_unk = EMU_ROMS; // ROMS
}

static void patch_gameboot(u8 boot_type)
{
	if(is_patching_gameboot) return;

	if(IS_ON_XMB && (file_size("/dev_flash/vsh/resource/custom_render_plugin.rco") >= MIN_RCO_SIZE))
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
				ps3mapi_patch_process(pid, address, value, len); patched_address1 = address;

				len = sprintf(value, "%slogo", "ps3"); // find ps3logo
				address = ps3mapi_find_offset(pid, patched_address1, 0x1800000, 4, value, len, value, patched_address2);

				if(address > patched_address1)
				{
					len = sprintf(value, "%slogo", (boot_type == 3) ? "psx" : id); // patch xxxlogo
					ps3mapi_patch_process(pid, address, value, len); patched_address2 = address;
				}

				const char *anim = ((boot_type == 5) || (boot_type == 6)) ? "other" : "game";

				len = sprintf(value, "%s_%sboot", "anim", "game"); // find anim_gameboot  for ps3/ps2/ps1/psp/rom
				address = ps3mapi_find_offset(pid, patched_address1, 0x1800000, 4, value, len, value, patched_address3);

				if(address > patched_address1)
				{
					len = sprintf(value, "%s__%sboot", id, anim); // patch xxx__otherboot / xxx__gameboot
					ps3mapi_patch_process(pid, address, value, len + 1); patched_address3 = address;
				}

				len = sprintf(value, "%s_%sboot", "anim", "other"); // find anim_otherboot for dvd/bdv
				address = ps3mapi_find_offset(pid, patched_address1, 0x1800000, 4, value, len, value, patched_address4);

				if(address > patched_address1)
				{
					len = sprintf(value, "%s__%sboot", id, anim); // patch xxx__otherboot / xxx__gameboot
					ps3mapi_patch_process(pid, address, value, len + 1); patched_address4 = address;
				}
			}

			char path[48];
			if(isDir("/dev_hdd0/tmp/gameboot"))
			{
				map_patched_modules();

				sprintf(path, "%s/%s_boot_stereo.ac3", "/dev_hdd0/tmp/gameboot", id);

				const char *snd = file_exists(path) ? path : NULL;
				sys_map_path("/dev_flash/vsh/resource/gameboot_multi.ac3",  snd);
				sys_map_path("/dev_flash/vsh/resource/gameboot_stereo.ac3", snd);

				const char *media[6] = {"PIC0.PNG", "PIC1.PNG", "PIC2.PNG", "SND0.AT3", "ICON1.PAM", "ICON0.PNG"};
				for(u8 i = 0; i < 6; i++)
				{
					sprintf(path, "%s/PS3_GAME/%s", PKGLAUNCH_DIR, media[i]);
					if(not_exists(path))
					{
						char src_path[40];
						sprintf(src_path, "%s/%s_%s", "/dev_hdd0/tmp/gameboot", id, media[i]);
						file_copy(src_path, path);
					}
				}
			}
		}

		is_patching_gameboot = false;
	}
}

static void patch_gameboot_by_type(const char *path)
{
	// customize gameboot per console emulator using DeViL303's custom_render_plugin.rco
	if(IS_ON_XMB && (file_size("/dev_flash/vsh/resource/custom_render_plugin.rco") >= MIN_RCO_SIZE))
	{
		set_mount_type(path);

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

			if(strstr(path, "SNES")) // MSNES, SNES, SNES9X, SNES9X2005, SNES9X2010, SNES9X_NEXT
				patch_gameboot(8); // sns
			else if(strstr(path, "NES") || strstr(path, "FCEUMM")) // NES, NESTOPIA, QNES, FCEUMM
				patch_gameboot(9); // nes
			else if(strstr(path, "GBA") || strstr(path, "VBA") || strstr(path, "DS"))  // GBA, MGBA, VBA, DS
				patch_gameboot(10); // gba
			else if(strstr(path, "GEN") || strstr(path, "MEGAD") || strstr(path, "MASTER") || strstr(path, "SEGACD") || strstr(path, "SG1000") || strstr(path, "PICO") || strstr(path, "GG") || strstr(path, "GEARBOY")) // GEN, GENESIS, MEGADRIVE, GEARBOY, GG, PICO
				patch_gameboot(11); // gen
			else if(strstr(path, "NEO") || strstr(path, "NGP")) // NEOCD, FBNEO, NEO, NEOGEO, NGP
				patch_gameboot(12); // neo
			else if(strstr(path, "PCE") || strstr(path, "PCFX") || strstr(path, "SGX")) // PCE, PCFX, SGX
				patch_gameboot(13); // pce
			else if(strstr(path, "MAME"))	// MAME, MAME078, MAME2000, MAME2003, MAMEPLUS
				patch_gameboot(14); // mam
			else if(strstr(path, "FBA"))	// FBA, FBA2012
				patch_gameboot(15); // fba
			else if(strstr(path, "ATARI") || strstr(path, "STELLA") || strstr(path, "HANDY") || strstr(path, "LYNX") || strstr(path, "JAGUAR")) // ATARI, ATARI2600, ATARI5200, ATARI7800, HATARI, HANDY, STELLA
				patch_gameboot(16); // ata
			else if(strstr(path, "GB") || strstr(path, "GAMBATTE") || strstr(path, "VB"))  // GB, GBC, GAMBATTE, VBOY
				patch_gameboot(17); // gby
			else if(strstr(path, "AMIGA") || strstr(path, "VICE"))  // AMIGA, VICE
				patch_gameboot(18); // cmd
			else if(strstr(path, "DOOM") || strstr(path, "QUAKE"))  // DOOM, QUAKE, QUAKE2
				patch_gameboot(19); // ids
			else
				patch_gameboot(7); // rom: 2048, BMSX, BOMBER, CANNONBALL, CAP32, COLECO, DOSBOX, FMSX, FUSE, GW, INTV, JAVAME, LUA, MSX, MSX2, NXENGINE, O2EM, PALM, POKEMINI, SCUMMVM, SGX, TGBDUAL, THEODORE, UZEM, VECX, WSWAM, ZX81
		}
		else
			patch_gameboot(0); // None
	}
}
#endif //#ifdef PATCH_GAMEBOOT
