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

		bool is_reset = false; char *param2 = param + 11; int ret = 0;
		if(islike(param2, "?wmreset")) is_reset=true;
		if(is_reset || islike(param2, "?wmconfig")) {reset_settings(); sprintf(param, "/delete_ps3%s", WMCONFIG);}
		if(is_reset || islike(param2, "?wmtmp")) {do_umount(true); sprintf(param, "/delete_ps3%s", WMTMP);}

		check_path_tags(param2);

		bool is_dir = isDir(param2);

		if(islike(param2 , "?history"))
		{
			delete_history(true);
			sprintf(tempstr, "%s : history", STR_DELETE);
			sprintf(param, "/");
		}
		else if(islike(param2 , "?uninstall"))
		{
			uninstall(param);
			goto reboot;
		}
		else
		{
			if(islike(param2, "/dev_hdd1")) mount_device(param2, NULL, NULL); // auto-mount device

			char *wildcard = strrchr(param2, '*');
			if(wildcard)
			{
				wildcard = strrchr(param2, '/'); *wildcard++ = NULL;
			}
			//ret = del(param2, islike(param, "/delete.ps3"));
			ret = scan(param2, islike(param, "/delete.ps3"), wildcard, SCAN_DELETE, NULL);

			sprintf(tempstr, "%s %s : ", STR_DELETE, ret ? STR_ERROR : ""); _concat(&sbuffer, tempstr);
			add_breadcrumb_trail(pbuffer, param2); sprintf(tempstr, "<br>");
			remove_filename(param2);
		}

		_concat(&sbuffer, tempstr);
		sprintf(tempstr, HTML_REDIRECT_TO_URL, param2, (is_dir | ret) ? HTML_REDIRECT_WAIT : 0); _concat(&sbuffer, tempstr);

		if(do_restart) goto reboot;
	}
	else
#endif
