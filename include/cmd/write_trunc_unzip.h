	if(islike(param, "/write.ps3") || islike(param, "/write_ps3") || islike(param, "/trunc.ps3") ||
	   islike(param, "/dozip.ps3") || islike(param, "/unzip.ps3"))
	{
		// /write.ps3<path>&t=<text>                   use | for line break (create text file)
		// /write_ps3<path>&t=<text>                   use | for line break (add text to file)
		// /write_ps3<path>&t=<text>&line=<num>        insert line(s) to text file in line position
		// /write.ps3<path>&t=<text>&line=<num>        replace line of text file in line position
		// /write.ps3<path>&pos=<offset>&t=<hex>       (patch file)
		// /write.ps3?f=<path>&t=<text>                use | for line break (create text file)
		// /write_ps3?f=<path>&t=<text>                use | for line break (add text to file)
		// /write_ps3?f=<path>&t=<text>&line=<num>     insert line(s) to text file in line position
		// /write.ps3?f=<path>&t=<text>&line=<num>     replace line of text file in line position
		// /write.ps3?f=<path>&t=<text>&find=<code>    insert <text> before <code> if <text> is not found
		// /write.ps3?f=<path>&pos=<offset>&t=<hex>    (patch file)
		// /write.ps3?f=<path>&pos=<offset>&b=<file>   (patch file with the content of a binary file)

		u64 offset = 0; u32 size = 0;
		char *filename = param + ((param[10] == '/') ? 10 : 13);
		char *pos = strstr(filename, "&t=");
		char *find = header; *find = 0;
		char *use_file = NULL;
		sys_addr_t sysmem2 = NULL;

		if(!pos)
		{
			use_file = pos = strstr(filename, "&b=");
		}

		if(pos)
		{
			*pos = NULL;
			char *data = pos + 3;

			if(use_file)
			{
				char *patch_file = data; pos = strchr(data, '&'); if(pos) {*pos = NULL, use_file = pos;}
				u32 buffer_size = (file_size(patch_file) < _64KB_) ? _64KB_ : _128KB_;
				sysmem2 = sys_mem_allocate(buffer_size);

				if(sysmem2)
				{
					data = (char*)sysmem2;
					size = read_file(patch_file, data, buffer_size, 0); // replace data with the content of the patch file
				}
				else
					goto exit_handleclient_www;
			}

			pos = strstr(filename, "&pos="); // /write.ps3?f=<path>&pos=<offset>&t=<hex> (this syntax allows include "&pos=" as text)
			if(pos)
				*pos = NULL; // truncate file name
			else
				pos = strstr(data, "&pos="); // /write.ps3?f=<path>&t=<hex>&pos=<offset> (original syntax)

			filepath_check(filename);

			if(pos)
			{
				*pos = NULL, pos += 5;

				//  get data offset
				offset = val(pos);

				if(use_file)
					write_file(filename, CELL_FS_O_CREAT | CELL_FS_O_WRONLY, data, offset, size, false);
				else
				{
					_memset(header, sizeof(header));

					//  write binary data
					if(isHEX(data))
						size = Hex2Bin(data, header);
					else
						size = strcopy(header, data);

					write_file(filename, CELL_FS_O_CREAT | CELL_FS_O_WRONLY, header, offset, size, false);

					*header = '\0';
				}
			}
			else
			{
				bool overwrite = islike(param, "/write.ps3");
				char *param2 = data;

				if(use_file)
				{
					// use &b= as param2 (restore & after file path)
					*use_file = '&', param2 = use_file;
				}
				else
				{
					// convert pipes to line breaks
					for(pos = param2; *pos; ++pos) if(!memcmp(pos, "||", 2)) memcpy(pos, "\r\n", 2);
					for(pos = param2; *pos; ++pos) if(*pos == '|') *pos = '\n';
					for(pos = param2; *pos; ++pos) if(*pos == '`') *pos = '\t';
				}

				pos = strstr(param2, "&line="); if(!pos) pos = strstr(filename, "&line=");

				if(!pos)
				{
					pos = strstr(param2, "&find="); if(!pos) pos = strstr(filename, "&find=");
					if(pos) strcpy(find, pos + 6);
				}

				if(pos)
				{
					*pos = NULL, pos += 6;

					// write or insert data at line number
					u32 buffer_size = (file_size(filename) < _64KB_) ? _64KB_ : _128KB_;
					sys_addr_t sysmem = sys_mem_allocate(buffer_size);

					if(sysmem)
					{
						strcat(data, "\r\n");
						int len = strlen(data);

						char *buffer = (char*)sysmem;
						size = read_file(filename, buffer, buffer_size, 0);

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
									prepend(pos, data, len);

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
							write_file(filename, CELL_FS_O_CREAT | CELL_FS_O_WRONLY | CELL_FS_O_APPEND, buffer, len + 1, size, false);
							//save_file(filename, buffer, -size);  // append rest of file
						}
						else if(size + len < buffer_size)
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
								size -= (w - c); buffer[size] = 0;
							}
							prepend(buffer + i, data, len);
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

			if(sysmem2)
				sys_memory_free(sysmem2);
		}
		else if(islike(param, "/trunc.ps3"))
		{
			// /trunc.ps3<path>
			// /trunc.ps3<path-pattern>

			char *wildcard = strchr(filename, '*');
			if(wildcard)
			{
				wildcard = strrchr(filename, '/'); *wildcard++ = NULL;
				scan(filename, true, wildcard, SCAN_TRUNCATE, NULL);
				strcpy(param, filename);
			}
			else if(isDir(filename))
			{
				scan(filename, true, NULL, SCAN_TRUNCATE, NULL);
				strcpy(param, filename);
			}
			else
			{
				create_file(filename);
				copy_path(param, filename);
			}
			goto redirect_url;
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
					strcopy(header, "/unzip.ps3");
				else
					strcopy(header, "/dozip.ps3");
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
