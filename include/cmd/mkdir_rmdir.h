#ifdef COPY_PS3
	if(islike(param, "/mkdir.ps3"))
	{
		// /mkdir.ps3        creates ISO folders in hdd0
		// /mkdir.ps3<path>  creates a folder & parent folders

		check_path_tags(param);

		if(param[10] == '/')
		{
			strcpy(param, param + 10);

			filepath_check(param);

			mkdir_tree(param);
			#ifdef USE_NTFS
			if(is_ntfs_path(param))
				ps3ntfs_mkdir(ntfs_path(param), DMODE);
			else
			#endif
			cellFsMkdir(param, DMODE);
		}
		else
		{
			mkdirs(param); // make hdd0 dirs GAMES, PS3ISO, PS2ISO, packages, etc.
		}

		is_binary = FOLDER_LISTING;
		goto html_response;
	}
	if(islike(param, "/rmdir.ps3"))
	{
		// /rmdir.ps3        deletes history files & remove empty ISO folders
		// /rmdir.ps3<path>  removes empty folder

		char *params = param + 10;
		check_path_tags(params);

		if(*params == '/')
		{
			#ifdef USE_NTFS
			if(is_ntfs_path(params)) {ps3ntfs_unlink(ntfs_path(params));}
			else
			#endif
			cellFsRmdir(params);
			remove_filename(params);
		}
		else
			{delete_history(true); strcpy(param, drives[0]);}

		is_binary = FOLDER_LISTING;
		goto html_response;
	}
#endif // #ifdef COPY_PS3
