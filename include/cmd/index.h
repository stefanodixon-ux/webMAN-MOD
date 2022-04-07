	else if(islike(param, "/index.ps3") || islike(param, "/sman.ps3"))
	{
		// /index.ps3                  show game list in HTML (refresh if cache file is not found)
		// /index.ps3?html             refresh game list in HTML
		// /index.ps3?launchpad        refresh game list in LaunchPad xml
		// /index.ps3?mobile           show game list in coverflow mode
		// /index.ps3?<query>          search game by device name, path or name of game
		// /index.ps3?<device>?<name>  search game by device name and name
		// /index.ps3?<query>&mobile   search game by device name, path or name of game in coverflow mode
		// /index.ps3?cover=<mode>     refresh game list in HTML using cover type (icon0, mm, disc, online)

		Check_Overlay();

		mobile_mode |= ((strstr(param, "?mob") != NULL) || (strstr(param, "&mob") != NULL));
		#ifdef LAUNCHPAD
		if(get_flag(param, "?launchpad")) {mobile_mode = LAUNCHPAD_MODE, auto_mount = false; sprintf(templn, "%s LaunchPad: %s", STR_REFRESH, STR_SCAN2); show_msg(templn);}
		#endif
		if(game_listing(buffer, templn, param, tempstr, mobile_mode, auto_mount) == false)
		{
			{ PS3MAPI_RESTORE_SC8_DISABLE_STATUS }
			{ PS3MAPI_DISABLE_ACCESS_SYSCALL8 }

			keep_alive = http_response(conn_s, header, param, CODE_SERVER_BUSY, STR_ERROR);

			is_busy = false;

			goto exit_handleclient_www;
		}

		// mount query found only 1 entry
		if(auto_mount && islike(buffer, "/mount.ps3")) {auto_mount = false; strcpy(param, buffer); goto redirect_url;}

		if(is_ps3_http)
		{
			char *pos = strstr(pbuffer, "&#x1F50D;"); // hide search icon
			if(pos) for(u8 c = 0; c < 9; c++) pos[c] = ' ';
		}
	}
