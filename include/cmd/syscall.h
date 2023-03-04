	if(islike(param, "/syscall.ps3?") || islike(param, "/syscall_ps3"))
	{
		// /syscall.ps3?<syscall-number>|0x<hex-value>|<decimal-value>|<string-value>
		// e.g. http://localhost/syscall.ps3?392|0x1004|0x07|0x036
		//      http://localhost/syscall.ps3?837|CELL_FS_UTILITY:HDD1|CELL_FS_FAT|/dev_hdd1|0|0|0|0
		//      http://localhost/syscall.ps3?838|/dev_hdd1|0|1

		u8 is_plain = islike(param, "/syscall_ps3");

		char *params = param + 13; check_path_tags(params);

		u64 sp[9], ret = 0; u8 n;
		u16 sc = parse_syscall(params, sp, &n);

		if(sc == 840)
		{	// FS_DISK_FREE
			char *disk_size = header + 0x80, *dev_name = header; strcpy(dev_name, params);
			ret = get_free_space(dev_name); free_size(dev_name, disk_size);
			sprintf(param, "<a href=%s>%s</a>: %s (%llu %s)", dev_name, dev_name, disk_size, ret, STR_BYTE);
		}
		else
		if(/* sc == 200 || */ sc == 904)
		{	// ccapi_get_process_mem || dex_get_process_mem
			if(sp[2] > MAX_LINE_LEN) sp[2] = MAX_LINE_LEN; _memset(header, sp[2]);
			{system_call_4(sc, sp[0], sp[1], sp[2], (u64)(u32)header); ret = p1;}

			for(int i = 0; i < (u16)sp[2]; i++) sprintf(param + (2 * i), "%02X", (u8)header[i]);
		}
		else
		{
			ret = call_syscall(sc, sp, n);

			if(is_plain)
			{
				sprintf(param, "0x%llx", ret); if(!ret) param[1] = 0;
			}
			else
				sprintf(param, "syscall%i(%i) => 0x%llx", n, sc, ret);
		}

		keep_alive = http_response(conn_s, header, "/syscall.ps3", is_plain ? CODE_PLAIN_TEXT : CODE_HTTP_OK, param);
		goto exit_handleclient_www;
	}
