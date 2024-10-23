	if((refreshing_xml == 0) && islike(param, "/refresh"))
	{
		if(islike(param + 8, "_ps3"))
		{
			char *msg = param;
			refresh_xml(msg);

			reload_xmb(0);

			#ifdef WM_REQUEST
			if(!wm_request)
			#endif
			keep_alive = http_response(conn_s, header, param, CODE_HTTP_OK, param);
			goto exit_handleclient_www;
		}

		#ifdef USE_NTFS
		if(webman_config->ntfs)
		{
			get_game_info();
			skip_prepntfs = (strcmp(_game_TitleID, "BLES80616") == 0); // /refresh.ps3 called from prepNTFS application
		}
		#endif
	}
