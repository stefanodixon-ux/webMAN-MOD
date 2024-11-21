#define CD_SECTOR_SIZE_2048     2048

#ifndef LITE_EDITION
#define PLAYSTATION      "PLAYSTATION "

#ifdef COBRA_ONLY
static u32 detect_cd_sector_size(const char *buffer)
{
	if(!buffer) return 2352;

	u16 sec_size[7] = {2352, 2048, 2336, 2448, 2328, 2340, 2368};
	for(u8 n = 0; n < 7; n++)
	{
		if( (!strncmp(buffer + ((sec_size[n]<<4) + 0x20), PLAYSTATION, 0xC)) ||
			(!strncmp(buffer + ((sec_size[n]<<4) + 0x19), "CD001", 5) && buffer[(sec_size[n]<<4) + 0x18] == 0x01) ) return sec_size[n];
	}

	return 2352;
}
#endif

#if defined(USE_INTERNAL_NTFS_PLUGIN) || defined(USE_INTERNAL_NET_PLUGIN)
static u32 default_cd_sector_size(size_t discsize)
{
	if(!(discsize % 0x930)) return 2352;
	if(!(discsize % 0x920)) return 2336;
	if(!(discsize % 0x990)) return 2448;
	if(!(discsize % 0x800)) return 2048;
	if(!(discsize % 0x940)) return 2368; // not supported by Cobra

	return 2352;
}
#endif
#endif

static void select_ps1emu(const char *path)
{
	webman_config->ps1emu = pad_select_netemu(path, webman_config->ps1emu);

	unmap_path("/dev_flash/ps1emu/ps1_netemu.self");
	unmap_path("/dev_flash/ps1emu/ps1_emu.self");

	if(webman_config->ps1emu)
	{
		sys_map_path("/dev_flash/ps1emu/ps1_netemu.self", "///dev_flash/ps1emu/ps1_emu.self");
		sys_map_path("/dev_flash/ps1emu/ps1_emu.self"   , "///dev_flash/ps1emu/ps1_netemu.self");

		if(pad_data.len > 0) show_msg2("ps1_netemu.self", STR_ENABLED);
	}
	else
	{
		if(pad_data.len > 0) show_msg2("ps1_emu.self", STR_ENABLED);
	}
}
