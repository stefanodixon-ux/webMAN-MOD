#ifdef USE_NTFS
	// /mount.ps3/dev_ntfs* => convert ntfs path to cached path
	if(islike(param, "/mount") && is_ntfs_path(param + 10))
	{
		char *filename = param + 10; change_cue2iso(filename);
		concat2(header, filename, ".ntfs[");
		int flen = get_name(filename, header, GET_WMTMP);

		for(int i = 2; i < 9; i++) // "PS3ISO", "BDISO", "DVDISO", "PS2ISO", "PSXISO", "PSXGAMES", "PSPISO"
			if(strstr(header, paths[i])) {sprintf(filename + flen, ".ntfs[%s]", paths[i]); break;}

		if(not_exists(filename))
		{
			check_ntfs_volumes();
			prepNTFS(0);
		}
	}
#endif
	if(islike(param, "/mount.ps3?"))
	{
		// /mount.ps3?<query>  search game & mount if only 1 match is found

		if(islike(param, "/mount.ps3?http"))
		{
			char *url  = param + 11;
			do_umount(false);  open_browser(url, 0);
			keep_alive = http_response(conn_s, header, param, CODE_HTTP_OK, url);
			goto exit_handleclient_www;
		}
		else
			{memcpy(header, "/index.ps3", 10); memcpy(param, "/index.ps3", 10); auto_mount = true;}
	}
