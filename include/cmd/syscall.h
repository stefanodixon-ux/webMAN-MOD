	if(islike(param, "/syscall.ps3?") || islike(param, "/syscall_ps3"))
	{
		// /syscall.ps3?<syscall-number>|0x<hex-value>|<decimal-value>|<string-value>
		// e.g. http://localhost/syscall.ps3?392|0x1004|0x07|0x036
		//      http://localhost/syscall.ps3?837|CELL_FS_UTILITY:HDD1|CELL_FS_FAT|/dev_hdd1|0|0|0|0
		//      http://localhost/syscall.ps3?838|/dev_hdd1|0|1

		char *p, *params = param + 13;
		u64 sp[9], ret = 0; u8 n;
		u16 sc = (u16)val(params); check_path_tags(params);

		u8 is_plain = islike(param, "/syscall_ps3");

		for(n = 0; n <= 8; n++)
		{
			sp[n] = 0, p = strchr(params, '|'); if(!p) break;
			params = p + 1, *p = NULL;
			sp[n] = (u64)val(params); if(!sp[n] && (*params != '0')) sp[n] = (u64)(u32)(params);
		}

		if(sc == 840)
		{	// FS_DISK_FREE
			char *disk_size = header + 0x80, *dev_name = header; strcpy(dev_name, params);
			ret = get_free_space(dev_name); free_size(dev_name, disk_size);
			sprintf(param, "<a href=%s>%s</a>: %s (%llu %s)", dev_name, dev_name, disk_size, ret, STR_BYTE);
		}
		else
		if(sc == 200 || sc == 904)
		{	// ccapi_get_process_mem || dex_get_process_mem
			if(sp[2] > MAX_LINE_LEN) sp[2] = MAX_LINE_LEN; memset(header, 0, sp[2]);
			{system_call_4(sc, sp[0], sp[1], sp[2], (uint64_t)(uint32_t)header); ret = p1;}

			for(int i = 0; i < (u16)sp[2]; i++) sprintf(param + (2 * i), "%02X", (u8)header[i]);
		}
		else
		{
			switch (n)
			{
				case 0: {system_call_0(sc); ret = p1;} break;
				case 1: {system_call_1(sc, sp[0]); ret = p1;} break;
				case 2: {system_call_2(sc, sp[0], sp[1]); ret = p1;} break;
				case 3: {system_call_3(sc, sp[0], sp[1], sp[2]); ret = p1;} break;
				case 4: {system_call_4(sc, sp[0], sp[1], sp[2], sp[3]); ret = p1;} break;
				case 5: {system_call_5(sc, sp[0], sp[1], sp[2], sp[3], sp[4]); ret = p1;} break;
				case 6: {system_call_6(sc, sp[0], sp[1], sp[2], sp[3], sp[4], sp[5]); ret = p1;} break;
				case 7: {system_call_7(sc, sp[0], sp[1], sp[2], sp[3], sp[4], sp[5], sp[6]); ret = p1;} break;
				case 8: {system_call_8(sc, sp[0], sp[1], sp[2], sp[3], sp[4], sp[5], sp[6], sp[7]); ret = p1;} break;
			}

			if(is_plain)
			{
				sprintf(param, "0x%x", ret); if(!ret) param[1] = 0;
			}
			else
				sprintf(param, "syscall%i(%i) => 0x%x", n, sc, ret);
		}

		keep_alive = http_response(conn_s, header, "/syscall.ps3", is_plain ? CODE_PLAIN_TEXT : CODE_HTTP_OK, param);
		goto exit_handleclient_www;
	}
