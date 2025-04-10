#ifdef VIRTUAL_PAD
	if(is_pad || islike(param, "/combo.ps3") || islike(param, "/play.ps3"))
	{
		// /pad.ps3                      (see vpad.h for details)
		// /combo.ps3                    simulates a combo without actually send the buttons
		// /play.ps3                     start game from disc icon
		// /play.ps3?col=<col>&seg=<seg>  click item on XMB
		// /play.ps3<path>               mount <path> and start game from disc icon
		// /play.ps3<script-path>        execute script. path must be a .txt or .bat file
		// /play.ps3?<titleid>           mount npdrm game and start game from app_home icon (e.g IRISMAN00, MANAGUNZ0, NPUB12345, etc.)
		// /play.ps3?<appname>           play movian, multiman, retroArch, rebug toolbox, remoteplay
		// /play.ps3?<search-name>       search game by name or path

		u8 ret = 0, is_combo = (param[2] == 'a') ? 0 : (param[1] == 'c') ? 2 : 1; // 0 = /pad.ps3   1 = /play.ps3   2 = /combo.ps3

		const char *buttons = param + 9 + is_combo;

		if(is_combo != 1) {if(!webman_config->nopad) ret = parse_pad_command(buttons, is_combo);} // do /pad.ps3 || /combo.ps3
		else // do /play.ps3
		{
			char *params = param + 9; if(*params == '?') params++;

			#ifdef COBRA_ONLY
			if((*params == NULL) && is_app_dir("/app_home", "PS3_GAME") && !is_app_dir("/dev_bdvd", "PS3_GAME") && not_exists("/dev_bdvd/SYSTEM.CNF")) goto launch_app;
			#endif

			if(islike(params, "snd_"))
			{
				play_rco_sound(params);

				#ifdef PS3MAPI
				strcopy(param, "/buzzer.ps3mapi");
				goto html_response;
				#else
				if(!mc) keep_alive = http_response(conn_s, header, param, CODE_VIRTUALPAD, params);

				goto exit_handleclient_www;
				#endif
			}

			strcpy(header, params); // backup original request

			// check /play.ps3<path>
			if(*params == '/')
			{
				if(not_exists(params))
				{
					find_slaunch_game(params, 10); // search in slaunch.bin
					urldec(params, header);
				}
				check_path_alias(params);
			}

			if(file_exists(params))
			{
				#ifdef COBRA_ONLY
				if(IS(params, "/app_home"))
				{
	launch_app:
					if(wait_for_xmb())
					{
						keep_alive = http_response(conn_s, header, param, CODE_BAD_REQUEST, param);
						goto exit_handleclient_www;
					}

					launch_app_home_icon(true);
					strcopy(param, "/cpursx.ps3");
				}
				else
				#endif
				#ifdef WM_CUSTOM_COMBO
				if(islike(params, WM_CUSTOM_COMBO))
				{
					parse_script(params, true);
					memcpy(param, "/edit.ps3", 9);
					is_popup = 1;
				}
				else
				#endif
				{
					prepend(param, "/mount.ps3", 1);

					if(is_ext(param, ".bat") || is_ext(param, ".txt"))
						ap_param = 0; // do not auto_play
					else
						ap_param = 2; // force auto_play
				}
				is_binary = WEB_COMMAND;
				goto html_response;
			}
			else
				strcpy(params, header); // restore original parameter

			// default: play.ps3?col=game&seg=seg_device
			char col[16], seg[80]; *col = *seg = NULL;

			#ifdef COBRA_NON_LITE
			strcpy(header, params); params = (char*)header;
			if(_islike(params, "movian") || IS(params, "HTSS00003"))
											 {strcopy(params, "col=tv&seg=HTSS00003"); mount_unk = APP_GAME;} else
			if(_islike(params, "remoteplay")){strcopy(params, "col=network&seg=seg_premo");} else
			if(_islike(params, "retro"))     {strcopy(params, "SSNE10000");} else
			if(_islike(params, "multiman"))  {strcopy(params, "BLES80608");} else
			if(_islike(params, "rebug"))     {strcopy(params, "RBGTLBOX2");}
			#endif

			#ifdef COBRA_ONLY
			char path[32];
			snprintf(path, sizeof(path), "%s%s", HDD0_GAME_DIR, params);

			if(*map_title_id && (*params == NULL))
			{
				patch_gameboot(0); // None
				launch_app_home_icon(true);
			}
			else if(*params && isDir(path))
			{
				patch_gameboot(3); // PS3

				set_app_home(path);
				sys_ppu_thread_sleep(1);

				mount_unk = APP_GAME;
				launch_app_home_icon(true);
			}
			else
			#endif
			{
				get_param("col=", col, params, 16); // game / video / friend / psn / network / music / photo / tv
				get_param("seg=", seg, params, 80);
				exec_xmb_item(col, seg, true);
			}
			mount_unk = EMU_OFF;
		}

		if(is_combo == 1 && param[10] != '?') sprintf(param, "/cpursx.ps3");
		else
		{
			if((ret == 'X') && IS_ON_XMB) goto reboot;

			if(!mc) keep_alive = http_response(conn_s, header, param, CODE_VIRTUALPAD, buttons);

			goto exit_handleclient_www;
		}
	}
#elif defined(LITE_EDITION)
	if(islike(param, "/play.ps3"))
	{
		// /play.ps3                     start game from disc icon

		// default: play.ps3?col=game&seg=seg_device
		launch_disc(true);

		strcopy(param, "/cpursx.ps3");
	}
#endif //  #ifdef VIRTUAL_PAD
