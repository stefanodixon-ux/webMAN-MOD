	if(mount_ps3)
	{
		// /mount_ps3/<path>[?random=<x>[&emu={ps1_netemu.self/ps1_netemu.self}][offline={0/1}][&to=/app_home]
		struct timeval tv;
		tv.tv_sec = 3;
		setsockopt(conn_s, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

		#ifdef COBRA_ONLY
		mount_app_home = get_flag(param, "&to=/app_home");
		#endif

		if(IS_ON_XMB && !(webman_config->combo2 & PLAY_DISC) && (strstr(param, ".ntfs[BD") == NULL) && (strstr(param, "/PSPISO") == NULL) && (strstr(param, ".ntfs[PSPISO]") == NULL))\
		{
			//sys_ppu_thread_sleep(1);

			if(get_explore_interface())
			{
				if(webman_config->ps2l && is_BIN_ENC(param))
					focus_first_item();
				else
					explore_close_all(param);
			}
		}

		if(sysmem) {sys_memory_free(sysmem); sysmem = NULL;}

		u8 ap = 1; // use webman_config->autoplay
		if((webman_config->autoplay == 0) && !(webman_config->combo2 & PLAY_DISC))
		{
			// check if holding CROSS to force auto-play
			if(is_pressed(CELL_PAD_CTRL_CROSS)) force_ap = ap = 2; // force auto_play
		}

		if(game_mount(pbuffer, templn, param, tempstr, mount_ps3, forced_mount)) ap_param = ap;

		if(!mc) http_response(conn_s, header, param, CODE_CLOSE_BROWSER, HTML_CLOSE_BROWSER); //auto-close browser

		keep_alive = 0; is_busy = false;

		goto exit_handleclient_www;
	}
	else
 #ifdef PS2_DISC
	if(forced_mount || islike(param, "/mount.ps3") || islike(param, "/mount.ps2") || islike(param, "/mount_ps2") || islike(param, "/copy.ps3"))
 #else
	if(forced_mount || islike(param, "/mount.ps3") || islike(param, "/copy.ps3"))
 #endif
	{
		// /mount.ps3?<search-name>
		// /mount.ps3/<path>[?random=<x>[&emu={ps1_netemu.self/ps1_netemu.self}][offline={0/1}][&to=/app_home]
		// /mount.ps3/unmount
		// /mount.ps2/<path>[?random=<x>]
		// /mount.ps2/unmount
		// /mount.ps3/<dev_path>&name=<device-name>&fs=<file-system>
		// /mount.ps3/unmount<dev_path>
		// /copy.ps3/<path>[&to=<destination>]
		// /copy.ps3/<path>[&to=<destination>]?restart.ps3

		char *params = param + 10;

		keep_alive = 0;
		Check_Overlay();

		#ifdef COBRA_ONLY
		if(islike(param, "/mount.ps3"))
		{
			mount_app_home = get_flag(params, "&to=/app_home");
		}
		#endif

		if(islike(param, "/mount.ps3/unmount"))
		{
			is_mounting = false;

			char *dev_path = (param + 18); // /mount.ps3/unmount<dev_path>
			if(*dev_path == '/')
			{
				if(isDir(dev_path)) {system_call_3(SC_FS_UMOUNT, (uint32_t)dev_path, 0, 1);}
				sprintf(param, "/"); is_binary = FOLDER_LISTING; mount_app_home = is_busy = false;
				goto html_response;
			}
		}
		else if(islike(param, "/copy.ps3")) ;

		else if(!islike(params, "/net") && !islike(params, WMTMP))
		{
			strcpy(templn, params);
			if(not_exists(params))
			{
				find_slaunch_game(params, 10); // search in slaunch.bin
				urldec(params, templn);
			}
			check_path_alias(params);

			// /mount.ps3/<dev_path>&name=<device-name>&fs=<file-system>
			char *dev_path = templn;
			char *dev_name = strstr(dev_path, "&name="); if(dev_name) {*dev_name = 0, dev_name += 6;}
			char *fs = strstr(dev_path, "&fs="); if(fs) {*fs = 0, fs += 3;} else fs = (char*)"CELL_FS_FAT";

			if(islike(dev_path, "/dev_") && (not_exists(dev_path) || dev_name))
			{
				mount_device(dev_path, dev_name, fs);

				if(isDir(dev_path))
				{
					strcpy(param, dev_path); is_binary = FOLDER_LISTING; mount_app_home = is_busy = false;
					goto html_response;
				}
				else
				{
					keep_alive = http_response(conn_s, header, param, CODE_PATH_NOT_FOUND, "404 Path not found"); mount_app_home = is_busy = false;
					goto exit_handleclient_www;
				}
			}
		}

		game_mount(pbuffer, templn, param, tempstr, mount_ps3, forced_mount);

		is_busy = false;
		disable_progress();
	}
