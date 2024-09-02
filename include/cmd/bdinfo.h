#ifdef BDINFO
	if(islike(param, "/bdinfo.ps3"))
	{
		// /bdinfo.ps3
		// /bdinfo.ps3/<path>

		char *filename = param + 11, *buffer = header;

		get_bdvd_info(filename, buffer);

		keep_alive = http_response(conn_s, param, "/bdinfo.ps3", CODE_HTTP_OK, buffer);

		goto exit_handleclient_www;
	}
#endif
