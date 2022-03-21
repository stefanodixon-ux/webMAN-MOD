#ifdef SECURE_FILE_ID // TO-DO: this feature is broken
	if(islike(param, "/secureid.ps3"))
	{
		hook_savedata_plugin();
		sprintf(param, "Save data plugin: %s", securfileid_hooked ? STR_ENABLED : STR_DISABLED);

		if(file_exists("/dev_hdd0/secureid.log") {sprintf(header, HTML_URL, "/dev_hdd0/secureid.log", "/dev_hdd0/secureid.log"); strcat(param, "<p>Download: "); strcat(param, header);}

		keep_alive = http_response(conn_s, header, param, CODE_HTTP_OK, param);
		goto exit_handleclient_www;
	}
#endif
