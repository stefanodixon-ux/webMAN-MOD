	if(islike(param, "/quit.ps3"))
	{
		// quit.ps3            Stops webMAN and sets fan based on settings
		// quit.ps3?0          Stops webMAN and sets fan to syscon mode
		// quit.ps3?1          Stops webMAN and sets fan to fixed speed specified in PS2 mode
		// quit.ps3<prx-path>  Stops webMAN and load vsh plugin

	#ifdef LOAD_PRX
	quit:
	#endif
		http_response(conn_s, header, param, CODE_HTTP_OK, param);

		if(sysmem) sys_memory_free(sysmem);

		restore_settings();

		if(strstr(param, "?0")) restore_fan(SYSCON_MODE);  //syscon
		if(strstr(param, "?1")) restore_fan(SET_PS2_MODE); //ps2 mode

		#ifdef COBRA_ONLY
		char *plugin_path = param + 9;
		check_path_alias(plugin_path);
		if(file_exists(plugin_path)) load_vsh_plugin(plugin_path);
		#endif

		wwwd_stop();
		sys_ppu_thread_exit(0);
	}
