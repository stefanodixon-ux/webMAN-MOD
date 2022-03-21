	if(islike(param, "/wait.ps3"))
	{
		// /wait.ps3?xmb
		// /wait.ps3?<secs>
		// /wait.ps3/<path>

		char *params = param + 9;

		if(*params == '/')
		{
			check_path_tags(params);
			wait_for(params, 30);
		}
		else if(islike(params, "?xmb"))
			wait_for_xmb();
		else
			sys_ppu_thread_sleep(val(params + 1));

		if(!mc) keep_alive = http_response(conn_s, header, param, CODE_HTTP_OK, param);

		goto exit_handleclient_www;
	}
