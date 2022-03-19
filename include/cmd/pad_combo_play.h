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

		char *buttons = param + 9 + is_combo;

		if(is_combo != 1) {if(!webman_config->nopad) ret = parse_pad_command(buttons, is_combo);} // do /pad.ps3 || /combo.ps3
		else // do /play.ps3
		{
			char *param2 = param + 9; if(*param2 == '?') param2++;

			#ifdef COBRA_ONLY
			if((*param2 == NULL) && is_app_dir("/app_home", "PS3_GAME") && !is_app_dir("/dev_bdvd", "PS3_GAME") && not_exists("/dev_bdvd/SYSTEM.CNF")) goto launch_app;
			#endif

			if(islike(param2, "snd_"))
			{
				play_rco_sound(param2);

				#ifdef PS3MAPI
				sprintf(param, "/buzzer.ps3mapi");
				goto html_response;
				#else
				if(!mc) keep_alive = http_response(conn_s, header, param, CODE_VIRTUALPAD, param2);

				goto exit_handleclient_www;
				#endif
			}

			strcpy(header, param2); // backup original request

			// check /play.ps3<path>
			if(*param2 == '/')
			{
				if(not_exists(param2))
				{
					find_slaunch_game(param2, 10); // search in slaunch.bin
					urldec(param2, header);
				}
				check_path_alias(param2);
			}

			if(file_exists(param2))
			{
				#ifdef COBRA_ONLY
				if(IS(param2, "/app_home"))
				{
	launch_app:
					if(wait_for_xmb())
					{
						keep_alive = http_response(conn_s, header, param, CODE_BAD_REQUEST, param);
						goto exit_handleclient_www;
					}

					launch_app_home_icon(true);
					sprintf(param, "/cpursx.ps3");
				}
				else
				#endif
				#ifdef WM_CUSTOM_COMBO
				if(islike(param2, WM_CUSTOM_COMBO))
				{
					parse_script(param2);
					memcpy(param, "/edit.ps3", 9);
					is_popup = 1;
				}
				else
				#endif
				{
					strcpy(header, param2);
					sprintf(param, "/mount.ps3%s", header);

					if(is_ext(header, ".bat") || is_ext(header, ".txt"))
						ap_param = 0; // do not auto_play
					else
						ap_param = 2; // force auto_play
				}
				is_binary = WEB_COMMAND;
				goto html_response;
			}
			else
				strcpy(param2, header); // restore original parameter

			// default: play.ps3?col=game&seg=seg_device
			char col[16], seg[80]; *col = *seg = NULL;
	#ifdef COBRA_ONLY
			#ifndef LITE_EDITION
			strcpy(header, param2); param2 = (char*)header;
			if(_islike(param2, "movian") || IS(param2, "HTSS00003"))
											 {sprintf(param2, "col=tv&seg=HTSS00003"); mount_unk = APP_GAME;} else
			if(_islike(param2, "remoteplay")){sprintf(param2, "col=network&seg=seg_premo");} else
			if(_islike(param2, "retro"))     {sprintf(param2, "SSNE10000");} else
			if(_islike(param2, "multiman"))  {sprintf(param2, "BLES80608");} else
			if(_islike(param2, "rebug"))     {sprintf(param2, "RBGTLBOX2");}
			#endif

			char path[32];
			snprintf(path, 32, "%s%s", HDD0_GAME_DIR, param2);

			if(*map_title_id && (*param2 == NULL))
			{
				patch_gameboot(0);
				launch_app_home_icon(true);
			}
			else if((*param2 != NULL) && isDir(path))
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
				get_param("col=", col, param2, 16); // game / video / friend / psn / network / music / photo / tv
				get_param("seg=", seg, param2, 80);
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

		sprintf(param, "/cpursx.ps3");
	}
#endif //  #ifdef VIRTUAL_PAD
