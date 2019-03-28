// if/elseif/else/end
//   if exist <path>
//   if not exist <path>
//   if L1
//   if R1
//   abort if exist <path>
//   abort if not exist <path>

// map <path>=<path>
// unmap <path>
// md <path>
// del <path>
// copy <path>=<path>
// swap <path>=<path>
// ren <path>=<path>

// wait <1-9 secs>
// wait <path>
// lwait <path>

// logfile <pah>
// log <text>
// popup <text>
// beep1 / beep2 / beep3
// mute coldboot

// <webman_cmd> e.g. /mount.ps3<path>

#ifdef COPY_PS3
static void parse_script(const char *script_file)
{
	if(file_exists(script_file))
	{
		sys_addr_t sysmem = NULL;
		sys_memory_allocate(_64KB_, SYS_MEMORY_PAGE_SIZE_64K, &sysmem); 
		char *buffer = (char*)sysmem, *cr, *pos, *dest = NULL; u16 l = 0; u8 exec_mode = true, enable_db = true;
		size_t buffer_size = read_file(script_file, buffer, _64KB_, 0); buffer[buffer_size] = 0;
		char log_file[STD_PATH_LEN + 1] = TMP_DIR "/log.txt";

		script_running = true;

		while(*buffer)
		{
			parse_cmd:
			if(++l > 999 || !working) break;
			while( (*buffer == '\n') || (*buffer == '\r') || (*buffer == ' ') || (*buffer == '\t')) buffer++; if(*buffer == 0) break;

			pos = strstr(buffer, "\n");
			if(pos)
			{
				if(pos) *pos = NULL; //EOL
				cr = strstr(buffer, "\r"); if(cr) *cr = NULL;

				if(exec_mode) {dest = strstr(buffer, "=/"); if(enable_db && !strstr(buffer, "/dev_blind")) {enable_dev_blind(NO_MSG); enable_db = false;}}
				if(dest)
				{
					*dest = NULL, dest++; //split parameters
	#ifdef COBRA_ONLY
					if(_islike(buffer, "map /"))  {buffer += 4;}
		#ifndef WM_REQUEST
					if(*buffer == '/')            {sys_map_path(buffer, dest);} else
		#else
					if(*buffer == '/')            {if(islike(buffer, "/mount") || strstr(buffer, ".ps3") != NULL || strstr(buffer, "_ps3") != NULL) handle_file_request(buffer); else sys_map_path(buffer, dest);} else
		#endif
	#elif defined(WM_REQUEST)
					if(*buffer == '/')            {if(islike(buffer, "/mount") || strstr(buffer, ".ps3") != NULL || strstr(buffer, "_ps3") != NULL) handle_file_request(buffer);} else
	#endif
					if(_islike(buffer, "ren /"))  {buffer += 4; cellFsRename(buffer, dest);} else
					if(_islike(buffer, "copy /")) {buffer += 5; if(isDir(buffer)) folder_copy(buffer, dest); else file_copy(buffer, dest, COPY_WHOLE_FILE);} else
					if(_islike(buffer, "swap /")) {buffer += 5; sprintf(cp_path, "%s", buffer); char *slash = strrchr(cp_path, '/'); sprintf(slash, "/~swap"); cellFsRename(buffer, cp_path); cellFsRename(dest, buffer); cellFsRename(cp_path, dest);}
				}
				else if(*buffer == '#' || *buffer == ';' || *buffer == '*') ; // remark
				else if(_islike(buffer, "else")) {exec_mode ^= 1; buffer += 4; if(exec_mode && *buffer) goto parse_cmd;}
				else if(_islike(buffer, "end"))  exec_mode = true;
				else if(exec_mode)
				{
	#ifdef WM_REQUEST
					if(*buffer == '/')               {if(islike(buffer, "/mount") || strstr(buffer, ".ps3") != NULL || strstr(buffer, "_ps3") != NULL) handle_file_request(buffer);} else
	#endif
					if(_islike(buffer, "del /"))     {buffer += 4; del(buffer, RECURSIVE_DELETE);} else
					if(_islike(buffer, "md /"))      {buffer += 3; mkdir_tree(buffer);} else
					if(_islike(buffer, "wait /"))    {buffer += 5; wait_for(buffer, 5);} else
					if(_islike(buffer, "lwait /"))   {buffer += 6; wait_for(buffer, 10);} else
					if(_islike(buffer, "wait "))     {buffer += 5; wait_path("/dev_hdd0", (u8)(*buffer-'0'), false);} else
					if(_islike(buffer, "beep"))      {if(buffer[4] == '3') {BEEP3;} else if(buffer[4] == '2') {BEEP2;} else {BEEP1;}} else
					if(_islike(buffer, "popup "))    {buffer += 6; show_msg(buffer);} else
					if(_islike(buffer, "log "))      {buffer += 4; save_file(log_file, buffer, APPEND_TEXT); save_file(log_file, "\r\n", APPEND_TEXT);} else
					if(_islike(buffer, "logfile /")) {buffer += 8; sprintf(log_file, "%s", buffer);} else
	#ifdef COBRA_ONLY
					if(_islike(buffer, "unmap /"))   {buffer += 6; sys_map_path(buffer, NULL);} else
					if(_islike(buffer, "mute coldboot"))
					{
						sys_map_path((char*)(VSH_RESOURCE_DIR "coldboot_stereo.ac3"), SYSMAP_PS3_UPDATE);
						sys_map_path((char*)(VSH_RESOURCE_DIR "coldboot_multi.ac3"), SYSMAP_PS3_UPDATE);
					}
					else
	#endif
					if(_islike(buffer, "if ") || _islike(buffer, "abort if "))
					{
						bool ret = false; u8 ifmode = (_islike(buffer, "if ")) ? 3 : 9; buffer += ifmode;

						if(_islike(buffer, "exist /")) {buffer += 6; ret = file_exists(buffer);} else
						if(_islike(buffer, "not exist /")) {buffer += 10; ret = !file_exists(buffer);}
						else
						{
							CellPadData pad_data = pad_read();
							if(pad_data.len > 0)
							{
								if(_islike(buffer, "L1")) {ret = (pad_data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_L1);}
								if(_islike(buffer, "R1")) ret = (pad_data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_R1);
							}
						}

						if(ifmode == 9) {if(ret) break;} else if(!ret) exec_mode = false;
					}
				}

				buffer = pos + 1, dest = NULL;
			}
			else
				break;
		}
		sys_memory_free(sysmem);

		script_running = false;
	}
}
#endif // #ifdef COPY_PS3
