#ifdef PS3_BROWSER
	if(islike(param, "/browser.ps3") || islike(param, "/xmb.ps3"))
	{
		// /browser.ps3?<url>                  open url on PS3 browser
		// /xmb.ps3$exit                       exit to xmb
		// /xmb.ps3$exit_to_update             exit to system update
		// /xmb.ps3$reloadgame                 reload ps3 game
		// /xmb.ps3$rsx_pause                  pause rsx processor
		// /xmb.ps3$rsx_continue               continue rsx processor
		// /xmb.ps3$block_servers              block url of PSN servers in lv2
		// /xmb.ps3$restore_servers            restore url of PSN servers in lv2
		// /xmb.ps3$show_idps                  show idps/psid (same as R2+O)
		// /xmb.ps3$xregistry(<id>)            show value by id from xregistry.sys
		// /xmb.ps3$xregistry(<id>)=<value>    update value by id in xregistry.sys
		// /xmb.ps3$disable_syscalls           disable CFW syscalls
		// /xmb.ps3$toggle_rebug_mode          toggle rebug mode (swap VSH REX/DEX)
		// /xmb.ps3$toggle_normal_mode         toggle normal mode (swap VSH REX/CEX)
		// /xmb.ps3$toggle_debug_menu          toggle debug menu (DEX/CEX)
		// /xmb.ps3$toggle_cobra               toggle Cobra (swap stage2)
		// /xmb.ps3$toggle_ps2emu              toggle ps2emus between /dev_hdd0/game/RBGTLBOX2/USRDIR/ and /dev_blind/ps2emu/
		// /xmb.ps3$ps2emu                     enable default ps2 emulator on fat consoles only
		// /xmb.ps3$ps2_netemu                 enable ps2_netemu on fat consoles only
		// /xmb.ps3$enable_classic_ps2_mode    creates 'classic_ps2_mode' to enable PS2 classic in PS2 Launcher (old rebug)
		// /xmb.ps3$disable_classic_ps2_mode   deletes 'classic_ps2_mode' to enable PS2 ISO in PS2 Launcher (old rebug)
		// /xmb.ps3/<webman_cmd>               execute webMAN command on PS3 browser
		// /xmb.ps3$<explore_plugin_command>   execute explore_plugin command on XMB (http://www.psdevwiki.com/ps3/Explore_plugin#Example_XMB_Commands)
		// /xmb.ps3*<xmb_plugin_command>       execute xmb_plugin commands on XMB (http://www.psdevwiki.com/ps3/Xmb_plugin#Function_23_Examples)
		// /xmb.ps3$slaunch                    start slaunch
		// /xmb.ps3$vsh_menu                   start vsh_menu
		// /xmb.ps3$home                       go to webMAN Games
		// /xmb.ps3$home*                      go to webMAN Games + reload_category game
		// /xmb.ps3$eject                      eject emulated disc (hide disc icon)
		// /xmb.ps3$insert                     insert emulated disc (show disc icon)
		// /xmb.ps3$music                      play xmb music
		// /xmb.ps3$video                      play xmb video
		// /xmb.ps3$screenshot<path>           capture XMB screen
		// /xmb.ps3$screenshot?show            capture XMB screen show
		// /xmb.ps3$screenshot?show?fast       capture XMB screen (25% smaller)
		// /xmb.ps3$ingame_screenshot          enable screenshot in-game on CFW without the feature (same as R2+O)

		char *param2 = param + (islike(param, "/xmb.ps3") ? 8 : 12);
		char *url = param2 + 1;
		check_path_tags(param2);

		if(islike(param2, "$home"))
		{
			goto_xmb_home(param2[5] != 0);
		}
		else if(islike(param2, "$exit"))
		{
			int is_ingame = View_Find("game_plugin");

			if(is_ingame)
			{
				game_interface = (game_plugin_interface *)plugin_GetInterface(is_ingame, 1);
				if(strchr(param2, 'u')) // $exit_to_update
					game_interface->ExitToUpdate();
				else
					game_interface->ExitGame(val(param2 + 5)); // $exit3 = remote play, $exit4 = focus remote play

				sprintf(param, "/cpursx.ps3");
				goto html_response;
			}
		}
		else if(islike(param2, "$reloadgame"))
		{
			int is_ingame = View_Find("game_plugin");

			if(is_ingame)
			{
				game_interface = (game_plugin_interface *)plugin_GetInterface(is_ingame, 1);
				game_interface->ReloadGame();
			}
		}
		else
		#ifdef COBRA_ONLY
		if(islike(param2, "$eject"))
		{
			if(islike(param2 + 6, "/dev_usb"))
				fake_eject_event(USB_MASS_STORAGE((u8)val(param2 + 14)));
			else
				cobra_send_fake_disc_eject_event();
		}
		else
		if(islike(param2, "$insert"))
		{
			if(islike(param2 + 7, "/dev_usb"))
				fake_insert_event(USB_MASS_STORAGE((u8)val(param2 + 15)), DEVICE_TYPE_USB);
			else
			{
				cobra_send_fake_disc_insert_event();
				cobra_disc_auth();
			}
		}
		else
		#endif
		#ifdef PLAY_MUSIC
		if(islike(param2, "$music"))
		{
			start_xmb_player("music");
		}
		else
		if(islike(param2, "$video"))
		{
			start_xmb_player("video");
		}
		else
		#endif
		if(islike(param2, "$rsx"))
		{
			static u8 rsx = 1;
			if(strstr(param2, "pause")) rsx = 1;
			if(strstr(param2, "cont"))  rsx = 0;
			rsx_fifo_pause(rsx); // rsx_pause / rsx_continue
			rsx ^= 1;
		}
		else
		if(islike(param2, "$block_servers"))
		{
			block_online_servers(true);
		}
		else
		if(islike(param2, "$restore_servers"))
		{
			restore_blocked_urls(true);
		}
		else
		/*if(islike(param2, "$dlna"))
		{
			int status = 2;
			if(param2[5] == '?') status = val(param2 + 6);
			toggle_dlna(status);
		}
		else*/
		#ifdef SPOOF_CONSOLEID
		if(islike(param2, "$show_idps"))
		{
			show_idps(header);
		}
		else
		#endif
		#ifdef DEBUG_XREGISTRY
		if(islike(param2, "$xregistry(/"))
		{
			param2 += 11; char *pos = strchr(param2, ')');
			if(pos)
			{
				*pos = 0; u32 value; *header = 0;
				if(pos[1] == '=')
				{
					strcpy(header, pos + 2);
					value = val(header);
					get_xreg_value(param2, value, header, false);
				}
				else
					value = get_xreg_value(param2, 0, header, true);
				*pos = ')';

				if(*header)
					sprintf(pos + 1, " => %s", header);
				else
					sprintf(pos + 1, " => %i (0x%04x)", value, value);
			}
		}
		else
		#endif
		#ifndef LITE_EDITION
		if(islike(param2, "$xregistry("))
		{
			int value, len, size; param2 += 11;
			int id = val(param2);

			size = get_xreg_entry_size(id);

			char *pos = strstr(param2, ")="); // save
			if(pos)
			{
				if(size >= 0x80)
				{
					pos += 2, len = strlen(pos);
					if(size > 0x80)
						xusers()->SetRegistryString(xusers()->GetCurrentUserNumber(), id, pos, len);
					else
						xregistry()->saveRegistryStringValue(id, pos, len);
				}
				else
				{
					value = val(pos + 2);
					if(size)
						xusers()->SetRegistryValue(xusers()->GetCurrentUserNumber(), id, value);
					else
						xregistry()->saveRegistryIntValue(id, value);
				}
			}

			len = strlen(param2);

			if(size >= 0x80)
			{
				char *pos2 = strchr(param2, ','); if(pos2) size = val(pos2 + 1); if(size <= 0) size = 0x80;
				if(size > 0x80)
					xusers()->GetRegistryString(xusers()->GetCurrentUserNumber(), id, header, size);
				else
					xregistry()->loadRegistryStringValue(id, header, size);
				sprintf(param2 + len, " => %s", header);
			}
			else
			{
				if(size)
					xusers()->GetRegistryValue(xusers()->GetCurrentUserNumber(), id, &value);
				else
					xregistry()->loadRegistryIntValue(id, &value);
				sprintf(param2 + len, " => %i (0x%04x)", value, value);
			}
		}
		else
		if(islike(param2, "$ingame_screenshot"))
		{
			enable_ingame_screenshot();
		}
		else
		#endif // #ifndef LITE_EDITION
		#ifdef REMOVE_SYSCALLS
		if(islike(param2, "$disable_syscalls"))
		{
			disable_cfw_syscalls(strcasestr(param, "ccapi")!=NULL);
		}
		else
		#endif
		#ifdef PS3MAPI
		if(islike(param2, "$restore_syscalls"))
		{
			restore_cfw_syscalls();
		}
		else
		#endif
		#ifdef REX_ONLY
		if(islike(param2, "$toggle_rebug_mode"))
		{
			if(toggle_rebug_mode()) goto reboot;
		}
		else
		if(islike(param2, "$toggle_normal_mode"))
		{
			if(toggle_normal_mode()) goto reboot;
		}
		else
		if(islike(param2, "$toggle_debug_menu"))
		{
			toggle_debug_menu();
		}
		else
		#endif
		#ifdef COBRA_ONLY
		 #ifndef LITE_EDITION
		if(islike(param2, "$toggle_cobra"))
		{
			if(toggle_cobra()) goto reboot;
		}
		else
		if(islike(param2, "$toggle_ps2emu"))
		{
			toggle_ps2emu();
		}
		else
		if(islike(param2, "$ps2emu"))
		{
			enable_ps2netemu_cobra(0); // enable default ps2 emulator on fat consoles only
		}
		else
		if(islike(param2, "$ps2_netemu"))
		{
			enable_ps2netemu_cobra(1); // enable ps2_netemu on fat consoles only
		}
		else
		if(strstr(param2, "le_classic_ps2_mode"))
		{
			bool classic_ps2_enabled;

			if(islike(param2, "$disable_"))
			{
				// $disable_classic_ps2_mode
				classic_ps2_enabled = true;
			}
			else
			if(islike(param2, "$enable_"))
			{
				// $enable_classic_ps2_mode
				classic_ps2_enabled = false;
			}
			else
			{
				// $toggle_classic_ps2_mode
				classic_ps2_enabled = file_exists(PS2_CLASSIC_TOGGLER);
			}

			if(classic_ps2_enabled)
				disable_classic_ps2_mode();
			else
				enable_classic_ps2_mode();

			show_status("PS2 Classic", classic_ps2_enabled ? STR_DISABLED : STR_ENABLED);

			sys_ppu_thread_sleep(3);
		}
		else
		 #endif // #ifndef LITE_EDITION
		#endif // #ifdef COBRA_ONLY
		if(IS_ON_XMB || *param2 == '?' || *param2 == '/')
		{   // in-XMB
			#ifdef COBRA_ONLY
			if(islike(param2, "$vsh_menu")) {start_vsh_gui(true); sprintf(param, "/cpursx.ps3"); goto html_response;}
			else
			if(islike(param2, "$slaunch")) {start_vsh_gui(false); sprintf(param, "/cpursx.ps3"); goto html_response;}
			else
			#endif
			#ifdef XMB_SCREENSHOT
			if(islike(param2, "$screenshot"))
			{
				char *filename = param2 + 11; if(strchr(filename, '/')) filename = strchr(filename, '/');
				bool fast = get_flag(param2, "?fast");
				bool show = get_flag(param2, "?show");

				if(show && (*filename != '/'))
					sprintf(header, "%s/screenshot.bmp", WMTMP);
				else
					sprintf(header, "%s", filename);

				saveBMP(header, false, fast);
				if(show)
				{
					sprintf(param, "%s", header); is_binary = true;
					goto retry_response;
				}
				*url = 0; add_breadcrumb_trail2(url, NULL, header);
			}
			else
			#endif
			{
				if(*param2 == NULL) sprintf(param2, "/");
				if(*param2 == '/' ) {do_umount(false); check_path_alias(param2); sprintf(header, "http://%s%s", local_ip, param2); open_browser(header, 0);} else
				if(*param2 == '$' ) {if(get_explore_interface()) exec_xmb_command(url);} else
				if(*param2 == '?' ) {do_umount(false);  open_browser(url, 0);} else
									{					open_browser(url, 1);} // example: /browser.ps3*regcam:reg?   More examples: http://www.psdevwiki.com/ps3/Xmb_plugin#Function_23

				if(*param2 != '$' ) if(!(webman_config->minfo & 1)) show_msg(url);
			}
		}
		else
			sprintf(url, "ERROR: Not in XMB!");

		if(!mc) keep_alive = http_response(conn_s, header, param, CODE_HTTP_OK, url);

		goto exit_handleclient_www;
	}
#endif // #ifdef PS3_BROWSER
