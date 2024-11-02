#ifdef SECURE_FILE_ID // TO-DO: this feature is broken
	if(islike(param, "/secureid.ps3"))
	{
		hook_savedata_plugin();
		concat_text(param, "Save data plugin:", securfileid_hooked ? STR_ENABLED : STR_DISABLED);

		if(file_exists("/dev_hdd0/secureid.log") concat2(param, "<p>Download: ", strfmt(HTML_URL, "/dev_hdd0/secureid.log", "/dev_hdd0/secureid.log")); 
 
		keep_alive = http_response(conn_s, header, param, CODE_HTTP_OK, param);
		goto exit_handleclient_www;
	}
#endif
