	if((refreshing_xml == 0) && islike(param, "/refresh"))
	{
		if(islike(param + 8, "_ps3"))
		{
			refresh_xml(param);

			if(IS_ON_XMB && is_app_dir(_HDD0_GAME_DIR, "RELOADXMB") && is_app_home_onxmb())
			{
				reload_xmb();
				sys_ppu_thread_sleep(3);
				if(IS_ON_XMB) launch_app_home_icon(true);
			}

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
