#ifdef COBRA_ONLY
static void patch_ps2_demo(const char *iso_file)
{
	u32 offset, root;
	char data[0x40];
	char game_id[12];    memset(game_id,  0, sizeof(game_id));
	char title_id[0x10]; memset(title_id, 0, sizeof(title_id));

	read_file(iso_file, data, 0x3C, 0x80A0);
	root = offset = *((u32*)(data + 2)) * 0x800; // 0x105 * 800

	#define GAME_EXT_BLACKLISTED "SCUS-97275SCUS-97474SCUS-97545SCUS-97340SCUS-97341SCUS-97342SCUS-97442"

	for(u8 entry = 0; entry < 0x20; entry++)
	{
		read_file(iso_file, data, 0x3C, offset);
		if(*data == 0x3C)
		{
			char *entry_name = data + 0x21;

			if(*title_id)
			{
				if(!memcmp(entry_name, title_id, 0xD))
				{
					entry_name[3] = 'X'; // patch exe name in directory
					patch_file(iso_file, data, offset, 0x3C);
					break;
				}
			}
			else if(!memcmp(entry_name, "SYSTEM.CNF;1", 0xC))
			{
				u32 offset2 = *((u32*)(data + 0x6)) * 0x800;
				read_file(iso_file, data, 0x40, offset2);

				// parse SYSTEM.CNF
				char *exe_name = NULL;
				for(int i = 0; i < 0x2A; i++)
					if(!strncasecmp(data + i, "cdrom0:", 7)) {exe_name = data + i + 8; break;}

				if(exe_name)
				{
					memcpy(title_id, exe_name, 0xD); // S**S_000.00
					sprintf(game_id, "%.4s-%.3s%.2s", title_id, title_id + 5, title_id + 9); // S**S-00000
					if(((title_id[0] == 'S') && (title_id[3] == 'D') && (title_id[4] == '_') & (title_id[8] == '.')) || (strstr(GAME_EXT_BLACKLISTED, game_id) != NULL)) // S**D_###.##;1
					{
						exe_name[3] = 'X'; // patch SYSTEM.CNF

						cellFsChmod(iso_file, MODE);
						patch_file(iso_file, data, offset2, 0x40);

						offset = root; // re-scan root directory to patch exec name
						entry = 0;
						continue;
					}
					else
						*game_id = *title_id = 0;
					break;
				}
			}
		}
		offset += *data;
	}

	// Patch CONFIG file if demo was patched
	if(*game_id)
	{
		sys_addr_t sysmem = NULL;
		if(sys_memory_allocate(_64KB_, SYS_MEMORY_PAGE_SIZE_64K, &sysmem) == CELL_OK)
		{
			char *config = (char*)sysmem;
			char *config_buf = (char*)sysmem + _2KB_;
			sprintf(config, "%s.CONFIG", iso_file);
			int config_size = read_file(config, config_buf, _2KB_, 0);
			if((config_size > 10) && !memcmp(config_buf + (config_size - 10), game_id, 10)) // S**S-00000
			{
				save_file(config, config_buf, config_size - 10);
			}
			sys_memory_free(sysmem);
		}
	}
}
#endif