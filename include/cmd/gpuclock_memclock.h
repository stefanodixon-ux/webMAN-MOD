#ifdef OVERCLOCKING
if(islike(param, "/gpuclock.ps3") || islike(param, "/memclock.ps3"))
{
	bool gpu = (param[1] == 'g');

	if(param[13] == '?')
	{
		u16 mhz = (u16)val(param + 14); // new gpu clock speed (300-1200)
		overclock(mhz, gpu);

		char *pos = strchr(param, '|');
		if(pos)
		{
			mhz = (u16)val(pos + 1); // new gpu clock speed (300-1200)
			overclock(mhz, !gpu);
		}
	}

	show_rsxclock(param);

	keep_alive = http_response(conn_s, header, "/gpuclock.ps3", CODE_HTTP_OK, param);

	goto exit_handleclient_www;
}
#endif
