#include <cell/cell_fs.h>
#include <sys/timer.h>
#include <sys/syscall.h>

#define LCASE(a)	(a | 0x20)
#define BETWEEN(a, b, c)	( ((a) <= (b)) && ((b) <= (c)) )

#define PS2_CLASSIC_ISO_CONFIG		"/dev_hdd0//game/PS2U10000/USRDIR/CONFIG"
#define PS2_CLASSIC_ISO_PATH		"/dev_hdd0//game/PS2U10000/USRDIR/ISO.BIN.ENC"

#define SC_FS_LINK						(810)
#define SC_SYS_POWER 					(379)
#define SYS_SOFT_REBOOT 				0x0200
#define SYS_HARD_REBOOT					0x1200
#define SYS_REBOOT						0x8201 /*load LPAR id 1*/
#define SYS_SHUTDOWN					0x1100

extern int stdc_B6D92AC3(const char *s1, const char *s2);								// strcasecmp()
int strcasecmp(const char *s1, const char *s2) {if(!s2 || !s2) return -1; return stdc_B6D92AC3(s1, s2);}
int sys_map_path(const char *oldpath, const char *newpath);

#include "cobra/cobra.h"
#include "cue_file.h"
#include "explore_plugin.h"

static int sysLv2FsLink(const char *oldpath, const char *newpath)
{
	system_call_2(SC_FS_LINK, (u64)(u32)oldpath, (u64)(u32)newpath);
	return_to_user_prog(int);
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

static u8 h2b(const char hex) // hex char to byte
{
	u8 c = LCASE(hex);
	if(BETWEEN('0', c, '9'))
		c -= '0'; // 0-9
	else if(BETWEEN('a', c, 'f'))
		c -= 'W'; // 10-15
	return c;
}

static int mount(const char * action)
{
	int err = 1;

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
		char *ext = path + (len > 4 ? len - 4 : 0);
		char *split = path + (len > 6 ? len - 6 : 0);
		char *files[64]; char parts[64][len + 2];

		int count = 1;
		if(!strcasecmp(split, ".iso.0") || !strcasecmp(split, ".ISO.0"))
		{
			struct CellFsStat s;

			for(int i = 0; i < 64; i++)
			{
				if(i < 10)
					split[5] = '0' + (int)(i % 10);
				else
				{
					split[5] = '0' + (int)(i / 10);
					split[6] = '0' + (int)(i % 10);
					split[7] = 0;
				}
				if(cellFsStat(path, &s) != CELL_FS_SUCCEEDED) break;
				strcpy(parts[i], path); files[i] = parts[i]; count = i + 1;
			}
			split[5] = '0'; split[6] = 0;
		}
		else
			files[0] = path;

		int cue = 0;

		TrackDef tracks[MAX_TRACKS];
		unsigned int num_tracks = 1;

		tracks[0].lba = 0;
		tracks[0].is_audio = 0;

		if(!strcasecmp(ext, ".cue"))
		{
retry:		strcpy(ext, (++cue == 1) ? ".bin" : ".BIN");
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
		else if (!strcasecmp(ext, ".enc") && strstr(path, "/PS2ISO/"))
		{
			cellFsUnlink(PS2_CLASSIC_ISO_PATH);
			sysLv2FsLink(path, PS2_CLASSIC_ISO_PATH);

			cellFsUnlink(PS2_CLASSIC_ISO_CONFIG);
			strcpy(path + len, ".CONFIG");
			sysLv2FsLink(path, PS2_CLASSIC_ISO_CONFIG);
			strcpy(path + len - 8, ".CONFIG");
			sysLv2FsLink(path, PS2_CLASSIC_ISO_CONFIG);

			err = 0; show_msg("Use PS2 Launcher to play the game");
		}
		else if (!strcasecmp(ext, ".iso") || !strcasecmp(ext, ".bin") || !strcasecmp(ext, ".img") || !strcasecmp(ext, ".mdf"))
		{
			if(strstr(path, "/PS3ISO/"))
			{
				err = cobra_mount_ps3_disc_image(files, count);
			}
			else if(strstr(path, "/dev_hdd0/PS2ISO/"))
			{
				err = cobra_mount_ps2_disc_image(files, count, tracks, num_tracks);
			}
			else if(strstr(path, "/PSXISO/"))
			{
				char iso_file[len + 1]; strcpy(iso_file, path);

				strcpy(ext, ".cue"); num_tracks = read_cue(path, tracks);
				if(!num_tracks)
				{
					strcpy(path, iso_file); strcpy(ext, ".CUE");
					num_tracks = read_cue(path, tracks); if(num_tracks < 1) num_tracks = 1;
				}

				err = cobra_mount_psx_disc_image(iso_file, tracks, num_tracks); strcpy(path, iso_file);
			}
			else if(strstr(path, "/PSPISO/"))
			{
				err = cobra_set_psp_umd(path, NULL, "/dev_hdd0/tmp/wm_icons/psp_icon.png");

				if(!err) show_msg("Use PSP Launcher to play the game");
			}
			else if(strstr(path, "/BDISO/"))
			{
				err = cobra_mount_bd_disc_image(files, count);
			}
			else if(strstr(path, "/DVDISO/"))
			{
				err = cobra_mount_dvd_disc_image(files, count);
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
	else if(strstr(action, "/restart.ps3") == action)
	{
		system_call_3(SC_SYS_POWER, SYS_SOFT_REBOOT, NULL, 0);
	}
	else if(strstr(action, "/shutdown.ps3") == action)
	{
		system_call_4(SC_SYS_POWER, SYS_SHUTDOWN, 0, 0, 0);
	}
	
	return err;
}