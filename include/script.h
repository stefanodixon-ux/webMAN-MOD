// # remmark
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

// wait xmb
// wait <1-9 secs>
// wait <path>
// lwait <path>

// logfile <pah>
// log <text>
// popup <text>
// beep1 / beep2 / beep3
// mute coldboot

// <webman_cmd> e.g. /mount.ps3<path>

#define EVENT_BOOT_INIT	1
#define EVENT_AUTOEXEC	2
#define	EVENT_ON_XMB	3

#ifdef COPY_PS3

#define line	buffer	/* "line", "path" and "buffer" are synonyms */
#define path	buffer	/* "line", "path" and "buffer" are synonyms */
#define IS_WEB_COMMAND(line)	(islike(line, "/mount") || (strstr(line, ".ps3") != NULL) || (strstr(line, "_ps3") != NULL))

static void parse_script(const char *script_file)
{
	if(script_running) return;

	if(file_exists(script_file))
	{
		u32 max_size = _64KB_;
		sys_addr_t sysmem = NULL;
		if(sys_memory_allocate(max_size, SYS_MEMORY_PAGE_SIZE_64K, &sysmem)) return;

		char *buffer = (char*)sysmem, *cr, *pos, *dest = NULL; u16 l = 0;
		u8 exec_mode = true, enable_db = true, do_else = true;
		size_t buffer_size = read_file(script_file, buffer, max_size, 0); buffer[buffer_size] = 0;
		char log_file[STD_PATH_LEN] = SC_LOG_FILE;

		script_running = true;

		while(*buffer)
		{
			parse_cmd:
			if(++l > 9999 || !working) break;

			while( *buffer && (*buffer <= ' ')) buffer++; // skip blank chars \n \r \t

			if(*buffer == 0) break;

			// process line
			pos = strstr(line, "\n");
			if(pos)
			{
				if(pos) *pos = NULL; //EOL
				cr = strstr(line, "\r"); if(cr) *cr = NULL;

				if(exec_mode)
				{
					dest = strstr(line, "=/");
					if(!dest) dest = strstr(line, ",/");
					if(!dest) dest = strstr(line, ", /");
					if(enable_db && (strstr(line, "/dev_blind") != NULL)) {enable_dev_blind(NO_MSG); enable_db = false;}
				}
				if(dest)
				{
					*dest++ = NULL; if(*dest == ' ') dest++; //split parameters
					char *wildcard = strchr(line, '*');
	#ifdef COBRA_ONLY
					if(_islike(line, "map /"))  {line += 4;}
					if(*line == '/') {if(IS_WEB_COMMAND(line)) handle_file_request(line); else if(IS(path, "/app_home")) set_app_home(path); else sys_map_path(path, dest);} else
	#else
					if(*line == '/') {if(IS_WEB_COMMAND(line)) handle_file_request(line);} else
	#endif
					if(_islike(line, "ren /"))  {path += 4; if(wildcard) {*wildcard++ = NULL;  scan(path, true, wildcard, SCAN_RENAME, dest);} else cellFsRename(path, dest);} else
					if(_islike(line, "copy /")) {path += 5; if(wildcard) {*wildcard++ = NULL;  scan(path, true, wildcard, SCAN_COPY, dest);} else if(isDir(path)) folder_copy(path, dest); else _file_copy(path, dest);} else
					if(_islike(line, "swap /")) {path += 5; sprintf(cp_path, "%s", path); char *slash = get_filename(cp_path); sprintf(slash, "/~swap"); cellFsRename(path, cp_path); cellFsRename(dest, path); cellFsRename(cp_path, dest);} else
					if(_islike(line, "move /")) {path += 5; if(wildcard) {*wildcard++ = NULL;  scan(path, true, wildcard, SCAN_MOVE, dest);} else cellFsRename(path, dest);} else
					if(_islike(line, "list /")) {path += 5; if(wildcard) {*wildcard++ = NULL;} scan(path, true, wildcard, SCAN_LIST, dest);} else
					if(_islike(line, "cpbk /")) {path += 5; if(wildcard) {*wildcard++ = NULL;} scan(path, true, wildcard, SCAN_COPYBK, dest);}
				}
				else if(*line == '#' || *line == ';' || *line == '*') ; // remark
				else if(_islike(line, "else") && do_else) {if(exec_mode) do_else = false; exec_mode ^= 1; line += 4; if(exec_mode && *line) goto parse_cmd;}
				else if(_islike(line, "end")) {exec_mode = do_else = true;}
				else if(exec_mode)
				{
					if(*line == '/')               {if(IS_WEB_COMMAND(line)) handle_file_request(line);} else
					if(_islike(line, "del /"))     {path += 4; char *wildcard = strchr(path, '*'); if(wildcard) {*wildcard++ = NULL; scan(path, true, wildcard, SCAN_DELETE, NULL);} else del(path, RECURSIVE_DELETE);} else
					if(_islike(line, "md /"))      {path += 3; mkdir_tree(path);} else
					if(_islike(line, "wait xmb"))  {wait_for_xmb();} else
					if(_islike(line, "wait /"))    {path += 5; wait_for(path, 5);} else
					if(_islike(line, "wait "))     {line += 5; wait_path("/dev_hdd0", (u8)val(line), false);} else
					if(_islike(line, "lwait /"))   {path += 6; wait_for(path, 10);} else
					#ifdef PS3MAPI
					if(_islike(line, "beep"))      {play_sound_id((u8)(line[4]));} else
					#else
					if(_islike(line, "beep"))      {if(line[4] == '3') {BEEP3;} else if(line[4] == '2') {BEEP2;} else {BEEP1;}} else
					#endif
					if(_islike(line, "popup "))    {line += 6; show_msg(line);} else
					if(_islike(line, "log "))      {line += 4; save_file(log_file, line, APPEND_TEXT);} else
					if(_islike(line, "logfile /")) {path += 8; snprintf(log_file, STD_PATH_LEN, "%s", path);} else
	#ifdef UNLOCK_SAVEDATA
					if(_islike(line, "unlock /"))  {path += 7; scan(path, true, "/PARAM.SFO", SCAN_UNLOCK_SAVE, NULL);} else
	#endif
	#ifdef COBRA_ONLY
					if(_islike(line, "unmap /"))   {path += 6; sys_map_path(path, NULL);} else
					if(_islike(line, "mute coldboot"))
					{
						sys_map_path(VSH_RESOURCE_DIR "coldboot_stereo.ac3", SYSMAP_EMPTY_DIR);
						sys_map_path(VSH_RESOURCE_DIR "coldboot_multi.ac3", SYSMAP_EMPTY_DIR);
					}
					else
	#endif
					if(_islike(line, "if ") || _islike(line, "abort if "))
					{
						#define ABORT_IF	9

						bool ret = false; u8 ifmode = (_islike(line, "if ")) ? 3 : ABORT_IF; line += ifmode; do_else = true;

						if(_islike(line, "exist /"))     {path +=  6; ret = file_exists(path);} else
						if(_islike(line, "not exist /")) {path += 10; ret = not_exists(path);}
						else
						{
							pad_data = pad_read(); // check for hold L1 or R1 in script
							if(pad_data.len > 0)
							{
								if(_islike(line, "L1")) {ret = (pad_data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_L1);}
								if(_islike(line, "R1")) {ret = (pad_data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_R1);}
							}
						}

						if(ifmode == ABORT_IF) {if(ret) break;} else if(!ret) exec_mode = false;
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

static void script_thread(u64 event_id)
{
	if(event_id == 1)
		parse_script("/dev_hdd0/boot_init.txt");
	else if(event_id == 2)
		parse_script("/dev_hdd0/autoexec.bat");
	else if(file_exists("/dev_hdd0/onxmb.bat"))
	{
		wait_for_xmb();
		sys_ppu_thread_sleep(3);
		parse_script("/dev_hdd0/onxmb.bat");
	}

	sys_ppu_thread_exit(0);
}

static void start_event(u8 id)
{
	sys_ppu_thread_t t_id;
	sys_ppu_thread_create(&t_id, script_thread, id, THREAD_PRIO, THREAD_STACK_SIZE_WEB_CLIENT, SYS_PPU_THREAD_CREATE_JOINABLE, THREAD_NAME_CMD);
}

#undef line
#undef path
#undef IS_WEB_COMMAND

#else
#define start_event(a)
#endif // #ifdef COPY_PS3
