	if(islike(param, "/cpy.ps3") || islike(param, "/cut.ps3"))
	{
		// /cpy.ps3<path>  stores <path> in <cp_path> clipboard buffer for copy with /paste.ps3 (cp_mode = 1)
		// /cut.ps3<path>  stores <path> in <cp_path> clipboard buffer for move with /paste.ps3 (cp_mode = 2)

		cp_mode = islike(param, "/cut.ps3") ? CP_MODE_MOVE : CP_MODE_COPY;
		strncopy(cp_path, STD_PATH_LEN, param + 8);
		check_path_alias(cp_path);
		if(not_exists(cp_path) && !islike(cp_path, "/net"))
			cp_mode = CP_MODE_NONE;

		copy_path(param, cp_path);

		is_binary = FOLDER_LISTING;
		goto html_response;
	}
	else
	if(islike(param, "/paste.ps3"))
	{
		// /paste.ps3<path>  performs a copy or move of path stored in <cp_path clipboard> to <path> indicated in url

		if(file_exists(cp_path))
		{
			char *dest = header;
			strcpy(dest, param + 10); sprintf(param, "/copy.ps3%s|%s", dest, dest);
			is_binary = WEB_COMMAND;
			goto html_response;
		}
		else
			if(!mc) {keep_alive = http_response(conn_s, header, "/", CODE_GOBACK, HTML_REDIRECT_TO_BACK); goto exit_handleclient_www;}
	}
