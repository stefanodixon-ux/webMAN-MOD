	if(islike(param, "/netstatus.ps3"))
	{
		// /netstatus.ps3          toggle network access in registry
		// /netstatus.ps3?1        enable network access in registry
		// /netstatus.ps3?enable   enable network access in registry
		// /netstatus.ps3?0        disable network access in registry
		// /netstatus.ps3?disable  disable network access in registry

		// /netstatus.ps3?ftp      ftp is running?
		// /netstatus.ps3?netsrv   netsrv is running?
		// /netstatus.ps3?ps3mapi  ps3mapi is running?

		// /netstatus.ps3?stop-ftp      stop ftp server
		// /netstatus.ps3?stop-netsrv   stop net server
		// /netstatus.ps3?stop-ps3mapi  stop ps3mapi server
		// /netstatus.ps3?stop          stop ps3mapi+net+ftp servers

		// /netstatus.ps3?start-ftp      start ftp server
		// /netstatus.ps3?start-netsrv   start net server
		// /netstatus.ps3?start-ps3mapi  start ps3mapi server

		// /netstatus.ps3?reset-ftp      restart ftp server
		// /netstatus.ps3?reset-netsrv   restart net server
		// /netstatus.ps3?reset-ps3mapi  restart ps3mapi server

		s32 status = 0; char *label = NULL, *params = param + 15; xnet()->GetSettingNet_enable(&status);

		if(*params == 'f') {label = params, status = ftp_working;} else //ftp
		#ifdef PS3NET_SERVER
		if(*params == 'n') {label = params, status = net_working;} else //netsrv
		#endif
		#ifdef PS3MAPI
		if(*params == 'p') {label = params, status = ps3mapi_working;} else //ps3mapi
		#endif
		if(*params == 's' || *params == 'r') // start / stop / reset
		{
			if(params[2] == 'o' || *params == 'r')
			{
				char *service = params[4] == '-' ? params + 5 : NULL; // stop-***
				if( !params[4] || (*service == 'f')) {label = service, ftp_working = status = 0;} //ftp
				#ifdef PS3NET_SERVER
				if( !params[4] || (*service == 'n')) {label = service, net_working = status = 0;} //netsrv
				#endif
				#ifdef PS3MAPI
				if( !params[4] || (*service == 'p')) {label = service, ps3mapi_working = status = 0;} //ps3mapi
				#endif
				if(*params == 'r') sys_ppu_thread_sleep(2);
			}
			if(params[2] == 'a' || *params == 'r')
			{
				char *service = params[5] == '-' ? params + 6 : NULL; // start-***
				if(!ftp_working && (*service == 'f'))
					{label = service, status = 1; sys_ppu_thread_create(&thread_id_ftpd, ftpd_thread, NULL, THREAD_PRIO, THREAD_STACK_SIZE_FTP_SERVER, SYS_PPU_THREAD_CREATE_JOINABLE, THREAD_NAME_FTP);} // start ftp daemon immediately
				#ifdef PS3NET_SERVER
				if(!net_working && (*service == 'n'))
					{label = service, status = 1; sys_ppu_thread_create(&thread_id_netsvr, netsvrd_thread, NULL, THREAD_PRIO, THREAD_STACK_SIZE_NET_SERVER, SYS_PPU_THREAD_CREATE_JOINABLE, THREAD_NAME_NETSVR);}
				#endif
				#ifdef PS3MAPI
				if(!ps3mapi_working && (*service == 'p'))
					{label = service, status = 1; sys_ppu_thread_create(&thread_id_ps3mapi, ps3mapi_thread, NULL, THREAD_PRIO, THREAD_STACK_SIZE_PS3MAPI_SVR, SYS_PPU_THREAD_CREATE_JOINABLE, THREAD_NAME_PS3MAPI);}
				#endif
			}
		}
		else if(*params == 'i' && params[2] == '=') // /netstatus.ps3?ip=<address>
		{
			xnet()->SetSettingNet_ipAddress((address *)&params[3]);
		}
		else if(*params == 'm' && params[4] == '=') // /netstatus.ps3?mask=<ip-mask>
		{
			xnet()->SetSettingNet_ipAddress((address *)&params[5]);
		}
		else if(*params == 'r' && params[5] == '=') // /netstatus.ps3?route=<address>
		{
			xnet()->SetSettingNet_defaultRoute((address *)&params[6]);
		}
		else if(*params == 'd' && params[4] == '=') // /netstatus.ps3?dns1=<address>
		{
			if(params[3] == '2')
				xnet()->SetSettingNet_secondaryDns((address *)&params[5]);
			else
				xnet()->SetSettingNet_primaryDns((address *)&params[5]);
		}
		else
		{
			if(*params == 0) ; else // query status
			if(!param[14]) {status ^= 1; xnet()->SetSettingNet_enable(status);} else
			if( param[15] & 1) xnet()->SetSettingNet_enable(1); else //enable
			if(~param[15] & 1) xnet()->SetSettingNet_enable(0);      //disable

			xnet()->GetSettingNet_enable(&status);
		}

		sprintf(param, "%s : %s", label ? to_upper(label) : "Network", status ? STR_ENABLED : STR_DISABLED);

		#ifdef WM_REQUEST
		if(!wm_request)
		#endif
		{
			if(!mc) keep_alive = http_response(conn_s, header, "/netstatus.ps3", CODE_HTTP_OK, param);
		}

		show_msg(param);

		goto exit_handleclient_www;
	}
