// PS3GAME.INI flag or [gd] tag -> auto-map /dev_hdd0/game to dev_usbxxx/GAMEI for external game data / patches
// mount hdd0   -> /GAMES /GAMEZ /PS3ISO /PSXISO /PSXGAMES /PS2ISO /PSPISO /BDISO /DVDISO
// mount usb*   -> /GAMES /GAMEZ /PS3ISO /PSXISO /PSXGAMES /PSPISO /BDISO /DVDISO
//       iso.*  -> support split-iso
// mount ntfs   -> .ntfs[PS3ISO] .ntfs[PSXISO] .ntfs[PSPISO] .ntfs[BDISO] .ntfs[DVDISO] .ntfs[BDFILE]
//          ps2 -> are cached to hdd0
//       psxiso -> use rawseciso plugin by default on NTFS due multi-disc support
// mount net    -> /net0/PS3ISO /net0/PSXISO /net0/PSXGAMES /net0/PSPISO /net0/BDISO /net0/DVDISO /net0/GAMES /net0/PKG
//          ps2 -> are cached to hdd0
//    Dump with /copy.ps3/net0/***PS3***/GAMES/BLES12345  -> /dev_hdd0/PS3ISO/BLES12345.iso
//    Dump with /copy.ps3/net0/***DVD***/folder           -> /dev_hdd0/DVDISO/folder.iso

#ifdef COBRA_ONLY
	if(!cobra_version) {ret = false; goto finish;}
	{
		// --------------------------------------------
		// auto-map /dev_hdd0/game to dev_usbxxx/GAMEI
		// --------------------------------------------
		 #ifdef EXT_GDATA
		{
			// auto-enable external GD
			if(action != MOUNT_NORMAL) ;

			else if(strstr(_path, "/GAME"))
			{
				char extgdfile[strlen(_path) + 24], *extgdini = extgdfile;
				sprintf(extgdfile, "%s/PS3_GAME/PS3GAME.INI", _path); check_ps3_game(extgdfile);
				if(read_file(extgdfile, extgdini, 12, 0))
				{
					if((extgd == 0) &&  (extgdini[10] & (1<<1))) set_gamedata_status(1, false); else
					if((extgd == 1) && !(extgdini[10] & (1<<1))) set_gamedata_status(0, false);
				}
				else if(extgd) set_gamedata_status(0, false);
			}
			else if((strstr(_path, "PS3ISO") != NULL) && (strstr(_path, "[gd]") != NULL))
			{
				if(extgd == 0) set_gamedata_status(1, false);
			}
			else if(extgd)
				set_gamedata_status(0, false);
		}
		 #endif //#ifdef EXT_GDATA

		// ------------
		// get /net id
		// ------------

		if(islike(_path, "/net")) netid = _path[4];

	mount_again:

		// ---------------------
		// unmount current game
		// ---------------------

		do_umount(false);

		// ----------
		// mount iso
		// ----------
		if(!isDir(_path))
		{
			char templn[MAX_LINE_LEN];

			#ifdef MOUNT_PNG
			bool is_PNG = is_ext(_path, ".png");

			if(is_PNG && (mount_unk == EMU_OFF)) {read_file(_path, templn, 1, 0xFFE0); mount_unk = *templn;}
			#endif

			if( strstr(_path, "/PSXISO") || strstr(_path, "/PSXGAMES") || !extcmp(_path, ".ntfs[PSXISO]", 13) || (mount_unk == EMU_PSX)) {mount_unk = EMU_PSX; select_ps1emu(_path);}

			sys_ppu_thread_sleep(1);

			// --------------
			// get ISO parts
			// --------------

			u8 iso_parts = 1;

			size_t path_len = sprintf(templn, "%s", _path);

			CD_SECTOR_SIZE_2352 = 2352;

			if(is_iso_0(_path))
			{
				// count iso_parts
				for(; iso_parts < MAX_ISO_PARTS; iso_parts++)
				{
					#ifdef MOUNT_PNG
					if(is_PNG)
						sprintf(templn + path_len - 6, ".%i.PNG", iso_parts);
					else
					#endif
						sprintf(templn + path_len - 2, ".%i", iso_parts);

					if(not_exists(templn)) break;
				}
				#ifdef MOUNT_PNG
				if(is_PNG)
					templn[path_len - 6] = NULL; // remove .0.PNG
				else
				#endif
					templn[path_len - 2] = NULL; // remove .0
			}

			char *cobra_iso_list[iso_parts], iso_list[iso_parts][path_len + 2];

			change_cue2iso(_path);
			sprintf(iso_list[0], "%s", _path);
			cobra_iso_list[0] = (char*)iso_list[0];

			for(u8 n = 1; n < iso_parts; n++)
			{
				sprintf(iso_list[n], "%s.%i", templn, n);
				#ifdef MOUNT_PNG
				if(is_PNG) strcat(iso_list[n], ".PNG"); // .iso.#.PNG
				#endif
				cobra_iso_list[n] = (char*)iso_list[n];
			}

			// -----------------------
			// mount extFAT / NTFS ISO
			// -----------------------

			char *ntfs_ext = strstr(_path, ".ntfs[");
			if(ntfs_ext)
			{
				set_mount_type(_path);

	#ifdef USE_INTERNAL_NTFS_PLUGIN
				// ------------------------------------------------------------------------------------------------------------
				// launch ntfs psx & isos tagged [raw] with external rawseciso sprx (if available) (due support for multi PSX)
				// ------------------------------------------------------------------------------------------------------------
				if(islike(ntfs_ext, ".ntfs[PSXISO]") || (!strstr(_path, "[raw]")))
	#endif
				{
					if(multiCD) check_multipsx = !isDir("/dev_usb000"); // check eject/insert USB000 in mount_on_insert_usb()

					const char *rawseciso_sprx[3] = { WM_RES_PATH "/raw_iso.sprx",
													  VSH_MODULE_DIR "raw_iso.sprx",
													  WMTMP "/res/sman.ntf" };

					u8 n;
					for(n = 0; n < 3; n++)
						if(file_exists(rawseciso_sprx[n])) break;

					if(n < 3)
					{
						cellFsChmod(_path, MODE);

						sys_addr_t addr = 0;
						if(sys_memory_allocate(_64KB_, SYS_MEMORY_PAGE_SIZE_64K, &addr) == CELL_OK)
						{
							char *rawseciso_data = (char *)addr;
							u64 msiz = read_file(_path, rawseciso_data, _64KB_, 0);
							if(msiz > sizeof(rawseciso_args))
							{
								ret = (cobra_load_vsh_plugin(0, (char*)rawseciso_sprx[n], rawseciso_data, msiz) == CELL_OK);
								sys_ppu_thread_sleep(1);
							}
							else
								ret = false;

							sys_memory_free(addr);

	#ifdef USE_INTERNAL_NTFS_PLUGIN
							if(ret) goto mounted_ntfs;
	#endif
						}
					}
				}

	#ifdef USE_INTERNAL_NTFS_PLUGIN
				sys_addr_t addr = 0;
				if(sys_memory_allocate(_64KB_, SYS_MEMORY_PAGE_SIZE_64K, &addr) == CELL_OK)
				{
					char *rawseciso_data = (char*)addr;
					if(read_file(_path, rawseciso_data, _64KB_, 0) > sizeof(rawseciso_args))
					{
						sys_ppu_thread_create(&thread_id_ntfs, rawseciso_thread, (u64)addr, THREAD_PRIO, THREAD_STACK_SIZE_NTFS_ISO, SYS_PPU_THREAD_CREATE_JOINABLE, THREAD_NAME_NTFS);

						sys_ppu_thread_sleep(1);
					}
					else
						ret = false;

					sys_memory_free(addr);
				}
	#endif //#ifdef USE_INTERNAL_NTFS_PLUGIN

		mounted_ntfs:

				if(ret)
				{
					wait_for("/dev_bdvd", 3);

					if(IS(ntfs_ext, ".ntfs[PS3ISO]"))
					{
						get_name(templn, _path, NO_EXT);
						cache_icon0_and_param_sfo(templn);
						set_bdvd_as_app_home(); // mount (NTFS) PS3ISO in /app_home
	#ifdef FIX_GAME
						fix_game(_path, title_id, webman_config->fixgame);
	#endif
					}

#ifdef PKG_HANDLER
					if(!(webman_config->auto_install_pkg) && IS(ntfs_ext, ".ntfs[BDFILE]") && islike(ntfs_ext - 4, ".pkg"))
					{
						installPKG_all("/dev_bdvd", false);
						goto exit_mount;
					}
#endif
					// cache PS2ISO or PSPISO to HDD0
					bool is_ps2 = IS(ntfs_ext, ".ntfs[PS2ISO]");
					bool is_psp = IS(ntfs_ext, ".ntfs[PSPISO]");

					if(is_psp || is_ps2)
					{
						int fd;

						if(cellFsOpendir("/dev_bdvd", &fd) == CELL_FS_SUCCEEDED)
						{
							CellFsDirent entry; u64 read_e;

							while((cellFsReaddir(fd, &entry, &read_e) == CELL_FS_SUCCEEDED) && (read_e > 0))
							{
								if(entry.d_name[0] != '.') break;
							}
							cellFsClosedir(fd);

							if(entry.d_name[0] == NULL) goto exit_mount;

							if(is_psp)
							{
								*ntfs_ext = 0;
								sprintf(templn, "%s", _path);
								*ntfs_ext = '.';
							}

							sprintf(_path, "/dev_bdvd/%s", entry.d_name);

							if(not_exists(_path)) goto exit_mount;

							if(is_psp)
							{
								unlock_psp_launchers();
								ret = (cobra_set_psp_umd(_path, (char*)templn, (char*)"/dev_hdd0/tmp/wm_icons/psp_icon.png") == CELL_FS_SUCCEEDED);
								//goto copy_pspiso_to_hdd0;
							}
							else //if(is_ps2)
								goto copy_ps2iso_to_hdd0;
						}
					}
				}
				goto exit_mount;
			}

	 #ifdef NET_SUPPORT

			// -----------------------
			// mount /net ISO or path
			// -----------------------

			if(netid >= '0' && netid <= '4')
			{
				netiso_svrid = (netid & 0x0F);
				memset((void*)&netiso_args, 0, sizeof(_netiso_args));
/*
				if(is_netsrv_enabled(netiso_svrid) == false)
				{
					ret = false;
					goto exit_mount;
				}
*/
				if(_path[5] == NULL) strcat(_path, "/.");

				char *netpath = _path + 5, *pkg_slash = NULL;

				size_t len = sprintf(netiso_args.path, "%s", netpath);

				bool is_iso = false;
				char *ext = strrchr(netpath, '.');

				if(ext)
				{
					if(strlen(ext) == 4 || islike(ext, ".0"))
						is_iso = (strcasestr(ISO_EXTENSIONS, ext) != NULL);
				}

				mount_unk = netiso_args.emu_mode = EMU_BD;

				if(islike(netpath, "/PS3ISO") && is_iso) mount_unk = netiso_args.emu_mode = EMU_PS3; else
				if(islike(netpath, "/BDISO" ) && is_iso) mount_unk = netiso_args.emu_mode = EMU_BD;  else
				if(islike(netpath, "/DVDISO") && is_iso) mount_unk = netiso_args.emu_mode = EMU_DVD; else
				if(islike(netpath, "/PS2ISO") && is_iso) goto copy_ps2iso_to_hdd0;                   else
			//	if(islike(netpath, "/PSPISO") && is_iso) goto copy_pspiso_to_hdd0;                   else
				if(islike(netpath, "/PSPISO") && is_iso)
				{
					//mount_unk = netiso_args.emu_mode = EMU_BD;
					sprintf(netiso_args.path, "/***DVD***%s", "/PSPISO");
				}
				else if(islike(netpath, "/PSX") && is_iso)
				{
					TrackDef tracks[MAX_TRACKS];
					unsigned int num_tracks = 1;

					int ns = connect_to_remote_server(netiso_svrid);
					if(ns >= 0)
					{
						cellFsUnlink(TEMP_NET_PSXCUE);
						const char *cue_ext[4] = {".cue", ".ccd", ".CUE", ".CCD"};
						for(u8 e = 0; e < 4; e++)
						{
							strcpy(netiso_args.path + len - 4, cue_ext[e]);
							if(copy_net_file(TEMP_NET_PSXCUE, netiso_args.path, ns, _8KB_) == CELL_OK) break;
						}
						sclose(&ns);

						if(file_exists(TEMP_NET_PSXCUE))
						{
							char *cue_buf = malloc(_4KB_);
							if(cue_buf)
							{
								u16 cue_size = read_sfo(TEMP_NET_PSXCUE, cue_buf);
								cellFsUnlink(TEMP_NET_PSXCUE);

								num_tracks = parse_cue(templn, cue_buf, cue_size, tracks);
								free(cue_buf);
							}
						}
					}

					mount_unk = netiso_args.emu_mode = EMU_PSX;
					netiso_args.num_tracks = num_tracks;
					sprintf(netiso_args.path, "%s", netpath);

					ScsiTrackDescriptor *scsi_tracks = (ScsiTrackDescriptor *)netiso_args.tracks;

					scsi_tracks[0].adr_control = 0x14;
					scsi_tracks[0].track_number = 1;
					scsi_tracks[0].track_start_addr = 0;

					for(unsigned int t = 1; t < num_tracks; t++)
					{
						scsi_tracks[t].adr_control = 0x10;
						scsi_tracks[t].track_number = t + 1;
						scsi_tracks[t].track_start_addr = tracks[t].lba;
					}
				}
				else if((islike(netpath, "/GAMES") || islike(netpath, "/GAMEZ") || islike(netpath, "/PS3ISO")) && (strstr(netpath + 5, "/") != NULL))
				{
					mount_unk = netiso_args.emu_mode = EMU_PS3;
					if(!is_iso) sprintf(netiso_args.path, "/***PS3***%s", netpath);
					set_bdvd_as_app_home(); // mount (NET) PS3ISO in /app_home
				}
				else if(islike(netpath, "/ROMS/") && !is_iso)
				{
					//netiso_args.emu_mode = EMU_BD;
					mount_unk = EMU_ROMS;

					sprintf(netiso_args.path, "/***DVD***%s", "/ROMS");

					sprintf(templn, "/dev_bdvd/%s", netpath + 6);
					save_file(PKGLAUNCH_DIR "/USRDIR/launch.txt", templn, SAVE_ALL);
					copy_rom_media(templn);
				}
				else
				{
					//mount_unk = netiso_args.emu_mode = EMU_BD;
					if(is_ext(netpath, ".pkg"))
					{
						pkg_slash = get_filename(netpath); if(pkg_slash) *pkg_slash = NULL;
					}
					if(is_iso) ;
					else
						sprintf(netiso_args.path, "/***DVD***%s", netpath);
				}

				sprintf(netiso_args.server, "%s", webman_config->neth[netiso_svrid]);
				netiso_args.port = webman_config->netp[netiso_svrid];

				u8 n;
				const char *netiso_sprx[3] = { WM_RES_PATH "/netiso.sprx",
											   VSH_MODULE_DIR "netiso.sprx",
											   WMTMP "/res/sman.net"};

				for(n = 0; n < 3; n++)
					if(file_exists(netiso_sprx[n])) break;
retry_net:
				if((n < 3) && (!strstr(_path, "[net]")))
				{
					ret = (cobra_load_vsh_plugin(0, (char*)netiso_sprx[n], &netiso_args, sizeof(_netiso_args)) == CELL_OK);
				}
#ifdef USE_INTERNAL_NET_PLUGIN
				else
				{
					sys_ppu_thread_create(&thread_id_net, netiso_thread, 0, THREAD_PRIO, THREAD_STACK_SIZE_NET_ISO, SYS_PPU_THREAD_CREATE_JOINABLE, THREAD_NAME_NET);
				}
#endif
				if(ret)
				{
					if(netiso_args.emu_mode == EMU_BD)
					{
						wait_for("/dev_bdvd", 15);
						if(isDir("/dev_bdvd/PS3_GAME"))
						{
							do_umount(false);
							if(!is_iso) sprintf(netiso_args.path, "/***PS3***%s", netpath);
							netiso_args.emu_mode = mount_unk = EMU_PS3;
							goto retry_net; // mount as PS3
						}
					}

					if(netiso_args.emu_mode == EMU_PS3)
					{
						wait_for("/dev_bdvd", 15);

						get_name(templn, _path, GET_WMTMP);
						cache_icon0_and_param_sfo(templn);

						#ifdef FIX_GAME
						fix_game(_path, title_id, webman_config->fixgame);
						#endif
					}

					else if(is_iso && islike(netpath, "/PSPISO"))
					{
						mount_unk = EMU_PSP;
						unlock_psp_launchers();

						sprintf(templn, "/dev_bdvd/%s", netpath + 8);
						sprintf(_path,  "/dev_bdvd/%s", netpath + 8);

						sys_ppu_thread_sleep(1);

						ret = (cobra_set_psp_umd(_path, templn, (char*)"/dev_hdd0/tmp/wm_icons/psp_icon.png") == CELL_FS_SUCCEEDED);
					}

					else if(islike(netpath, "/ROMS/"))
					{
						mount_unk = EMU_ROMS;

						wait_for("/dev_bdvd", 15);

						sys_map_path(PKGLAUNCH_DIR, NULL);
						set_app_home (PKGLAUNCH_PS3_GAME);

						sys_map_path("/dev_bdvd/PS3_GAME", PKGLAUNCH_PS3_GAME);
						sys_map_path("/dev_bdvd/PS3_GAME/USRDIR/cores", isDir( RETROARCH_DIR0 ) ? RETROARCH_DIR0 "/USRDIR/cores" :
																		isDir( RETROARCH_DIR1 ) ? RETROARCH_DIR1 "/USRDIR/cores" :
																								  RETROARCH_DIR2 "/USRDIR/cores" );
					}

					else if(islike(netpath, "/GAMEI/"))
					{
						mount_unk = EMU_PS3;

						wait_for("/dev_bdvd", 15);
						sys_map_path(APP_HOME_DIR, "/dev_bdvd");

						sys_ppu_thread_sleep(1);
						launch_app_home_icon(webman_config->autoplay);

						mount_unk = EMU_GAMEI;
						goto exit_mount;
					}
#ifdef PKG_HANDLER
					else if(!(webman_config->auto_install_pkg) && (pkg_slash != NULL))
					{
						installPKG_all("/dev_bdvd", false);
					}
#endif
				}
				goto exit_mount;
			}

	#endif // #ifdef NET_SUPPORT

			// ------------------------------------------------------------------
			// mount PS3ISO / PSPISO / PS2ISO / DVDISO / BDISO stored on hdd0/usb
			// ------------------------------------------------------------------
			{
				ret = file_exists(iso_list[0]); if(!ret) goto exit_mount;

				// --------------
				// mount PS3 ISO
				// --------------

				if(strstr(_path, "/PS3ISO") || (mount_unk == EMU_PS3))
				{
	#ifdef FIX_GAME
					if(webman_config->fixgame != FIX_GAME_DISABLED)
					{
						fix_in_progress=true; fix_aborted = false;
						fix_iso(_path, 0x100000UL, true);
						fix_in_progress=false;
					}
	#endif //#ifdef FIX_GAME

					mount_unk = EMU_PS3;

					cobra_mount_ps3_disc_image(cobra_iso_list, iso_parts);
					sys_ppu_thread_usleep(2500);
					cobra_send_fake_disc_insert_event();
					set_bdvd_as_app_home(); // mount (normal) PS3ISO in /app_home

					{
						get_name(templn, _path, GET_WMTMP);
						cache_icon0_and_param_sfo(templn);
					}
				}

				// --------------
				// mount PSP ISO
				// --------------

				else if(strstr(_path, "/PSPISO") || strstr(_path, "/ISO/") || (mount_unk == EMU_PSP))
				{
					if(netid)
					{
	//copy_pspiso_to_hdd0:
						cache_file_to_hdd(_path, iso_list[0], "/PSPISO", templn);
					}

					mount_unk = EMU_PSP;

					unlock_psp_launchers();
					cobra_unset_psp_umd();

					if(file_exists(iso_list[0]))
					{
						int edat = 0;

						sprintf(templn, "%s.MINIS.EDAT", iso_list[0]);
						if(file_exists(templn))
						{
							if(isDir(PSP_LAUNCHER_MINIS))
							{
								sprintf(iso_list[1], "/%s", PSP_LAUNCHER_MINIS "/USRDIR/MINIS.EDAT");
								_file_copy(templn, iso_list[1]);
								edat = read_file(iso_list[1], templn, 4, 0);
							}

							if(isDir(PSP_LAUNCHER_REMASTERS))
							{
								sprintf(iso_list[1], "/%s", PSP_LAUNCHER_REMASTERS "/USRDIR/MINIS.EDAT");
								_file_copy(templn, iso_list[1]);
								edat = read_file(iso_list[1], templn, 4, 0);
							}
						}

						sprintf(templn, "%s.MINIS2.EDAT", iso_list[0]);
						if(file_exists(templn))
						{
							if(isDir(PSP_LAUNCHER_REMASTERS))
							{
								sprintf(iso_list[1], "/%s", PSP_LAUNCHER_REMASTERS "/USRDIR/MINIS2.EDAT");
								_file_copy(templn, iso_list[1]);
								edat = read_file(iso_list[1], templn, 4, 0);
							}
						}

						#ifndef LITE_EDITION
						// restore original psp_emulator.self (if it's swapped)
						swap_file(PSP_EMU_PATH, "psp_emulator.self", "psp_emulator.self.dec_edat", "psp_emulator.self.original");

						// check if decrypted MINIS.EDAT is detected
						if(edat)
						{
							if(!islike(templn, "NPD") && !payload_ps3hen)
							{
								// install psp_emulator.self with support for decrypted MINIS.EDAT
								if((c_firmware >= 4.82f) && file_exists(WM_RES_PATH "/psp_emulator.self"))
								{
									if(not_exists("/dev_flash/pspemu/psp_emulator.self.dec_edat")
									&& not_exists("/dev_flash/pspemu/psp_emulator.self.original"))
									{
										enable_dev_blind(NULL);
										_file_copy((char*)(WM_RES_PATH "/psp_emulator.self"), (char*)"/dev_blind/pspemu/psp_emulator.self.dec_edat");
									}
								}

								// swap psp_emulator.self if decrypted MINIS.EDAT is detected & psp_emulator.self.dec_edat is installed
								swap_file(PSP_EMU_PATH, "psp_emulator.self", "psp_emulator.self.original", "psp_emulator.self.dec_edat");
								show_msg("MINIS.EDAT is decrypted!");
							}
						}
						#endif

						int result = cobra_set_psp_umd(iso_list[0], NULL, (char*)"/dev_hdd0/tmp/wm_icons/psp_icon.png");

						if(result) ret = false;
					}
					else
						ret = false;
				}

				// --------------
				// mount PS2 ISO
				// --------------

				else if(strstr(_path, "/PS2ISO") || (mount_unk == EMU_PS2_DVD))
				{
					if(!islike(_path, "/dev_hdd0"))
					{
	copy_ps2iso_to_hdd0:
						cache_file_to_hdd(_path, iso_list[0], "/PS2ISO", templn);
					}

					webman_config->ps2emu = pad_select_netemu(_path, webman_config->ps2emu);

					enable_ps2netemu_cobra(webman_config->ps2emu); // 0 = ps2emu, 1 = ps2_netemu

					mount_unk = EMU_PS2_DVD;

					if(file_exists(iso_list[0]))
					{
						cellFsUnlink(TMP_DIR "/loadoptical"); //Cobra 8.x

						#ifndef LITE_EDITION
						// Auto-copy CONFIG from ManaGunZ
						sprintf(templn, "%s", _path);
						sprintf(_path, "%s.CONFIG", iso_list[0]);
						if(!webman_config->ps2config && not_exists(_path))
						{
							mount_ps_disc_image(templn, cobra_iso_list, iso_parts, EMU_PS2_DVD);
							sys_ppu_thread_usleep(2500);
							cobra_send_fake_disc_insert_event();

							char *id = strstr(iso_list[0], " ["); if(!id) id = strstr(iso_list[0], " (");
							if(id)
							{
								char title_id[12], game_id[12];
								get_ps_titleid_from_path(title_id, templn);
								sprintf(game_id, "%.4s_%.3s.%.2s", title_id, title_id + 4, title_id + 7);
								copy_ps2config_iso(game_id, _path);
							}
							else
							{
								wait_for("/dev_bdvd", 5);

								int fd;
								if(cellFsOpendir("/dev_bdvd", &fd) == CELL_FS_SUCCEEDED)
								{
									CellFsDirectoryEntry dir; u32 read_e;
									char *entry_name = dir.entry_name.d_name;

									while(working && (!cellFsGetDirectoryEntries(fd, &dir, sizeof(dir), &read_e) && read_e))
									{
										if( ((entry_name[0] | 0x20) == 's') && (dir.entry_name.d_namlen == 11) )
										{
											if(copy_ps2config_iso(entry_name, _path)) break;
										}
									}
									cellFsClosedir(fd);
								}
							}

							if(file_exists(_path)) {do_umount(false); wait_path("/dev_bdvd", 5, false);} else mount_unk = EMU_PS2_CD; // prevent mount ISO again if CONFIG was not created
						}
						#endif

						// mount PS2 ISO
						if(mount_unk == EMU_PS2_DVD)
							mount_ps_disc_image(templn, cobra_iso_list, iso_parts, EMU_PS2_DVD);

						// create "wm_noscan" to avoid re-scan of XML returning to XMB from PS2
						save_file(WMNOSCAN, NULL, SAVE_ALL);

						if(mount_unk == EMU_PS2_CD) goto exit_mount; // don't call cobra_send_fake_disc_insert_event again
					}
					else
						ret = false;
				}

				// --------------
				// mount PSX ISO
				// --------------

				else if(strstr(_path, "/PSXISO") || strstr(_path, "/PSXGAMES") || (mount_unk == EMU_PSX))
				{
					mount_unk = EMU_PSX;
					ret = mount_ps_disc_image(_path, cobra_iso_list, 1, EMU_PSX); if(multiCD) check_multipsx = !isDir("/dev_usb000"); // check eject/insert USB000 in mount_on_insert_usb()
				}

				// -------------------
				// mount DVD / BD ISO
				// ------------------

				else if(strstr(_path, "/BDISO")  || (mount_unk == EMU_BD))
				{
					mount_unk = EMU_BD;
					cobra_mount_bd_disc_image(cobra_iso_list, iso_parts);
				}
				else if(strstr(_path, "/DVDISO") || (mount_unk == EMU_DVD))
				{
					mount_unk = EMU_DVD;
					cobra_mount_dvd_disc_image(cobra_iso_list, iso_parts);
				}
				else
				{
					// mount iso as data
					cobra_mount_bd_disc_image(cobra_iso_list, iso_parts);
					sys_ppu_thread_usleep(2500);
					cobra_send_fake_disc_insert_event();

					if(!mount_unk)
					{
						wait_for("/dev_bdvd", 5);

						// re-mount using proper media type
						if(isDir("/dev_bdvd/PS3_GAME")) mount_unk = EMU_PS3; else
						if(isDir("/dev_bdvd/PS3_GM01")) mount_unk = EMU_PS3; else
						if(isDir("/dev_bdvd/VIDEO_TS")) mount_unk = EMU_DVD; else
						if(file_exists("/dev_bdvd/SYSTEM.CNF") || strcasestr(_path, "PS2")) mount_unk = EMU_PS2_DVD; else
						if(strcasestr(_path, "PSP")!=NULL && is_ext(_path, ".iso")) mount_unk = EMU_PSP; else
						if(!isDir("/dev_bdvd")) mount_unk = EMU_PSX; // failed to mount PSX CD as bd disc

						if(mount_unk) goto mount_again;
					}

					mount_unk = EMU_BD;
				}

				// ----------------------------------------------------------------------------------------
				// send_fake_disc_insert_event for mounted ISOs (PS3ISO/PS2ISO/PSXISO/PSPISO/BDISO/DVDISO)
				// ----------------------------------------------------------------------------------------
				sys_ppu_thread_usleep(2500);
				cobra_send_fake_disc_insert_event();

				//goto exit_mount;
			}
		}

		// ------------------
		// mount folder (JB)
		// ------------------

		else
		{
		#ifdef EXTRA_FEAT
			pad_data = pad_read(); // hold select to eject disc

			int special_mode = 0;
			if(pad_data.len > 0 && (pad_data.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_SELECT)) special_mode = true; //mount also app_home / eject disc

			if(special_mode) eject_insert(1, 0);
		#endif

			// -- fix game & get TitleID from PARAM.SFO
		#ifdef FIX_GAME
			fix_game(_path, title_id, webman_config->fixgame);
		#else
			char filename[STD_PATH_LEN + 20];

			sprintf(filename, "%s/PS3_GAME/PARAM.SFO", _path);
			getTitleID(filename, title_id, GET_TITLE_ID_ONLY);
		#endif
			// ----

			// -- reset USB bus
			if(!webman_config->bus)
			{
				if(islike(_path, "/dev_usb") && isDir(_path))
				{
					reset_usb_ports(_path);
				}
			}

			// -- mount game folder
			bool is_gameid = (*title_id >= 'A' && *title_id <= 'Z') && ISDIGIT(title_id[8]);
			if (!is_gameid)
				sprintf(title_id, "TEST00000");

			cobra_map_game(_path, title_id, mount_app_home | !(webman_config->app_home));

			mount_unk = EMU_PS3;
		}

		//goto exit_mount;
	}
#endif
