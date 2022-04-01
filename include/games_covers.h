#define TITLEID_LEN  10

static char wm_icons[14][60] = {WM_ICONS_PATH "/icon_wm_album_ps3.png", //024.png  [0]
								WM_ICONS_PATH "/icon_wm_album_psx.png", //026.png  [1]
								WM_ICONS_PATH "/icon_wm_album_ps2.png", //025.png  [2]
								WM_ICONS_PATH "/icon_wm_album_psp.png", //022.png  [3]
								WM_ICONS_PATH "/icon_wm_album_dvd.png", //023.png  [4]

								WM_ICONS_PATH "/icon_wm_ps3.png",       //024.png  [5]
								WM_ICONS_PATH "/icon_wm_psx.png",       //026.png  [6]
								WM_ICONS_PATH "/icon_wm_ps2.png",       //025.png  [7]
								WM_ICONS_PATH "/icon_wm_psp.png",       //022.png  [8]
								WM_ICONS_PATH "/icon_wm_dvd.png",       //023.png  [9]

								WM_ICONS_PATH "/icon_wm_settings.png",  //icon/icon_home.png  [10]
								WM_ICONS_PATH "/icon_wm_eject.png",     //icon/icon_home.png  [11]

								WM_ICONS_PATH "/icon_wm_bdv.png",       //024.png  [12]
								WM_ICONS_PATH "/icon_wm_retro.png",     //023.png  [13]
								};

enum nocov_options
{
	SHOW_MMCOVERS = 0,
	SHOW_ICON0    = 1,
	SHOW_DISC     = 2,
	ONLINE_COVERS = 3,
};

enum icon_type
{
	iPS3  = 5,
	iPSX  = 6,
	iPS2  = 7,
	iPSP  = 8,
	iDVD  = 9,
	iBDVD = 12,
	iROM  = 13,
};

#define HAS_TITLE_ID  ((*title_id >= 'A') && (*title_id <= 'Z'))

#define NO_ICON       (!*icon)

#define FROM_MOUNT  -99

#define SHOW_COVERS_OR_ICON0  (webman_config->nocov != SHOW_DISC)
#define SHOW_COVERS          ((webman_config->nocov == SHOW_MMCOVERS) || (webman_config->nocov == ONLINE_COVERS))

static const char *ext[4] = {".jpg", ".png", ".PNG", ".JPG"};

static const char *cpath[6] = {MM_ROOT_STD, MM_ROOT_STL, MM_ROOT_SSTL, MANAGUNZ, "/dev_hdd0/GAMES", "/dev_hdd0/GAMEZ"};

#ifndef ENGLISH_ONLY
static bool use_custom_icon_path = false, use_icon_region = false;
static bool is_devil303_server = false;
#endif

static bool covers_exist[9];
static bool covers_retro_exist[3];
static bool wm_icons_exists = false;

static bool HAS(char *icon)
{
	return ((*icon == 'h') || ((*icon == '/') && file_exists(icon) && (icon[strlen(icon) - 1] | 0x20) == 'g' ));
}

static void check_cover_folders(char *buffer)
{
#ifndef ENGLISH_ONLY
													covers_exist[0] = isDir(COVERS_PATH); // online url or custom path
#endif
		u8 p;
		for(p = 0; p < 3; p++)
		{
			sprintf(buffer, "%s/covers_retro/psx", cpath[p]); covers_retro_exist[p] = isDir(buffer);  // MM_ROOT_STD, MM_ROOT_STL, MM_ROOT_SSTL
		}
		for(p = 0; p < 6; p++)
		{
			sprintf(buffer, "%s/covers", cpath[p]); covers_exist[p + 1] = isDir(buffer);  // MM_ROOT_STD, MM_ROOT_STL, MM_ROOT_SSTL, "/dev_hdd0/GAMES", "/dev_hdd0/GAMEZ"
		}
													covers_exist[6] = isDir(WMTMP_COVERS);
													covers_exist[8] = isDir(WMTMP) && SHOW_COVERS_OR_ICON0; // WMTMP

#ifndef ENGLISH_ONLY
	if(!covers_exist[0]) {use_custom_icon_path = strstr(COVERS_PATH, "%s"); use_icon_region = strstr(COVERS_PATH, "%s/%s");} else {use_icon_region = use_custom_icon_path = false;}

	// disable custom icon from web repository if network is disabled //
	if(use_custom_icon_path && islike(COVERS_PATH, "http"))
	{
		char ip[ip_size] = "";
		netctl_main_9A528B81(ip_size, ip);
		if(*ip == NULL) use_custom_icon_path = false;

		is_devil303_server = islike(COVERS_PATH, LAUNCHPAD_COVER_SVR);
	}
#endif

	wm_icons_exists = file_exists(WM_ICONS_PATH "/icon_wm_ps3.png");

#ifdef MOUNT_ROMS
	covers_exist[7] = file_exists(WM_ICONS_PATH "/icon_wm_album_emu.png");
#endif

#ifdef LAUNCHPAD
	nocover_exists = file_exists(WM_ICONS_PATH "/icon_lp_nocover.png");
#endif
}

static u8 ex[4] = {0, 1, 2, 3};

static void swap_ex(u8 e)
{
	u8 s  = ex[e];
	ex[e] = ex[0];
	ex[0] = s;
}

static bool get_image_file(char *icon, int flen)
{
	for(u8 e = 0; e < 4; e++)
	{
		strcpy(icon + flen, ext[ex[e]]);

		if(file_exists(icon)) {swap_ex(e); return true;}
	}
	return false;
}

static size_t get_name(char *name, const char *filename, u8 cache)
{
	// name:
	//   returns file name without extension & without title id (cache == 0 -> file name keeps path, cache == 2 -> remove path first)
	//   returns file name with WMTMP path                      (cache == 1 -> remove path first)

	int flen, pos = 0;
	if(cache) {pos = strlen(filename); while((pos > 0) && filename[pos - 1] != '/') pos--;}
	if(cache == NO_PATH) cache = 0;

	if(cache == WM_COVERS)
		flen = sprintf(name, "%s/%s", WMTMP_COVERS, filename + pos);
	else if(cache)
	{
		#ifdef USE_NTFS
		if(is_ntfs_path(filename) && (pos > 21))
		{
			// convert /dev_ntfs0:/PS3ISO/folder/filename.iso
			//      to /dev_hdd0/tmp/wmtmp/[folder] filename.iso
			int p1 = strchr(filename + 12, '/') - filename + 1;
			int p2 = strchr(filename + p1, '/') - filename;
			sprintf(name, "%s/[%s", WMTMP, filename + p1);
			flen = (p2 - p1) + 21;
			flen += sprintf(name + flen, "] %s", filename + pos);
		}
		else
		#endif
			flen = sprintf(name, "%s/%s", WMTMP, filename + pos);
	}
	else
	{
		// remove prepend [PSPISO] or [PS2ISO]
		if((filename[pos] == '[') && (filename[pos + 7] == ']'))
		{
			char *prefix = (char*)filename + pos + 1;
			if(islike(prefix, paths[id_PSPISO])) pos += 9;
			if(islike(prefix, paths[id_PS2ISO])) pos += 9;
		}

		flen = sprintf(name, "%s", filename + pos);
	}

	if(is_BIN_ENC(name)) {flen -= 8; name[flen] = '\0';}

	if((flen > 2) && name[flen - 2] == '.' ) {flen -= 2; name[flen] = '\0';} // remove file extension (split iso)
	if((flen > 3) && name[flen - 3] == '.' ) {flen -= 3; name[flen] = '\0';} // remove file extension for roms (.gb .gg .vb)
	if((flen > 4) && name[flen - 4] == '.' ) {flen -= 4; name[flen] = '\0';} // remove file extension
	else if(strstr(filename + pos, ".ntfs["))
	{
		while(name[flen] != '.') flen--; name[flen] = '\0';
		if(cache != GET_WMTMP)
		{
			pos = flen - 4;
			if((pos > 0) && name[pos] == '.' && (strcasestr(ISO_EXTENSIONS, &name[pos])))
				{flen = pos; name[flen] = '\0';}
			else
				if(is_BIN_ENC(name)) {flen -= 8; name[flen] = '\0';}
		}
	}

	if(cache) return (size_t) flen;

	// remove title id from file name
	if(name[4] == '_' && name[8] == '.' && (*name == 'B' || *name == 'N' || *name == 'S' || *name == 'U') && ISDIGIT(name[9]) && ISDIGIT(name[10])) {flen = sprintf(name, "%s", &name[12]);}// SLES_000.00-Name
	if(name[9] == '-' && name[10]== '[') {flen = sprintf(name, "%s", name + 11) - 1; name[flen] = '\0';} // BLES00000-[Name]
	if(name[10]== '-' && name[11]== '[') {flen = sprintf(name, "%s", name + 12) - 1; name[flen] = '\0';} // BLES-00000-[Name]
	if(!webman_config->tid) // Name [BLES-00000]
	{
		char *p = strstr(name, " [");
		if(p && (p[2] == 'B' || p[2] == 'N' || p[2] == 'S' || p[2] == 'U') && ISDIGIT(p[7])) *p = NULL;
		flen = strlen(name);
	}

	return (size_t) flen;
}

static bool get_cover_by_titleid(char *icon, char *title_id)
{
	if(!HAS_TITLE_ID) return false;

	int flen;

	if(SHOW_COVERS)
	{
#ifndef ENGLISH_ONLY
		// Search covers in custom path
		if(covers_exist[0] && ((webman_config->nocov == SHOW_MMCOVERS) && (*COVERS_PATH == '/')))
		{
			flen = sprintf(icon, "%s/%s", COVERS_PATH, title_id);
			if(get_image_file(icon, flen)) return true;
		}
#endif

		// Search retro covers in MM_ROOT_STD, MM_ROOT_STL, MM_ROOT_SSTL
		if(*title_id == 'S')
		{
			for(u8 p = 0; p < 3; p++)
			{
				if(covers_retro_exist[p])
				{
					flen = sprintf(icon, "%s/covers_retro/psx/%.4s_%.3s.%.2s_COV", cpath[p],
									title_id,
									title_id + 4, title_id + 7);

					if(get_image_file(icon, flen)) return true;
				}
			}
		}

		// Search covers in MM_ROOT_STD, MM_ROOT_STL, MM_ROOT_SSTL, "/dev_hdd0/GAMES", "/dev_hdd0/GAMEZ"
		for(u8 p = 0; p < 6; p++)
			if(covers_exist[p + 1])
			{
				flen = sprintf(icon, "%s/covers/%s", cpath[p], title_id);
				if(get_image_file(icon, flen)) return true;
			}

		// Search covers in WMTMP_COVERS
		if(covers_exist[6])
		{
			flen = sprintf(icon, "%s/%s", WMTMP_COVERS, title_id);
			if(get_image_file(icon, flen)) return true;
		}

		// Search covers in WMTMP
		if(covers_exist[8])
		{
			flen = sprintf(icon, "%s/%s", WMTMP, title_id);
			if(get_image_file(icon, flen)) return true;
		}

		// Search online covers
#ifdef ENGLISH_ONLY
		if(webman_config->nocov == ONLINE_COVERS)
		{
			sprintf(icon, COVERS_PATH, title_id);
			return true;
		}
#else
		if(use_custom_icon_path && (webman_config->nocov == ONLINE_COVERS) && (COVERS_PATH[0] == 'h'))
		{
			if(is_devil303_server && (*title_id != 'B' && *title_id != 'N')) {*icon = NULL; return false;}

			if(use_icon_region) sprintf(icon, COVERS_PATH,  (title_id[2] == 'U') ? "US" :
															(title_id[2] == 'J') ? "JA" : "EN", title_id);
			else
								sprintf(icon, COVERS_PATH, title_id);
			return true;
		}
#endif
	}

	*icon = NULL;
	return false;
}

static bool get_cover_from_name(char *icon, const char *name, char *title_id) // get icon & title_id from name
{
	if(HAS(icon)) return true;

	// get cover from title_id in PARAM.SFO
	if(get_cover_by_titleid(icon, title_id)) return true;

	// get title_id from file name
	if(HAS_TITLE_ID) ;

	else if((*name == 'B' || *name == 'N' || *name == 'S' || *name == 'U') && ISDIGIT(name[6]) && ISDIGIT(name[7]))
	{
		if(name[4] == '_' && name[8] == '.')
			sprintf(title_id, "%.4s%.3s%.2s", name, name + 5, name + 9); //SCUS_999.99.filename.iso
		else if(ISDIGIT(name[8]))
			strncpy(title_id, name, TITLEID_LEN);
	}

	if(HAS_TITLE_ID) ;

	else if(islike(name + TITLEID_LEN, "-["))
		strncpy(title_id, name, TITLEID_LEN); // TITLEID-[NAME]
	else
	{
		char *pos;
		// search for BLES/UCES/SLES/BCUS/UCUS/SLUS/etc.
		for(pos = (char*)name; *pos; pos++)
			if((*pos == '[' || *pos == '(') && (pos[2] == 'L' || pos[2] == 'C') && ISDIGIT(pos[6]) && ISDIGIT(pos[7]))
			{
				if(pos[1] == 'B' || pos[1] == 'U') // B = PS3, U = PSP
				{
					strncpy(title_id, pos + 1, TITLEID_LEN); //BCES/BLES/BCUS/BLUS/BLJM/etc. || UCES/UCUS/ULES/ULUS/UCAS/UCKS/UCUS/ULJM/ULJS/etc.
					break;
				}
				else
				if(pos[1] == 'S') // S = PS1/PS2
				{
					get_ps_titleid_from_path(title_id, pos + 1); //SLES/SLUS/SLPM/SLPS/SLAJ/SLKA/SCES/SCUS/SCPS/SCAJ/SCKA
					break;
				}
			}

		if(*title_id == 0)
		{
			if((pos = strstr(name, "[NP")))
				strncpy(title_id, pos + 1, TITLEID_LEN); //NP*
			else if((pos = strstr(name, "[KTGS")))
				strncpy(title_id, pos + 1, TITLEID_LEN); //KTGS*
			else if((pos = strstr(name, "[KOEI")))
				strncpy(title_id, pos + 1, TITLEID_LEN); //KOEI*
			else if((pos = strstr(name, "[MRTC")))
				strncpy(title_id, pos + 1, TITLEID_LEN); //MRTC*
		}
	}

	if(title_id[4] == '-') strncpy(&title_id[4], &title_id[5], 5); title_id[TITLE_ID_LEN] = '\0';

	// get cover using titleID obtained from file name
	if(get_cover_by_titleid(icon, title_id)) return true;

	return false;
}

static void get_default_icon_from_folder(char *icon, u8 is_dir, const char *param, const char *entry_name, char *title_id, u8 f0)
{
	//this function is called only from get_default_icon

	if(SHOW_COVERS_OR_ICON0)
	{
			if(is_dir && (webman_config->nocov == SHOW_ICON0))
			{
				sprintf(icon, "%s/%s/PS3_GAME/ICON0.PNG", param, entry_name); check_ps3_game(icon);
				if(!HAS(icon))
					sprintf(icon, "%s/%s/ICON0.PNG", param, entry_name);
				return;
			}

			// get path/name and remove file extension
			int flen = sprintf(icon, "%s/%s", param, entry_name);

#ifdef COBRA_ONLY
			if(f0 == NTFS)
			{
				if(flen > 13 && icon[flen-13] == '.' && (!extcmp(icon, ".ntfs[PS3ISO]", 13) || !extcmp(icon, ".ntfs[PS2ISO]", 13)  || !extcmp(icon, ".ntfs[PSPISO]", 13) || !extcmp(icon, ".ntfs[DVDISO]", 13) || !extcmp(icon, ".ntfs[PSXISO]", 13) || !extcmp(icon, ".ntfs[BDFILE]", 13))) flen -= 13; else
				if(flen > 12 && icon[flen-12] == '.' &&  !extcmp(icon, ".ntfs[BDISO]" , 12)) flen -= 12;
				if(get_image_file(icon, flen)) return;
			}
#endif
			if(flen > 2 && icon[flen - 2] == '.') flen -= 2; // remove file extension (split iso)
			if(flen > 4 && icon[flen - 4] == '.') flen -= 4; // remove file extension
			else
			if(flen > 3 && icon[flen - 3] == '.') flen -= 3; // remove file extension for roms (.gb .gg .vb)

			// get covers from iso folder
			if((f0 < 7 || f0 > NTFS) || (f0 == NTFS && (webman_config->nocov == SHOW_ICON0)))
			{
				if(get_image_file(icon, flen)) return;
			}

			if(HAS_TITLE_ID && SHOW_COVERS) {get_cover_by_titleid(icon, title_id); if(HAS(icon)) return;}

			// get mm covers & titleID
			get_cover_from_name(icon, entry_name, title_id);

			// get covers named as titleID from iso folder e.g. /PS3ISO/BLES12345.JPG
			if(!is_dir && HAS_TITLE_ID && (f0 < 7 || f0 > NTFS))
			{
				if(HAS(icon)) return;

				char titleid[STD_PATH_LEN];
				char *pos = strchr(entry_name, '/');
				if(pos)
					{*pos = NULL; sprintf(titleid, "%s/%s", entry_name, title_id); *pos = '/';}
				else
					strcpy(titleid, title_id);

				flen = sprintf(icon, "%s/%s", param, titleid);
				if(get_image_file(icon, flen)) return;

				*icon = NULL;
			}

			// return ICON0
			if(is_dir)
			{
				sprintf(icon, "%s/%s/PS3_GAME/ICON0.PNG", param, entry_name); check_ps3_game(icon);
				if(!HAS(icon))
					sprintf(icon, "%s/%s/ICON0.PNG", param, entry_name);
				return;
			}

			// continue searching for covers
			if(SHOW_COVERS) return;

			// Search covers in WMTMP_COVERS
			if(covers_exist[6])
			{
				flen = get_name(icon, entry_name, WM_COVERS);
				if(get_image_file(icon, flen)) return;
			}

			// Search covers in WMTMP
			flen = get_name(icon, entry_name, GET_WMTMP);
			if(get_image_file(icon, flen)) return;

			*icon = NULL;
	}
}

static void get_default_icon_for_iso(char *icon, const char *param, char *file, int isdir, int ns)
{
	//this function is called only from get_default_icon

	int flen;

	if(is_BIN_ENC(file))
	{
		flen = sprintf(icon, "%s/%s", param, file);

		if(get_image_file(icon, flen)) return;

		flen -= 8; // remove .BIN.ENC
		if(flen > 0 && icon[flen] == '.')
		{
			if(get_image_file(icon, flen)) return;
		}
	}

	flen = sprintf(icon, "%s/%s", param, file);

	if(not_exists(icon))
	{
		flen = get_name(icon, file, GET_WMTMP); //wmtmp
	}

	if(!isdir && (flen > 13))
	{
		char *p = strstr(icon + flen - 13, ".ntfs[");
		if(p) {flen -= strlen(p), *p = NULL;}
	}

	if((flen > 2) && icon[flen - 2] == '.' ) {flen -= 2, icon[flen] = '\0';} // remove file extension (split iso)
	if((flen > 4) && icon[flen - 4] == '.' ) {flen -= 4, icon[flen] = '\0';} // remove file extension
	else
	if((flen > 3) && icon[flen - 3] == '.' ) {flen -= 3, icon[flen] = '\0';} // remove file extension

	//file name + ext
	if(get_image_file(icon, flen)) return;

	//copy remote file
	if(not_exists(icon))
	{
#ifdef NET_SUPPORT
		if(ns < 0) {*icon = NULL; return;}

		char remote_file[MAX_PATH_LEN];

		if(isdir)
		{
			if(webman_config->nocov == SHOW_DISC) return; // no icon0
			sprintf(remote_file, "%s/%s/PS3_GAME/ICON0.PNG", param, file);
			flen = get_name(icon, file, GET_WMTMP); sprintf(icon + flen, ".png");

			copy_net_file(icon, remote_file, ns);
			if(file_exists(icon)) return;
		}
		else
		{
			get_name(icon, file, NO_EXT);
			int tlen = sprintf(remote_file, "%s/%s", param, icon);

			int icon_len = get_name(icon, file, GET_WMTMP); //wmtmp

			for(u8 e = 0; e < 4; e++)
			{
				strcpy(icon + icon_len, ext[e]);
				if(file_exists(icon)) return;

				strcpy(remote_file + tlen, ext[e]);

				//Copy remote icon locally
				copy_net_file(icon, remote_file, ns);
				if(file_exists(icon)) return;
			}
		}
#endif //#ifdef NET_SUPPORT

		*icon = NULL;
	}
}

static enum icon_type get_default_icon_by_type(u8 f1)
{
	return  IS_PS3_TYPE    ? iPS3 :
			IS_PSXISO      ? iPSX :
			IS_PS2ISO      ? iPS2 :
			IS_PSPISO      ? iPSP :
#ifdef MOUNT_ROMS
			IS_ROMS_FOLDER ? iROM :
#endif
			IS_DVDISO      ? iDVD : iBDVD;
}

static enum icon_type get_default_icon(char *icon, const char *param, char *file, int is_dir, char *title_id, int ns, u8 f0, u8 f1)
{
	char filename[STD_PATH_LEN];

	if(ns == FROM_MOUNT)
		snprintf(filename, STD_PATH_LEN, "%s", file);
	else
		*filename = NULL;

	enum icon_type default_icon = get_default_icon_by_type(f1);

	if(webman_config->nocov == SHOW_DISC) {if(get_cover_from_name(icon, file, title_id)) return default_icon; goto no_icon0;}

	if(SHOW_COVERS && get_cover_from_name(icon, file, title_id)) return default_icon; // show mm cover

	// get icon from folder && copy remote icon
	get_default_icon_for_iso(icon, param, file, is_dir, ns);

	if(HAS(icon)) return default_icon;

	if(!IS_NET) get_default_icon_from_folder(icon, is_dir, param, file, title_id, f0);

	// continue using cover or default icon0.png
	if(HAS(icon)) return default_icon;

	//use the cached PNG from wmtmp if available
	int flen = get_name(icon, file, GET_WMTMP);

	if(get_image_file(icon, flen)) return default_icon;

no_icon0:
	if(HAS(icon)) return default_icon;

		if((webman_config->nocov == SHOW_ICON0) && get_cover_from_name(icon, file, title_id)) return default_icon; // show mm cover as last option (if it's disabled)

	//show the default icon by type
	{
		sprintf(icon, "%s/%s", param + (IS_NET ? 0 : 6), file);

			 if(strstr(icon, "PSX")) //if(strstr(param, "/PSX") || !extcmp(file, ".ntfs[PSXISO]", 13))
			default_icon = iPSX;
		else if(strstr(icon, "PS2")) //if(strstr(param, "/PS2ISO") || is_BIN_ENC(param) || !extcmp(file, ".ntfs[PS2ISO]", 13))
			default_icon = iPS2;
		else if(strstr(icon, "PSP")) //if(strstr(param, "/PSPISO") || strstr(param, "/ISO/") || !extcmp(file, ".ntfs[PSPISO]", 13))
			default_icon = iPSP;
		else if(strstr(icon, "DVD")) //if(strstr(param, "/DVDISO") || !extcmp(file, ".ntfs[DVDISO]", 13))
			default_icon = iDVD;
		else if(strstr(icon, "BDISO")) //if(strstr(param, "/BDISO") || !extcmp(file, ".ntfs[BDISO]", 13))
			default_icon = iBDVD;
		else if(strstr(icon, "/ROMS"))
			default_icon = iROM;
		else
			default_icon = iPS3;
	}

	if(!HAS(icon))
	{
		flen = sprintf(icon, "/dev_bdvd/%s", file);
		if(file_exists(icon))
		{
			if(get_image_file(icon, flen)) return default_icon;
			flen -= 4; icon[flen] = '\0';
			if(get_image_file(icon, flen)) return default_icon;
			sprintf(icon, "/dev_bdvd/%s.ICON0.PNG", file);
			if(file_exists(icon)) return default_icon;
		}
		strcpy(icon, wm_icons[default_icon]);
	}
	return default_icon;
}

static int get_title_and_id_from_sfo(char *param_sfo, char *title_id, const char *entry_name, char *icon, char *data, u8 f0)
{
	int ret = FAILED;

	bool use_filename = webman_config->use_filename;

	// read param.sfo
	unsigned char *mem = (u8*)data;
	u16 sfo_size = read_sfo(param_sfo, data);
	char *title = param_sfo;

	// get title_id & title from PARAM.SFO
	if(sfo_size && is_sfo(mem))
	{
		if((IS_HDD0 && islike(param_sfo + 9, "/game/")) || islike(param_sfo + 11, "/GAMEI/") || strstr(param_sfo, "_00-")) use_filename = false;

		parse_param_sfo(mem, title_id, title, sfo_size);

		if(SHOW_COVERS) get_cover_by_titleid(icon, title_id);

		ret = CELL_FS_SUCCEEDED;
	}

	if(use_filename)
	{
		if(NO_ICON && !HAS_TITLE_ID) get_cover_from_name(icon, entry_name, title_id); // get title_id from name

		ret = ~CELL_FS_SUCCEEDED;
	}

	if(ret != CELL_FS_SUCCEEDED)
	{
		get_name(title, entry_name, NO_PATH); if(!IS_NTFS) utf8enc(data, title, true); //use file name as title
	}

	return ret;
}

#ifdef COBRA_ONLY
static int get_name_iso_or_sfo(char *param_sfo, char *title_id, char *icon, const char *param, const char *entry_name, u8 f0, u8 f1, u8 uprofile, int flen, char *tempstr)
{
	// check entry path & returns file name without extension or path of sfo (for /PS3ISO) in param_sfo

	if(IS_NTFS)
	{   // ntfs
		char *ntfs_sufix = NULL;

		if(flen > 17) ntfs_sufix = strstr((char*)entry_name + flen - 14, ").ntfs[");

		if(ntfs_sufix)
		{
			ntfs_sufix--; u8 fprofile = (u8)(*ntfs_sufix); ntfs_sufix -= 2;

			if((fprofile >= '1') && (fprofile <= '4') && islike(ntfs_sufix, " ("))
			{
				// skip extended content of ntfs cached in /wmtmp if current user profile is 0
				if(uprofile == 0) return FAILED;

				fprofile -= '0';

				// skip non-matching extended content
				if(uprofile != fprofile) return FAILED;
			}
		}

		flen-=13; if(flen < 0) return FAILED;

		char *ntfs_ext = (char*)entry_name + flen;
		if(IS_PS3ISO && !IS(ntfs_ext, ".ntfs[PS3ISO]")) return FAILED;
		if(IS_PS2ISO && !IS(ntfs_ext, ".ntfs[PS2ISO]")) return FAILED;
		if(IS_PSXISO && !IS(ntfs_ext, ".ntfs[PSXISO]")) return FAILED;
		if(IS_PSPISO && !IS(ntfs_ext, ".ntfs[PSPISO]")) return FAILED;
		if(IS_DVDISO && !IS(ntfs_ext, ".ntfs[DVDISO]")) return FAILED;
		if(IS_BDISO  && !strstr(ntfs_ext, ".ntfs[BD" )) return FAILED;
	}

	char *title = param_sfo;

	if(IS_PS3ISO)
	{
		flen = get_name(param_sfo, entry_name, GET_WMTMP); strcat(param_sfo + flen, ".SFO"); // WMTMP
		if(not_exists(param_sfo))
		{
			get_name(tempstr, entry_name, NO_EXT);
			if(IS_NTFS || IS_NET)
				sprintf(param_sfo, "%s/%s.SFO", WMTMP, tempstr); // /PS3ISO
			else
				sprintf(param_sfo, "%s/%s.SFO", param, tempstr); // /PS3ISO
		}

		if(get_title_and_id_from_sfo(title, title_id, entry_name, icon, tempstr, f0) != CELL_FS_SUCCEEDED || !HAS_TITLE_ID)
		{
			if(!IS_NTFS)
			{
				char *iso_file = param_sfo;
				sprintf(iso_file, "%s/%s", param, entry_name); // get raw title ID from ISO

				if(read_file(iso_file, title_id, 11, 0x810) == 11)
				{
					strncpy(&title_id[4], &title_id[5], 5); title_id[TITLE_ID_LEN] = '\0';
				}
			}

			get_name(title, entry_name, NO_EXT);
		}
	}
	else
	{
		get_name(title, entry_name, NO_EXT);

		if(f1 >= id_BDISO && f1 <= id_ISO)
		{
			// info level: 0=Path, 1=Path + ID, 2=ID, 3=None
			if(webman_config->info == 1 || webman_config->info == 2)
			{
				get_cover_from_name(icon, entry_name, title_id); // get title_id from name
			}
		}
	}

	return CELL_OK;
}
#endif