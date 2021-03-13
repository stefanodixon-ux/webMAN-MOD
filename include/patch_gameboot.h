#ifdef PS3MAPI

#define BASE_PATCH_ADDRESS		0x800000UL

static u32 patched_address1 = BASE_PATCH_ADDRESS;
static u32 patched_address2 = BASE_PATCH_ADDRESS;
static u32 patched_address3 = BASE_PATCH_ADDRESS;
static u32 patched_address4 = BASE_PATCH_ADDRESS;

static void patch_gameboot(u8 boot_type)
{
	if(IS_ON_XMB && (file_size("/dev_flash/vsh/resource/custom_render_plugin.rco") >= 300000))
	{
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
						file_copy(src_path, path, COPY_WHOLE_FILE);
					}
				}
			}
		}
	}
}
#endif
