	if(islike(param, "/quit.ps3"))
	{
		// quit.ps3            Stops webMAN and sets fan based on settings
		// quit.ps3?0          Stops webMAN and sets fan to syscon mode
		// quit.ps3?1          Stops webMAN and sets fan to fixed speed specified in PS2 mode
		// quit.ps3<prx-path>  Stops webMAN and load vsh plugin
		// quit.ps3?unload     Stops webMAN and force unload from memory but causes an interrupt(exception) in the user PPU Thread

	#ifdef LOAD_PRX
	quit:
	#endif
		http_response(conn_s, header, param, CODE_HTTP_OK, param);

		if(sysmem) sys_memory_free(sysmem);

		restore_settings();

		if(get_flag(param, "?0")) restore_fan(SYSCON_MODE);  //syscon
		if(get_flag(param, "?1")) restore_fan(SET_PS2_MODE); //ps2 mode
		if(get_flag(param, "?u")) wm_unload_combo = 4; //force unload module from memory

		#ifdef COBRA_ONLY
		char *plugin_path = param + 9;
		check_path_alias(plugin_path);
		if(file_exists(plugin_path))
		{
			if(strstr(plugin_path, "/webftp_server"))
			{
				create_file(WM_RELOAD_FILE); // create semaphore file
				load_vsh_module(plugin_path);
				unload_me(3);
				goto exit_handleclient_www;
			}
			else
				load_vsh_module(plugin_path);
		}
		#endif

		wwwd_stop();
		sys_ppu_thread_exit(0);
	}
