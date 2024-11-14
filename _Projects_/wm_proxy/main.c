#include <sdk_version.h>
#include <string.h>
#include <cell/rtc.h>
#include <cell/cell_fs.h>
#include <sys/timer.h>

#include <sys/prx.h>
#include <sys/socket.h>
#include <sys/ppu_thread.h>
#include <sys/syscall.h>

#include <netinet/in.h>

#include "types.h"
#include "explore_plugin.h"

#define LCASE(a)	(a | 0x20)
#define BETWEEN(a, b, c)	( ((a) <= (b)) && ((b) <= (c)) )

SYS_MODULE_INFO(wm_proxy, 0, 1, 1);
SYS_MODULE_START(prx_start);
SYS_MODULE_STOP(prx_stop);
SYS_MODULE_EXIT(prx_stop);

int prx_start(size_t args, void *argp);
int prx_stop(void);

extern char *stdc_C5C09834(const char *str1, const char *str2);							// strstr()
inline char* strstr(const char *str1, const char *str2) {return stdc_C5C09834(str1, str2);}

extern int stdc_B6D92AC3(const char *s1, const char *s2);								// strcasecmp()
int strcasecmp(const char *s1, const char *s2) {if(!s2 || !s2) return -1; return stdc_B6D92AC3(s1, s2);}

static int (*vshtask_notify)(int, const char *) = NULL;

int cobra_umount_disc_image(void);
int cobra_map_game(const char *path, const char *title_id, int use_app_home);
int cobra_mount_ps3_disc_image(char *files[], unsigned int num);
int cobra_mount_dvd_disc_image(char *files[], unsigned int num);
int cobra_mount_bd_disc_image(char *files[], unsigned int num);
int cobra_mount_psx_disc_image(char *file);
int cobra_mount_ps2_disc_image(char *files[], int num);
int cobra_send_fake_disc_eject_event(void);
int cobra_send_fake_disc_insert_event(void);
int sys_map_path(const char *oldpath, const char *newpath);

static void * getNIDfunc(const char * vsh_module, uint32_t fnid)
{
	uint32_t table = (*(uint32_t*)0x1008C) + 0x984; // vsh table address

	while(((uint32_t)*(uint32_t*)table) != 0)
	{
		uint32_t* export_stru_ptr = (uint32_t*)*(uint32_t*)table;

		const char* lib_name_ptr =  (const char*)*(uint32_t*)((char*)export_stru_ptr + 0x10);

		if(strncmp(vsh_module, lib_name_ptr, strlen(lib_name_ptr))==0)
		{
			uint32_t lib_fnid_ptr = *(uint32_t*)((char*)export_stru_ptr + 0x14);
			uint32_t lib_func_ptr = *(uint32_t*)((char*)export_stru_ptr + 0x18);
			uint16_t count = *(uint16_t*)((char*)export_stru_ptr + 6); // number of exports
			for(int i = 0; i < count; i++)
			{
				if(fnid == *(uint32_t*)((char*)lib_fnid_ptr + i*4))
				{
					return (void**)*((uint32_t*)(lib_func_ptr) + i);
				}
			}
		}
		table += 4;
	}
	return 0;
}

static explore_plugin_interface *get_explore_interface(void)
{
	int (*View_Find)(const char *) = getNIDfunc("paf", 0xF21655F3);
	if(!View_Find) return NULL;

	int (*plugin_GetInterface)(int,int) = getNIDfunc("paf", 0x23AFB290);
	if(!plugin_GetInterface) return NULL;

	int view = View_Find("explore_plugin");
	if(view)
		explore_interface = (explore_plugin_interface *)plugin_GetInterface(view, 1);

	return explore_interface;
}

static void exec_xmb_command(const char *cmd)
{
	if(get_explore_interface())
	{
		explore_interface->ExecXMBcommand(cmd, 0, 0);
	}
}

static void show_msg(const char* msg)
{
	//if(!vshtask_notify)
		vshtask_notify = getNIDfunc("vshtask", 0xA02D46E7);

	//if(strlen(msg)>128) msg[128]=0;

	if(vshtask_notify)
		vshtask_notify(0, msg);
}

static uint8_t h2b(const char hex) // hex char to byte
{
	uint8_t c = LCASE(hex);
	if(BETWEEN('0', c, '9'))
		c -= '0'; // 0-9
	else if(BETWEEN('a', c, 'f'))
		c -= 'W'; // 10-15
	return c;
}

static void wm_plugin_init (int view);
static int  wm_plugin_start(void * view);
//static int  wm_plugin_stop (void);
static void wm_plugin_exit (void);
static void wm_plugin_action(const char * action);
static int setInterface(unsigned int view);

static int (*plugin_SetInterface)(int view, int interface, void * Handler);
static int (*plugin_SetInterface2)(int view, int interface, void * Handler);

static void *wm_plugin_action_if[3] = {(void*)(wm_plugin_action), 0, 0};

static void wm_plugin_init (int view)		{plugin_SetInterface( view, 0x41435430 /*ACT0*/, wm_plugin_action_if);}
static int  wm_plugin_start(void * view)	{return SYS_PRX_START_OK;}
//static int  wm_plugin_stop (void)			{return SYS_PRX_STOP_OK;}
static void wm_plugin_exit (void)			{return;}

#define wm_plugin_stop prx_stop

static void *wm_plugin_functions[4] =
	{
		(void*)(wm_plugin_init),
		(int* )(wm_plugin_start),
		(int* )(wm_plugin_stop),
		(void*)(wm_plugin_exit)
	};

static int setInterface(unsigned int view)
{
	plugin_SetInterface  = getNIDfunc("paf", 0xA1DC401);
	plugin_SetInterface2 = getNIDfunc("paf", 0x3F7CB0BF);
	plugin_SetInterface2(view, 1, (void*)wm_plugin_functions);
	return 0;
}

static int connect_to_webman(void)
{
	int s = socket(AF_INET, SOCK_STREAM, 0);
	if(s < 0)
	{
		return -1;
	}

	struct sockaddr_in sin;

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = 0x7F000001;	//127.0.0.1 (localhost)
	sin.sin_port = htons(80);			//http port (80)

	struct timeval tv;
	tv.tv_usec = 0;

	tv.tv_sec = 3;
	setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

	if(connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		return -1;
	}

	tv.tv_sec = 60;
	setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

	return s;
}

static void sclose(int *socket_e)
{
	//if(*socket_e != -1)
	{
		shutdown(*socket_e, SHUT_RDWR);
		socketclose(*socket_e);
		//*socket_e = -1;
	}
}

#define HTML_RECV_SIZE	2048
#define HTML_RECV_LAST	2045 // HTML_RECV_SIZE-3
#define HTML_RECV_LASTP	2042 // HTML_RECV_LAST-3

static void wm_plugin_action(const char * action)
{
	if(*action == 'G') action += 4;  // skip GET
	if(*action != '/') action += 16; // using http://127.0.0.1/ or http://localhost/
	if(*action != '/') return;

	int err = 0;

	int s = connect_to_webman();
	if(s >= 0)
	{
		char proxy_action[HTML_RECV_SIZE];
		memcpy(proxy_action, "GET ", 4);

		u32 pa = 4;

		if(*action == '/')
		{
			u8 is_path = !strstr(action, ".ps") && !strstr(action, "_ps");

			for(;*action && (pa < HTML_RECV_LAST); action++, pa++)
			{
				if(*action == 0x20)
					proxy_action[pa] = 0x2B;
				else if((*action == 0x2B) && is_path)
				{
					if(pa > HTML_RECV_LASTP) break;
					memcpy(proxy_action + pa, "%2B", 3); pa += 2; //+
				}
				else
					proxy_action[pa] = *action;
			}

			memcpy(proxy_action + pa, "\r\n\0", 3); pa +=2;
			send(s, proxy_action, pa, 0);
		}
		sclose(&s);
	}
	else
	{
		err = 1;

		if(strstr(action, "/mount.ps3") == action || strstr(action, "/mount_ps3") == action)
		{
			cobra_umount_disc_image();
			cobra_send_fake_disc_eject_event();

			char *t = (char*)action;
			for(char *c = t; *c; c++, t++)
			{
				if(*c == '+')
					*t = ' ';
				else if(*c == '%')
					{*t = (h2b(c[1])<<4) + h2b(c[2]); c += 2;}
				else
					*t = *c;
			}
			*t = 0;

			char *path = (char*)(action + 10); int len = strlen(path);
			char *files[1] = { path };
			char *ext = path + (len > 4 ? len - 4 : 0);

			int cue = 0;

			if(!strcasecmp(ext, ".cue") || !strcasecmp(ext, ".ccd"))
			{
retry:			strcpy(ext, (++cue == 1) ? ".bin" : ".BIN");
			}

			show_msg(path);

			if(strstr(path, "/net"))
			{
			}
			else if(strstr(path, "/GAMES/") || strstr(path, "/GAMEZ/"))
			{
				err = cobra_map_game(path, "TEST00000", 1);
			}
			else if(strstr(path, "/ROMS/"))
			{
				err = cobra_map_game("/dev_hdd0//game/PKGLAUNCH", "PKGLAUNCH", 1);

				sys_map_path("/dev_bdvd/PS3_GAME/USRDIR/cores", "/dev_hdd0//game/RETROARCH/USRDIR/cores");
				sys_map_path("/app_home/PS3_GAME/USRDIR/cores", "/dev_hdd0//game/RETROARCH/USRDIR/cores");

				int fd;
				if(cellFsOpen("/dev_hdd0//game/PKGLAUNCH/USRDIR/launch.txt", CELL_FS_O_CREAT | CELL_FS_O_WRONLY | CELL_FS_O_TRUNC, &fd, NULL, 0) == CELL_FS_SUCCEEDED)
				{
					cellFsWrite(fd, (void *)path, len, NULL);
					cellFsClose(fd);

					if(cellFsOpen("/dev_hdd0//game/PKGLAUNCH/PS3_GAME/PARAM.SFO", CELL_FS_O_WRONLY, &fd, NULL, 0) == CELL_FS_SUCCEEDED)
					{
						cellFsWriteWithOffset(fd, 0x378, (void *)path, 0x80, NULL);
						cellFsClose(fd);
					}
				}
			}
			else if (!strcasecmp(ext, ".iso") || !strcasecmp(ext, ".bin") || !strcasecmp(ext, ".img") || !strcasecmp(ext, ".mdf"))
			{
				if(strstr(path, "/PS3ISO/"))
				{
					err = cobra_mount_ps3_disc_image(files, 1);
				}
				else if(strstr(path, "/dev_hdd0/PS2ISO/"))
				{
					err = cobra_mount_ps2_disc_image(files, 1);
				}
				else if(strstr(path, "/PSXISO/"))
				{
					err = cobra_mount_psx_disc_image(path);
				}
				else if(strstr(path, "/BDISO/"))
				{
					err = cobra_mount_bd_disc_image(files, 1);
				}
				else if(strstr(path, "/DVDISO/"))
				{
					err = cobra_mount_dvd_disc_image(files, 1);
				}
			}

			if(!err)
			{
				exec_xmb_command("close_all_list");
				sys_timer_usleep(500000);
				cobra_send_fake_disc_insert_event();
			}
			else if(cue == 1) goto retry;
		}
	}

	if(err) show_msg("webMAN not ready!");
}

int prx_start(size_t args, void *argp)
{
	setInterface(*(unsigned int*)argp);
	return SYS_PRX_RESIDENT;
}

int prx_stop(void)
{
	return SYS_PRX_STOP_OK;
}
