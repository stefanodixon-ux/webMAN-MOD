#ifdef CALC_MD5
	if(islike(param, "/md5.ps3"))
	{
		char *filename = param + 8, *buffer = header;

		check_path_alias(filename);

		sprintf(buffer, "File: ");
		add_breadcrumb_trail(buffer, filename);

		struct CellFsStat buf; cellFsStat(filename, &buf);
		unsigned long long sz = (unsigned long long)buf.st_size;

		char md5[33];
		calc_md5(filename, md5);

		sprintf(param, "%s<p>Size: %llu bytes<p>MD5: %s<p>", buffer, sz, md5);

		keep_alive = http_response(conn_s, header, "/md5.ps3", CODE_HTTP_OK, param);

		goto exit_handleclient_www;
	}
#endif
