#define MAX_ISOS 2000

static size_t nt[MAX_ISOS];
static char   ff[MAX_ISOS][256];
static int nn = 0;

/*
void addlog(char *msg1, char *msg2)
{
	FILE* f;
	f = fopen("/dev_hdd0/prepiso2.log", "a+b");
	if(f)
	{
		char msg[300];
		sprintf(msg, "%s %s\r\n", msg1, msg2);
		fwrite(msg, 1, strlen(msg), f);
		fclose (f);
	}
}
*/

static int s_mode; // 0 = copy png/jpg/sfo, 1 = add iso/bin/img/mdf, 2 = create cache file

static int readfile_exfat(char *file, char *data, u64 size)
{
	if(size == 0) return FAILED;

	FIL fd;
	UINT re;

	if (!f_open(&fd, file, FA_READ))
	{
		f_read(&fd, data, size, &re);
		f_close(&fd);
		return SUCCESS;
	}

	return FAILED;
}

static int copy_exfat(char *src_file, char *out_file, u64 size)
{
	if(size == 0) return FAILED;

	FIL fd;	   // File objects

	if (!f_open (&fd, src_file, FA_READ))
	{
		char *mem = NULL;

		if((mem = malloc(size)) != NULL)
		{
			f_lseek (&fd, 0);
			UINT re;
			f_read (&fd, (void *)mem, size, &re);
			f_close (&fd);

			SaveFile(out_file, mem, re);

			free(mem);

			return (re == size) ? SUCCESS : FAILED;
		}
		else
		{
			f_close (&fd);
		}
	}
	return FAILED;
}

//dir contents
static int dir_read (char *dpath)
{
	if(!dpath || *dpath == 0) return FR_OK;

	char fn[256];
	char *subpath = NULL; int slen = 0;

	FDIR dir;
	FRESULT res = FR_OK;

	if(s_mode <= 1)
	{
		// find subdir name
		for(int i = strlen(dpath) - 1; i >= 6; i--)
			if(dpath[i] == '/')
			{
				subpath = dpath + i + 1;
				slen = strlen(subpath);
				break;
			}

		res = f_opendir(&dir, dpath);					   /* Open the directory */
		if (res == FR_OK)
		{
			bool is_iso;
			static FILINFO fno;
			for(;;)
			{
				FRESULT res1 = f_readdir(&dir, &fno);	   /* Read a directory item */
				if (res1 != FR_OK || fno.fname[0] == 0)
				{
					break;
				}
				if (fno.fattrib & AM_DIR)
				{				/* scan directories recursively */
					if(fno.fname[0] == '.') continue;
					snprintf (fn, 255, "%s/%s", dpath, fno.fname);
					dir_read (fn);
				}
				else if(s_mode == 0)
				{
					if(strstr(fno.fname, ".png") || strstr(fno.fname, ".PNG") || strstr(fno.fname, ".jpg") || strstr(fno.fname, ".JPG") || strstr(fno.fname, ".SFO"))
					{
						if(subpath && slen && strncmp(subpath, fno.fname, slen))
						{
							snprintf (wm_path, 255, CACHE_PATH "/[%s] %s", subpath, fno.fname);
						}
						else
							snprintf (wm_path, 255, CACHE_PATH "/%s", fno.fname);

						if((not_exists(wm_path)) && (fno.fsize < 4194304))
						{
							snprintf(fn, 255, "%s/%s", dpath, fno.fname);
							copy_exfat(fn, wm_path, fno.fsize);
						}
					}
				}
				else if(s_mode == 1)
				{
					int flen = strlen(fno.fname);
					if(flen < 4) continue; flen -= 4;
					char *ext = fno.fname + flen;

					//--- create .ntfs[BDFILES] for 4="VIDEO", 5="MOVIES", 6="PKG", 7="Packages", 8="packages", 9="BDFILE", 10="PS2ISO", 11="PSPISO", 12="MUSIC", 13="THEME", 14="UPDATE", 15="ROMS"
					if(g_mode >= 4)
					{
						// store path for conversion after the directory scan (fflib_file_to_sectors breaks the scan)
						nt[nn] = fno.fsize;
						snprintf (ff[nn], 255, "%s/%s", dpath, fno.fname);
						if(++nn >= MAX_ISOS) break;
						continue;
					}
					//---------------

					snprintf(fn, 255, "%s/%s", dpath, fno.fname);

					is_iso =	( (strcasestr(ext, ".iso")) ) ||
					(g_mode && (( (strcasestr(ext, ".bin")) ) ||
								( (strcasestr(ext, ".img")) ) ||
								( (strcasestr(ext, ".mdf")) ) ));

					// is multi-part ISO?
					if(!is_iso && flen > 2) is_iso = strcasestr(ext - 2, ".iso.0");

					if(!is_iso && (g_mode == PS3ISO))
					{
						if(subpath && slen && strncmp(subpath, fno.fname, slen))
						{
							flen = snprintf (wm_path, 255, CACHE_PATH "/[%s] %s", subpath, fno.fname);
						}
						else
							flen = snprintf (wm_path, 255, CACHE_PATH "/%s", fno.fname);

						// If the key exists, copy it to CACHE_PATH to
						// decrypt on-the-fly with Cobra when the ISO is mounted (By Evilnat)
						if(strcasestr(ext, ".key"))
						{
							copy_exfat(fn, wm_path, 0x10);
							continue;
						}
						// If the dkey exists, we convert it to disckey and copy it to CACHE_PATH
						// to decrypt on-the-fly with Cobra when the ISO is mounted (By Evilnat)
						else if(strcasestr(ext - 1, ".dkey"))
						{
							char dkey[0x20];
							if(!readfile_exfat(fn, dkey, 0x20))
							{
								strcpy(wm_path + flen - 5, ".key");

								uint8_t disckey[0x10];
								convert_dkey_to_key(disckey, dkey);
								SaveFile(wm_path, (char *)disckey, 0x10);
								continue;
							}
						}
					}

					//--- is ISO?
					if(is_iso)
					{
						cd_sector_size = 2352;
						cd_sector_size_param = 0;
						num_tracks = 1;

						if(g_mode == PSXISO)
						{
							FIL fd;
							int ret;

							// detect CD sector size
							if(!f_open (&fd, fn, FA_READ))
							{
								char buffer[20];
								u16 sec_size[7] = {2352, 2048, 2336, 2448, 2328, 2340, 2368};
								for(u8 n = 0; n < 7; n++)
								{
									f_lseek (&fd, ((sec_size[n]<<4) + 0x18));
									UINT lbr;
									f_read (&fd, (void *)buffer, 20, &lbr);
									if(  (memcmp(buffer + 8, "PLAYSTATION ", 0xC) == 0) ||
										((memcmp(buffer + 1, "CD001", 5) == 0) && buffer[0] == 0x01) ) {cd_sector_size = sec_size[n]; break;}
								}
								f_close(&fd);

								if(cd_sector_size & 0xf) cd_sector_size_param = cd_sector_size<<8;
								else if(cd_sector_size != 2352) cd_sector_size_param = cd_sector_size<<4;
							}

							// parse CUE file
							int path_len = snprintf (fn, 255, "%s/%s", dpath, fno.fname);
							const char *cue_ext[4] = {".cue", ".ccd", ".CUE", ".CCD"};
							for(u8 e = 0; e < 4; e++)
							{
								strcpy(fn + path_len - 4, cue_ext[e]);
								ret = f_open (&fd, fn, FA_READ);
								if(ret == SUCCESS) break;
							}

							if (ret >= SUCCESS)
							{
								UINT r = 0;
								f_read (&fd, (char *)cue_buf, sizeof(cue_buf), &r);
								f_close(&fd);

								if (r > 0)
								{
									char templn[MAX_LINE_LEN];
									num_tracks = parse_cue(templn, (char *)cue_buf, r, tracks);
								}
							}
						}

						// store path for conversion after the directory scan (fflib_file_to_sectors breaks the scan)
						nt[nn] = num_tracks;
						snprintf (ff[nn], 255, "%s/%s", dpath, fno.fname);
						if(++nn >= MAX_ISOS) break;
					}
				}
			}
			f_closedir(&dir);
		}
	}
	else // if(s_mode == 2)
	{
		int flen, dlen, plen, parts, ext_len;

		char *subdir, *filename;

		for(int f = 0; f < nn; f++)
		{
			// full path
			flen = snprintf (fn, 255, "%s", ff[f]);

			// clear sections
			for(int i = 0; i < MAX_SECTIONS; i++) sections[i] = sections_size[i] = 0;

			// ISO to sectors
			parts = fflib_file_to_sectors (fn, sections, sections_size, MAX_SECTIONS, 1);

			// get multi-part file sectors
			if((flen > 6) && strcasestr(fn + flen - 6, ".iso.0"))
			{
				FILINFO fno;
				char iso_path[MAX_PATH_LEN];

				strcpy(iso_path, fn);

				for (u8 o = 1; o < 64; o++)
				{
					if(parts >= MAX_SECTIONS) break;

					sprintf(iso_path + flen - 1, "%i", o);
					if(f_stat (iso_path, &fno) != FR_OK) break;

					parts += fflib_file_to_sectors (iso_path, sections + parts, sections_size + parts, MAX_SECTIONS - parts, 1);
				}
			}

			if (parts >= MAX_SECTIONS) continue;

			// get file name
			for(dlen = flen; dlen >= 6; dlen--) if(fn[dlen] == '/') break; // find last slash
			filename = fn + dlen + 1; fn[dlen] = 0; 

			// get subdir
			for(slen = dlen; slen >= 2; slen--) if(fn[slen] == '/') break; // find subdir name

			if(slen >= 6)
			{
				subdir = fn + slen + 1; slen = strlen(subdir);

				// replace [ ] in subdir
				for(int c = 0; subdir[c]; c++)
					if(subdir[c] == '[') subdir[c] = '('; else
					if(subdir[c] == ']') subdir[c] = ')';

				// prepend [subdir] to file name
				if(slen && strncmp(subdir, filename, slen))
					plen = snprintf (wm_path, 255, "[%s] %s", subdir, filename);
				else
					plen = snprintf (wm_path, 255, "%s", filename);
			}
			else
				plen = snprintf (wm_path, 255, "%s", filename);

			fn[dlen] = '/'; // restore full path
			ext_len = 4; if((plen > 6) && wm_path[plen - 6] == '.') ext_len = 6;// multi-part file

			// remove file extension
			if(g_remove_ext)
			{
				if((plen > ext_len) && wm_path[plen - ext_len] == '.') wm_path[plen - ext_len] = 0;
			}

			// create cache file
			if(g_mode >= 4)
			{
				char *ext = fn + flen - ext_len;
				make_fake_iso(g_mode, ext, filename, fn, device_id, nt[f]);
			}
			else
			{
				num_tracks = nt[f];
				build_file(wm_path, parts, num_tracks, device_id, g_profile, g_mode);
			}
		}

		nn = 0;
	}
	//
	return res;
}

static void scan_exfat(void)
{
	fflib_init();
	for(u8 port = 0; port < FF_VOLUMES; port++) // 0-7, 64-79, 120-130
	{
		if(port == 8)  port = 64;
		if(port == 80) port = 120;

		sprintf(path, "/dev_usb%03u/", port);
		if(file_exists(path)) continue; // skip scanning of this USB port if it is readable by PS3 file system

		device_id = USB_MASS_STORAGE(port);
		int ret = fflib_attach (port, device_id, 1);
		if (ret == FR_OK)
		{
			for (u8 p = 0; p < 2; p++)
			{
				for (u8 profile = 0; profile < 6; profile++)
				{
					g_profile = profile;
					for(u8 m = 0; m < MAX_MODES; m++) //0="PS3ISO", 1="BDISO", 2="DVDISO", 3="PSXISO", 4="VIDEO", 5="MOVIES", 6="PKG", 7="Packages", 8="packages", 9="BDFILE", 10="PS2ISO", 11="PSPISO", 12="MUSIC", 13="THEME", 14="UPDATE", 15="ROMS", 16="CD", 17="DVD"
					{
						g_mode = m;

						snprintf(path, sizeof(path), "%u:/%s%s%s", port, prefix[p], c_path[m], SUFIX(profile));
						for(s_mode = 0; s_mode < 3; s_mode++)
						{
							char drn[6]; /* Register work area to the default drive */
							FATFS fs;    /* Work area (filesystem object) for logical drive */
							sprintf(drn, "%u:/", port);
							f_mount(&fs, drn, 0);

							dir_read (path);

							f_mount(0, drn, 0);
						}
					}
				}
			}
		}
		fflib_detach (port);
	}
}
