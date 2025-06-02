#define IS_INSTALLING		(View_Find("game_plugin") != 0)
#define IS_INSTALLING_NAS	(View_Find("nas_plugin")  != 0)
#define IS_DOWNLOADING		(View_Find("download_plugin") != 0)

#define PKG_MAGIC				0x7F504B47
#define XMM0					0x584d4d30

#ifdef PKG_HANDLER

// /install_ps3<pkg-path>             Keeps pkg after installation
// /install.ps3<pkg-path>             Deletes pkg after installation
// /install.ps3<pkg-path>?            Installs only if it's a new game, othewise navigates to the installation folder
// /install.ps3?url=<url>             Downloads pkg to /dev_hdd0/packages & installs the pkg after download (pkg is deleted after installation)
// /install_ps3?url=<url>             Downloads pkg to /dev_hdd0/packages & installs the pkg after download (pkg is kept after installation)
// /install.ps3?to=<path>&url=<url>   Same as /download.ps3

// /download.ps3?url=<url>            Downloads file to /dev_hdd0/packages. pkg files are downloaded to /dev_hdd0/tmp/downloader then moved to /dev_hdd0/packages
// /download.ps3?to=<path>&url=<url>  Downloads file to <path>

#include <cell/http.h>

extern wchar_t *stdc_F06EED36(wchar_t *ptr, wchar_t wc, size_t num);                  // wmemset()
#define wmemset stdc_F06EED36

#define MAX_URL_LEN    360
#define MAX_DLPATH_LEN 240

#define MAX_PKGPATH_LEN 240
static char pkg_path[MAX_PKGPATH_LEN];

static wchar_t pkg_durl[MAX_URL_LEN];
static wchar_t pkg_dpath[MAX_DLPATH_LEN];

static u8 pkg_dcount = 0;
static u8 pkg_auto_install = 0;
static bool pkg_delete_after_install = true;
static char install_path[64];
static time_t pkg_install_time = 0;

static bool install_in_progress = false;
static bool installing_pkg = false;

typedef struct {
	u32 magic; // 0x7F504B47 //
	u32 version;
	u16 sdk_type;
	u16 SCE_header_type;
	u32 meta_offset;
	u64 size; // size of sce_hdr + sizeof meta_hdr
	u64 pkg_size;
	u64 unk1;
	u64 data_size;
	char publisher[6]; // ??
	char sep;
	char title_id[9];
} _pkg_header;

static u64 get_pkg_size_and_install_time(const char *pkgfile)
{
	_pkg_header pkg_header;

	int fd; *install_path = pkg_header.pkg_size = pkg_install_time = 0;

	if(cellFsOpen(pkgfile, CELL_FS_O_RDONLY, &fd, NULL, 0) == CELL_FS_SUCCEEDED)
	{
		if(cellFsRead(fd, (void *)&pkg_header, sizeof(pkg_header), NULL) == CELL_FS_SUCCEEDED && pkg_header.magic == PKG_MAGIC)
		{
			concat3(install_path, HDD0_GAME_DIR, pkg_header.title_id, "/PS3LOGO.DAT");

			struct CellFsStat s;
			if(cellFsStat(install_path, &s) == CELL_FS_SUCCEEDED) pkg_install_time = s.st_mtime; // prevents pkg deletion if user cancels install

			install_path[24] = '\0';
		}
		cellFsClose(fd);
	}

	return pkg_header.pkg_size; // also returns module variableS: pkg_install_time & install_path
}

static void wait_for_xml_download(char *filename, char *param)
{
	char *xml = strstr(filename, ".xm!");
	if(xml)
	{
		xml = strchr(filename, '~');

		struct CellFsStat s; u64 size = 475000; if(xml) size = val(xml + 1); else xml = strstr(filename, ".xm!");
/*
		while(IS_DOWNLOADING)
		{
			sys_timer_sleep(1);
		}
*/
		for(u8 retry = 0; retry < 15; retry++)
			{if(!working || (cellFsStat(filename, &s) == CELL_FS_SUCCEEDED && s.st_size >= size)) break; sys_ppu_thread_sleep(2);}

		if(s.st_size < size) {cellFsUnlink(filename); return;}

		strcpy(param, filename); strcpy(xml, ".xml");
		cellFsUnlink(filename);
		cellFsRename(param, filename);
	
#ifdef VIRTUAL_PAD
		if(IS_DOWNLOADING) press_cancel_button(0);
#endif
	}
}

static void wait_for_pkg_install(void)
{
	sys_ppu_thread_sleep(5);

	while (working && IS_INSTALLING) sys_ppu_thread_sleep(2);

	time_t install_time = pkg_install_time;  // set time before install
	get_pkg_size_and_install_time(pkg_path); // get time after install

	if(working && pkg_delete_after_install && islike(pkg_path, INT_HDD_ROOT_PATH) && (strstr(pkg_path, "/GAME") == NULL))
	{
		if(pkg_install_time != install_time) cellFsUnlink(pkg_path);
	}

	if(do_restart && (pkg_install_time == install_time)) do_restart = false; // installation canceled
}

static int get_xmm0_interface(void)
{
	//if(xmm0_interface == 0) // getting xmb_plugin xmm0 interface for loading plugin sprx
	{
		xmm0_interface = (xmb_plugin_xmm0 *)plugin_GetInterface(View_Find("xmb_plugin"), XMM0);
		if(xmm0_interface == 0) return FAILED;
	}
	return CELL_OK;
}

static void LoadBlankPage(void)
{
	//if(game_ext_interface == 0) // test if game_ext_plugin is loaded for interface access
	{
		game_ext_interface = (game_ext_plugin_interface *)plugin_GetInterface(View_Find("game_ext_plugin"), 1);
		if(game_ext_interface == 0) return;
	}
	game_ext_interface->LoadPage();
}

static int LoadPluginById(int id, void *handler)
{
	if(get_xmm0_interface()) return FAILED;

	return xmm0_interface->LoadPlugin3(id, handler, 0);
}

static int LoadPluginByName(char *plugin_name)
{
	int id = 0;
	if(!strstr(plugin_name, "_plugin")) return 0;
	if(strstr(plugin_name, "nas")) id = nas_plugin; else
	if(strstr(plugin_name, "xai")) id = xai_plugin; else
	if(strstr(plugin_name, "edy")) id = edy_plugin; else
	if(strstr(plugin_name, "ps3")) id = ps3_savedata_plugin; else
	if(strstr(plugin_name, "vmc")) id = vmc_savedata_plugin; else
	if(strstr(plugin_name, "hknw")) id = hknw_plugin; else
	if(strstr(plugin_name, "sacd")) id = sacd_plugin; else
	if(strstr(plugin_name, "user")) id = user_plugin; else
	if(strstr(plugin_name, "dlna")) id = dlna_plugin; else
	if(strstr(plugin_name, "cddb")) id = eula_cddb_plugin; else
	if(strstr(plugin_name, "premo")) id = premo_plugin; else
	if(strstr(plugin_name, "regcam")) id = regcam_plugin; else
	if(strstr(plugin_name, "wboard")) id = wboard_plugin; else
	if(strstr(plugin_name, "remote")) id = remotedownload_plugin; else
	if(strstr(plugin_name, "update")) id = software_update_plugin; else
	if(strstr(plugin_name, "device")) id = deviceconf_plugin; else
	if(strstr(plugin_name, "netconf")) id = netconf_plugin; else
	if(strstr(plugin_name, "sysconf")) id = sysconf_plugin; else
	if(strstr(plugin_name, "kensaku")) id = kensaku_plugin; else
	if(strstr(plugin_name, "game_ext")) id = game_ext_plugin; else
	if(strstr(plugin_name, "gamedata")) id = gamedata_plugin; else
	if(strstr(plugin_name, "filecopy")) id = filecopy_plugin; else
	if(strstr(plugin_name, "npsignin")) id = npsignin_plugin; else
	if(strstr(plugin_name, "newstore")) id = newstore_plugin; else
	if(strstr(plugin_name, "download")) id = download_plugin; else
	if(strstr(plugin_name, "playlist")) id = playlist_plugin; else
	if(strstr(plugin_name, "strviewer")) id = strviewer_plugin; else
	if(strstr(plugin_name, "webrender")) id = webrender_plugin; else
	if(strstr(plugin_name, "webbrowser")) id = webbrowser_plugin; else
	if(strstr(plugin_name, "photoviewer")) id = photoviewer_plugin; else
	if(strstr(plugin_name, "audioplayer")) id = audioplayer_plugin; else
	if(strstr(plugin_name, "videoplayer")) id = videoplayer_plugin; else
	if(strstr(plugin_name, "videodownloader")) id = videodownloader_plugin; else
	if(strstr(plugin_name, "game")) id = game_plugin;

	//system_plugin=0x00,
	//xmb_plugin=0x01,
	//explore_plugin=0x02,
	//category_setting_plugin=0x03,
	//print_plugin=0x09,
	//bdp_plugin=0x11,
	//bdp_disccheck_plugin=0x12,
	//bdp_storage_plugin=0x13,
	//friendim_plugin=0x1E,
	//friendml_plugin=0x1F,
	//avc_plugin=0x20,
	//avc2_text_plugin=0x21,
	//np_trophy_plugin=0x24,
	//np_trophy_ingame=0x25,
	//friendtrophy_plugin=0x26,
	//profile_plugin=0x27,
	//thumthum_plugin=0x2A,
	//micon_lock_plugin=0x2B,
	//idle_plugin=0x36,
	//poweroff_plugin=0x39,
	//videoeditor_plugin=0x3A,
	//scenefolder_plugin=0x3B,
	//eula_hcopy_plugin=0x3C,
	//mtpinitiator_plugin=0x3E,
	//campaign_plugin=0x3F,

	if(id) {LoadPluginById(id, NULL); sys_ppu_thread_sleep(1);}
	return id;
}

static void unloadSysPluginCallback(void)
{
	//Add potential callback process
	//show_msg((char *)"plugin shutdown via xmb call launched");
}

static void UnloadPluginById(int id)
{
	if(get_xmm0_interface()) return;

	u8 retry = 0;

	while(xmm0_interface->IsPluginViewAvailable(id))
	{
		xmm0_interface->Shutdown(id, (void *)unloadSysPluginCallback, 1);
		sys_ppu_thread_usleep(100000); if(++retry > 100) break;
	}
}

static void unload_system_plugins(void)
{
	UnloadPluginById(nas_plugin);
	UnloadPluginById(wboard_plugin);
	UnloadPluginById(np_trophy_plugin);
	UnloadPluginById(sysconf_plugin);
}

static void unload_plugin_modules(bool all)
{
	// Unload conflicting plugins
	if(View_Find("webbrowser_plugin")) UnloadPluginById(webbrowser_plugin);
	if(View_Find("webrender_plugin")) UnloadPluginById(webrender_plugin);

	if(all)
	{
		from_xmb_ps3 = false;
		unload_system_plugins();
	}

#ifdef VIRTUAL_PAD
	if(IS_ON_XMB)
	{
		press_cancel_button(0);
	}
#endif

	if(!get_explore_interface()) return;

	exec_xmb_command("close_all_list");
	if(all)
	{
		sys_ppu_thread_sleep(2);
		exec_xmb_command2("focus_category %s", "game");
		exec_xmb_command2("reload_category %s", "game");
	}
}

static void downloadPKG_thread(void)
{
	//if(download_interface == 0) // test if download_interface is loaded for interface access
	{
		download_interface = (download_plugin_interface *)plugin_GetInterface(View_Find("download_plugin"), 1);
		if(download_interface == 0) return;
	}
	download_interface->DownloadURL(0, pkg_durl, pkg_dpath);
}

#ifdef FULL_EDITION
static void webrender_thread(void)
{
	webrender_interface = (webrender_plugin_interface *)plugin_GetInterface(View_Find("webrender_plugin"), 1);
}

static void webbrowser_thread(void)
{
	webbrowser_interface = (webbrowser_plugin_interface *)plugin_GetInterface(View_Find("webbrowser_plugin"), 1);
}
#endif

static int download_file(const char *param, char *msg)
{
	int ret = FAILED;

	s32 net_enabled = 0;
	xnet()->GetSettingNet_enable(&net_enabled);

	if(!net_enabled)
	{
		concat2(msg,  "ERROR: ", "network disabled");
		return ret;
	}

	if(IS_INGAME)
	{
		concat2(msg,  "ERROR: ", "download from XMB");
		return ret;
	}

	char *msg_durl = msg;
	char *msg_dpath = msg + MAX_URL_LEN + 16;

	concat2(msg_durl,  "ERROR: ", "Invalid URL");
	sprintf(msg_dpath, "Download canceled");

	char pdurl[MAX_URL_LEN];
	char pdpath[MAX_DLPATH_LEN];

	wmemset(pkg_durl, 0, MAX_URL_LEN); // Use wmemset from stdc.h instead of reinitialising wchar_t with a loop.
	wmemset(pkg_dpath, 0, MAX_DLPATH_LEN);

	_memset(pdurl,  MAX_URL_LEN);
	_memset(pdpath, MAX_DLPATH_LEN);

	int len;
	size_t conv_num = 0;

	if(islike(param, "?to="))  //Use of the optional parameter
	{
		len = get_param("&url=", pdurl, param, MAX_DLPATH_LEN); if(!islike(pdurl, "http")) goto end_download_process;
		conv_num = mbstowcs((wchar_t *)pkg_durl, (const char *)pdurl, len + 1);  //size_t stdc_FCAC2E8E(wchar_t *dest, const char *src, size_t max)

		len = get_param("?to=", pdpath, param, MAX_DLPATH_LEN); if(*pdpath != '/') goto end_download_process;
		filepath_check(pdpath);
	}
	else if(islike(param, "?url="))
	{
		len = get_param("?url=", pdurl, param, MAX_DLPATH_LEN); if(!islike(pdurl, "http")) goto end_download_process;
		conv_num = mbstowcs((wchar_t *)pkg_durl,(const char *)pdurl, len + 1);  //size_t stdc_FCAC2E8E(wchar_t *dest, const char *src, size_t max)

		len = strcopy(pdpath, is_ext(pdurl, ".p3t") ? "/dev_hdd0/theme" : DEFAULT_PKG_PATH);
	}

	if(conv_num)
	{
		filepath_check(pdpath); // replace $USERID$ with current user id & remove invalid chars

		mkdir_tree(pdpath);

		if(isDir(pdpath) || (cellFsMkdir(pdpath, DMODE) == CELL_FS_SUCCEEDED)) ;

		else if(isDir(DEFAULT_PKG_PATH) || cellFsMkdir(DEFAULT_PKG_PATH, DMODE) == CELL_FS_SUCCEEDED)
		{
			len = strcopy(pdpath, DEFAULT_PKG_PATH);
		}
		else
		{
			len = strcopy(pdpath, INT_HDD_ROOT_PATH);
		}

		concat_text(msg_dpath, "To:", pdpath);
		if(IS(pdpath, DEFAULT_PKG_PATH) && strstr(pdurl, ".pkg"))
		{
			len = strcopy(pdpath, TEMP_DOWNLOAD_PATH); pkg_dcount++;
		}

		conv_num = mbstowcs((wchar_t *)pkg_dpath, (const char *)pdpath, len + 1);

		if(conv_num)
		{
			unload_system_plugins();

			sys_ppu_thread_sleep(3);

			mkdir_tree(pdpath); cellFsMkdir(pdpath, DMODE);

			concat_text(msg_durl, "Downloading", pdurl);

			LoadPluginById(download_plugin, (void *)downloadPKG_thread);
			ret = CELL_OK;
		}
		else
			concat2(msg_durl, "ERROR: ", "Setting storage location");
	}

end_download_process:
	sprintf(msg, "%s\n%s", msg_durl, msg_dpath);
	return ret;
}

static void installPKG_thread(void)
{
	LoadBlankPage();

	installing_pkg = true;

	if(is_ext(pkg_path, ".p3t"))
		game_ext_interface->installTheme(pkg_path, (char*)"");
	else
		game_ext_interface->installPKG(pkg_path);

	installing_pkg = false;
}

static int installPKG(const char *pkgpath, char *msg)
{
	int ret = FAILED;

	unload_system_plugins();

	if(IS_INGAME)
	{
		unload_plugin_modules(true);

		if(IS_INGAME)
		{
			concat2(msg,  "ERROR: ", "install from XMB");
			return ret;
		}
	}

	size_t pkg_path_len = strlen(pkgpath);

	if (pkg_path_len < MAX_PKGPATH_LEN)
	{
#ifdef COBRA_ONLY
		if(islike(pkgpath, "/net"))
		{
			cache_file_to_hdd((char*)pkgpath, pkg_path, "/tmp/downloader", msg); pkg_dcount++;
		}
		else
#endif
		if(*pkgpath == '?')
		{
			pkg_auto_install++;
			return download_file(pkgpath, msg);
		}
		else
			strncopy(pkg_path, MAX_PKGPATH_LEN, pkgpath);

		if(file_exists(pkg_path))
		{
			if(is_ext(pkg_path, ".pkg") || is_ext(pkg_path, ".p3t")) //check if file has a .pkg extension or not and treat accordingly
			{
				LoadPluginById(game_ext_plugin, (void *)installPKG_thread);

				get_pkg_size_and_install_time(pkg_path); // set original pkg_install_time

				sprintf(msg, "%s%s\nTo: %s", "Installing ", pkg_path, install_path);

				ret = CELL_OK;
			}
		}
	}

	if(ret) concat2(msg,  "ERROR: ", pkgpath);
	return ret;
}

static const char *INSTALL_PKG_PATH; // input parameter of installPKG_combo_thread

static void installPKG_combo_thread(__attribute__((unused)) u64 arg)
{
	if(install_in_progress && installing_pkg && IS_INSTALLING) return;

	install_in_progress = true;

	int fd, ret = FAILED;

	if(cellFsOpendir(INSTALL_PKG_PATH, &fd) == CELL_FS_SUCCEEDED)
	{
		CellFsDirent dir; u64 read_e;

		char pkgfile[MAX_PKGPATH_LEN];
		u16 plen = concat2(pkgfile, INSTALL_PKG_PATH, "/");
		char *path_file = pkgfile + plen;

		while(working && (cellFsReaddir(fd, &dir, &read_e) == CELL_FS_SUCCEEDED) && (read_e > 0))
		{
			if(is_ext(dir.d_name, ".pkg"))
			{
				strcopy(path_file, dir.d_name);

				char msg[MAX_PATH_LEN];
				ret = installPKG(pkgfile, msg); if(!(webman_config->minfo & 1)) show_msg(msg);
				if(ret == CELL_OK) wait_for_pkg_install();

				ret = CELL_OK;
			}
		}
		cellFsClosedir(fd);

		if(ret == FAILED) { BEEP2 }
	}

	sys_ppu_thread_sleep(2);
	install_in_progress = false;

	sys_ppu_thread_exit(0);
}

static void installPKG_all(const char *path, bool delete_after_install)
{
	pkg_delete_after_install = delete_after_install;

	if(!install_in_progress && IS_ON_XMB)
	{
		check_path_tags((char*)path);

		INSTALL_PKG_PATH = path;

		if(isDir(INSTALL_PKG_PATH))
		{
			sys_ppu_thread_t thread_id;
			sys_ppu_thread_create(&thread_id, installPKG_combo_thread, NULL, THREAD_PRIO, THREAD_STACK_SIZE_INSTALL_PKG, SYS_PPU_THREAD_CREATE_NORMAL, THREAD_NAME_INSTALLPKG);
		}
	}
}

static void poll_downloaded_pkg_files(char *msg)
{
	if(pkg_dcount)
	{
		CellFsDirent entry; u64 read_e; int fd; u16 pkg_count = 0;

		if(cellFsOpendir(TEMP_DOWNLOAD_PATH, &fd) == CELL_FS_SUCCEEDED)
		{
			char *dlfile = msg;

			int plen = strcopy(dlfile, TEMP_DOWNLOAD_PATH);

			while(working && (cellFsReaddir(fd, &entry, &read_e) == CELL_FS_SUCCEEDED) && (read_e > 0))
			{
				if(is_ext(entry.d_name, ".pkg"))
				{
					strcopy(dlfile + plen, entry.d_name); pkg_count++;
					cellFsChmod(dlfile, MODE);

					u64 pkg_size = get_pkg_size_and_install_time(dlfile); if(pkg_size == 0) continue;

					struct CellFsStat s;
					if(cellFsStat(dlfile, &s) == CELL_FS_SUCCEEDED && pkg_size == s.st_size)
					{
						char pkgfile[MAX_PATH_LEN];
						u16 pkg_len = concat2(pkgfile, DEFAULT_PKG_PATH, entry.d_name);
						for(u8 retry = 1; retry < 255; retry++)
						{
							if(cellFsRename(dlfile, pkgfile) == CELL_FS_SUCCEEDED) break;
							sprintf(pkgfile + pkg_len - 4, " (%i).pkg", retry);
						}

						if(pkg_count) pkg_count--, pkg_dcount--;

						if(pkg_auto_install) {pkg_auto_install--; installPKG(pkgfile, msg); wait_for_pkg_install();}
					}
				}
			}
			cellFsClosedir(fd);
		}
	}
}
#endif // #ifdef PKG_HANDLER
