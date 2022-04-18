	if(islike(param, "/stat.ps3") || islike(param, "/chmod.ps3"))
	{
		// /chdmod.ps3<path>                  - change file permissions to 0777
		// /chdmod.ps3<path>&mode=<mode>      - change file permissions to specific mode
		// /stat.ps3<path>                    - count files & folder size
		// /stat.ps3<path>&id=<title_id>      - link identical files in /hdd0/game using MD5
		// /stat.ps3<path>&id=<title_id>?fast - don't check MD5

		do_chmod = islike(param, "/chmod.ps3");

		char *buffer = header;
		char *path = param + (do_chmod ? 10 : 9);

		check_path_alias(path);

		if(do_chmod)
		{
			char *pos = strstr(path, "&mode="); if(pos) {*pos = NULL; new_mode = oct(pos + 6);}
		}

		check_md5 = !get_flag(path, "?fast");
		stitle_id = strstr(path, "&id="); if(stitle_id) {*stitle_id = NULL, stitle_id += 4;}

		if(!islike(path, INT_HDD_ROOT_PATH)) stitle_id = NULL;

		if(stitle_id) cellFsUnlink(FILE_LIST_TXT);

		*buffer = NULL; add_breadcrumb_trail(buffer, path);

		dir_count = file_count = 0;
		u64 dir_size = folder_size(path);

		sprintf(param, "%s<p>"
						"Size: %llu (%i.%i MB)<br>"
						"Dir(s): %llu<br>"
						"File(s): %llu", buffer, dir_size, dir_size>>20, ((10 * (((u32)dir_size) % _1MB_)) + _512KB_) / _1MB_, dir_count, file_count);

		keep_alive = http_response(conn_s, header, "/stat.ps3", CODE_HTTP_OK, param);

		new_mode = 0777;

		goto exit_handleclient_www;
	}
