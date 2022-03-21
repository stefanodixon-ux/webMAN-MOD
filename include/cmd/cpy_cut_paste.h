	if(islike(param, "/cpy.ps3") || islike(param, "/cut.ps3"))
	{
		// /cpy.ps3<path>  stores <path> in <cp_path> clipboard buffer for copy with /paste.ps3 (cp_mode = 1)
		// /cut.ps3<path>  stores <path> in <cp_path> clipboard buffer for move with /paste.ps3 (cp_mode = 2)

		cp_mode = islike(param, "/cut.ps3") ? CP_MODE_MOVE : CP_MODE_COPY;
		snprintf(cp_path, STD_PATH_LEN, "%s", param + 8);
		check_path_alias(cp_path);
		if(not_exists(cp_path)) cp_mode = CP_MODE_NONE;

		strcpy(param, cp_path); remove_filename(param);

		is_binary = FOLDER_LISTING;
		goto html_response;
	}
	else
	if(islike(param, "/paste.ps3"))
	{
		// /paste.ps3<path>  performs a copy or move of path stored in <cp_path clipboard> to <path> indicated in url

		#define PASTE_CMD	10

		char *source = header, *dest = cp_path;
		if(file_exists(cp_path))
		{
			sprintf(source, "/copy.ps3%s", cp_path);
			strcpy(dest, param + PASTE_CMD);
			strcpy(param, source); strcat(dest, get_filename(param));
			is_binary = WEB_COMMAND;
			goto html_response;
		}
		else
			if(!mc) {keep_alive = http_response(conn_s, header, "/", CODE_GOBACK, HTML_REDIRECT_TO_BACK); goto exit_handleclient_www;}
	}
