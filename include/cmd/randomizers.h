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
	else {
	// /wallpaper.ps3	[0]
	// /earth.ps3		[1]
	// /canyon.ps3		[2]
	// /lines.ps3		[3]
	// /coldboot.ps3	[4]
	// /theme.ps3		[5]
	// /impose.ps3		[7]
	// /psn_icons.ps3	[8]
	// /clock.ps3		[9]
	for(u8 res_id = 0; res_id < 10; res_id++)
		if(islike(param, vsh_res_action[res_id]))
		{
			// /wallpaper.ps3?random
			// /wallpaper.ps3           show selected
			// /wallpaper.ps3?next      select next id
			// /wallpaper.ps3?prev      select prev id
			// /wallpaper.ps3?<id>      set id 1-254 (.png)
			// /wallpaper.ps3?disable   use dev_flash (id=255)

			if(!cobra_version || syscalls_removed) goto exit_nocobra_error;

			char *value = strstr(param, ".ps3") + 4; if(*value) ++value;

			if(*value == 'r') {map_vsh_resource(res_id, 0, param, true); *value = 0;} // call vsh_random_res.h

			if(*value)
			{
				u8 id = (u8)val(value);
				if(*value == 'n') id = ++(webman_config->resource_id[res_id]);	// ?next
				if(*value == 'p') id = --(webman_config->resource_id[res_id]);	// ?prev
				if(*value == 'd') id = DEFAULT_RES;								// ?default or ?disable
				res_id = map_vsh_resource(res_id, id, param, true);				// set resource (or query if id == 0)
			}
			else
				res_id = map_vsh_resource(res_id, MAP_SELECTED, param, false);	// random resource

			if(!res_id)
				strcopy(header, "Random");
			else
				strcopy(header, "Fixed");

			// create url ?next
			sprintf(html_base_path, "%s?next", vsh_res_action[res_id]);

			keep_alive = http_response(conn_s, header, param, CODE_PREVIEW_FILE, param);
			goto exit_handleclient_www;
		}
	}
#endif // #ifdef VISUALIZERS
