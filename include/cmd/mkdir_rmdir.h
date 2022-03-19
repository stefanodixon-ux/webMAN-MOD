#ifdef COPY_PS3
	if(islike(param, "/mkdir.ps3"))
	{
		// /mkdir.ps3        creates ISO folders in hdd0
		// /mkdir.ps3<path>  creates a folder & parent folders

		check_path_tags(param);

		if(param[10] == '/')
		{
			sprintf(param, "%s", param + 10);

			filepath_check(param);

			mkdir_tree(param);
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

		check_path_tags(param);

		if(param[10] == '/')
		{
			sprintf(param, "%s", param + 10);
			#ifdef USE_NTFS
			if(is_ntfs_path(param)) {param[10] = ':'; ps3ntfs_unlink(param + 5);}
			else
			#endif
			cellFsRmdir(param);
			remove_filename(param);
		}
		else
			{delete_history(true); sprintf(param, "/dev_hdd0");}

		is_binary = FOLDER_LISTING;
		goto html_response;
	}
#endif // #ifdef COPY_PS3
