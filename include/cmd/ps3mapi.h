#ifdef PS3MAPI
	if(islike(param, "/ps3mapi.ps3"))
	{
		char *wildcard = NULL;
		char *cmd = buffer + _4KB_; strcopy(cmd, param + ((param[12] == '/') ? 12 : 13));

		if(file_exists(cmd) && !isDir(cmd)) strcat(cmd, "*");

		if(*cmd == '/')
		{
			if((wildcard = strchr(cmd, '*'))) {wildcard = strrchr(cmd, '/'), *wildcard++ = NULL;}
		}

		if(file_exists(cmd))
		{
			save_file(FILE_LIST_TXT, "{\"files\": [\r\n", SAVE_ALL);
			scan(cmd, (strstr(cmd, "//") != NULL), wildcard, SCAN_LIST_JSON, FILE_LIST_TXT);
			save_file(FILE_LIST_TXT, "]}\r\n", APPEND_TEXT);

			strcpy(param, FILE_LIST_TXT);
			is_busy = false, allow_retry_response = false;
			goto html_listing;
		}
		else
		{
			ps3mapi_command(0, 0, 0, cmd);
			keep_alive = http_response(conn_s, buffer, param, CODE_JSON_RESPONSE, cmd);
		}
	}
	else if(islike(param, "/home.ps3mapi"))
	{
		ps3mapi_home(pbuffer, html);
	}
	else
	if(islike(param, "/buzzer.ps3mapi") || islike(param, "/beep.ps3"))
	{
		ps3mapi_buzzer(pbuffer, html, param);
	}
	else
	if(islike(param, "/led.ps3mapi"))
	{
		ps3mapi_led(pbuffer, html, param);
	}
	else
	if(islike(param, "/notify.ps3mapi"))
	{
		ps3mapi_notify(pbuffer, html, param);
	}
	else
	if(islike(param, "/mappath.ps3mapi"))
	{
		ps3mapi_mappath(pbuffer, html, param);
	}
	else
	if(islike(param, "/syscall.ps3mapi"))
	{
		ps3mapi_syscall(pbuffer, html, param);
	}
	else
	if(islike(param, "/syscall8.ps3mapi"))
	{
		ps3mapi_syscall8(pbuffer, html, param);
	}
	else
	if(islike(param, "/patch.ps3"))
	{
		// example: /patch.ps3?addr=19A8000&stop=19D0000&find=fs_path&rep=2&offset=16&val=2E4A5047000000002E706E67000000002E676966000000002E626D70000000002E67696D000000002E504E4700

		memcpy(param + 2, ".ps3mapi?", 9); // make alias of /getmem.ps3mapi
		ps3mapi_getmem(pbuffer, html, param);
	}
	else
	if(islike(param, "/getmem.ps3mapi"))
	{
		ps3mapi_getmem(pbuffer, html, param);
	}
	else
	if(islike(param, "/setmem.ps3mapi"))
	{
		ps3mapi_setmem(pbuffer, html, param);
	}
	else
	if(islike(param, "/payload.ps3mapi"))
	{
		ps3mapi_payload(pbuffer, html, param);
	}
	else
	if(islike(param, "/setidps.ps3mapi"))
	{
		ps3mapi_setidps(pbuffer, html, param);
	}
	else
	if(islike(param, "/vshplugin.ps3mapi"))
	{
		ps3mapi_vshplugin(pbuffer, html, param);
	}
	else
	if(islike(param, "/kernelplugin.ps3mapi"))
	{
		ps3mapi_kernelplugin(pbuffer, html, param);
	}
	else
	if(islike(param, "/gameplugin.ps3mapi"))
	{
		ps3mapi_gameplugin(pbuffer, html, param);
	}
	else
#endif // #ifdef PS3MAPI
