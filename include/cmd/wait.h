	if(islike(param, "/wait.ps3"))
	{
		// /wait.ps3?xmb
		// /wait.ps3?<secs>
		// /wait.ps3/<path>

		char *param2 = param + 9;

		if(*param2 == '/')
		{
			check_path_tags(param2);
			wait_for(param2, 30);
		}
		else if(islike(param2, "?xmb"))
			wait_for_xmb();
		else
			sys_ppu_thread_sleep(val(param2 + 1));

		if(!mc) keep_alive = http_response(conn_s, header, param, CODE_HTTP_OK, param);

		goto exit_handleclient_www;
	}
