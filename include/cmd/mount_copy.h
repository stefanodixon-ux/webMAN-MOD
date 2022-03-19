#ifndef LITE_EDITION
	if(islike(param, "/mount.ps3?"))
	{
		// /mount.ps3?<query>  search game & mount if only 1 match is found

		if(islike(param, "/mount.ps3?http"))
		{
			char *url  = param + 11;
			do_umount(false);  open_browser(url, 0);
			keep_alive = http_response(conn_s, header, param, CODE_HTTP_OK, url);
			goto exit_handleclient_www;
		}
		else
			{memcpy(header, "/index.ps3", 10); memcpy(param, "/index.ps3", 10); auto_mount = true;}
	}
#endif
#ifdef USE_NTFS
	// /mount.ps3/dev_ntfs* => convert ntfs path to cached path
	if(islike(param, "/mount") && is_ntfs_path(param + 10))
	{
		char *filename = param + 10;
		strcpy(header, filename);
		int flen = get_name(filename, header, GET_WMTMP);

		for(int i = 2; i < 9; i++) // "PS3ISO", "BDISO", "DVDISO", "PS2ISO", "PSXISO", "PSXGAMES", "PSPISO"
			if(strstr(header, paths[i])) {sprintf(filename + flen, ".ntfs[%s]", paths[i]); break;}

		if(not_exists(filename))
		{
			check_ntfs_volumes();
			prepNTFS(0);
		}
	}
#endif
	else if(mount_ps3)
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
			pad_data = pad_read(); // check if holding CROSS to force auto-play
			if(pad_data.len > 0 && (pad_data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] == CELL_PAD_CTRL_CROSS)) force_ap = ap = 2; // force auto_play
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

		keep_alive = 0;

		#ifdef COBRA_ONLY
		if(islike(param, "/mount.ps3"))
		{
			mount_app_home = get_flag(param, "&to=/app_home");
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

		else if(!islike(param + 10, "/net") && !islike(param + 10, WMTMP))
		{
			char *param2 = param + 10; strcpy(templn, param2);
			if(not_exists(param2))
			{
				find_slaunch_game(param2, 10); // search in slaunch.bin
				urldec(param2, templn);
			}
			check_path_alias(param2);

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
	}
