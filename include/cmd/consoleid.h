#ifdef SPOOF_CONSOLEID
	bool is_psid = islike(param, "/psid.ps3");
	bool is_idps = islike(param, "/idps.ps3");

	if(islike(param, "/consoleid.ps3")) is_psid = is_idps = true;

	if(is_psid | is_idps)
	{
		// /idps.ps3        copy idps & act.dat (current user) to /dev_usb000 or /dev_hdd0
		// /idps.ps3<path>  copy idps to <path> (binary file)
		// /psid.ps3        copy psid & act.dat (current user) to /dev_usb000 or /dev_hdd0
		// /psid.ps3<path>  copy psid to <path> (binary file)
		// /consoleid.ps3   copy psid, idps & act.dat (current user) to /dev_usb000 or /dev_hdd0

		save_idps_psid(is_psid, is_idps, header, param);

		keep_alive = http_response(conn_s, header, param, CODE_HTTP_OK, param);
		goto exit_handleclient_www;
	}
#endif
