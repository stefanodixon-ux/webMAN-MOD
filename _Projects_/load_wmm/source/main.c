#include <ppu-lv2.h>
#include <sys/file.h>
#include <stdio.h>
#include <string.h>
#include <io/pad.h>

#define BUTTON_SQUARE        0x80
#define BUTTON_CROSS         0x40
#define BUTTON_CIRCLE        0x20
#define BUTTON_TRIANGLE      0x10
#define BUTTON_R1            0x08
#define BUTTON_L1            0x04
#define BUTTON_R2            0x02
#define BUTTON_L2            0x01

#define SC_SYS_POWER		(379)
#define SYS_REBOOT			0x8201

#define SC_RING_BUZZER  (392)

#define BEEP1 { lv2syscall3(SC_RING_BUZZER, 0x1004, 0x4,   0x6); }
#define BEEP2 { lv2syscall3(SC_RING_BUZZER, 0x1004, 0x7,  0x36); }
#define BEEP3 { lv2syscall3(SC_RING_BUZZER, 0x1004, 0xa, 0x1b6); }

#define SYSCALL8_OPCODE_UNLOAD_VSH_PLUGIN	0x364F
#define SYSCALL8_OPCODE_LOAD_VSH_PLUGIN		0x1EE7

#define SUCCESS 0
#define FAILED -1

#define PLUGIN_PATH		"/dev_hdd0/plugins/webftp_server.sprx"
#define PLUGIN_LINE		PLUGIN_PATH "\n"

static int cobra_unload_vsh_plugin(unsigned int slot)
{
	lv2syscall2(8, SYSCALL8_OPCODE_UNLOAD_VSH_PLUGIN, (u64)slot);
	return_to_user_prog(int);
}

static int cobra_load_vsh_plugin(unsigned int slot, const char *path, void *arg, u32 arg_size)
{
	lv2syscall5(8, SYSCALL8_OPCODE_LOAD_VSH_PLUGIN, slot, (u64)path, (u64)arg, arg_size);
	return_to_user_prog(int);
}

static bool file_exists(const char *path)
{
	sysFSStat stat;
	return (sysLv2FsStat(path, &stat) == SUCCESS);
}

int main()
{
	unsigned int slot = 1;
	unsigned int button = 0;

	padInfo padinfo;
	padData paddata;

	ioPadInit(7);

	int n, r;
	for(r = 0; r < 10; r++)
	{
		ioPadGetInfo(&padinfo);
		for(n = 0; n < 7; n++)
		{
			if(padinfo.status[n])
			{
				ioPadGetData(n, &paddata);
				button = (paddata.button[2] << 8) | (paddata.button[3] & 0xff);
				break;
			}
		}
		if(button) break; else usleep(20000);
	}
	ioPadEnd();

	if(button & (BUTTON_L1))    slot = 2;
	if(button & (BUTTON_L2))    slot = 3;
	if(button & (BUTTON_R1))    slot = 4;
	if(button & (BUTTON_R2))    slot = 5;
	if(button & (BUTTON_CROSS)) slot = 6;

	cobra_unload_vsh_plugin(slot);
	sleep(1);
	if(file_exists(PLUGIN_PATH))
	{
		if(button & (BUTTON_SQUARE))
		{
			FILE* f = fopen("/dev_hdd0/boot_plugins.txt", "w");
			if(f != NULL)
			{
				fwrite((const char*)PLUGIN_LINE, 1, sizeof(PLUGIN_LINE) - 1, f);
				fclose(f);
			}
			{lv2syscall3(SC_SYS_POWER, SYS_REBOOT, 0, 0); return_to_user_prog(int);}
		}
		cobra_load_vsh_plugin(slot, PLUGIN_PATH, NULL, 0);
	}
	else
		BEEP3;

	return 0;
}
