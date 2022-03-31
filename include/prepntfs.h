#ifdef USE_NTFS
#define MAX_SECTIONS	(int)((_64KB_-sizeof(rawseciso_args))/8)

//static char paths [13][10] = {"GAMES", "GAMEZ", "PS3ISO", "BDISO", "DVDISO", "PS2ISO", "PSXISO", "PSXGAMES", "PSPISO", "ISO", "video", "GAMEI", "ROMS"};

enum ntfs_folders
{
	mPS3 = 2,
	mBLU = 3,
	mDVD = 4,
	mPS2 = 5,
	mPSX = 6,
	mMAX = 7,
};

static u64 device_id;
static u8  ntfs_m;

static char *ntfs_subdir = NULL;

static u16 ntfs_count = 0;
static u8 prepntfs_working = false;

static u8  *plugin_args = NULL;
static u32 *sectionsP = NULL;
static u32 *sections_sizeP = NULL;

static sys_addr_t sysmem_p = NULL;

static void create_ntfs_file(char *path, char *filename, size_t plen)
{
	int parts = ps3ntfs_file_to_sectors(path, sectionsP, sections_sizeP, MAX_SECTIONS, 1);

	if(parts <= 0) return;

	unsigned int num_tracks;
	int emu_mode = EMU_PS3;
	TrackDef tracks[MAX_TRACKS];
	ScsiTrackDescriptor *scsi_tracks;

	size_t extlen = 4;
	char tmp_path[MAX_PATH_LEN];

	rawseciso_args *p_args;

	// get multi-part file sectors
	if(is_iso_0(filename))
	{
		size_t nlen = sprintf(tmp_path, "%s", path);
		extlen = 6, --nlen;

		for(u8 o = 1; o < 64; o++)
		{
			if(parts >= MAX_SECTIONS) break;

			sprintf(tmp_path + nlen, "%i", o);
			if(not_exists(tmp_path)) break;

			parts += ps3ntfs_file_to_sectors(tmp_path, sectionsP + (parts * sizeof(u32)), sections_sizeP + (parts * sizeof(u32)), MAX_SECTIONS - parts, 1);
		}
	}

	if(parts >= MAX_SECTIONS)
	{
		return;
	}
	else if(parts > 0)
	{
		num_tracks = 1;
		struct stat bufn; int cd_sector_size = 0, cd_sector_size_param = 0;

			 if(ntfs_m == mPS3) emu_mode = EMU_PS3;
		else if(ntfs_m == mBLU) emu_mode = EMU_BD;
		else if(ntfs_m == mDVD) emu_mode = EMU_DVD;
		else if(ntfs_m == mPSX)
		{
			emu_mode = EMU_PSX;

			if(ps3ntfs_stat(path, &bufn) < 0) return;
			cd_sector_size = default_cd_sector_size(bufn.st_size);

			// detect CD sector size
			if(sysmem_p || sys_memory_allocate(_64KB_, SYS_MEMORY_PAGE_SIZE_64K, &sysmem_p) == CELL_OK)
			{
				char *buffer = (char*)sysmem_p;
				read_file(path, buffer, _48KB_, 0);
				cd_sector_size = detect_cd_sector_size(buffer);
			}

			if(cd_sector_size & 0xf) cd_sector_size_param = cd_sector_size<<8;
			else if(cd_sector_size != 2352) cd_sector_size_param = cd_sector_size<<4;

			if(change_ext(path, 4, cue_ext))
			{
				if(sysmem_p || sys_memory_allocate(_64KB_, SYS_MEMORY_PAGE_SIZE_64K, &sysmem_p) == CELL_OK)
				{
					char *cue_buf = (char*)sysmem_p;
					int cue_size = read_file(path, cue_buf, _8KB_, 0);

					char *templn = path;
					num_tracks = parse_cue(templn, cue_buf, cue_size, tracks);
				}
			}
		}

		p_args = (rawseciso_args *)plugin_args; memset(p_args, 0, sizeof(rawseciso_args));
		p_args->device = device_id;
		p_args->emu_mode = emu_mode;
		p_args->num_sections = parts;

		uint32_t array_len = parts * sizeof(uint32_t);

		memcpy64(plugin_args + sizeof(rawseciso_args) + array_len, sections_sizeP, array_len);

		int max = MAX_SECTIONS - ((num_tracks * sizeof(ScsiTrackDescriptor)) / 8);

		if(parts >= max)
		{
			return;
		}

		p_args->num_tracks = num_tracks | cd_sector_size_param;

		scsi_tracks = (ScsiTrackDescriptor *)(plugin_args + sizeof(rawseciso_args) + (2 * array_len));

		scsi_tracks[0].adr_control = 0x14;
		scsi_tracks[0].track_number = 1;
		scsi_tracks[0].track_start_addr = 0;

		for(unsigned int t = 1; t < num_tracks; t++)
		{
			scsi_tracks[t].adr_control = 0x10;
			scsi_tracks[t].track_number = t + 1;
			scsi_tracks[t].track_start_addr = tracks[t].lba;
		}

		int slen = strlen(filename) - extlen;
		filename[slen] = '\0';

		snprintf(tmp_path, sizeof(tmp_path), "%s/%s%s.SFO", WMTMP, filename, SUFIX2(profile));
		if(not_exists(tmp_path)) {filename[slen] = '.', slen += extlen, extlen = 0;} // create file with .iso extension

		if(ntfs_subdir && (strncmp(ntfs_subdir, filename, slen) != 0))
		{
			sprintf(tmp_path, "[%s] %s", ntfs_subdir, filename);
			strcpy(filename, tmp_path);
		}

		snprintf(tmp_path, sizeof(tmp_path), "%s/%s%s.ntfs[%s]", WMTMP, filename, SUFIX2(profile), paths[ntfs_m]);

		save_file(tmp_path, (char*)plugin_args, (sizeof(rawseciso_args) + (2 * array_len) + (num_tracks * sizeof(ScsiTrackDescriptor)))); ntfs_count++;

		if(ntfs_m == mPS3)
		{
			snprintf(tmp_path, sizeof(tmp_path), "%s/%s%s.SFO", WMTMP, filename, SUFIX2(profile));
			if(not_exists(tmp_path))
			{
				// extract PARAM.SFO from ISO
				if(sysmem_p || sys_memory_allocate(_64KB_, SYS_MEMORY_PAGE_SIZE_64K, &sysmem_p) == CELL_OK)
				{
					char *buffer = (char*)sysmem_p;
					read_file(path, buffer, _4KB_, 0x10800);
					u16 start = 0x40; u64 lba = getlba(buffer, _4KB_, "PARAM.SFO;1", 11, &start);
					if(lba)
					{
						read_file(path, buffer, _4KB_, lba * 0x800ULL);
						save_file(tmp_path, buffer, _4KB_);
					}
				}
			}
		}

		if(!get_image_file(path, plen - extlen)) return; // not found image in NTFS

		plen = sprintf(tmp_path, "%s/%s", WMTMP, filename);
		if( get_image_file(tmp_path, plen - extlen) ) return; // found image in WMTMP

		// copy external image to WMTMP
		force_copy(path, tmp_path);

/*
		for_sfo:
		if(ntfs_m == mPS3) // mount PS3ISO
		{
			strcpy(path + plen - 3, "SFO");
			if(not_exists(path))
			{
				if(isDir("/dev_bdvd")) do_umount(false);

				sys_ppu_thread_create(&thread_id_ntfs, rawseciso_thread, (u64)plugin_args, THREAD_PRIO, THREAD_STACK_SIZE_NTFS_ISO, SYS_PPU_THREAD_CREATE_JOINABLE, THREAD_NAME_NTFS);

				wait_for("/dev_bdvd/PS3_GAME/PARAM.SFO", 2);

				if(file_exists("/dev_bdvd/PS3_GAME/PARAM.SFO"))
				{
					file_copy("/dev_bdvd/PS3_GAME/PARAM.SFO", path);

					strcpy(path + plen - 3, "PNG");
					if(not_exists(path))
						file_copy("/dev_bdvd/PS3_GAME/ICON0.PNG", path);
				}

				sys_ppu_thread_t t;
				sys_ppu_thread_create(&t, rawseciso_stop_thread, 0, 0, THREAD_STACK_SIZE_STOP_THREAD, SYS_PPU_THREAD_CREATE_JOINABLE, STOP_THREAD_NAME);
				while(rawseciso_loaded) {sys_ppu_thread_usleep(50000);}
			}
		}
*/
	}
}

static void scan_path_ntfs(const char *path, bool chk_dirs)
{
	DIR_ITER *pdir = NULL;

	typedef struct
	{
		char path[STD_PATH_LEN];
	} t_line_entries;

	pdir = ps3ntfs_opendir(path);
	if(pdir)
	{
		sys_addr_t sysmem = NULL;
		if(chk_dirs) sys_memory_allocate(_64KB_, SYS_MEMORY_PAGE_SIZE_64K, &sysmem);

		t_line_entries *dir_entries = (t_line_entries *)sysmem;
		u16 max_entries = (_64KB_ / sizeof(t_line_entries));

		int idx = 0;

		struct stat st;
		CellFsDirent dir;
		char dir_entry[STD_PATH_LEN];
		size_t plen = snprintf(dir_entry, STD_PATH_LEN, "%s/", path);

		while(ps3ntfs_dirnext(pdir, dir.d_name, &st) == 0)
		{
			if(dir.d_name[0] == '.') continue;
			size_t flen = snprintf(dir_entry + plen, STD_PATH_LEN - plen, "%s", dir.d_name);

			if(st.st_mode & S_IFDIR)
			{
				if(sysmem && (idx < max_entries)) {sprintf(dir_entries[idx++].path, "%s", dir.d_name);}
			}
			else if(is_iso_file(dir.d_name, flen, ntfs_m, 0))
			{
				create_ntfs_file(dir_entry, dir.d_name, plen + flen);
			}
		}
		ps3ntfs_dirclose(pdir);

		for(int i = 0; i < idx; i++)
		{
			snprintf(dir_entry + plen, STD_PATH_LEN - plen, "%s", dir_entries[i].path);

			ntfs_subdir = dir_entries[i].path;
			for(int c = 0; ntfs_subdir[c]; c++)
				if(ntfs_subdir[c] == '[') ntfs_subdir[c] = '('; else
				if(ntfs_subdir[c] == ']') ntfs_subdir[c] = ')';

			scan_path_ntfs(dir_entry, false);
		}

		if(sysmem) sys_memory_free(sysmem);
	}
}

static int prepNTFS(u8 clear)
{
	if(prepntfs_working || skip_prepntfs) {skip_prepntfs = false; return 0;}
	prepntfs_working = true;

	const char *prefix[2] = {"/", "/PS3/"};

	cellFsMkdir(TMP_DIR, DMODE);
	cellFsMkdir(WMTMP, DMODE);
	cellFsChmod(WMTMP, DMODE);
	cellFsUnlink(WMTMP "/games.html");
	int fd = NONE;
	char path[STD_PATH_LEN];
	bool retry;

	// remove ntfs files from WMTMP
	do
	{
		retry = false;
		if(cellFsOpendir(WMTMP, &fd) == CELL_FS_SUCCEEDED)
		{
			u16 dlen = sprintf(path, "%s/", WMTMP);
			char *ext, *ntfs_file = path + dlen;

			CellFsDirectoryEntry dir; u32 read_f;
			char *entry_name = dir.entry_name.d_name;

			while(!cellFsGetDirectoryEntries(fd, &dir, sizeof(dir), &read_f) && read_f)
			{
				ext = strstr(entry_name, ".ntfs[");
				if(ext && (clear || (mountCount <= 0) ||
				  (!IS(ext, ".ntfs[BDFILE]") && !IS(ext, ".ntfs[PS2ISO]") && !IS(ext, ".ntfs[PSPISO]")))
				)
				{
					sprintf(ntfs_file, "%s", entry_name);
					cellFsUnlink(path); retry = true;
				}
			}
			cellFsClosedir(fd);
		}
	}
	while (retry);

	// allocate memory
	sys_addr_t addr = NULL;

	if(mountCount <= 0) mount_all_ntfs_volumes(); //check_ntfs_volumes();

	if(mountCount <= 0) {mountCount = NTFS_UNMOUNTED; goto exit_prepntfs;}

	if(sys_memory_allocate(_64KB_, SYS_MEMORY_PAGE_SIZE_64K, &addr) != CELL_OK) goto exit_prepntfs;

	if(!addr) goto exit_prepntfs;

	plugin_args    = (u8 *)(addr);
	sectionsP      = (u32*)(addr + sizeof(rawseciso_args));
	sections_sizeP = (u32*)(addr + sizeof(rawseciso_args) + _32KB_);

	// scan
	ntfs_count = 0;

	for(u8 i = 0; i < mountCount; i++)
	{
		device_id = USB_MASS_STORAGE((mounts[i].interface->ioType & 0x0F));

		for(u8 n = 0; n < 2; n++)
		{
			for(u8 profile = 0; profile < 5; profile++)
			{
				for(u8 m = mPS3; m < mMAX; m++)
				{
					if(m == mPS2) continue;

					ntfs_m = m;
					ntfs_subdir = NULL;

					snprintf(path, sizeof(path), "%s:%s%s%s", mounts[i].name, prefix[n], paths[m], SUFIX(profile));
					scan_path_ntfs(path, true);
				}
			}
		}
	}

exit_prepntfs:
	if(sysmem_p) sys_memory_free(sysmem_p); sysmem_p = NULL;
	if(addr) sys_memory_free(addr); addr = NULL;

	plugin_args = NULL;
	sectionsP = NULL;
	sections_sizeP = NULL;

	prepntfs_working = false;
	return ntfs_count;
}
#endif
