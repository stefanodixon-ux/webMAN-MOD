#ifdef FULL_EDITION
	if(islike(param, "/err.ps3"))
	{
		char *params = param + 9, *buffer = header;

		int errnum = val(params);

		sprintf(buffer, "Error 0x%x: %s", errnum, strerror(errnum));

		keep_alive = http_response(conn_s, param, "/err.ps3", CODE_HTTP_OK, buffer);

		goto exit_handleclient_www;
	}
#endif
