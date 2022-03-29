#define _IS_ON_XMB_		((sec & 1) && (gTick.tick == rTick.tick))
#define _IS_IN_GAME_	((sec & 1) && (gTick.tick >  rTick.tick))

// combos.h
static u8 show_persistent_popup = 0;

static bool toggle_snd0 = false;

static void poll_start_play_time(void)
{
	#ifdef OFFLINE_INGAME
	s32 status = 0;
	#endif

	if(IS_ON_XMB)
	{
		//if(gTick.tick != rTick.tick) vshnet_setUpdateUrl("http://127.0.0.1/dev_hdd0/ps3-updatelist.txt"); // re-apply redirection of custom update file returning to XMB
#ifdef COBRA_ONLY
 #ifdef WM_PROXY_SPRX
		if(gTick.tick != rTick.tick)
		{
			gTick = rTick;

			apply_remaps(); // re-apply remaps returning to XMB

			#ifdef VISUALIZERS
			char *param = html_base_path;
			randomize_vsh_resources(false, param);
			*param = NULL;
			#endif

			start_event(EVENT_ON_XMB);
		}
 #endif
#endif
		gTick = rTick;

	#ifdef OFFLINE_INGAME
		if(net_status >= 0)
		{
			xnet()->GetSettingNet_enable(&status);
			xnet()->SetSettingNet_enable(net_status);
			net_status = NONE; if(net_status && !status) vshNotify_WithIcon(ICON_NETWORK, ONLINE_TAG);
			cellFsUnlink(WMNET_DISABLED);
		}
	#endif

		if(toggle_snd0 && webman_config->nosnd0) { toggle_snd0 = false; cellFsChmod("/dev_bdvd/PS3_GAME/SND0.AT3", NOSND); } /* disable SND0.AT3 on XMB */
	}
	else if(gTick.tick == rTick.tick) /* the game started a moment ago */
	{
		cellRtcGetCurrentTick(&gTick);

		if(!toggle_snd0 && webman_config->nosnd0) { toggle_snd0 = true; cellFsChmod("/dev_bdvd/PS3_GAME/SND0.AT3", MODE); } /* re-enable SND0.AT3 in-game */

		close_ftp_sessions_idle();

	#ifdef PS3MAPI
		// unmap gameboot audio
		sys_map_path("/dev_flash/vsh/resource/gameboot_multi.ac3",  NULL);
		sys_map_path("/dev_flash/vsh/resource/gameboot_stereo.ac3", NULL);
	#endif
	#ifdef PATCH_GAMEBOOT
		patched_address1 = patched_address2 = patched_address3 = patched_address4 = BASE_PATCH_ADDRESS;
	#endif

	#ifdef OFFLINE_INGAME
		if((webman_config->spp & 4) || (net_status >= 0))
		{
			get_game_info();

			if(strlen(_game_TitleID) == 9 && View_Find("nas_plugin_module") == 0)
			{
				bool set_net_setatus = true;
				if(net_status < 0)
				{
					char online_title_ids[512];
					read_file(WMOFFLINE_GAMES, online_title_ids, 512, 0); // auto-disable network only on these title ids
					if(*online_title_ids) set_net_setatus = strstr(online_title_ids, _game_TitleID);
					else
					{
						read_file(WMONLINE_GAMES, online_title_ids, 512, 0);  // auto-disable network except on these title ids
						set_net_setatus = (strstr(online_title_ids, _game_TitleID) == NULL);
					}
				}

				if(set_net_setatus)
				{
					xnet()->GetSettingNet_enable(&status);
					xnet()->SetSettingNet_enable(net_status < 0 ? 0 : net_status);
					if(status && (net_status <= 0)) {save_file(WMNET_DISABLED, NULL, SAVE_ALL); vshNotify_WithIcon(ICON_NETWORK, OFFLINE_TAG);}
					net_status = status;
				}
			}
		}
	#endif
		start_event(EVENT_INGAME);
	}
}

static void poll_thread(__attribute__((unused)) u64 arg)
{
	u8 sec = 0;

	// fancontrol2.h
	u8 t1 = 0, t2 = 0;
	u8 lasttemp = 0;
	const u8 step = 3; // polling every 3 seconds
	const u8 step_up = 5;
	u8 stall = 0;
	u8 smoothstep = 0;
	int delta = 0;
	u8 oc = 0; // overheat control timer

	char msg[0x100];

	old_fan = 0;
	while(working)
	{
		#ifdef REMOVE_SYSCALLS
		// check if syscalls were restored
		if(syscalls_removed != CFW_SYSCALLS_REMOVED(TOC))
		{
			syscalls_removed = CFW_SYSCALLS_REMOVED(TOC);
			#ifdef PS3MAPI
			if(!syscalls_removed) restore_cfw_syscalls();
			#else
			if(!syscalls_removed) disable_signin_dialog();
			#endif
		}
		#endif

		// dynamic fan control
		#include "fancontrol2.h"

		// Poll combos for 3 seconds
		#include "combos.h"

		// detect aprox. time when a game is launched & set network connect status
		#ifdef OFFLINE_INGAME
		if((sec % 3) == 0 || (webman_config->spp & 4)) poll_start_play_time();
		#else
		if((sec % 3) == 0) poll_start_play_time();
		#endif

		// USB Polling
		if((sec >= 120) && !webman_config->poll) // check USB drives each 120 seconds
		{
			usb_keep_awake(oc);
			sec = 0;
		}
		sec += step;

#ifdef PKG_HANDLER
		// Poll downloaded pkg files (if is on XMB)
		if(_IS_ON_XMB_) poll_downloaded_pkg_files(msg);
#endif

#ifdef COBRA_ONLY
		// Poll insert USB
		mount_on_insert_usb(_IS_ON_XMB_, msg);
#endif

#ifdef PHOTO_GUI
		// Poll requests via local file
		if(webman_config->launchpad_xml) continue; // poll wm_request file only if PhotoGUI is enabled

		if(_IS_IN_GAME_) continue; // slow down poll in-game

		if(file_exists(WMREQUEST_FILE))
		{
			loading_html++;
			sys_ppu_thread_t t_id;
			if(working) sys_ppu_thread_create(&t_id, handleclient_www, WM_FILE_REQUEST, THREAD_PRIO, THREAD_STACK_SIZE_WEB_CLIENT, SYS_PPU_THREAD_CREATE_NORMAL, THREAD_NAME_WEB);
		}
#endif

	}

	sys_ppu_thread_exit(0);
}
