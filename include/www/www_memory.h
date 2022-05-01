			u32 BUFFER_SIZE_HTML = _64KB_;

			if(show_info_popup || islike(param, "/cpursx.ps3")) is_cpursx = 1;
			else
			if((is_binary == FOLDER_LISTING) || islike(param, "/index.ps3") || islike(param, "/sman.ps3") || islike(param, "/gameplugin.ps3mapi"))
			{
							{BUFFER_SIZE_HTML  = _3MB_; sysmem = sys_mem_allocate(BUFFER_SIZE_HTML);}
				if(!sysmem) {BUFFER_SIZE_HTML  = _2MB_; sysmem = sys_mem_allocate(BUFFER_SIZE_HTML);}
				if(!sysmem)
				{
					BUFFER_SIZE_HTML = get_buffer_size(webman_config->foot);

					get_meminfo();

					if(meminfo.avail < (BUFFER_SIZE_HTML + MIN_MEM)) BUFFER_SIZE_HTML = get_buffer_size(3); //MIN+
					if(meminfo.avail < (BUFFER_SIZE_HTML + MIN_MEM)) BUFFER_SIZE_HTML = get_buffer_size(1); //MIN
				}
			}

			while((!sysmem) && (BUFFER_SIZE_HTML > 0) && sys_memory_allocate(BUFFER_SIZE_HTML, SYS_MEMORY_PAGE_SIZE_64K, &sysmem) != CELL_OK) {BUFFER_SIZE_HTML -= _64KB_; sys_ppu_thread_usleep(5000);}

			// one last try before give up
			if(!sysmem)
				{BUFFER_SIZE_HTML = _64KB_; sysmem = sys_mem_allocate(BUFFER_SIZE_HTML);}

			if(!sysmem)
			{
				keep_alive = http_response(conn_s, header, param, CODE_SERVER_BUSY, "503 Out of memory. Try again.");
				goto exit_handleclient_www;
			}

			char *buffer = (char*)sysmem;
			t_string sbuffer; _alloc(&sbuffer, buffer);
