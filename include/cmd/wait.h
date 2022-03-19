	if(islike(param, "/wait.ps3"))
	{
		// /wait.ps3?xmb
		// /wait.ps3?<secs>
		// /wait.ps3/<path>

		if(param[9] == '/')
			wait_for(param + 9, 30);
		else if(islike(param + 9, "?xmb"))
			wait_for_xmb();
		else
			sys_ppu_thread_sleep(val(param + 10));

		if(!mc) keep_alive = http_response(conn_s, header, param, CODE_HTTP_OK, param);

		goto exit_handleclient_www;
	}
