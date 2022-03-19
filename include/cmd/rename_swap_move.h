	if(islike(param, "/rename.ps3") || islike(param, "/swap.ps3") || islike(param, "/move.ps3"))
	{
		// /rename.ps3<path>|<dest>       rename <path> to <dest>
		// /rename.ps3<path>&to=<dest>    rename <path> to <dest>
		// /rename.ps3<path>.bak          removes .bak extension
		// /rename.ps3<path>.bak          removes .bak extension
		// /rename.ps3<path>|<dest>?restart.ps3
		// /rename.ps3<path>&to=<dest>?restart.ps3
		// /move.ps3<path>|<dest>         move <path> to <dest>
		// /move.ps3<path>&to=<dest>      move <path> to <dest>
		// /move.ps3<path>|<dest>?restart.ps3
		// /move.ps3<path>&to=<dest>?restart.ps3
		// /swap.ps3<file1>|<file2>       swap <file1> & <file2>
		// /swap.ps3<file1>&to=<file2>    swap <file1> & <file2>
		// /swap.ps3<file1>|<file2>?restart.ps3
		// /swap.ps3<file1>&to=<file2>?restart.ps3

		#define SWAP_CMD	9
		#define RENAME_CMD	11

		size_t cmd_len = islike(param, "/rename.ps3") ? RENAME_CMD : SWAP_CMD;

		char *source = param + cmd_len, *dest = strchr(source, '|');
		if(dest) {*dest++ = NULL;} else {dest = strstr(source, "&to="); if(dest) {*dest = NULL, dest+=4;}}

		if(dest && (*dest != '/') && is_ext(source, ".bak")) {size_t flen = strlen(source); *dest = *param + flen; strncpy(dest, source, flen - 4);}

		check_path_alias(source);

		if(islike(source, "/dev_blind") || islike(source, "/dev_hdd1")) mount_device(source, NULL, NULL); // auto-mount source device
		if(islike(dest,   "/dev_blind") || islike(dest,   "/dev_hdd1")) mount_device(dest,   NULL, NULL); // auto-mount destination device

		if(dest && *dest == '/')
		{
			filepath_check(dest);
	#ifdef COPY_PS3
			char *wildcard = strchr(source, '*');
			if(wildcard)
			{
				wildcard = strrchr(source, '/'); *wildcard++ = NULL;
				scan(source, true, wildcard, SCAN_MOVE, dest);
			}
			else if(islike(param, "/move.ps3"))
			{
				if(isDir(source))
				{
					if(folder_copy(source, dest) >= CELL_OK) del(source, true);
				}
				else
				{
					if(file_copy(source, dest, COPY_WHOLE_FILE) >= CELL_OK) del(source, false);
				}
			}
			else
	#endif
			if((cmd_len == SWAP_CMD) && file_exists(source) && file_exists(dest))
			{
				sprintf(header, "%s.bak", source);
				cellFsRename(source, header);
				cellFsRename(dest, source);
				cellFsRename(header, dest);
			}
	#ifdef USE_NTFS
			else if(is_ntfs_path(source) || is_ntfs_path(dest))
			{
				u8 ps = 0, pt = 0;
				if(is_ntfs_path(source)) {ps = 5, source[10] = ':';}
				if(is_ntfs_path(dest)) {pt = 5, dest[10] = ':';}

				ps3ntfs_rename(source + ps, dest + pt);
			}
	#endif
			else
				cellFsRename(source, dest);

			sprintf(param, "%s", remove_filename(dest));
			if(do_restart) goto reboot;
		}
		else
		{
			sprintf(param, "%s", remove_filename(source));
			if(!isDir(param)) sprintf(param, "/");
		}

		is_binary = FOLDER_LISTING;
		goto html_response;
	}
