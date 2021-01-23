	if(is_binary == BINARY_FILE) // binary file
	{
		if(keep_alive)
		{
			header_len += sprintf(header + header_len,  "Keep-Alive: timeout=3,max=250\r\n"
														"Connection: keep-alive\r\n");
		}

		header_len += sprintf(header + header_len, "Content-Length: %llu\r\n\r\n", (unsigned long long)c_len);
		send(conn_s, header, header_len, 0);

		size_t buffer_size = 0; if(sysmem) sys_memory_free(sysmem); sysmem = NULL;

		for(u8 n = MAX_PAGES; n > 0; n--)
			if(c_len >= ((n-1) * _64KB_) && sys_memory_allocate(n * _64KB_, SYS_MEMORY_PAGE_SIZE_64K, &sysmem) == CELL_OK) {buffer_size = n * _64KB_; break;}

		//if(!sysmem && sys_memory_allocate(_64KB_, SYS_MEMORY_PAGE_SIZE_64K, &sysmem)!=0)
		if(buffer_size < _64KB_)
		{
			keep_alive = http_response(conn_s, header, param, CODE_SERVER_BUSY, STR_ERROR);
			goto exit_handleclient_www;
		}

		if(islike(param, "/dev_bdvd"))
			sysLv2FsBdDecrypt(); // decrypt dev_bdvd files

		char *buffer = (char*)sysmem;
		int fd;
		#ifdef USE_NTFS
		if(is_ntfs)
		{
			fd = ps3ntfs_open(param + 5, O_RDONLY, 0);
			if(fd <= 0) is_ntfs = false;
		}
		#endif
		if(is_ntfs || cellFsOpen(param, CELL_FS_O_RDONLY, &fd, NULL, 0) == CELL_FS_SUCCEEDED)
		{
			u64 read_e = 0, pos;

			#ifdef USE_NTFS
			if(is_ntfs) ps3ntfs_seek64(fd, 0, SEEK_SET);
			else
			#endif
			cellFsLseek(fd, 0, CELL_FS_SEEK_SET, &pos);

			while(working)
			{
				#ifdef USE_NTFS
				if(is_ntfs) read_e = ps3ntfs_read(fd, (void *)buffer, BUFFER_SIZE_FTP);
				#endif
				if(is_ntfs || cellFsRead(fd, (void *)buffer, buffer_size, &read_e) == CELL_FS_SUCCEEDED)
				{
					if(read_e > 0)
					{
						#ifdef UNLOCK_SAVEDATA
						if(webman_config->unlock_savedata) unlock_param_sfo(param, (unsigned char*)buffer, (u16)read_e);
						#endif
						if(send(conn_s, buffer, (size_t)read_e, 0) < 0) break;
					}
					else
						break;
				}
				else
					break;

				//sys_ppu_thread_usleep(1668);
			}
			#ifdef USE_NTFS
			if(is_ntfs) ps3ntfs_close(fd);
			else
			#endif
			cellFsClose(fd);
		}

		goto exit_handleclient_www;
	}
