// mount NPDRM game  -> /dev_hdd0/game/<titleid>
// mount GAMEI game  -> /dev_usb*/GAMEI/<titleid>
// mount ROMS game   -> /dev_***/ROMS/ or *.SELF or *.ext (.ZIP.GBA.NES.UNIF.GB.GBC.DMG.MD.SMD.GEN.SMS.GG.SG.IOS.FLAC.NGP.NGC.PCE.SGX.VB.VBOY.WS.WSC.FDS.EXE.WAD.IWAD.SMC.FIG.SFC.GD3.GD7.DX2.BSX.SWC.A26.PAK.LUA.ADF.DMS.FDI.IPF.UAE.A78.MGW.LNX.VEC.J64.JAG.PRG.XFD.XEX)
// mount PS2 Classic -> *.BIN.ENC

#ifdef COBRA_ONLY
	// ------------------
	// mount NPDRM game
	// ------------------
	if(islike(_path, "/dev_hdd0/game"))
	{
		set_apphome(_path);

		char col[8], seg[16]; *col = NULL, *seg = NULL; ret = isDir(_path);
		if(is_app_home_onxmb()) {mount_unk = APP_GAME; launch_disc(col, seg, true); ret = true;}

		mount_unk = EMU_MAX;
		goto exit_mount;
	}

	// ------------------
	// mount GAMEI game
	// ------------------
 #ifdef PKG_LAUNCHER
	{
		char *pos = strstr(_path, "/GAMEI/");
		if(pos)
		{
			sys_map_path(PKGLAUNCH_DIR, _path0);
			get_value(map_title_id, pos + 7, TITLE_ID_LEN);
			sprintf(_path, "/dev_hdd0/game/%s", map_title_id);
			sys_map_path(_path, _path0);

			mount_unk = EMU_GAMEI;
			goto exit_mount;
		}
	}
 #endif

	// ------------------
	// mount ROMS game
	// ------------------
 #ifdef MOUNT_ROMS
	if(isDir(PKGLAUNCH_DIR))
	{
		int plen = strlen(_path) - 4;
		if(plen < 0) plen = 0;
		else if(_path[plen + 2] == '.') plen+=2;
		else if(_path[plen + 1] == '.') plen++;

		if((strstr(_path, "/ROMS/") != NULL) || (strcasestr(_path, ".SELF") != NULL) || (strcasestr(ROMS_EXTENSIONS, _path + plen) != NULL))
		{
			do_umount(false);

			sys_map_path(PKGLAUNCH_DIR, NULL);
			sys_map_path(PKGLAUNCH_DIR "/PS3_GAME/USRDIR/cores", "/dev_hdd0/game/SSNE10000/USRDIR/cores");

			cobra_map_game(PKGLAUNCH_DIR, "PKGLAUNCH", 0);

			save_file(PKGLAUNCH_DIR "/USRDIR/launch.txt", _path, 0);
			mount_unk = EMU_ROMS;
			goto mounting_done; //goto exit_mount;
		}
	}
 #endif // #ifdef MOUNT_ROMS

#endif // #ifdef COBRA_ONLY

	// ------------------
	// mount PS2 Classic
	// ------------------
	if(!extcmp(_path, ".BIN.ENC", 8))
	{
		char temp[STD_PATH_LEN + 16];

		if(isDir(PS2_CLASSIC_PLACEHOLDER))
		{
			copy_in_progress = true, copied_count = 0;

			sprintf(temp, "PS2 Classic\n%s", strrchr(_path, '/') + 1);
			show_msg(temp);

 #ifndef LITE_EDITION
			if(c_firmware >= 4.65f)
			{   // Auto create "classic_ps2 flag" for PS2 Classic (.BIN.ENC) on rebug 4.65.2
				do_umount(false);
				enable_classic_ps2_mode();
			}
 #endif
			cellFsUnlink(PS2_CLASSIC_ISO_CONFIG);
			cellFsUnlink(PS2_CLASSIC_ISO_PATH);
			if(file_copy(_path, (char*)PS2_CLASSIC_ISO_PATH, COPY_WHOLE_FILE) == 0)
			{
				if(file_exists(PS2_CLASSIC_ISO_ICON ".bak") == false)
					_file_copy((char*)PS2_CLASSIC_ISO_ICON, (char*)(PS2_CLASSIC_ISO_ICON ".bak"));

				size_t len = sprintf(temp, "%s.png", _path);
				if(file_exists(temp) == false) sprintf(temp, "%s.PNG", _path);
				if(file_exists(temp) == false && len > 12) sprintf(temp + len - 12, ".png"); // remove .BIN.ENC
				if(file_exists(temp) == false && len > 12) sprintf(temp + len - 12, ".PNG");

				cellFsUnlink(PS2_CLASSIC_ISO_ICON);
				if(file_exists(temp))
					_file_copy(temp, (char*)PS2_CLASSIC_ISO_ICON);
				else
					_file_copy((char*)(PS2_CLASSIC_ISO_ICON ".bak"), (char*)PS2_CLASSIC_ISO_ICON);

				sprintf(temp, "%s.CONFIG", _path);
				if(file_exists(temp) == false && len > 12) strcat(temp + len - 12, ".CONFIG\0");
				if(file_exists(temp) == false)
				{
					char *game_id = strstr(_path, "[SL"); 
					if(!game_id)
						  game_id = strstr(_path, "[SC");
					if(game_id)
					{
						if(game_id[4] == '_')
							sprintf(temp, "/dev_hdd0/game/PS2CONFIG/USRDIR/CONFIG/ENC/%.11s.ENC", game_id + 1); //[SLxS_000.00]
						else
							sprintf(temp, "/dev_hdd0/game/PS2CONFIG/USRDIR/CONFIG/ENC/%.4s_%3s.%.2s.ENC",
											game_id + 1,  // SLES, SLUS, SLPM, SLPS, SCES, SCUS, SCPS
											game_id + 5,  // _000.00
											game_id + 8); // [SLxS00000]
					}
				}

				cellFsUnlink(PS2_CLASSIC_ISO_CONFIG);
				_file_copy(temp, (char*)PS2_CLASSIC_ISO_CONFIG);

				if(webman_config->fanc) restore_fan(SET_PS2_MODE); //fan_control( ((webman_config->ps2temp*255)/100), 0);

				// create "wm_noscan" to avoid re-scan of XML returning to XMB from PS2
				save_file(WMNOSCAN, NULL, 0);

				sprintf(temp, "\"%s\" %s", strrchr(_path, '/') + 1, STR_LOADED2);
			}
			else
				{sprintf(temp, "PS2 Classic\n%s", STR_ERROR); ret = false;}

			copy_in_progress = false;
		}
		else
		{
			sprintf(temp, "PS2 Classic Placeholder %s", STR_NOTFOUND);
			ret = false;
		}

		show_msg(temp);
		goto exit_mount;
	}

 #ifndef LITE_EDITION
	if((c_firmware >= 4.65f) && strstr(_path, "/PS2ISO")!=NULL)
	{   // Auto remove "classic_ps2" flag for PS2 ISOs on rebug 4.65.2
		disable_classic_ps2_mode();
	}
 #endif
