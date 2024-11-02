	if(islike(param, "/games.ps3"))
	{
		// /games.ps3
		// /index.ps3?mobile
		// /dev_hdd0/xmlhost/game_plugin/mobile.html

 mobile_response:
		mobile_mode = true; const char *params = param + 10;

		if(not_exists(MOBILE_HTML))
			{concat2(param, "/index.ps3", params); mobile_mode = false;}
		else if(strstr(param, "?g="))
			strcopy(param, MOBILE_HTML);
		else if(strchr(param, '?'))
			concat2(param, "/index.ps3", params);
		else if(not_exists(GAMELIST_JS))
			concat2(param, "/index.ps3", "?mobile");
		else
			strcopy(param, MOBILE_HTML);
	}
	else mobile_mode = false;
