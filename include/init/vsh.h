#define EXPLORE_CLOSE_ALL   3

static void * getNIDfunc(const char * vsh_module, u32 fnid, s32 offset)
{
	// 0x10000 = ELF
	// 0x10080 = segment 2 start
	// 0x10200 = code start

	u32 table = (*(u32*)0x1008C) + 0x984; // vsh table address

	while((u32)*(u32*)table)
	{
		u32 *export_stru_ptr = (u32*)*(u32*)table; // ptr to export stub, size 2C, "sys_io" usually... Exports:0000000000635BC0 stru_635BC0:    ExportStub_s <0x1C00, 1, 9, 0x39, 0, 0x2000000, aSys_io, ExportFNIDTable_sys_io, ExportStubTable_sys_io>

		const char *lib_name_ptr =  (const char*)*(u32*)((char*)export_stru_ptr + 0x10);

		if(strcmp(vsh_module, lib_name_ptr) == 0)
		{
			// we got the proper export struct
			u32 lib_fnid_ptr = *(u32*)((char*)export_stru_ptr + 0x14);
			u32 lib_func_ptr = *(u32*)((char*)export_stru_ptr + 0x18);
			u16 count = *(u16*)((char*)export_stru_ptr + 6); // number of exports
			for(int i = 0; i < count; i++)
			{
				if(fnid == *(u32*)((char*)lib_fnid_ptr + (i * 4)))
				{
					// take address from OPD
					return (void**)*((u32*)(lib_func_ptr) + i) + offset;
				}
			}
		}
		table += 4;
	}
	return 0;
}

static sys_memory_container_t get_vsh_memory_container(void)
{
	if(!webman_config->vsh_mc || IS_INGAME) return 0;
	return vsh_memory_container_by_id(webman_config->vsh_mc);
}

static sys_addr_t sys_mem_allocate(u32 bytes)
{
	sys_addr_t sysmem = NULL;
	sys_memory_container_t vsh_mc = get_vsh_memory_container();
	u32 flags = (bytes & _3MB_) ? SYS_MEMORY_PAGE_SIZE_1M : SYS_MEMORY_PAGE_SIZE_64K;
	if(!vsh_mc) vsh_mc = vsh_memory_container_by_id(4);
	if( vsh_mc) sys_memory_allocate_from_container(bytes, vsh_mc, flags, &sysmem);
	if(!sysmem) sys_memory_allocate(bytes, SYS_MEMORY_PAGE_SIZE_64K, &sysmem);
	return sysmem;
}

static explore_plugin_interface *get_explore_interface(void)
{
	int view = View_Find("explore_plugin");
	if(view)
		explore_interface = (explore_plugin_interface *)plugin_GetInterface(view, 1);
	else
		explore_interface = NULL;

	return explore_interface;
}

static int get_game_info(void)
{
	_game_TitleID[0] = _game_Title[0] = 0;

	if(IS_ON_XMB) return 0; // prevents game_plugin detection during PKG installation

	int is_ingame = View_Find("game_plugin");

	if(is_ingame)
	{
		char _game_info[0x120];
		game_interface = (game_plugin_interface *)plugin_GetInterface(is_ingame, 1);
		game_interface->gameInfo(_game_info);

		snprintf(_game_TitleID, 10, "%s", _game_info+0x04);
		snprintf(_game_Title,   64, "%s", _game_info+0x14);
	}

	return is_ingame;
}

static void wait_for_title_id(void)
{
	for(u8 retry = 0; retry < 10; retry++)
	{
		get_game_info();
		if(*_game_TitleID) break;
		sys_ppu_thread_sleep(1);
	}
}

static bool abort_autoplay(void)
{
	pad_data = pad_read(); // abort auto-play holding L2 or pressing arrow keys
	if( (pad_data.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & (CELL_PAD_CTRL_DOWN | CELL_PAD_CTRL_UP | CELL_PAD_CTRL_LEFT | CELL_PAD_CTRL_RIGHT)) ||
		(pad_data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & (CELL_PAD_CTRL_L2 | CELL_PAD_CTRL_CIRCLE)))
	{
		if((mount_unk == APP_GAME) || ((pad_data.button[CELL_PAD_BTN_OFFSET_DIGITAL1] == CELL_PAD_CTRL_START) && (pad_data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] == CELL_PAD_CTRL_L2))) return false;
		if(webman_config->autoplay && !webman_config->nobeep) play_rco_sound("snd_system_ng");
		return true;
	}
	return false;
}

static bool wait_for_abort(u32 usecs)
{
	if(IS_INGAME) return true;

	if(usecs < 50  ) usecs *= 1000000; // convert to microseconds
	if(usecs < 1000) usecs *= 1000;    // convert to milliseconds

	for(u32 s = 0; s <= usecs; s+=200000)
	{
		if(abort_autoplay()) return true; //200ms
	}

	return false;
}

static void exec_xmb_command(const char *cmd)
{
	explore_interface->ExecXMBcommand(cmd, 0, 0);
}

static void exec_xmb_command2(const char *cmd, const char *param)
{
	char explore_command[128]; // info: http://www.psdevwiki.com/ps3/explore_plugin
	sprintf(explore_command, cmd, param);
	exec_xmb_command(explore_command);
}

#ifdef PKG_HANDLER
static void unload_plugin_modules(bool all);
#endif
static void explore_close_all(const char *path)
{
	if(IS_INGAME) return;

#ifdef PKG_HANDLER
	unload_plugin_modules(false);
#endif

	if(get_explore_interface())
	{
		exec_xmb_command("close_all_list");
		if(strstr(path, "BDISO") || strstr(path, "DVDISO"))
			exec_xmb_command2("focus_category %s", "video");
		else
			exec_xmb_command2("focus_category %s", "game");
	}
}

static void focus_first_item(void)
{
	if(IS_ON_XMB)
	{
		exec_xmb_command("focus_index 0");
	}
}

static bool explore_exec_push(u32 usecs, u8 focus_first)
{
	if(IS_INGAME) return false;

	if(explore_interface)
	{
		if(wait_for_abort(usecs)) return false;

		if(focus_first)
		{
			focus_first_item();
		}

		if(abort_autoplay() || IS_INGAME) return false;

		if(focus_first)
		{
			exec_xmb_command("open_list nocheck");
			if(wait_for_abort(500)) return false;
			focus_first_item();
		}
		else
		{
			gTick.tick =  rTick.tick + 1; // notify in-game
			exec_xmb_command("exec_push");
		}

		return true;
	}
	return false;
}

static void exec_xmb_item(char *category, char *seg_name, bool execute)
{
	u8 n;

#ifdef COBRA_ONLY
	unload_vsh_gui();
#endif

	for(n = 0; n < 15; n++) {if(get_explore_interface()) break; if(wait_for_abort(2)) return;}

	if(IS(seg_name, "seg_device")) wait_for("/dev_bdvd", 15);

	if(n) {if(wait_for_abort(3)) return;}

	if(explore_interface)
	{
		// default category
		if(!*category) sprintf(category, "game");

		// default segment
		if(mount_unk == APP_GAME) sprintf(seg_name, "seg_gamedebug"); else
		if(!*seg_name) sprintf(seg_name, webman_config->noBD ? "seg_gamedebug" : "seg_device");

		if(!IS(seg_name, "seg_device") || isDir("/dev_bdvd"))
		{
			u8 wait, retry = 0, timeout = 10, icon_found = 0;

			while(View_Find("webrender_plugin") || View_Find("webbrowser_plugin"))
			{
				if(wait_for_abort(50)) return; if(++retry > 100) break;
			}

			// use segment for media type
			if(IS(category, "game") && IS(seg_name, "seg_device"))
			{
				if(isDir("/dev_bdvd/PS3_GAME")) {timeout = 40, icon_found = timeout - 5;} else
				if(file_exists("/dev_bdvd/SYSTEM.CNF")) {timeout = 4, icon_found = 0;} else
				if(isDir("/dev_bdvd/BDMV") )    {sprintf(category, "video"); sprintf(seg_name, "seg_bdmav_device");} else
				if(isDir("/dev_bdvd/VIDEO_TS")) {sprintf(category, "video"); sprintf(seg_name, "seg_dvdv_device" );} else
				if(isDir("/dev_bdvd/AVCHD"))    {sprintf(category, "video"); sprintf(seg_name, "seg_avchd_device");} else
				return;
			}
			else {timeout = 1, icon_found = 1;}

			if(mount_unk >= EMU_ROMS) {timeout = 1, icon_found = 1;}

			//if(!webman_config->autoplay) execute = false;

			for(n = 0; n < timeout; n++)
			{
				if(abort_autoplay() || IS_INGAME) return;

				if((n < icon_found) && file_exists(XMB_DISC_ICON)) {n = icon_found;}
				wait = (n < icon_found) || execute;

				if(wait) {if(wait_for_abort(50)) return;}
				exec_xmb_command("close_all_list");
				if(wait) {if(wait_for_abort(150)) return;}
				exec_xmb_command2("focus_category %s", category);
				if(wait) {if(wait_for_abort(100)) return;}
				exec_xmb_command2("focus_segment_index %s", seg_name);
				if(wait) {if(wait_for_abort(100)) return;}
			}

			if(mount_unk == APP_GAME) mount_unk = EMU_OFF; // allow abort

			if(wait_for_abort(200)) return;

			if(mount_unk >= EMU_ROMS) sys_ppu_thread_sleep(1);

			if(execute) explore_exec_push(0, false);
		}
		//else if(!webman_config->nobeep) {BEEP3}
	}
}

static int has_app_home = NONE;

static bool is_app_home_onxmb(void)
{
	if(has_app_home >= 0) return (bool)has_app_home;

	sys_addr_t sysmem = sys_mem_allocate(_64KB_); has_app_home = false;
	if(sysmem)
	{
		char *buffer = (char*)sysmem;
		size_t read_e = read_file(CATEGORY_GAME_XML, buffer, _8KB_, 0);
		has_app_home = (read_e > 100) && strstr(buffer, "seg_gamedebug");
		sys_memory_free(sysmem);
	}

	return (bool)has_app_home;
}

static void launch_disc(bool exec)
{
	char category[8], seg_name[16]; *category = *seg_name = NULL;
	exec_xmb_item(category, seg_name, exec);
}

static bool launch_app_home_icon(bool exec)
{
	if(not_exists("/app_home/PS3_GAME/USRDIR/EBOOT.BIN") && not_exists("/app_home/USRDIR/EBOOT.BIN"))
		launch_disc(exec);
	else if(is_app_home_onxmb())
	{
		mount_unk = APP_GAME;
		launch_disc(exec);
		return true;
	}
	return false;
}

static void goto_xmb_home(bool reload_game)
{
	if(IS_ON_XMB && get_explore_interface())
	{
		play_rco_sound("snd_system_ok");
		exec_xmb_command2("focus_category %s", "network");		 // force lose focus before focus game column
		exec_xmb_command2("focus_category %s", "game");			 // return focus to game column
		exec_xmb_command2("focus_segment_index %s", "xmb_app3"); // focus on webMAN Games
		exec_xmb_command2("reload_category %s", "music");		 // prevent xmb stuck on music albums
		exec_xmb_command2("reload_category %s", "video");		 // prevent xmb stuck on video albums
		if(reload_game)
		{
			sys_ppu_thread_usleep(1000);
			exec_xmb_command2("focus_category %s", "network");	 // force lose focus before focus game column
			exec_xmb_command2("reload_category %s", "game");	 // prevent xmb stuck on game column
			exec_xmb_command2("focus_category %s", "game");		 // return focus to game column
		}
	}
}

#ifdef PLAY_MUSIC
static void start_xmb_player(const char* column)
{
	if(IS_ON_XMB && get_explore_interface())
	{
		exec_xmb_command("close_all_list");
		sys_ppu_thread_sleep(1);
		exec_xmb_command2("focus_category %s", column);
		sys_ppu_thread_sleep(1);
		exec_xmb_command("scroll_list 9999");
		if(wait_for_abort(2)) return;
		parse_pad_command("triangle", 0);
		if(wait_for_abort(2)) return;
		parse_pad_command("cross", 0);
		if(wait_for_abort(2)) return;
		parse_pad_command("psbtn", 0);
		sys_ppu_thread_sleep(1);
		exec_xmb_command2("focus_category %s", "game");
	}
}
#endif

#ifdef COBRA_ONLY
static void reload_xmb(void)
{
	if(!cobra_version) return;
	if(IS_ON_XMB)
	{
		mount_unk = EMU_OFF;
		if(is_app_dir(_HDD0_GAME_DIR, "RELOADXMB") && is_app_home_onxmb())
		{
			set_app_home(RELOADXMB_DIR);
			mount_unk = APP_GAME;
		}
		else if(file_exists(RELOADXMB_ISO))
		{
			mount_unk = mount_game(RELOADXMB_ISO, 0); // MOUNT_SILENT
		}
		if(mount_unk)
		{
			patch_gameboot(0); // None
			launch_disc(true);
			mount_unk = EMU_OFF;
		}
	}
}
#else
static void reload_xmb(void)
{
	// hold L2 to cancel reload xmb
	if(is_pressed(CELL_PAD_CTRL_L2)) return; // hold L2 to cancel reload xmb

	if(IS_ON_XMB && is_app_dir(_HDD0_GAME_DIR, "RELOADXMB"))
	{
		if(!get_explore_interface()) return;

		exec_xmb_command("close_all_list");
		exec_xmb_command2("focus_category %s", "network");
		exec_xmb_command2("focus_segment_index %s", "-1");
		if(wait_for_abort(1)) return;
		explore_exec_push(0, false);
	}
}
#endif
