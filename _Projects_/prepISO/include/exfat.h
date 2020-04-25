static char subpath[MAX_PATH_LEN];

//dir contents
static int dir_read (char *dpath)
{
    /* Register work area to the default drive */
    FATFS fs;   /* Work area (filesystem object) for logical drive */
    char drn[5];
    snprintf(drn, 4, "%.3s", dpath);
    f_mount(&fs, drn, 0);

    char fn[256];

    FDIR dir;
    FRESULT res;
    res = f_opendir(&dir, dpath);                       /* Open the directory */
    if (res == FR_OK)
    {
        bool is_iso;
        static FILINFO fno;
        for (;;)
        {
            FRESULT res1 = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res1 != FR_OK || fno.fname[0] == 0)
            {
                f_closedir(&dir);
                return res;  /* Break on error or end of dir */
            }
            if (fno.fattrib & AM_DIR)
            {                /* scan directories recursively */
                snprintf (subpath, 255, "%s", fno.fname);
                snprintf (fn, 255, "%s/%s", dpath, fno.fname);
                dir_read (fn);
            }
            else
            {
				int flen = strlen(fno.fname);
				if(flen < 4) continue; flen -= 4;
				char *ext = fno.fname + flen;

				//--- create .ntfs[BDFILES] for 4="VIDEO", 5="MOVIES", 6="PKG", 7="Packages", 8="packages", 9="BDFILE", 10="PS2ISO", 10="PSPISO"
				if(g_mode >= 4)
				{
					snprintf (fn, 255, "%s/%s", dpath, fno.fname);
					make_fake_iso(g_mode, ext, fno.fname, fn, device_id, fno.fsize);
					continue;
				}
				//---------------

				//--- is ISO?
				is_iso =	( (strcasestr(ext, ".iso")) ) ||
				(g_mode && (( (strcasestr(ext, ".bin")) ) ||
							( (strcasestr(ext, ".img")) ) ||
							( (strcasestr(ext, ".mdf")) ) ));

				if(is_iso)
				{
					cd_sector_size = 2352;
					cd_sector_size_param = 0;
					num_tracks = 1; // TO-DO: detect number of tracks for psx_emu

					if(g_mode == PSXISO)
					{
						FIL fd;
						int ret = 0;

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
						strcpy(fn + path_len - 3, "CUE");
						if(f_open (&fd, fn, FA_READ))
						{
							strcpy(fn + path_len - 3, "cue");
							if(f_open (&fd, fn, FA_READ)) ret = -1;
						}

						if (ret >= 0)
						{
							UINT r = 0;
							f_read (&fd, (char *)cue_buf, sizeof(cue_buf), &r);
							f_close(&fd);

							if (r > 0)
							{
								char dummy[64];

								if (cobra_parse_cue(cue_buf, r, tracks, 100, &num_tracks, dummy, sizeof(dummy)-1) != 0)
								{
									num_tracks = 1;
								}
							}
						}
					}

					snprintf (fn, 255, "%s/%s", dpath, fno.fname);
					int parts = fflib_file_to_sectors (fn, sections, sections_size, MAX_SECTIONS, 1);
					if (parts >= MAX_SECTIONS) continue;

					snprintf (fno.fname + flen, 255, " [exFAT]%s", ext);

					if(*subpath)
						snprintf (fn, 255, "[%s] %s", subpath, fno.fname);
					else
						snprintf (fn, 255, "%s", fno.fname);

					build_file(fn, parts, num_tracks, device_id, g_profile, g_mode);
				}
            }
        }
        f_closedir(&dir);
    }
    //
    f_mount(0, drn, 0);
    //
    return res;
}

static void scan_exfat(void)
{
	fflib_init();
	for(u8 port = 0; port < 8; port++)
	{
		device_id = USB_MASS_STORAGE(port);
		int ret = fflib_attach (port, device_id, 1);
		if (ret == FR_OK)
		{
			for (u8 profile = 0; profile < 6; profile++)
			{
				g_profile = profile;
				for(u8 m = 0; m < 12; m++) //0="PS3ISO", 1="BDISO", 2="DVDISO", 3="PSXISO", 4="VIDEO", 5="MOVIES", 6="PKG", 7="Packages", 8="packages", 9="BDFILE", 10="PS2ISO", 10="PSPISO"
				{
					g_mode = m;
					*subpath = 0;

					snprintf(path, sizeof(path), "%u:/%s%s", port, c_path[m], SUFIX(profile));
					dir_read (path);
				}
			}
		}
		fflib_detach (port);
	}
}