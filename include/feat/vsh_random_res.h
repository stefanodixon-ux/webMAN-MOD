#ifdef VISUALIZERS

#define MAP_SELECTED	0x00
#define DEFAULT_RES		0xFF

static u8 map_vsh_resource(u8 res_id, u8 id, char *param, u8 set)
{
	const char *hdd_path = vsh_res_path[res_id];
							// TMP_DIR "/wallpaper"     [0]
							// TMP_DIR "/earth"         [1]
							// TMP_DIR "/canyon"        [2]
							// TMP_DIR "/lines"         [3]
							// TMP_DIR "/coldboot"      [4]
							// TMP_DIR "/theme"         [5] (6 = last selected theme)
							// TMP_DIR "/impose"        [7]
							// TMP_DIR "/psn_icons"     [8]
							// TMP_DIR "/system_plugin" [9]

	const char *res_path =  (res_id == 1) ? "qgl/earth.qrc" :
							(res_id == 2) ? "qgl/canyon.qrc":
							(res_id == 3) ? "qgl/lines.qrc" :
							(res_id == 4) ? "coldboot_stereo.ac3":
							(res_id == 8) ? "xmb_plugin_normal.rco":
							(res_id == 9) ? "system_plugin.rco":
											"impose_plugin.rco"; // 7

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
				sprintf(param, "%s/%i%s", hdd_path, _id, ".png"); // wallpaper
			else if(res_id == 4)
				sprintf(param, "%s/%i%s", hdd_path, _id, ".ac3"); // coldboot
			else if(res_id == 5)
			{
				if(_id == webman_config->resource_id[6]) continue;
				sprintf(param, "%s/%i%s", hdd_path, _id, ".p3t"); // theme
			}
			else if(res_id == 7 || res_id == 9)
				sprintf(param, "%s/%i%s", hdd_path, _id, ".rco"); // impose
			else if(res_id == 8)
				sprintf(param, "%s/%i%s", hdd_path, _id, "/xmb_plugin_normal.rco"); // psn_icons
			else
				sprintf(param, "%s/%i%s", hdd_path, _id, ".qrc"); // lines, earth, canyon

			if(id == DEFAULT_RES) break; loop = 1;
		}
		while(_id && not_exists(param));

		if(file_exists(param))
		{
			if(res_id == 5)
			{
				if(set || (_id > 0))
				{
					int fd; bool do_scan = true; char entry[0x200];
					while(do_scan)
					{
						do_scan = false;
						if(cellFsOpendir("/dev_hdd0/theme", &fd) == CELL_FS_SUCCEEDED)
						{
							CellFsDirent dir; u64 read_e;
							while((cellFsReaddir(fd, &dir, &read_e) == CELL_FS_SUCCEEDED) && (read_e > 0))
							{
								if(islike(dir.d_name, "CD_")) {do_scan = true; snprintf(entry, 0x200, "%s/%s", "/dev_hdd0/theme", dir.d_name); cellFsUnlink(entry);}
							}
							cellFsClosedir(fd);
						}
					}
					wait_for_xmb();
					installPKG(param, entry);
					set = save = true, webman_config->resource_id[6] = _id; // last selected theme
				}
			}
			else if(res_id)
			{
				sys_map_path(strconcat(VSH_RESOURCE_DIR, res_path), param);
				if(res_id == 4)
					sys_map_path("/dev_flash/vsh/resource/coldboot_multi.ac3",  param);
				if(res_id == 8)
				{
					sprintf(param, "%s/%i/xmb_ingame.rco", hdd_path, _id); // psn_icons
					sys_map_path("/dev_flash/vsh/resource/xmb_ingame.rco",  param);
					sprintf(param, "%s/%i%s", hdd_path, _id, ".png"); // show preview
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
				unmap_path("/dev_flash/vsh/resource/xmb_ingame.rco");
			if(res_id == 5)
				webman_config->resource_id[6] = 0; // reset last selected theme
			else
				concat2(param, VSH_RESOURCE_DIR, res_path);
			unmap_path(param);
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
