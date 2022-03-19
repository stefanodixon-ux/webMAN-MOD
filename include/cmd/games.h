	if(islike(param, "/games.ps3"))
	{
		// /games.ps3
		// /index.ps3?mobile
		// /dev_hdd0/xmlhost/game_plugin/mobile.html

 mobile_response:
		mobile_mode = true; char *param2 = param + 10;

		if(not_exists(MOBILE_HTML))
			{sprintf(param, "/index.ps3%s", param2); mobile_mode = false;}
		else if(strstr(param, "?g="))
			sprintf(param, MOBILE_HTML);
		else if(strchr(param, '?'))
			sprintf(param, "/index.ps3%s", param2);
		else if(not_exists(GAMELIST_JS))
			sprintf(param, "/index.ps3?mobile");
		else
			sprintf(param, MOBILE_HTML);
	}
	else mobile_mode = false;
