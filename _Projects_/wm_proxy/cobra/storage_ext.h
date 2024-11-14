#include "storage.h"

int sys_map_path(const char *oldpath, const char *newpath)
{
	char *paths[1] = { (char*)oldpath }, *new_paths[1] = { (char*)newpath };
	system_call_4(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_MAP_PATHS, (u64)(u32)paths, (u64)(u32)new_paths, 1);
	return (int)p1;
}

/* int sys_map_paths(char *paths[], char *new_paths[], unsigned int num)
{
	system_call_4(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_MAP_PATHS, (u64)(u32)paths, (u64)(u32)new_paths, num);
	return (int)p1;
} */

// storage.h inline functions merged
/*
int sys_storage_open(u64 device_id, u64 unk, sys_device_handle_t *device_handle, u64 unk2)
{
	system_call_4(SYS_STORAGE_OPEN, device_id, unk, (u64)(u32)device_handle, unk2);
	return (int)p1;
}

int sys_storage_close(sys_device_handle_t device_handle)
{
	system_call_1(SYS_STORAGE_CLOSE, device_handle);
	return (int)p1;
}
*/
static int sys_storage_ext_get_emu_state(sys_emu_state_t *state)
{
	system_call_2(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_GET_EMU_STATE, (u64)(u32)state);
	return (int)p1;
}

static int sys_storage_ext_mount_ps3_discfile(unsigned int filescount, char *files[])
{
	system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_MOUNT_PS3_DISCFILE, filescount, (u64)(u32)files);
	return (int)p1;
}

static int sys_storage_ext_mount_dvd_discfile(unsigned int filescount, char *files[])
{
	system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_MOUNT_DVD_DISCFILE, filescount, (u64)(u32)files);
	return (int)p1;
}

static int sys_storage_ext_mount_bd_discfile(unsigned int filescount, char *files[])
{
	system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_MOUNT_BD_DISCFILE, filescount, (u64)(u32)files);
	return (int)p1;
}

static int sys_storage_ext_mount_psx_discfile(char *file, unsigned int trackscount, ScsiTrackDescriptor *tracks)
{
	system_call_4(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_MOUNT_PSX_DISCFILE, (u64)(u32)file, trackscount, (u64)(u32)tracks);
	return (int)p1;
}

static int sys_storage_ext_mount_ps2_discfile(unsigned int filescount, char *files[], unsigned int trackscount, ScsiTrackDescriptor *tracks)
{
	system_call_5(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_MOUNT_PS2_DISCFILE, filescount, (u64)(u32)files, trackscount, (u64)(u32)tracks);
	return (int)p1;
}

int sys_storage_ext_get_disc_type(unsigned int *real_disctype, unsigned int *effective_disctype, unsigned int *fake_disctype)
{
	system_call_4(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_GET_DISC_TYPE, (u64)(u32)real_disctype, (u64)(u32)effective_disctype, (u64)(u32)fake_disctype);
	return (int)p1;
}

int sys_storage_ext_umount_discfile(void)
{
	system_call_1(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_UMOUNT_DISCFILE);
	return (int)p1;
}

int sys_storage_ext_fake_storage_event(u64 event, u64 param, u64 device)
{
	system_call_4(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_FAKE_STORAGE_EVENT, event, param, device);
	return (int)p1;
}

/*
static int sys_storage_ext_mount_encrypted_image(char *image, char *mount_point, char *filesystem, u64 nonce)
{
	system_call_5(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_MOUNT_ENCRYPTED_IMAGE, (u64)(u32)image, (u64)(u32)mount_point, (u64)(u32)filesystem, nonce);
	return (int)p1;
}
*/

/* static int sys_storage_ext_read_ps3_disc(void *buf, u64 start_sector, u32 count)
{
	system_call_4(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_READ_PS3_DISC, (u64)(u32)buf, start_sector, count);
	return (int)p1;
} */

/* static int sys_storage_send_device_command(sys_device_handle_t device_handle, unsigned int command, void *indata, u64 inlen, void *outdata, u64 outlen)
{
	system_call_6(SYS_STORAGE_SEND_DEVICE_COMMAND, device_handle, command, (u64)(u32)indata, inlen, (u64)(u32)outdata, outlen);
	return (int)p1;
} */

//int sys_storage_execute_device_command(sys_device_handle_t device_handle, unsigned int command, void *indata, u64 inlen, void *outdata, u64 outlen, u64 *ret);

// Not really storage funcs, but...
/*
static int sys_ss_disc_auth(u64 func, u64 param)
{
	system_call_2(864, func, param);
	return (int)p1;
}
*/

/* static int sys_ss_disc_auth(u64 func, u64 param)
{
	system_call_2(864, func, param);
	return (int)p1;
} */
/*
typedef struct
{
	char mount_point[32];
	char filesystem[32];
	char block_dev[32];
	u8 unk[52];
} __attribute__((packed)) CellFsMountInfo;

int cellFsUtilMount(const char *block_dev, const char *filesystem, const char *mount_point, int unk, int read_only, int unk2, char *argv[], int argc);
int cellFsUtilUmount(const char *mount_point, u64 unk, int force);
*/
/* static int cellFsUtilGetMountInfo(CellFsMountInfo *info, u64 size, u64 *rsize)
{
	system_call_3(842, (u64)(u32)info, size, (u64)(u32)rsize);
	return (int)p1;
} */

/* static int cellFsUtilGetMountInfoSize(u64 *size)
{
	system_call_1(841, (u64)(u32)size);
	return (int)p1;
} */
/*
int cellFsUtilGetSpaceInfo(const char *mount_point, u64 *total_size, u64 *free_size);

static u64 get_device(char *name) __attribute__((unused));
static u64 get_device(char *name)
{
	if (strcmp(name, "CELL_FS_IOS:ATA_HDD") == 0)
		return ATA_HDD;

	if (strcmp(name, "CELL_FS_IOS:BDVD_DRIVE") == 0)
		return BDVD_DRIVE;

	if (strcmp(name, "CELL_FS_IOS:PATA0_HDD_DRIVE") == 0)
		return PATA0_HDD_DRIVE;

	if (strcmp(name, "CELL_FS_IOS:PATA0_BDVD_DRIVE") == 0)
		return PATA0_BDVD_DRIVE;

	if (strcmp(name, "CELL_FS_IOS:PATA1_HDD_DRIVE") == 0)
		return PATA1_HDD_DRIVE;

	if (strcmp(name, "CELL_FS_IOS:PATA1_BDVD_DRIVE") == 0)
		return PATA1_BDVD_DRIVE;

	if (strcmp(name, "CELL_FS_IOS:BUILTIN_FLASH") == 0)
		return BUILTIN_FLASH;

	if (strcmp(name, "CELL_FS_IOS:COMPACT_FLASH") == 0)
		return COMPACT_FLASH;

	if (strcmp(name, "CELL_FS_IOS:MEMORY_STICK") == 0)
		return MEMORY_STICK;

	if (strcmp(name, "CELL_FS_IOS:SD_CARD") == 0)
		return SD_CARD;

	if (strncmp(name, "CELL_FS_IOS:USB_MASS_STORAGE", 28) == 0)
	{
		if (!isdigit(name[28]) || !isdigit(name[29]) || !isdigit(name[30]) || name[31] != 0)
			return 0;

		unsigned int num = ((name[28]-'0')*100) + ((name[29]-'0')*10) + (name[30]-'0');

		if (num > 127)
			return 0;

		return (num < 6) ? USB_MASS_STORAGE_1(num) : USB_MASS_STORAGE_2(num);
	}

	if (strcmp(name, "CELL_FS_IOS:BUILTIN_FLSH1") == 0)
		return FLASH_PARTITION(2);

	if (strcmp(name, "CELL_FS_IOS:BUILTIN_FLSH2") == 0)
		return FLASH_PARTITION(3);

	if (strcmp(name, "CELL_FS_IOS:BUILTIN_FLSH3") == 0)
		return FLASH_PARTITION(4);

	if (strcmp(name, "CELL_FS_IOS:BUILTIN_FLSH4") == 0)
		return FLASH_PARTITION(5);

	if (strcmp(name, "CELL_FS_UTILITY:HDD0") == 0)
		return HDD_PARTITION(1);

	if (strcmp(name, "CELL_FS_UTILITY:HDD1") == 0)
		return HDD_PARTITION(2);

	if (strcmp(name, "CELL_FS_UTILITY:HDD2") == 0)
		return HDD_PARTITION(3);

	if (strcmp(name, "CELL_FS_UTILITY:HDD") == 0)
		return ATA_HDD;

	return 0;
}
*/
