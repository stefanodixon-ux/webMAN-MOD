#ifdef VISUALIZERS

#define MAP_SELECTED	0x00
#define DEFAULT_RES		0xFF

static u8 map_vsh_resource(u8 res_id, u8 id, char *param, u8 set)
{
	const char *hdd_path =  (res_id == 0) ? "/dev_hdd0/tmp/wallpaper": // 0
							(res_id == 1) ? "/dev_hdd0/tmp/earth"    : // 1
							(res_id == 2) ? "/dev_hdd0/tmp/canyon"   : // 2
							(res_id == 3) ? "/dev_hdd0/tmp/lines"    : // 3
							(res_id == 4) ? "/dev_hdd0/tmp/coldboot" : // 4
							(res_id == 7) ? "/dev_hdd0/tmp/impose"   : // 7
							(res_id == 8) ? "/dev_hdd0/tmp/psn_icons": // 8
							(res_id == 9) ? "/dev_hdd0/tmp/system_plugin": // 9
											"/dev_hdd0/tmp/theme";     // 5 & 6 (last selected theme)

	const char *res_path =  (res_id == 1) ? "/dev_flash/vsh/resource/qgl/earth.qrc" :
							(res_id == 2) ? "/dev_flash/vsh/resource/qgl/canyon.qrc":
							(res_id == 3) ? "/dev_flash/vsh/resource/qgl/lines.qrc" :
							(res_id == 4) ? "/dev_flash/vsh/resource/coldboot_stereo.ac3":
							(res_id == 8) ? "/dev_flash/vsh/resource/xmb_plugin_normal.rco":
							(res_id == 9) ? "/dev_flash/vsh/resource/system_plugin.rco":
											"/dev_flash/vsh/resource/impose_plugin.rco"; // 7

	if(isDir(hdd_path))
	{
		if(!set && !id)	// MAP_SELECTED
		{
			id = webman_config->resource_id[res_id];
		}

		u8 _id, save = false;
		if(!id)	// random
		{
			CellRtcTick nTick; cellRtcGetCurrentTick(&nTick);
			_id = nTick.tick % 0x100;
		}
		else
			_id = id;	// fixed

		u8 loop = 0;
		do
		{
			if(loop) _id /= 2;

			if(res_id == 0)
				sprintf(param, "%s/%i.png", hdd_path, _id); // wallpaper
			else if(res_id == 4)
				sprintf(param, "%s/%i.ac3", hdd_path, _id); // coldboot
			else if(res_id == 5)
				sprintf(param, "%s/%i.p3t", hdd_path, _id); // theme
			else if(res_id == 7 || res_id == 9)
				sprintf(param, "%s/%i.rco", hdd_path, _id); // impose
			else if(res_id == 8)
				sprintf(param, "%s/%i/xmb_plugin_normal.rco", hdd_path, _id); // psn_icons
			else
				sprintf(param, "%s/%i.qrc", hdd_path, _id); // lines, earth, canyon

			if(id == DEFAULT_RES) break; loop = 1;
		}
		while(_id && not_exists(param));

		if(file_exists(param))
		{
			if(res_id == 5)
			{
				if((set || (_id > 0)) && (webman_config->resource_id[6] != _id))
				{
					char msg[0x100];
					scan("/dev_hdd0/theme/", false, "CD_*.p3t", SCAN_DELETE, msg); // delete temporary themes
					wait_for_xmb();
					installPKG(param, msg);
					set = save = true, webman_config->resource_id[6] = _id; // last selected theme
				}
			}
			else if(res_id)
			{
				sys_map_path(res_path, param);
				if(res_id == 4)
					sys_map_path("/dev_flash/vsh/resource/coldboot_multi.ac3",  param);
				if(res_id == 8)
				{
					sprintf(param, "%s/%i/xmb_ingame.rco", hdd_path, _id); // psn_icons
					sys_map_path("/dev_flash/vsh/resource/xmb_ingame.rco",  param);
					sprintf(param, "%s/%i.png", hdd_path, _id); // show preview
				}
			}
			else
			{
				char bg[48];
				sprintf(bg, "%s/%08i/theme/wallpaper.png", HDD0_HOME_DIR, xusers()->GetCurrentUserNumber());
				cellFsUnlink(bg);
				return sysLv2FsLink(param, bg);
			}
		}
		else if(res_id)
		{
			if(id != DEFAULT_RES) id = 0;
			if(res_id == 8)
				sys_map_path("/dev_flash/vsh/resource/xmb_ingame.rco",  NULL);
			if(res_id == 5)
				webman_config->resource_id[6] = 0; // reset last selected theme
			else
				strcpy(param, res_path);
			sys_map_path(param, NULL);
		}

		if(set && (save || (webman_config->resource_id[res_id] != id)))
		{
			webman_config->resource_id[res_id] = id;

			save_settings(); // save if setting changed
		}
	}
	return id;
}

static void randomize_vsh_resources(bool apply_theme, char *param)
{
	map_vsh_resource(0, MAP_SELECTED, param, false); // wallpaper.png
	map_vsh_resource(1, MAP_SELECTED, param, false); // earth.qrc
	map_vsh_resource(2, MAP_SELECTED, param, false); // canyon.qrc
	map_vsh_resource(3, MAP_SELECTED, param, false); // lines.qrc
	map_vsh_resource(7, MAP_SELECTED, param, false); // impose_plugin.rco
	map_vsh_resource(8, MAP_SELECTED, param, false); // xmb_plugin_normal.rco + xmb_ingame.rco
	map_vsh_resource(9, MAP_SELECTED, param, false); // system_plugin.rco
	if(!apply_theme) return;
	map_vsh_resource(5, MAP_SELECTED, param, false); // theme.p3t
}
#endif // #ifdef VISUALIZERS
