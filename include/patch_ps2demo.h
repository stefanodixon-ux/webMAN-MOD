#ifdef COBRA_ONLY
static void patch_ps2_demo(const char *iso_file)
{
	u32 offset, root;
	char data[0x40];
	char title_id[0x10]; memset(title_id, 0, sizeof(title_id));

	read_file(iso_file, data, 0x3C, 0x80A0);
	root = offset = *((u32*)(data + 2)) * 0x800; // 0x105 * 800

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
					write_file(iso_file, CELL_FS_O_WRONLY, data, offset, 0x3C, false);
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
					memcpy(title_id, exe_name, 0xD);
					if((title_id[0] == 'S') && (title_id[3] == 'D') && (title_id[4] == '_') & (title_id[8] == '.')) // S**D_###.##;1
					{
						exe_name[3] = 'X'; // patch SYSTEM.CNF

						cellFsChmod(iso_file, MODE);
						write_file(iso_file, CELL_FS_O_WRONLY, data, offset2, 0x40, false);

						offset = root; // re-scan root directory to patch exec name
						entry = 0;
						continue;
					}
					break;
				}
			}
		}
		offset += *data;
	}
}
#endif