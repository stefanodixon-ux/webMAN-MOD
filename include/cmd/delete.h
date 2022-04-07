#ifndef LITE_EDITION
	if(islike(param, "/delete.ps3") || islike(param, "/delete_ps3"))
	{
		// /delete_ps3<path>      deletes <path>
		// /delete.ps3<path>      deletes <path> and recursively delete subfolders
		// /delete_ps3<path>?restart.ps3
		// /delete.ps3<path>?restart.ps3

		// /delete.ps3?wmreset    deletes wmconfig & clear /dev_hdd0/tmp/wmtmp
		// /delete.ps3?wmconfig   deletes wmconfig
		// /delete.ps3?wmtmp      clear /dev_hdd0/tmp/wmtmp
		// /delete.ps3?history    deletes history files & remove empty ISO folders
		// /delete.ps3?uninstall  uninstall webMAN MOD & delete files installed by updater

		bool is_reset = false; char *params = param + 11; int ret = 0;
		if(islike(params, "?wmreset")) is_reset=true;
		if(is_reset || islike(params, "?wmconfig")) {reset_settings(); sprintf(param, "/delete_ps3%s", WMCONFIG);}
		if(is_reset || islike(params, "?wmtmp")) {do_umount(true); sprintf(param, "/delete_ps3%s", WMTMP);}

		check_path_tags(params);

		bool is_dir = isDir(params);

		if(islike(params , "?history"))
		{
			delete_history(true);
			sprintf(tempstr, "%s : history", STR_DELETE); _concat(&sbuffer, tempstr);
			sprintf(params, "/"); // redirect to root
		}
		else if(islike(params , "?uninstall"))
		{
			uninstall(param);
			goto reboot;
		}
		else
		{
			if(islike(params, "/dev_hdd1")) mount_device(params, NULL, NULL); // auto-mount device

			Check_Overlay();

			char *wildcard = strrchr(params, '*');
			if(wildcard)
			{
				wildcard = strrchr(params, '/'); *wildcard++ = NULL;
			}
			ret = scan(params, islike(param, "/delete.ps3"), wildcard, SCAN_DELETE, NULL);

			sprintf(tempstr, "%s %s : ", STR_DELETE, ret ? STR_ERROR : ""); _concat(&sbuffer, tempstr);
			add_breadcrumb_trail(pbuffer, params); _concat(&sbuffer, "<br>");

			remove_filename(params); // redirect to parent folder
		}

		sprintf(tempstr, HTML_REDIRECT_TO_URL, params, (is_dir | ret) ? HTML_REDIRECT_WAIT : 0); _concat(&sbuffer, tempstr);

		if(do_restart) goto reboot;
	}
	else
#endif
