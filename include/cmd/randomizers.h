#ifdef VISUALIZERS
	#ifdef PATCH_GAMEBOOT
	else if(islike(param, "/gameboot.ps3"))
	{
		// /gameboot.ps3?<id>      set gameboot <0-19>

		if(!cobra_version || syscalls_removed) goto exit_nocobra_error;

		patch_gameboot(val(param + 14)); *header = NULL;

		if(!mc) keep_alive = http_response(conn_s, header, param, CODE_PREVIEW_FILE, param);
		goto exit_handleclient_www;
	}
	#endif
	else if(islike(param, "/wallpaper.ps3") ||
			islike(param, "/earth.ps3")     ||
			islike(param, "/canyon.ps3")    ||
			islike(param, "/lines.ps3")     ||
			islike(param, "/theme.ps3")     ||
			islike(param, "/impose.ps3")    ||
			islike(param, "/clock.ps3")     ||
			islike(param, "/psn_icons.ps3") ||
			islike(param, "/coldboot.ps3"))
	{
		// /wallpaper.ps3?random
		// /wallpaper.ps3           show selected
		// /wallpaper.ps3?next      select next id
		// /wallpaper.ps3?prev      select prev id
		// /wallpaper.ps3?<id>      set id 1-254 (.png)
		// /wallpaper.ps3?disable   use dev_flash (id=255)

		if(!cobra_version || syscalls_removed) goto exit_nocobra_error;

		u8 id;
		u8 res_id = (param[1] == 'w') ? 0: // 0 = wallpaper
					(param[1] == 'e') ? 1: // 1 = earth
					(param[4] == 'y') ? 2: // 2 = canyon
					(param[1] == 'l') ? 3: // 3 = lines
					(param[5] == 'b') ? 4: // 4 = coldboot
					(param[1] == 'i') ? 7: // 7 = impose
					(param[1] == 'p') ? 8: // 8 = psn_icons
					(param[5] == 'k') ? 9: // 9 = clock
										5; // 5 = theme (6 = last selected theme)

		char *value = strstr(param, ".ps3") + 4;

		// create url ?next
		{id = *value; *value = 0; sprintf(html_base_path, "%s?next", param); *value = id;}

		if(*value) ++value;

		if(*value == 'r') {map_vsh_resource(res_id, 0, param, true); *value = 0;} // call vsh_random_res.h

		if(*value)
		{
			id = (u8)val(value);
			if(*value == 'n') id = ++(webman_config->resource_id[res_id]);	// ?next
			if(*value == 'p') id = --(webman_config->resource_id[res_id]);	// ?prev
			if(*value == 'd') id = DEFAULT_RES;								// ?default or ?disable
			res_id = map_vsh_resource(res_id, id, param, true);				// set resource (or query if id == 0)
		}
		else
			res_id = map_vsh_resource(res_id, MAP_SELECTED, param, false);	// random resource

		if(!res_id)
			sprintf(header, "Random");
		else
			sprintf(header, "Fixed");

		keep_alive = http_response(conn_s, header, param, CODE_PREVIEW_FILE, param);
		goto exit_handleclient_www;
	}
#endif // #ifdef VISUALIZERS
