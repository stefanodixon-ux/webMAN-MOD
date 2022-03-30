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

		char *params = param + (islike(param, "/xmb.ps3") ? 8 : 12);
		char *url = params + 1;
		check_path_tags(params);

		if(islike(params, "$home"))
		{
			goto_xmb_home(params[5] != 0);
		}
		else if(islike(params, "$exit"))
		{
			int is_ingame = View_Find("game_plugin");

			if(is_ingame)
			{
				game_interface = (game_plugin_interface *)plugin_GetInterface(is_ingame, 1);
				if(strchr(params, 'u')) // $exit_to_update
					game_interface->ExitToUpdate();
				else
					game_interface->ExitGame(val(params + 5)); // $exit3 = remote play, $exit4 = focus remote play

				sprintf(param, "/cpursx.ps3");
				goto html_response;
			}
		}
		else if(islike(params, "$reloadgame"))
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
		if(islike(params, "$eject"))
		{
			if(islike(params + 6, "/dev_usb"))
				fake_eject_event(USB_MASS_STORAGE((u8)val(params + 14)));
			else
				cobra_send_fake_disc_eject_event();
		}
		else
		if(islike(params, "$insert"))
		{
			if(islike(params + 7, "/dev_usb"))
				fake_insert_event(USB_MASS_STORAGE((u8)val(params + 15)), DEVICE_TYPE_USB);
			else
			{
				cobra_send_fake_disc_insert_event();
				cobra_disc_auth();
			}
		}
		else
		#endif
		#ifdef PLAY_MUSIC
		if(islike(params, "$music"))
		{
			start_xmb_player("music");
		}
		else
		if(islike(params, "$video"))
		{
			start_xmb_player("video");
		}
		else
		#endif
		if(islike(params, "$rsx"))
		{
			static u8 rsx = 1;
			if(strstr(params, "pause")) rsx = 1;
			if(strstr(params, "cont"))  rsx = 0;
			rsx_fifo_pause(rsx); // rsx_pause / rsx_continue
			rsx ^= 1;
		}
		else
		if(islike(params, "$block_servers"))
		{
			block_online_servers(true);
		}
		else
		if(islike(params, "$restore_servers"))
		{
			restore_blocked_urls(true);
		}
		else
		/*if(islike(params, "$dlna"))
		{
			int status = 2;
			if(params[5] == '?') status = val(params + 6);
			toggle_dlna(status);
		}
		else*/
		#ifdef SPOOF_CONSOLEID
		if(islike(params, "$show_idps"))
		{
			show_idps(header);
		}
		else
		#endif
		#ifdef DEBUG_XREGISTRY
		if(sys_admin && islike(params, "$xregistry(/"))
		{
			params += 11; char *pos = strchr(params, ')');
			if(pos)
			{
				*pos = '\0'; u32 value; memset(header, 0, sizeof(header));
				if(pos[1] == '=')
				{
					strcpy(header, pos + 2);
					value = val(header);
					get_xreg_value(params, value, header, false);
				}
				else
					value = get_xreg_value(params, 0, header, true);
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
		if(sys_admin && islike(params, "$xregistry("))
		{
			int value, len, size; params += 11;
			int id = val(params);

			size = get_xreg_entry_size(id);

			char *pos = strstr(params, ")="); // save
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

			len = strlen(params);

			if(size >= 0x80)
			{
				memset(header, 0, sizeof(header));
				char *pos2 = strchr(params, ','); if(pos2) size = val(pos2 + 1); if(size <= 0) size = 0x80;
				if(size > 0x80)
					xusers()->GetRegistryString(xusers()->GetCurrentUserNumber(), id, header, size);
				else
					xregistry()->loadRegistryStringValue(id, header, size);
				sprintf(params + len, " => %s", header);
			}
			else
			{
				if(size)
					xusers()->GetRegistryValue(xusers()->GetCurrentUserNumber(), id, &value);
				else
					xregistry()->loadRegistryIntValue(id, &value);
				sprintf(params + len, " => %i (0x%04x)", value, value);
			}
		}
		else
		if(islike(params, "$ingame_screenshot"))
		{
			enable_ingame_screenshot();
		}
		else
		#endif // #ifndef LITE_EDITION
		#ifdef REMOVE_SYSCALLS
		if(islike(params, "$disable_syscalls"))
		{
			disable_cfw_syscalls(strcasestr(param, "ccapi")!=NULL);
		}
		else
		#endif
		#ifdef PS3MAPI
		if(islike(params, "$restore_syscalls"))
		{
			restore_cfw_syscalls();
		}
		else
		#endif
		#ifdef REX_ONLY
		if(islike(params, "$toggle_rebug_mode"))
		{
			if(toggle_rebug_mode()) goto reboot;
		}
		else
		if(islike(params, "$toggle_normal_mode"))
		{
			if(toggle_normal_mode()) goto reboot;
		}
		else
		if(islike(params, "$toggle_debug_menu"))
		{
			toggle_debug_menu();
		}
		else
		#endif
		#ifdef COBRA_ONLY
		 #ifndef LITE_EDITION
		if(islike(params, "$toggle_cobra"))
		{
			if(toggle_cobra()) goto reboot;
		}
		else
		if(islike(params, "$toggle_ps2emu"))
		{
			toggle_ps2emu();
		}
		else
		if(islike(params, "$ps2"))
		{
			enable_ps2netemu_cobra(strstr(params, "net") != NULL); // enable ps2_netemu on fat consoles only
		}
		else
		if(strstr(params, "le_classic_ps2_mode"))
		{
			bool classic_ps2_enabled;

			if(islike(params, "$disable_"))
			{
				// $disable_classic_ps2_mode
				classic_ps2_enabled = true;
			}
			else
			if(islike(params, "$enable_"))
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
		if(IS_ON_XMB || *params == '?' || *params == '/')
		{   // in-XMB
			#ifdef COBRA_ONLY
			if(islike(params, "$vsh_menu")) {start_vsh_gui(true); sprintf(param, "/cpursx.ps3"); goto html_response;}
			else
			if(islike(params, "$slaunch")) {start_vsh_gui(false); sprintf(param, "/cpursx.ps3"); goto html_response;}
			else
			#endif
			#ifdef XMB_SCREENSHOT
			if(islike(params, "$screenshot"))
			{
				bool fast = get_flag(params, "?fast");
				bool show = get_flag(params, "?show");

				strcpy(header, params + 11);
				char *bmpfile = strchr(header, '/'); if(!bmpfile) bmpfile = header;

				if(show && (*bmpfile != '/'))
					sprintf(bmpfile, "%s/screenshot.bmp", WMTMP);

				saveBMP(bmpfile, false, fast);

				if(show)
				{
					strcpy(param, bmpfile); is_binary = true;
					goto html_listing;
				}
				*url = 0; add_breadcrumb_trail2(url, NULL, header);
			}
			else
			#endif
			{
				if(*params == NULL) sprintf(params, "/");
				if(*params == '/' ) {do_umount(false); check_path_alias(params); sprintf(header, "http://%s%s", local_ip, params); open_browser(header, 0);} else
				if(*params == '$' ) {if(get_explore_interface()) exec_xmb_command(url);} else
				if(*params == '?' ) {do_umount(false);  open_browser(url, 0);} else
									{					open_browser(url, 1);} // example: /browser.ps3*regcam:reg?   More examples: http://www.psdevwiki.com/ps3/Xmb_plugin#Function_23

				if(*params != '$' ) if(!(webman_config->minfo & 1)) show_msg(url);
			}
		}
		else
			sprintf(url, "ERROR: Not in XMB!");

		if(!mc) keep_alive = http_response(conn_s, header, param, CODE_HTTP_OK, url);

		goto exit_handleclient_www;
	}
#endif // #ifdef PS3_BROWSER
