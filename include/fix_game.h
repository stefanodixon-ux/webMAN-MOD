#define CFW_420		0xA410ULL /* 42000 */
#define MIN_CFW		0x2710ULL /* 10000 */
#define MAX_CFW		0xD6D8ULL /* 55000 */

static bool fix_in_progress = false;
static bool fix_aborted = false;

#if defined(COPY_PS3) || defined(USE_NTFS) || defined(FIX_GAME)
static u32 fixed_count = 0;
#endif

enum FIX_GAME_MODES
{
	FIX_GAME_AUTO,
	FIX_GAME_QUICK,
	FIX_GAME_FORCED,
	FIX_GAME_DISABLED
};

#include "param_sfo.h"

#ifdef COBRA_ONLY
 #if defined(USE_NTFS) || defined(FIX_GAME)
static u64 getlba(const char *s1, u16 n1, const char *s2, u16 n2, u16 *start)
{
	for(u16 n = *start + 0x1F; n < (n1 - n2); n++)
	{
		if(memcmp(&s1[n], s2, n2) == 0)
		{
			while(n > 0x1D && s1[n--] != 0x01); n-=0x1C, fixed_count++;
			u32 lba = (s1[n] & 0xFF) + ((s1[n+1] & 0xFF) << 8) + ((s1[n+2] & 0xFF) << 16) + ((s1[n+3] & 0xFF) << 24);
			*start = n + 0x1C + n2;
			return lba;
		}
	}
	return 0;
}
 #endif
#endif

#ifdef FIX_GAME
static bool fix_sfo_attribute(unsigned char *mem, u16 sfo_size)
{
	READ_SFO_HEADER(false)

	FOR_EACH_SFO_FIELD()
	{
		if(!memcmp((char *) &mem[str], "ATTRIBUTE", 9))
		{
			if(webman_config->fixgame == FIX_GAME_FORCED) {mem[pos+2]|=0x2; mem[pos+4]|=0xA5; return true;} // PS3_EXTRA + BGM + RemotePlay
			if(!(mem[pos+2] & 2)) {mem[pos+2]|=0x2; return true;}                                           // PS3_EXTRA
			break;
		}

		READ_NEXT_SFO_FIELD()
	}

	return false;
}

static u8 plevel = 0;

static void fix_game_folder(char *path)
{
	if(plevel >= 6) return; // limit recursion up to 6 levels

	int fd;

	show_progress(path, OV_FIX);

	if(working && cellFsOpendir(path, &fd) == CELL_FS_SUCCEEDED)
	{
#ifdef COPY_PS3
		strcpy(current_file, path);
#endif
		struct CellFsStat s;

		CellFsDirectoryEntry dir; u32 read_e;
		char *entry_name = dir.entry_name.d_name;

		u16 plen = sprintf(path, "%s/", path);
		char *filename  = path + plen;

		while(working && (!cellFsGetDirectoryEntries(fd, &dir, sizeof(dir), &read_e) && read_e))
		{
			if(fix_aborted) break;
			if(entry_name[0] == '.') continue;

			strcpy(filename, entry_name);

			if(!extcasecmp(entry_name, ".sprx", 5) || !extcasecmp(entry_name, ".self", 5) || IS(entry_name, "EBOOT.BIN"))
			{
				if(cellFsStat(path, &s) != CELL_FS_SUCCEEDED || s.st_size < 0x500) continue;

				int fdw, offset; u64 bytes_read = 0; u64 ps3_sys_version = 0;

				cellFsChmod(path, MODE); fixed_count++; //fix file read-write permission

				if(cellFsOpen(path, CELL_FS_O_RDWR, &fdw, NULL, 0) == CELL_FS_SUCCEEDED)
				{
					cellFsReadWithOffset(fdw, 0xC, (void *)&offset, 4, &bytes_read); offset-=0x78;

				retry_offset:
					if(offset < 0x90 || offset > 0x800) offset = !extcasecmp(entry_name, ".sprx", 5) ? 0x258 : 0x428;
					cellFsReadWithOffset(fdw, offset, (void *)&ps3_sys_version, 8, NULL);

					if(ps3_sys_version > CFW_420 && ps3_sys_version < MAX_CFW)
					{
						if(offset != 0x278 && offset != 0x428)
						{
							offset = (offset == 0x258) ? 0x278 : 0; goto retry_offset;
						}

						ps3_sys_version = CFW_420;

						cellFsWriteWithOffset(fdw, offset, (void *)(&ps3_sys_version), 8, NULL);
					}
					cellFsClose(fdw);
				}
				cellFsClose(fdw);
			}
			else if(isDir(path) && (webman_config->fixgame != FIX_GAME_QUICK))
			{
				fix_game_folder(path);
			}
		}

		path[plen] = '\0';
		cellFsClosedir(fd);
		if(plevel) plevel--;
	}
}

#ifdef COBRA_ONLY
static void fix_iso(char *iso_file, u64 maxbytes, bool patch_update)
{
	struct CellFsStat buf;

	if(islike(iso_file, "/net") || strstr(iso_file, ".ntfs[")) ; else
	if(fix_aborted || (cellFsStat(iso_file, &buf) != CELL_FS_SUCCEEDED) || (c_firmware >= LATEST_CFW)) return;

	int fd; char title_id[10], update_path[STD_PATH_LEN];

#ifdef COPY_PS3
	sprintf(current_file, "%s", iso_file);
#endif

	show_progress(iso_file, OV_FIX);

	cellFsChmod(iso_file, MODE); //fix file read-write permission

	if(cellFsOpen(iso_file, CELL_FS_O_RDWR, &fd, NULL, 0) == CELL_FS_SUCCEEDED)
	{
		u64 chunk_size=_4KB_; char chunk[chunk_size]; u64 ps3_sys_version;
		u64 bytes_read = 0, lba = 0, pos = 0xA000ULL;

		bool fix_sfo = true, fix_eboot = true, fix_ver = false;

		u64 size = buf.st_size;
		if(maxbytes > 0 && size > maxbytes) size = maxbytes;
		if(size > pos) size -= pos; else size = 0;

		u16 start, offset;

		while(size)
		{
			if(fix_aborted) break;

			if(fix_sfo)
			{
				cellFsReadWithOffset(fd, pos, chunk, chunk_size, &bytes_read); if(!bytes_read) break;

				start = 0, lba = getlba(chunk, chunk_size, "PARAM.SFO;1", 11, &start);

				if(lba)
				{
					lba *= 0x800ULL, fix_sfo = false;

					cellFsReadWithOffset(fd, lba, (void *)&chunk, chunk_size, &bytes_read); if(!bytes_read) break;

					fix_ver = fix_param_sfo((unsigned char *)chunk, title_id, FIX_SFO, (u16)bytes_read);

					if(patch_update)
					{
						if(is_app_dir(HDD0_GAME_DIR, title_id)) fix_ver = false;
					}

					if(fix_ver)
					{
						cellFsWriteWithOffset(fd, lba, chunk, chunk_size, &bytes_read);
					}
					else goto exit_fix; //do not fix if sfo version is ok

					if(size > lba) size = lba;

					sprintf(chunk, "%s %s", STR_FIXING, iso_file);
					show_msg(chunk);

					start = 0, lba = getlba(chunk, chunk_size, "PS3_DISC.SFB;1", 14, &start), lba *= 0x800ULL, chunk_size = 0x800; //1 sector
					if(lba > 0 && size > lba) size = lba;
				}
			}

			for(u8 t = (fix_eboot ? 0 : 1); t < 5; t++)
			{
				cellFsReadWithOffset(fd, pos, chunk, chunk_size, &bytes_read); if(!bytes_read) break;

				lba = start = 0;

				while(true)
				{
					sys_ppu_thread_usleep(1000);
					if(fix_aborted) goto exit_fix;

					if(t == 0) lba = getlba(chunk, chunk_size, "EBOOT.BIN;1", 11, &start); else
					if(t == 1) lba = getlba(chunk, chunk_size, ".SELF;1", 7, &start);      else
					if(t == 2) lba = getlba(chunk, chunk_size, ".self;1", 7, &start);      else
					if(t == 3) lba = getlba(chunk, chunk_size, ".SPRX;1", 7, &start);      else
					if(t == 4) lba = getlba(chunk, chunk_size, ".sprx;1", 7, &start);

					if(lba)
					{
						#define IS_EBOOT	(t == 0)
						#define IS_SPRX		(t >= 3)

						if(IS_EBOOT) fix_eboot = false;

						lba *= 0x800ULL;

						cellFsReadWithOffset(fd, lba, (void *)&chunk, chunk_size, &bytes_read); if(!bytes_read) break;

						memcpy(&offset, chunk + 0xC, 4); offset -= 0x78;

						for(u8 retry = 0; retry < 3; retry++)
						{
							if(retry == 1) {if(IS_SPRX) offset = 0x258; else offset = 0x428;}
							if(retry == 2) {if(IS_SPRX) offset = 0x278; else break;}

							memcpy(&ps3_sys_version, chunk + offset, 8); if(ps3_sys_version >= MIN_CFW && ps3_sys_version < MAX_CFW) break;
						}

						if(ps3_sys_version > CFW_420 && ps3_sys_version < MAX_CFW)
						{
							ps3_sys_version = CFW_420;
							cellFsWriteWithOffset(fd, lba + offset, (void *)(&ps3_sys_version), 8, NULL);
						}
						else goto exit_fix;

						if(IS_EBOOT) break;

						#undef IS_EBOOT
						#undef IS_SPRX

					} else break;
				}
			}

			if(!bytes_read) break;

			pos  += chunk_size;
			size -= chunk_size;
			if(chunk_size > size) chunk_size = (int) size;

			sys_ppu_thread_usleep(1000);
		}
exit_fix:
		cellFsClose(fd);
	}
	else
	{
		get_name(update_path, get_filename(iso_file) + 1, GET_WMTMP); strcat(update_path, ".SFO");
		getTitleID(update_path, title_id, GET_TITLE_ID_ONLY);
	}

	// fix update folder
	sprintf(update_path, "%s%s/PARAM.SFO", HDD0_GAME_DIR, title_id);
	if(getTitleID(update_path, title_id, FIX_SFO) || webman_config->fixgame==FIX_GAME_FORCED)
	{
		sprintf(update_path, "%s%s/USRDIR", HDD0_GAME_DIR, title_id);
		fix_game_folder(update_path);
	}
}
#endif //#ifdef COBRA_ONLY

static void fix_game(char *game_path, char *title_id, u8 fix_type)
{
	memset(title_id, 0, 10);

	if(c_firmware >= LATEST_CFW) return;

	if(file_exists(game_path) || islike(game_path, "/net") || strstr(game_path, ".ntfs["))
	{
		fix_in_progress = true, fix_aborted = false, fixed_count = 0;

#ifdef COBRA_ONLY
		if(is_ext(game_path, ".iso") || is_iso_0(game_path))
			fix_iso(game_path, 0x100000UL, false);
		else
#endif //#ifdef COBRA_ONLY

		{
			// -- get TitleID from PARAM.SFO
			char filename[STD_PATH_LEN];

			if(!get_flag(game_path, "/PS3_GAME")) get_flag(game_path, "/USRDIR");

			if(islike(game_path, "/net") || strstr(game_path, ".ntfs["))
				{get_name(filename, get_filename(game_path) + 1, GET_WMTMP); strcat(filename, ".SFO");}
			else
				sprintf(filename, "%s/PARAM.SFO", game_path);

			if(not_exists(filename)) sprintf(filename, "%s/PS3_GAME/PARAM.SFO", game_path);
			if(not_exists(filename)) {wait_for("/dev_bdvd", 10); sprintf(filename, "/dev_bdvd/PS3_GAME/PARAM.SFO");}

			char paramsfo[_4KB_];
			unsigned char *mem = (u8*)paramsfo;

			u64 bytes_read = read_sfo(filename, paramsfo);
			if(is_sfo(mem))
			{
				// fix ps3 extra or bgm + remoteplay + ps3 extra
				char tmp_path[STD_PATH_LEN]; sprintf(tmp_path, "%s/PS3_EXTRA", game_path); bool has_ps3_extra = isDir(tmp_path);
				if((fix_type == FIX_GAME_FORCED || (has_ps3_extra && fix_type != FIX_GAME_DISABLED)) && fix_sfo_attribute(mem, (u16)bytes_read))
				{
					save_file(filename, paramsfo, bytes_read);
				}

				tmp_path[10] = '\0';

				// get titleid & fix game folder if version is higher than cfw
				if((fix_param_sfo(mem, title_id, FIX_SFO, (u16)bytes_read) || fix_type == FIX_GAME_FORCED) && fix_type != FIX_GAME_DISABLED && !islike(tmp_path, "/net") && !islike(tmp_path, "/dev_bdvd") && !strstr(game_path, ".ntfs["))
				{
					save_file(filename, paramsfo, bytes_read);

					sprintf(filename, "%s %s", STR_FIXING, game_path);
					show_msg(filename);

					sprintf(filename, "%s/PS3_GAME/USRDIR", game_path);  // fix bdvd game

					fix_game_folder(filename);
				}

				// fix PARAM.SFO on hdd0
				for(u8 i = 0; i < 2; i++)
				{
					sprintf(filename, "%s%s%s/PARAM.SFO", HDD0_GAME_DIR, title_id, i ? "" : "/C00");

					bytes_read = read_sfo(filename, paramsfo);
					if(is_sfo(mem))
					{
						// fix ps3 extra or bgm + remoteplay + ps3 extra
						if((fix_type == FIX_GAME_FORCED || (has_ps3_extra && fix_type != FIX_GAME_DISABLED)) && fix_sfo_attribute(mem, (u16)bytes_read))
						{
							save_file(filename, paramsfo, bytes_read);
						}

						if((fix_param_sfo(mem, title_id, FIX_SFO, (u16)bytes_read) || fix_type == FIX_GAME_FORCED) && fix_type!=FIX_GAME_DISABLED)
						{
							save_file(filename, paramsfo, bytes_read);

							if(i == 0) continue;

							sprintf(filename, "%s %s%s", STR_FIXING, HDD0_GAME_DIR, title_id);
							show_msg(filename);

							sprintf(filename, "%s%s/USRDIR", HDD0_GAME_DIR, title_id);  // fix update folder in /dev_hdd0/game

							fix_game_folder(filename);
						}
					}
				}
			}
			// ----

		}

		fix_in_progress = false, fixed_count = 0;

		if(webman_config->fixgame == FIX_GAME_FORCED) {webman_config->fixgame=FIX_GAME_QUICK; save_settings();}
	}
	disable_progress();
}

#endif //#ifdef FIX_GAME
