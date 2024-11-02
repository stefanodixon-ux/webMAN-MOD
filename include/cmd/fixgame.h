#ifdef FIX_GAME
	if(islike(param, "/fixgame.ps3"))
	{
		// /fixgame.ps3<path>  fix PARAM.SFO and EBOOT.BIN / SELF / SPRX in ISO or folder
		// /fixgame.ps3<param_sfo>&attrib=<value>

		char *game_path = param + 12, titleID[10];
		char *attrib = strstr(game_path, "&attrib=");
		u32 attribute = get_valuen32(game_path, "&attrib="); if(attrib) *attrib = NULL;

		check_path_alias(game_path);

		strcopy(header, STR_FIXING);

		if(file_exists(game_path))
			keep_alive = http_response(conn_s, header, param, CODE_PREVIEW_FILE, game_path);
		else
			keep_alive = http_response(conn_s, header, param, CODE_PATH_NOT_FOUND, "404 Path not found");

		if(attrib)
		{
			char sfo[_4KB_];
			u16 sfo_size = read_sfo(game_path, sfo);
			if(patch_param_sfo(game_path, (unsigned char *)sfo, sfo_size, attribute))
			{
				save_file(game_path, (void*)sfo, sfo_size);
			}
		}
		else
			fix_game(game_path, titleID, FIX_GAME_FORCED); // fix game folder

		show_msg_with_icon(ICON_GAME, "Fixed");
		goto exit_handleclient_www;
	}
#endif
