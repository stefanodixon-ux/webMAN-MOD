	if(islike(param, "/restart.ps3") || islike(param, "/reboot.ps3"))
	{
		// /reboot.ps3           Hard reboot
		// /restart.ps3          Reboot using default mode (VSH reboot is the default); skip content scan on reboot
		// /restart.ps3?0        Allow scan content on restart
		// /restart.ps3?quick    Quick reboot (load LPAR id 1)
		// /restart.ps3?vsh      VSH Reboot
		// /restart.ps3?soft     Soft reboot
		// /restart.ps3?hard     Hard reboot
		// /restart.ps3?<mode>$  Sets the default restart mode for /restart.ps3
		// /restart.ps3?min      Reboot & show min version
	 reboot:
		css_exists = common_js_exists = false;

		http_response(conn_s, header, param, CODE_HTTP_OK, param);
		setPluginExit();

		if(sysmem) sys_memory_free(sysmem);

		del_turnoff(2);

		bool allow_scan = get_flag(param,"?0");
		if (!allow_scan) create_file(WM_NOSCAN_FILE);

		bool is_restart = IS(param, "/restart.ps3");

		char mode = 'h', *params = strchr(param, '?');
		#ifndef LITE_EDITION
		if(params) {mode = LCASE(params[1]); if(strchr(param, '$')) {webman_config->default_restart = mode; save_settings();}} else if(is_restart) mode = webman_config->default_restart;
		#else
		if(params)  mode = LCASE(params[1]); else if(is_restart) mode = webman_config->default_restart;
		#endif
		if(mode == 'q')
			{system_call_3(SC_SYS_POWER, SYS_REBOOT, NULL, 0);} // (quick reboot) load LPAR id 1
		else
		if(mode == 's')
			{system_call_3(SC_SYS_POWER, SYS_SOFT_REBOOT, NULL, 0);} // soft reboot
		else
		if(mode == 'h')
			{system_call_3(SC_SYS_POWER, SYS_HARD_REBOOT, NULL, 0);} // hard reboot
		#ifndef LITE_EDITION
		else
		if(mode == 'm')
			reboot_show_min_version(""); // show min version
		#endif
		else //if(mode == 'v' || is_restart)
			vsh_reboot(); // VSH reboot

		goto exit_handleclient_www;
	}
