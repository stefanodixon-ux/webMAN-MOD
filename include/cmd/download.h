#ifdef PKG_HANDLER
	if(islike(param, "/download.ps3"))
	{
		// /xmb.ps3/download.ps3?url=<url>            (see pkg_handler.h for details)
		// /xmb.ps3/download.ps3?to=<path>&url=<url>

		char msg[MAX_LINE_LEN], filename[STD_PATH_LEN]; _memset(msg, sizeof(msg)); *filename = NULL;

		setPluginActive();

		int ret = download_file(strchr(param_original, '%') ? (param_original + 13) : (param + 13), msg);

		char *dlpath = strchr(msg, '\n'); // get path in "...\nTo: /path/"

		if(dlpath)
		{
			*dlpath = NULL; // limit string to url in "Downloading http://blah..."

			const char *dlfile = get_filename(msg);
			if(dlfile) snprintf(filename, STD_PATH_LEN, "%s%s", dlpath + 5, dlfile);

			*dlpath = '\n'; // restore line break
		}

		#ifdef WM_REQUEST
		if(!wm_request)
		#endif
		{
			if(!mc) keep_alive = http_response(conn_s, header, param, (ret == FAILED) ? CODE_BAD_REQUEST : (is_ps3_http)  ? CODE_BLANK_PAGE : CODE_DOWNLOAD_FILE, msg);
		}

		if(!(webman_config->minfo & 1)) show_msg(msg);

		// wait while content is downloading
		sys_ppu_thread_sleep(5);

		while(IS_DOWNLOADING)
			sys_ppu_thread_sleep(2);

		wait_for_xml_download(filename, param);

		setPluginInactive(); if(from_xmb_ps3) unload_plugin_modules(true);
		goto exit_handleclient_www;
	}
#endif