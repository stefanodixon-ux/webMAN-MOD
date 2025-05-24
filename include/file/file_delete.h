#ifdef COPY_PS3
#include "file_scan.h"

static int del(const char *path, u8 recursive)
{
	return scan(path, recursive, NULL, SCAN_DELETE, NULL);
}
/*
static int del(const char *path, u8 recursive)
{
	if(recursive == RECURSIVE_DELETE) ; else
	if(!sys_admin || !working) return FAILED;

#ifdef USE_NTFS
	if(!isDir(path))
	{
		if(is_ntfs_path(path))
			return ps3ntfs_unlink(ntfs_path(path));
		else
			return cellFsUnlink(path);
	}
#else
	if(!isDir(path)) return cellFsUnlink(path);
#endif

	if(strlen(path) < 11 || islike(path, "/dev_bdvd") || islike(path, "/dev_flash") || islike(path, "/dev_blind")) return FAILED;

	int fd; bool is_ntfs = false;

	copy_aborted = false;

#ifdef USE_NTFS
	struct stat bufn;
	DIR_ITER *pdir;

	if(is_ntfs_path(path))
	{
		pdir = ps3ntfs_opendir(ntfs_path(path));
		if(pdir) is_ntfs = true;
	}
#endif

	if(is_ntfs || cellFsOpendir(path, &fd) == CELL_FS_SUCCEEDED)
	{
		CellFsDirent dir; u64 read_e;

		char entry[STD_PATH_LEN];

		while(working)
		{
#ifdef USE_NTFS
			if(is_ntfs)
			{
				if(ps3ntfs_dirnext(pdir, dir.d_name, &bufn)) break;
				if(dir.d_name[0]=='$' && path[12] == 0) continue;
			}
			else
#endif
			if((cellFsReaddir(fd, &dir, &read_e) != CELL_FS_SUCCEEDED) || (read_e == 0)) break;

			if(copy_aborted) break;
			if(dir.d_name[0] == '.' && (dir.d_name[1] == '.' || dir.d_name[1] == '\0')) continue;

			sprintf(entry, "%s/%s", path, dir.d_name);

			if(isDir(entry))
				{if(recursive) del(entry, recursive);}
#ifdef USE_NTFS
			else if(is_ntfs)
				ps3ntfs_unlink(ntfs_path(entry));
#endif
			else
				cellFsUnlink(entry);
		}

#ifdef USE_NTFS
		if(is_ntfs) ps3ntfs_dirclose(pdir);
		else
#endif
		cellFsClosedir(fd);

		if(copy_aborted) return FAILED;
	}
	else
		return FAILED;

	if(recursive)
	{
#ifdef USE_NTFS
		if(is_ntfs) ps3ntfs_unlink(ntfs_path(path));
		else
#endif
		cellFsRmdir(path);
	}

	return CELL_FS_SUCCEEDED;
}*/
#endif

static void unlink_file(const char *drive, const char *path, const char *file)
{
	char filename[64];
	concat_path2(filename, drive, path, file); cellFsUnlink(filename);
}

#ifndef LITE_EDITION
#ifdef ARTEMIS_PRX
static void clear_codelist(void);
#endif
static void uninstall(char *param)
{
	if(get_explore_interface()) exec_xmb_command("close_all_list");

	if(file_size("/dev_hdd0/boot_plugins.txt")             < 45) cellFsUnlink("/dev_hdd0/boot_plugins.txt");
	if(file_size("/dev_hdd0/boot_plugins_nocobra.txt")     < 46) cellFsUnlink("/dev_hdd0/boot_plugins_nocobra.txt");
	if(file_size("/dev_hdd0/boot_plugins_nocobra_dex.txt") < 46) cellFsUnlink("/dev_hdd0/boot_plugins_nocobra_dex.txt");

	// delete files
	sprintf(param, "plugins/");
	for(u8 i = 0; i < 2; i++)
	{
		unlink_file(drives[0], param, "webftp_server.sprx");
		unlink_file(drives[0], param, "webftp_server_lite.sprx");
		unlink_file(drives[0], param, "webftp_server_ps3mapi.sprx");
		unlink_file(drives[0], param, "webftp_server_noncobra.sprx");
		unlink_file(drives[0], param, "wm_vsh_menu.sprx");
		unlink_file(drives[0], param, "slaunch.sprx");
		unlink_file(drives[0], param, "raw_iso.sprx");
		*param = NULL;
	}

	unlink_file(TMP_DIR, "wm_vsh_menu", ".cfg");

	cellFsUnlink(WM_CONFIG_FILE);
	cellFsUnlink(WM_NOSCAN_FILE);
	cellFsUnlink(WM_REQUEST_FILE);
	cellFsUnlink(WM_UNLOAD_FILE);
	cellFsUnlink(WM_RELOAD_FILE);
	cellFsUnlink(WM_NETDISABLED);
	cellFsUnlink(WM_ONLINE_IDS_FILE);
	cellFsUnlink(WM_OFFLINE_IDS_FILE);
	#ifdef ARTEMIS_PRX
	clear_codelist();
	#else
	cellFsUnlink(ARTEMIS_CODES_LOG);
	#endif

	#ifdef COPY_PS3
	for(u8 i = 0; i < 4; i++)
		cellFsUnlink(script_events[i]);

	// delete folders & subfolders
	del(WMTMP, RECURSIVE_DELETE);
	del(WM_RES_PATH, RECURSIVE_DELETE);
	del(WM_LANG_PATH, RECURSIVE_DELETE);
	del(WM_ICONS_PATH, RECURSIVE_DELETE);
	del(WM_COMBO_PATH, RECURSIVE_DELETE);
	del(WM_INGAME_PATH, RECURSIVE_DELETE);
	del(HTML_BASE_PATH, RECURSIVE_DELETE);
	del(VSH_MENU_IMAGES, RECURSIVE_DELETE);
	del(PS2CONFIG_PATH, RECURSIVE_DELETE);
	del(WM_GAMEBOOT_PATH, RECURSIVE_DELETE);
	del(WM_EXTRACT_PATH, RECURSIVE_DELETE);

	#ifdef VISUALIZERS
	for(u8 res_id = 0; res_id < 10; res_id++)
		del(vsh_res_path[res_id], RECURSIVE_DELETE);
	#endif

	#endif

	cellFsUnlink(NEW_LIBAUDIO_PATH);
	cellFsUnlink(NEW_EXPLORE_PLUGIN_PATH);

	restore_fan(SYSCON_MODE);

	concat2(param, "/delete.ps3", "?uninstall");
}
#endif

static void delete_history(bool delete_folders)
{
	int fd; char path[64];

	if(cellFsOpendir(HDD0_HOME_DIR, &fd) == CELL_FS_SUCCEEDED)
	{
		CellFsDirectoryEntry dir; u32 read_e;
		const char *entry_name = dir.entry_name.d_name;

		while(working && (!cellFsGetDirectoryEntries(fd, &dir, sizeof(dir), &read_e) && read_e))
		{
			unlink_file(HDD0_HOME_DIR, entry_name, "/etc/boot_history.dat");
			unlink_file(HDD0_HOME_DIR, entry_name, "/community/CI.TMP");
			unlink_file(HDD0_HOME_DIR, entry_name, "/community/MI.TMP");
			unlink_file(HDD0_HOME_DIR, entry_name, "/community/PTL.TMP");
		}
		cellFsClosedir(fd);
	}

	unlink_file(drives[0], "vsh/pushlist/", "game.dat");
	unlink_file(drives[0], "vsh/pushlist/", "patch.dat");

	if(!delete_folders || !working) return;

	for(u8 p = 0; p < 10; p++)
	{
		concat_path(path, drives[0], paths[p]); cellFsRmdir(path);
		strcat(path, AUTOPLAY_TAG); 				 cellFsRmdir(path);
	}
	cellFsRmdir("/dev_hdd0/PKG");
}

static void del_turnoff(u8 beeps)
{
	#ifdef COBRA_ONLY
	unload_vsh_gui();
	unload_vsh_plugin("VshFpsCounter");
	#endif

	do_umount(false);
	cellFsUnlink("/dev_hdd0/tmp/turnoff");
	cellFsUnlink(WM_RELOAD_FILE); // delete semaphore file

	#ifdef WM_REQUEST
	cellFsUnlink(WM_REQUEST_FILE);
	#endif
	#ifdef WEB_CHAT
	cellFsUnlink(WMCHATFILE);
	#endif
	#ifdef USE_NTFS
	unmount_all_ntfs_volumes();
	#endif
	#ifdef MOUNT_ROMS
	if(ROMS_EXTENSIONS) {free(ROMS_EXTENSIONS); ROMS_EXTENSIONS = NULL;}
	#endif

	if(!webman_config->nobeep)
	{
		if(beeps == 1) { BEEP1 }
		if(beeps == 2) { BEEP2 }
	}
}
