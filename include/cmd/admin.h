#ifdef SYS_ADMIN_MODE
	if(islike(param, "/admin.ps3"))
	{
		// /admin.ps3?enable&pwd=<password>  enable admin mode
		// /admin.ps3?disable                disable admin mode
		// /admin.ps3?0                      disable admin mode

		if(param[10] == 0 || param[11] == 0) ; else
		if(~param[11] & 1) sys_admin = 0; else
		{
			sys_admin = check_password(param);
		}

		sprintf(param, "ADMIN %s", sys_admin ? STR_ENABLED : STR_DISABLED);

		if(!mc) keep_alive = http_response(conn_s, header, param, CODE_RETURN_TO_ROOT, param);

		goto exit_handleclient_www;
	}
#endif
