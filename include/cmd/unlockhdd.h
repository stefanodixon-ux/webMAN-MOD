	if(sys_admin && islike(param, "/unlockhdd.ps3"))
	{
		// /unlockhdd.ps3
		// /unlockhdd.ps3?enable
		// /unlockhdd.ps3?disable
		// /unlockhdd.ps3?<0-9>&opt=<0-1>
		u8 opt = strstr(param, "&opt=0") ? 0 : 1;
		u32 ret = hdd_unlock_space(param[15], opt);

		u64 siz = get_free_space(drives[0]); int dm = ((siz>>20) % KB) / 100;
		sprintf(param, "<a href=%s>%s</a>: %i.%i %s (%llu %s) &bull; min: %u%% / optim: %u",
						drives[0], drives[0],
						siz>>30, dm, STR_GBFREE,
						siz, STR_BYTE,
						(ret & 0xF), (ret>>8));

		keep_alive = http_response(conn_s, header, "/unlockhdd.ps3", CODE_RETURN_TO_ROOT, param);
		goto exit_handleclient_www;
	}
