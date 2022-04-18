	if(islike(param, "/shutdown.ps3"))
	{
		// /shutdown.ps3        Shutdown using VSH
		// /shutdown.ps3?vsh    Shutdown using VSH
		// /shutdown.ps3?off    Power off

		css_exists = common_js_exists = false;

		http_response(conn_s, header, param, CODE_HTTP_OK, param);
		setPluginExit();

		if(sysmem) sys_memory_free(sysmem);

		del_turnoff(1);

		if(param[14] != 'o')
			vsh_shutdown(); // shutdown using VSH
		else
			{system_call_4(SC_SYS_POWER, SYS_SHUTDOWN, 0, 0, 0);}

		sys_ppu_thread_exit(0);
	}
