	if(islike(param, "/dev_hdd1")) mount_device("/dev_hdd1", NULL, NULL); // auto-mount /dev_hdd1

	struct CellFsStat buf; bool is_net = false;

	#ifdef USE_NTFS
	is_ntfs = is_ntfs_path(param);
	#endif

	#ifdef COBRA_ONLY
	if(islike(param, "/net") && (param[4] >= '0' && param[4] <= '4')) //net0/net1/net2/net3/net4
	{
		is_binary = FOLDER_LISTING, is_net = true;
	}
	else
	#endif
	#ifdef USE_NTFS
	if(is_ntfs)
	{
		if(mountCount == NTFS_UNMOUNTED) mount_all_ntfs_volumes();

		char *sort = strstr(param, "?sort=");
		if(sort) {sort_by = sort[6]; if(strstr(sort, "desc")) sort_order = -1; *sort = NULL;}

		char *npath = (char*)ntfs_path(param); // ntfs0:/
		if(npath[6] != '/') {npath[6] = '/', npath[7] = 0;}

		struct stat bufn;
		if(ps3ntfs_stat(ntfs_path(param), &bufn) < 0)
		{
			keep_alive = http_response(conn_s, header, param, CODE_PATH_NOT_FOUND, "404 Path not found");
			goto exit_handleclient_www;
		}

		buf.st_size = bufn.st_size;
		buf.st_mode = bufn.st_mode;

		if(bufn.st_mode & S_IFDIR) is_binary = FOLDER_LISTING;
	}
	else
	#endif
		is_binary = (*param == '/') && (cellFsStat(param, &buf) == CELL_FS_SUCCEEDED);

	if(is_binary == BINARY_FILE) ;

	else if(*param == '/')
	{
		char *sort = strstr(param, "?sort=");
		if(sort) {sort_by = sort[6]; if(strstr(sort, "desc")) sort_order = -1; *sort = NULL;}

		sort = strchr(param, '?');
		if(sort)
		{
			file_query = sort + 1;
			*sort = NULL;
		}

		if(is_net || not_exists(param))
		{
			sort = strrchr(param, '#');
			if(sort) *sort = NULL;
		}

		if(is_net) goto html_response;

		if(islike(param, "/favicon.ico")) {sprintf(param, "%s", wm_icons[iPS3]);}
		else check_path_alias(param);

		is_binary = is_ntfs || (cellFsStat(param, &buf) == CELL_FS_SUCCEEDED); allow_retry_response = true;
	}

	if(is_binary)
	{
		c_len = buf.st_size;
		if(buf.st_mode & S_IFDIR) is_binary = FOLDER_LISTING; // folder listing
	}
	#ifdef COPY_PS3
	else if(allow_retry_response && islike(param, "/dev_") && strchr(param, '*') != NULL)
	{
		bool reply_html = !strstr(param, "//");
		char *FILE_LIST = reply_html ? (char*)FILE_LIST_HTM : (char*)FILE_LIST_TXT;
		cellFsUnlink(FILE_LIST);
		if(reply_html)
		{
			#ifndef EMBED_JS
			sprintf(header, SCRIPT_SRC_FMT, FS_SCRIPT_JS);
			save_file(FILE_LIST, HTML_HEADER, SAVE_ALL);
			save_file(FILE_LIST, header, APPEND_TEXT);
			save_file(FILE_LIST, "<body onload='try{t2lnks()}finally{}' bgcolor=#333 text=white vlink=white link=white><pre>", APPEND_TEXT);
			#else
			save_file(FILE_LIST, HTML_HEADER, SAVE_ALL);
			save_file(FILE_LIST, "<body bgcolor=#333 text=white vlink=white link=white><pre>", APPEND_TEXT);
			#endif
		}

		Check_Overlay();

		char *wildcard = strchr(param, '*');
		if(wildcard)
		{
			wildcard = strrchr(param, '/'); *wildcard++ = NULL;
		}
		scan(param, true, wildcard, (reply_html ? SCAN_LIST_SIZE : SCAN_LIST), FILE_LIST);

		strcpy(param, FILE_LIST);
		is_busy = false, allow_retry_response = false;
		goto html_listing;
	}
	#endif
	else
	{
		int code =  is_busy ?				 CODE_SERVER_BUSY :
					strstr(param, ".ps3" ) ? CODE_NOT_IMPLEMENTED :
					islike(param, "/dev_") ? CODE_PATH_NOT_FOUND :
											 CODE_BAD_REQUEST;

		http_response(conn_s, header, param, code, is_busy ? "503 Server is Busy" :
							 (code == CODE_NOT_IMPLEMENTED)? param : //"501 Not implemented" :
							 (code == CODE_PATH_NOT_FOUND) ? "404 Path not found" :
															 "400 Bad Request");

		keep_alive = 0;
		goto exit_handleclient_www;
	}
