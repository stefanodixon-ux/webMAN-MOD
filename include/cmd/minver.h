	if(islike(param, "/minver.ps3"))
	{
		u8 data[0x20];
		_memset(data, sizeof(data));

		int ret = GetApplicableVersion(data);
		if(ret)
			sprintf(param, "Applicable Version failed: %x\n", ret);
		else
			sprintf(param, "Minimum Downgrade: %x.%02x", data[1], data[3]);

		keep_alive = http_response(conn_s, header, param, CODE_HTTP_OK, param);
		goto exit_handleclient_www;
	}
