	if(islike(param, "/games.ps3"))
	{
		// /games.ps3
		// /index.ps3?mobile
		// /dev_hdd0/xmlhost/game_plugin/mobile.html

 mobile_response:
		mobile_mode = true; const char *params = param + 10;

		if(is_ps3_http)
			sprintf(param, GAMES_HTML);
		else if(not_exists(MOBILE_HTML))
			{sprintf(param, "/index.ps3%s", params); mobile_mode = false;}
		else if(strstr(param, "?g="))
			sprintf(param, MOBILE_HTML);
		else if(strchr(param, '?'))
			sprintf(param, "/index.ps3%s", params);
		else if(not_exists(GAMELIST_JS))
			sprintf(param, "/index.ps3?mobile");
		else
			sprintf(param, MOBILE_HTML);
	}
	else mobile_mode = false;
