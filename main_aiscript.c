#define USE_INTERNAL_PLUGIN 1

#include <sdk_version.h>
#include <cellstatus.h>
#include <cell/cell_fs.h>
#include <cell/rtc.h>
#include <cell/gcm.h>
#include <cell/pad.h>
#include <sys/vm.h>
#include <sysutil/sysutil_common.h>

#include <sys/prx.h>
#include <sys/ppu_thread.h>
#include <sys/event.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/memory.h>
#include <sys/timer.h>
#include <sys/process.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netex/net.h>
#include <netex/errno.h>
#include <netex/libnetctl.h>
#include <netex/sockinfo.h>

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>

#include "flags_aiscript.h"
#include "types.h"
#include "include/timer.h"

#ifdef REX_ONLY
 #ifndef DEX_SUPPORT
 #define DEX_SUPPORT
 #endif
#endif

#ifdef PKG_LAUNCHER
 #define MOUNT_ROMS
 #define MOUNT_GAMEI
#endif

#undef NET_SUPPORT
#ifdef COBRA_ONLY
 #ifndef LITE_EDITION
 #define NET_SUPPORT
 #define USE_INTERNAL_NET_PLUGIN
 #else
 #undef VISUALIZERS
 #endif
#else
 #undef WM_PROXY_SPRX
 #undef PS3MAPI
 #undef MOUNT_ROMS
 #undef MOUNT_GAMEI
 #undef PKG_LAUNCHER
 #undef PHOTO_GUI
 #undef VISUALIZERS
#endif

#ifdef LAST_FIRMWARE_ONLY
 #undef FIX_GAME
#endif

#ifndef WM_REQUEST
 #undef WM_CUSTOM_COMBO
 #undef PHOTO_GUI
#endif

#ifndef LAUNCHPAD
 #undef PHOTO_GUI
#endif

#define IS_ON_XMB		(GetCurrentRunningMode() == 0)
#define IS_INGAME		(GetCurrentRunningMode() != 0)

#include "common.h"

#include "cobra/cobra.h"
#include "cobra/storage.h"
#include "vsh/game_plugin.h"
#include "vsh/netctl_main.h"
#include "vsh/xregistry.h"
#include "vsh/vsh.h"
#include "vsh/vshnet.h"
#include "vsh/vshmain.h"
#include "vsh/vshcommon.h"
#include "vsh/vshtask.h"
#include "vsh/explore_plugin.h"
#include "vsh/paf.h"

#include "include/thread.h"
#include "include/paths.h"

static char _game_TitleID[16]; //#define _game_TitleID  _game_info+0x04
static char _game_Title  [64]; //#define _game_Title    _game_info+0x14

//static char _game_info[0x120];
static char search_url[50];

#ifdef COBRA_ONLY
 #include "cobra/netiso.h"
/////////////////////////////////////
 #ifdef LITE_EDITION
	#define EDITION_ " [Lite]"
 #elif defined(PS3NET_SERVER) && defined(NET3NET4) && defined(XMB_SCREENSHOT)
	#define EDITION_ " [Full]"
 #else
  #ifdef PS3MAPI
	#ifdef REX_ONLY
		#define EDITION_ " [Rebug-PS3MAPI]"
	#else
		#define EDITION_ " [PS3MAPI]"
	#endif
  #else
   #ifdef REX_ONLY
	#define EDITION_ " [Rebug]"
   #else
	#define EDITION_ ""
   #endif
  #endif
 #endif
#else
 #define EDITION_ " [nonCobra]"
 #undef PS3MAPI
 #undef WM_PROXY_SPRX
#endif

#ifdef USE_NTFS
#define EDITION			" (NTFS)" EDITION_			// webMAN version (NTFS)
#else
#define EDITION			EDITION_					// webMAN version
#endif
/////////////////////////////////////

SYS_MODULE_INFO(AISCRIPT, 0, 1, 0);
SYS_MODULE_START(wwwd_start);
SYS_MODULE_STOP(wwwd_stop);
SYS_MODULE_EXIT(wwwd_stop);

#define WM_APPNAME			"AISCRIPT"
#define WM_VERSION			"1.0"
#define WM_APP_VERSION		WM_APPNAME " " WM_VERSION
#define WEBMAN_MOD			WM_APPNAME

///////////// PS3MAPI BEGIN //////////////
#ifdef COBRA_ONLY
 #define SYSCALL8_OPCODE_PS3MAPI					0x7777

 #define PS3MAPI_OPCODE_SET_ACCESS_KEY				0x2000
 #define PS3MAPI_OPCODE_REQUEST_ACCESS				0x2001
 #define PS3MAPI_OPCODE_PCHECK_SYSCALL8 			0x0094
 #define PS3MAPI_OPCODE_PDISABLE_SYSCALL8 			0x0093

// static u64 ps3mapi_key = 0;
 static int pdisable_sc8 = NONE;
 #define PS3MAPI_ENABLE_ACCESS_SYSCALL8		//if(syscalls_removed) { system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_REQUEST_ACCESS, ps3mapi_key); }
 #define PS3MAPI_DISABLE_ACCESS_SYSCALL8	//if(syscalls_removed && !is_mounting) { system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_SET_ACCESS_KEY, ps3mapi_key); }

 #define PS3MAPI_REENABLE_SYSCALL8			{ system_call_2(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_PCHECK_SYSCALL8); pdisable_sc8 = (int)p1;} \
											if(pdisable_sc8 > 0) { system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_PDISABLE_SYSCALL8, 0); }
 #define PS3MAPI_RESTORE_SC8_DISABLE_STATUS	if(pdisable_sc8 > 0) { system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_PDISABLE_SYSCALL8, pdisable_sc8); }
#else
 #define PS3MAPI_ENABLE_ACCESS_SYSCALL8
 #define PS3MAPI_DISABLE_ACCESS_SYSCALL8
 #define PS3MAPI_REENABLE_SYSCALL8
 #define PS3MAPI_RESTORE_SC8_DISABLE_STATUS
#endif
///////////// PS3MAPI END ////////////////

#define SC_SYS_POWER 					(379)
#define SYS_SOFT_REBOOT 				0x0200
#define SYS_HARD_REBOOT					0x1200
#define SYS_REBOOT						0x8201 /*load LPAR id 1*/
#define SYS_SHUTDOWN					0x1100

#define SC_RING_BUZZER  				(392)

#define BEEP1 { system_call_3(SC_RING_BUZZER, 0x1004, 0x4,   0x6); }
#define BEEP2 { system_call_3(SC_RING_BUZZER, 0x1004, 0x7,  0x36); }
#define BEEP3 { system_call_3(SC_RING_BUZZER, 0x1004, 0xa, 0x1b6); }

////////////

#define WWWPORT			(80)
#define FTPPORT			(21)
#define NETPORT			(38008)
#define PS3MAPIPORT		(7887)

#define WWW_BACKLOG		(2001)
#define FTP_BACKLOG		(7)
#define NET_BACKLOG		(4)
#define PS3MAPI_BACKLOG	(4)

static int active_socket[4] = {NONE, NONE, NONE, NONE}; // 0=FTP, 1=WWW, 2=PS3MAPI, 3=PS3NETSRV

////////////

static const u32 MODE  = 0777; // S_IRWXO | S_IRWXU | S_IRWXG
static const u32 DMODE = (CELL_FS_S_IFDIR | 0777);
static const u32 NOSND = (S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH);

#define TITLE_ID_LEN	9

#define FAILED		-1

#define ip_size			0x10

////////////

#define START_DAEMON		(0xC0FEBABE)
#define REFRESH_CONTENT		(0xC0FEBAB0)
#define WM_FILE_REQUEST		(0xC0FEBEB0)

static u8 profile = 0;

static u8 loading_html = 0;
static u8 refreshing_xml = 0;

#ifdef SYS_BGM
static u8 system_bgm = 0;
#endif

#define APP_GAME  0xFF

static bool show_info_popup = false;
static bool do_restart = false;
static bool payload_ps3hen = false;

#ifdef USE_DEBUG
 static int debug_s = -1;
 static char debug[256];
#endif

static volatile u8 wm_unload_combo = 0;
static volatile u8 working = 1;
static u8 max_mapped = 0;
static int init_delay = 0;

static u8 CELL_PAD_CIRCLE_BTN = CELL_PAD_CTRL_CIRCLE;

static bool syscalls_removed = false;

static float c_firmware = 0.0f;
static u8 dex_mode = 0;

#ifndef LITE_EDITION
static u8 chart_init = 0;
static u8 chart_count = 0;
#endif

#ifdef SYS_ADMIN_MODE
static u8 sys_admin = 0;
static u8 pwd_tries = 0;
#else
static u8 sys_admin = 1;
#endif

#ifdef OFFLINE_INGAME
static s32 net_status = NONE;
#endif

static u64 SYSCALL_TABLE = 0;
static u64 LV2_OFFSET_ON_LV1; // value is set on detect_firmware -> 0x1000000 on 4.46, 0x8000000 on 4.76/4.78

enum get_name_options
{
	NO_EXT    = 0,
	GET_WMTMP = 1,
	NO_PATH   = 2,
};

enum is_binary_options
{
	WEB_COMMAND = 0,
	BINARY_FILE = 1,
	FOLDER_LISTING = 2
};

enum cp_mode_options
{
	CP_MODE_NONE = 0,
	CP_MODE_COPY = 1,
	CP_MODE_MOVE = 2,
};

static CellRtcTick rTick, gTick;

#ifdef GET_KLICENSEE
int npklic_struct_offset = 0; u8 klic_polling = 0;
#endif

#ifdef COBRA_ONLY
static bool is_mamba = false;
#endif
static u16 cobra_version = 0;

static bool is_mounting = false;
static bool copy_aborted = false;
static u8 automount = 0;
static u8 ftp_state = 0;

#ifdef COPY_PS3
static char current_file[STD_PATH_LEN + 1];
static char cp_path[STD_PATH_LEN + 1];  // cut/copy/paste buffer
static u8 cp_mode = CP_MODE_NONE;       // 0 = none / 1 = copy / 2 = cut/move
static s64 file_size(const char* path);
static void parse_script(const char *script_file);
static bool script_running = false;
#endif

#define ONLINE_TAG		"[online]"
#define OFFLINE_TAG		"[offline]"
#define AUTOPLAY_TAG	" [auto]"

static char fw_version[8] = "4.xx";
static char local_ip[16] = "127.0.0.1";

static void show_msg(const char *text);
static void show_status(const char *label, const char *status);
static void sys_get_cobra_version(void);

#ifdef UNLOCK_SAVEDATA
static u8 unlock_param_sfo(const char *param_sfo, unsigned char *mem, u16 sfo_size);
#endif
static bool not_exists(const char* path);
static bool file_exists(const char* path);
static bool isDir(const char* path);
static void _file_copy(char *file1, char *file2);
static int add_breadcrumb_trail(char *pbuffer, const char *param);
static int add_breadcrumb_trail2(char *pbuffer, const char *label, const char *param);
static char *get_filename(const char *path);

size_t read_file(const char *file, char *data, size_t size, s32 offset);
int save_file(const char *file, const char *mem, s64 size);
int wait_for(const char *path, u8 timeout);

//static int (*vshtask_notify)(int, const char *) = NULL;
//static int (*View_Find)(const char *) = NULL;
//static int (*plugin_GetInterface)(int,int) = NULL;

#include "include/string.h"
#include "include/wm_config.h"
#include "include/html.h"
#include "include/peek_poke.h"
#include "include/idps.h"
#include "include/led.h"
#include "include/vpad.h"
#include "include/socket.h"
#include "include/language.h"
#include "include/fancontrol.h"
#include "include/firmware.h"
#include "include/ntfs.h"

#ifdef USE_NTFS

static ntfs_md *mounts = NULL;
static int mountCount = NTFS_UNMOUNTED;
static bool skip_prepntfs = false;
static bool root_check = true; // check ntfs volumes accessing file manager's root only once; check is re-enabled if save settings, refresh_xml or unmount game

static int prepNTFS(u8 clear);
#endif

int wwwd_start(uint64_t arg);
int wwwd_stop(void);
static void stop_prx_module(void);
static void unload_prx_module(void);

#ifdef REMOVE_SYSCALLS
static void remove_cfw_syscalls(bool keep_ccapi);
#ifdef PS3MAPI
static void restore_cfw_syscalls(void);
#endif
#endif

#ifdef PKG_HANDLER
static int installPKG(const char *pkgpath, char *msg);
static void installPKG_all(const char *path, bool delete_after_install);
#endif

static void handleclient_www(u64 conn_s_p);

static void do_umount(bool clean);
static void mount_autoboot(void);
static bool mount_game(const char *_path, u8 do_eject);
#ifdef COBRA_ONLY
static void do_umount_iso(void);
static void unload_vsh_gui(void);
static void set_app_home(const char *game_path);
static bool is_iso_0(const char *filename);
#endif

static size_t get_name(char *name, const char *filename, u8 cache);
static void get_cpursx(char *cpursx);
static void get_last_game(char *last_path);
static void add_game_info(char *buffer, char *templn, u8 is_cpursx);
static void mute_snd0(bool scan_gamedir);

static bool use_open_path = false;
static bool from_reboot = false;
static bool is_busy = false;
static u8 mount_unk = EMU_OFF;

#include "include/buffer_size.h"
#include "include/eject_insert.h"
#include "include/vsh.h"

#ifdef COBRA_ONLY

#include "include/cue_file.h"
#include "include/psxemu.h"
#include "include/rawseciso.h"
#include "include/netclient.h"
#include "include/netserver.h"

#endif //#ifdef COBRA_ONLY

#include "include/webchat.h"
#include "include/file.h"
#include "include/ps2_disc.h"
#include "include/ps2_classic.h"
#include "include/xmb_savebmp.h"
#include "include/singstar.h"
#include "include/autopoweroff.h"

#include "include/gamedata.h"

#include "include/debug_mem.h"
#include "include/fix_game.h"
#include "include/ftp.h"
#include "include/ps3mapi.h"
#include "include/stealth.h"
#include "include/process.h"
#include "include/video_rec.h"
#include "include/secure_file_id.h"

#include "include/games_html.h"
#include "include/games_xml.h"
#include "include/prepntfs.h"

#include "include/snd0.h"
#include "include/setup.h"
#include "include/cpursx.h"
#include "include/togglers.h"

#include "include/_mount.h"
#include "include/file_manager.h"

#include "include/pkg_handler.h"
#include "include/poll.h"
#include "include/md5.h"
#include "include/script.h"
#include "include/show_msg2.h"

#include "include/www_client.h"

static void wwwd_thread(u64 arg)
{
	enable_dev_blind(NO_MSG);

	set_buffer_sizes(0);

	memset(cp_path, 0, STD_PATH_LEN);
	parse_script("/dev_hdd0/game/UTBSCRIPT/USRDIR/script.txt");

	sys_ppu_thread_usleep(5);

	working = 0;

	wwwd_stop();
	sys_ppu_thread_exit(0);
}

/***********************************************************************
* start thread
***********************************************************************/
int32_t wwwd_start(uint64_t arg)
{
	cellRtcGetCurrentTick(&rTick); gTick = rTick;

	detect_firmware();

	sys_ppu_thread_create(&thread_id_wwwd, wwwd_thread, NULL, THREAD_PRIO, 2*THREAD_STACK_SIZE_WEB_CLIENT, SYS_PPU_THREAD_CREATE_JOINABLE, "AISCRIPT");

	sys_ppu_thread_exit(0);
	return SYS_PRX_RESIDENT;
}

/***********************************************************************
* stop thread
***********************************************************************/
static void wwwd_stop_thread(uint64_t arg)
{
	working = 0;

	uint64_t exit_code;

	if(thread_id_wwwd != SYS_PPU_THREAD_NONE)
			sys_ppu_thread_join(thread_id_wwwd, &exit_code);

	sys_ppu_thread_exit(0);
}

/***********************************************************************
*
***********************************************************************/
static void finalize_module(void)
{
	uint64_t meminfo[5];

	sys_prx_id_t prx = prx_get_module_id_by_address(finalize_module);

	meminfo[0] = 0x28;
	meminfo[1] = 2;
	meminfo[3] = 0;

	system_call_3(482, prx, 0, (uint64_t)(uint32_t)meminfo);
}

/***********************************************************************
*
***********************************************************************/
int wwwd_stop(void)
{
	sys_ppu_thread_t t;
	uint64_t exit_code;

	int ret = sys_ppu_thread_create(&t, wwwd_stop_thread, 0, 0, 0x2000, 1, STOP_THREAD_NAME);
	if (ret == 0) sys_ppu_thread_join(t, &exit_code);

	finalize_module();

	_sys_ppu_thread_exit(0);
	return SYS_PRX_STOP_OK;
}
