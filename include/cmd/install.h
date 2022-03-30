#ifdef PKG_HANDLER
	if(islike(param, "/install.ps3") || islike(param, "/install_ps3"))
	{
		// /install.ps3<pkg-path>  (see pkg_handler.h for details)
		// /install.ps3<pkg-path>? conditional install
		// /install_ps3<pkg-path>  install & keep pkg
		// /install.ps3?url=<url>  download, auto-install pkg & delete pkg
		// /install_ps3?url=<url>  download, auto-install pkg & keep pkg in /dev_hdd0/packages
		// /install.ps3<pkg-path>?restart.ps3
		// /install.ps3?url=<url>?restart.ps3
		// /install.ps3<p3t-path>  install ps3 theme
		// /install.ps3<directory> install pkgs in directory (GUI)
		// /install.ps3$           install webman addons (GUI)
		// /install_ps3$all        install all pkg files in /dev_hdd0/packages & delete pkg
		// /install_ps3            install all pkg files in /dev_hdd0/packages
		// /install_ps3$           install all webman addons
		// /install_ps3<path>      install pkg files in <path>

		char *pkg_file = param + 12; if(*pkg_file == '/') check_path_alias(pkg_file);

		bool install_ps3 = (param[8] == '_');

		if(islike(pkg_file, "/net"))
		{
			sprintf(param, "%s%s", "/mount_ps3", pkg_file); // use /mount.ps3 command to install the remote pkg
			is_binary = WEB_COMMAND;
			goto html_response;
		}

		pkg_delete_after_install = islike(pkg_file, "$all");

		if(!*pkg_file || pkg_delete_after_install)
		{
			strcpy(pkg_file, DEFAULT_PKG_PATH);
		}
		else if( *pkg_file == '$' )
		{
			strcpy(pkg_file, WM_RES_PATH);
		}

		if(isDir(pkg_file))
		{
			if(install_ps3) // /install_ps3/<path> = install all files
			{
				installPKG_all(pkg_file, pkg_delete_after_install); // keep files
				sys_ppu_thread_sleep(1); // wait to copy "pkgfile" variable in installPKG_combo_thread
			}
			is_binary = WEB_COMMAND;
			goto html_response;
		}

		size_t last_char = strlen(param) - 1;
		if(param[last_char] == '?')
		{
			param[last_char] = '\0';
			get_pkg_size_and_install_time(param + 12);
			if(isDir(install_path))
			{
				strcpy(param, install_path);
				is_binary = FOLDER_LISTING;
				goto html_response;
			}
		}

		char msg[MAX_LINE_LEN]; memset(msg, 0, sizeof(msg));

		setPluginActive();

		pkg_delete_after_install = (install_ps3 == false);

		int ret = installPKG(param + 12, msg);

		#ifdef WM_REQUEST
		if(!wm_request)
		#endif
		{
			if(!mc) keep_alive = http_response(conn_s, header, param, (ret == FAILED) ? CODE_BAD_REQUEST : CODE_INSTALL_PKG, msg);
		}

		if(!(webman_config->minfo & 1)) show_msg(msg);

		if(pkg_delete_after_install || do_restart)
		{
			if(loading_html) loading_html--;

			wait_for_pkg_install();

			setPluginInactive();
			if(do_restart) goto reboot;
			if(mc) goto exit_handleclient_www;
		}

		setPluginInactive();

		goto exit_handleclient_www;
	}
#endif // #ifdef PKG_HANDLER
