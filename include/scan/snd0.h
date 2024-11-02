#ifdef MUTE_SND0
static sys_ppu_thread_t t_snd0_thread_id = SYS_PPU_THREAD_NONE;
static u8 prev_nosnd0 = 0;

static void snd0_thread(__attribute__((unused)) u64 arg)
{
	int fd;
	if(cellFsOpendir("/dev_hdd0/game", &fd) == CELL_FS_SUCCEEDED)
	{
		prev_nosnd0 = webman_config->nosnd0;

		CellFsDirectoryEntry entry; size_t read_e; char path[64];
		int mode = webman_config->nosnd0 ? NOSND : MODE; // toggle file access permissions

		u8 path_len = strcopy(path, HDD0_GAME_DIR);
		char *snd0_file = path + path_len;

		while(working)
		{
			if(cellFsGetDirectoryEntries(fd, &entry, sizeof(entry), &read_e) || !read_e) break;
			if(entry.entry_name.d_namlen != TITLE_ID_LEN) continue;
			concat2(snd0_file, entry.entry_name.d_name, "/SND0.AT3"); cellFsChmod(snd0_file, mode);
			concat2(snd0_file, entry.entry_name.d_name, "/ICON1.PAM"); cellFsChmod(snd0_file, mode);
		}
		cellFsClosedir(fd);
	}

	t_snd0_thread_id = SYS_PPU_THREAD_NONE;
	sys_ppu_thread_exit(0);
}

static void mute_snd0(bool scan_gamedir)
{
	cellFsChmod("/dev_bdvd/PS3_GAME/ICON1.PAM", webman_config->nosnd0 ? NOSND : MODE);

	if(!scan_gamedir) return;

	if((t_snd0_thread_id == SYS_PPU_THREAD_NONE) && !payload_ps3hen)
		sys_ppu_thread_create(&t_snd0_thread_id, snd0_thread, NULL, THREAD_PRIO, THREAD_STACK_SIZE_64KB, SYS_PPU_THREAD_CREATE_NORMAL, THREAD_NAME_SND0);
}
#endif // #ifdef MUTE_SND0

