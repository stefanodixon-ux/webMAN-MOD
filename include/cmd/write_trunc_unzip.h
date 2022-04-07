	if(islike(param, "/write.ps3") || islike(param, "/write_ps3") || islike(param, "/trunc.ps3") ||
	   islike(param, "/dozip.ps3") || islike(param, "/unzip.ps3"))
	{
		// /write.ps3<path>&t=<text> use | for line break (create text file)
		// /write_ps3<path>&t=<text> use | for line break (add text to file)
		// /write_ps3<path>&t=<text>&line=<num> insert line(s) to text file in line position
		// /write.ps3<path>&t=<text>&line=<num> replace line of text file in line position
		// /write.ps3<path>&t=<hex>&pos=<offset>          (patch file)
		// /write.ps3?f=<path>&t=<text> use | for line break (create text file)
		// /write_ps3?f=<path>&t=<text> use | for line break (add text to file)
		// /write_ps3?f=<path>&t=<text>&line=<num> insert line(s) to text file in line position
		// /write.ps3?f=<path>&t=<text>&line=<num> replace line of text file in line position
		// /write.ps3?f=<path>&t=<text>&find=<code> insert <text> before <code> if <text> is not found
		// /write.ps3?f=<path>&t=<hex>&pos=<offset>       (patch file)

		u64 offset = 0; u32 size = 0;
		char *filename = param + ((param[10] == '/') ? 10 : 13);
		char *pos = strstr(filename, "&t=");
		char *find = header; *find = 0;

		if(pos)
		{
			*pos = NULL;
			char *data = pos + 3;
			filepath_check(filename);

			pos = strstr(data, "&pos=");

			if(pos)
			{
				*pos = NULL, pos += 5;

				//  get data offset
				offset = val(pos);

				//  write binary data
				if(isHEX(data))
					size = Hex2Bin(data, header);
				else
					size = sprintf(header, "%s", data);

				write_file(filename, CELL_FS_O_CREAT | CELL_FS_O_WRONLY, header, offset, size, false);
			}
			else
			{
				bool overwrite = islike(param, "/write.ps3");

				// convert pipes to line breaks
				for(pos = data; *pos; ++pos) if(!memcmp(pos, "||", 2)) memcpy(pos, "\r\n", 2);
				for(pos = data; *pos; ++pos) if(*pos == '|') *pos = '\n';
				for(pos = data; *pos; ++pos) if(*pos == '`') *pos = '\t';

				pos = strstr(data, "&line=");

				if(!pos)
				{
					pos = strstr(data, "&find=");
					if(pos) strcpy(find, pos + 6);
				}

				if(pos)
				{
					// write or insert data at line number
					sys_addr_t sysmem = NULL;
					if(sys_memory_allocate(_128KB_, SYS_MEMORY_PAGE_SIZE_64K, &sysmem) == CELL_OK)
					{
						*pos = NULL, pos += 6;

						strcat(data, "\n");
						u16 len = strlen(data);
						char *buffer = (char*)sysmem;
						size = read_file(filename, buffer, _128KB_, 0);

						u16 line = (u16)val(pos);

						if(*find) // find text
						{
							char *pos = strstr(buffer, data);
							if(!pos) // if buffer does not have the new data
							{
								pos = strstr(buffer, find);
								if(pos) // if the text is found
								{
									// insert data at found offset (pos)
									for(int i = strlen(pos); i >= 0; i--) pos[i + len] = pos[i];
									memcpy(pos, data, len);

									// save file
									save_file(filename, buffer, SAVE_ALL);
								}
							}
						}
						else if(line <= 1)
						{
							save_file(filename, data, SAVE_ALL); // write line data
							if(overwrite)
							{
								pos = strstr(buffer, "\n");
								if(pos) {size -= ++pos - buffer; buffer = pos;} // skip first line
							}
							write_file(filename, CELL_FS_O_APPEND | CELL_FS_O_CREAT | CELL_FS_O_WRONLY, buffer, len + 1, size, false);
							//save_file(filename, buffer, -size);  // append rest of file
						}
						else if(size + len < _64KB_)
						{
							u32 i, c = 0, w = 0; --line;
							// find offset of line to write
							for(i = 0; i < size; i++) if(buffer[i] == '\n' && (++c >= line)) {i++; break;}
							// skip line to overwrite
							if(overwrite)
							{
								// find end of current line
								for(w = i; w < size; w++) if(buffer[w] == '\n') {w++; break;}
								// remove current line
								for(c = i; c < size; c++, w++) buffer[c] = buffer[w];
								size -= (w - c);
							}
							// move forward rest of file
							for(c = size; c >= i; c--) buffer[c + len] = buffer[c];
							// set line data
							for(c = 0; c < len; c++) buffer[i + c] = data[c];
							// write file data
							save_file(filename, buffer, SAVE_ALL);
						}
						sys_memory_free(sysmem);
					}
				}
				else if(overwrite)
					save_file(filename, data, SAVE_ALL); // write.ps3 (write file)
				else
					save_file(filename, data, APPEND_TEXT); // write_ps3 (add line)
			}
		}
		else if(islike(param, "/trunc.ps3"))
		{
			// /trunc.ps3<path>
			// /trunc.ps3<path-pattern>

			char *wildcard = strchr(filename, '*');
			if(wildcard)
			{
				Check_Overlay();
				wildcard = strrchr(filename, '/'); *wildcard++ = NULL;
				scan(filename, true, wildcard, SCAN_TRUNCATE, NULL);
			}
			else if(isDir(filename))
			{
				Check_Overlay();
				scan(filename, true, NULL, SCAN_TRUNCATE, NULL);
			}
			else
				save_file(filename, "", SAVE_ALL);
		}
		#ifdef COBRA_ONLY
		else if(islike(param, "/dozip.ps3") || islike(param, "/unzip.ps3"))
		{
			// /dozip.ps3<path>
			// /dozip.ps3<path>&to=<dest-path>
			// /unzip.ps3<zip-file>&to=<dest-path>

			if(!cobra_version || syscalls_removed) goto exit_nocobra_error;

			if(IS_ON_XMB)
			{
				bool is_unzip = (param[1] == 'u');
				char *launch_txt = header;
				char *dest = strstr(filename, "&to=");

				if(dest) {*dest = NULL, dest+=4;}

				if(is_unzip || dest)
				{
					if(is_unzip)
					{
						if(dest)
							sprintf(launch_txt, "%s\n%s/", filename, dest);
						else
						{
							int len = sprintf(launch_txt, "%s\n%s", filename, filename);
							sprintf(get_filename(launch_txt), "/");
							mkdir_tree(launch_txt + (len / 2));
						}
					}
					else
						sprintf(launch_txt, "%s\n%s%s.zip", filename, dest, get_filename(filename));
				}
				else
				#ifdef USE_NTFS
				if(islike(filename, "/dev_flash") || is_ntfs_path(filename))
					sprintf(launch_txt, "%s\n%s%s.zip", filename, drives[0], get_filename(filename));
				else
					sprintf(launch_txt, "%s\n%s%s.zip", filename, "", filename);
				#else
				if(islike(filename, "/dev_flash"))
					sprintf(launch_txt, "%s\n%s%s.zip", filename, drives[0], get_filename(filename));
				else
					sprintf(launch_txt, "%s\n%s%s.zip", filename, "", filename);
				#endif

				do_umount(false);
				cobra_map_game(PKGLAUNCH_DIR, PKGLAUNCH_ID, true);
				save_file(PKGLAUNCH_DIR "/USRDIR/launch.txt", launch_txt, SAVE_ALL);

				launch_app_home_icon(true);

				if(is_unzip)
					sprintf(header, "/unzip.ps3");
				else
					sprintf(header, "/dozip.ps3");
			}
			else
			{
				keep_alive = http_response(conn_s, header, param, CODE_SERVER_BUSY, "ERROR: Not in XMB!");
				goto exit_handleclient_www;
			}
		}
		#endif

		header[10] = '\0';

		keep_alive = http_response(conn_s, header, param, CODE_PREVIEW_FILE, filename);
		goto exit_handleclient_www;
	}
