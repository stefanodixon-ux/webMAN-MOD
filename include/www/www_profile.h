#ifndef LITE_EDITION
	//-- select content profile
	if(strstr(param, ".ps3?"))
	{
		u8 uprofile = profile; char url[10];

		bool is_index_ps3 = islike(param, "/index.ps3?");

		if( is_index_ps3 || islike(param, "/refresh.ps3") ) {char mode, *cover_mode = strstr(param, "?cover="); if(cover_mode) {custom_icon = true; mode = LCASE(*(cover_mode + 7)), *cover_mode = NULL; webman_config->nocov = (mode == 'o') ? ONLINE_COVERS : (mode == 'd' || mode == 'n') ? SHOW_DISC : (mode == 'i') ? SHOW_ICON0 : SHOW_MMCOVERS;}}

		for(u8 i = 0; i <= 5; i++)
		{
			sprintf(url, "?%i",    i); if(strstr(param, url)) {profile = i; break;}
			sprintf(url, "usr=%i", i); if(strstr(param, url)) {profile = i; break;}

			if(is_index_ps3) {sprintf(url, "_%i", i); if(strstr(param, url)) {profile = i; break;}}
		}

		if (uprofile != profile) {webman_config->profile = profile; save_settings();}
		if((uprofile != profile) || is_index_ps3) {DELETE_CACHED_GAMES}
	}
	//--
#endif
