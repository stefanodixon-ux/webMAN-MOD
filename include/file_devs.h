#define NO_MSG							NULL

#define SC_BDVD_DECRYPT 				(36)
#define SC_FS_MOUNT 					(837)
#define SC_FS_UMOUNT					(838)
#define SC_FS_DISK_FREE 				(840)

//////////////////////////////////////////////////////////////

/*
static u64 syscall_837(const char *device, const char *format, const char *point, u32 a, u32 b, u32 c, void *buffer, u32 len)
{
	system_call_8(SC_FS_MOUNT, (u64)device, (u64)format, (u64)point, a, b, c, (u64)buffer, len);
	return_to_user_prog(u64);
}
*/

static int sysLv2FsBdDecrypt(void)
{
	system_call_1(SC_BDVD_DECRYPT, (u64) "/dev_bdvd");
	return_to_user_prog(int);
}

static void mount_device(const char *dev_path, const char *dev_name, const char *file_system)
{
	if(!sys_admin) return;

	if(!dev_path || (*dev_path != '/') || isDir(dev_path)) return;

	if(islike(dev_path, "/dev_blind"))
		{system_call_8(SC_FS_MOUNT, (u64)(char*)"CELL_FS_IOS:BUILTIN_FLSH1", (u64)(char*)"CELL_FS_FAT", (u64)(char*)"/dev_blind", 0, 0, 0, 0, 0);}
	else if(!dev_name || !file_system || islike(dev_path, "/dev_hdd1"))
		{system_call_8(SC_FS_MOUNT, (u64)(char*)"CELL_FS_UTILITY:HDD1", (u64)(char*)"CELL_FS_FAT", (u32)dev_path, 0, 0, 0, 0, 0);}
	else if(islike(dev_name, "CELL_FS_") && islike(file_system, "CELL_FS_"))
		{system_call_8(SC_FS_MOUNT, (u32)dev_name, (u32)file_system, (u32)dev_path, 0, 0, 0, 0, 0);}
}

static void enable_dev_blind(const char *msg)
{
	if(!sys_admin) return;

	mount_device("/dev_blind", NULL, NULL);

	if(!msg) return;

	show_msg(msg);
	sys_ppu_thread_sleep(2);
}

static void disable_dev_blind(void)
{
	system_call_3(SC_FS_UMOUNT, (u64)(char*)"/dev_blind", 0, 1);
}

#ifdef USE_NTFS
static void get_ntfs_disk_space(const char *dev_name, u64 *freeSize, u64 *devSize)
{
	struct statvfs vbuf;
	ps3ntfs_statvfs(dev_name, &vbuf);
	*freeSize = ((u64)vbuf.f_bfree * (u64)vbuf.f_bsize);
	*devSize  = (u64)vbuf.f_blocks * (u64)vbuf.f_bsize;
}
#endif

static u64 get_free_space(const char *dev_name)
{
	u64 freeSize = 0, devSize = 0;

#ifdef USE_NTFS
	if(is_ntfs_path(dev_name))
	{
		char ntfs[8];
		strncpy(ntfs, ntfs_path(dev_name), 8); ntfs[7] = 0; // ntfs0:/
		get_ntfs_disk_space(ntfs, &freeSize, &devSize);
		return freeSize;
	}
#endif
	if(!islike(dev_name, "/dev_")) return 0;

	{system_call_3(SC_FS_DISK_FREE, (u64)(u32)(dev_name), (u64)(u32)&devSize, (u64)(u32)&freeSize);}
	return freeSize;
/*
	u32 blockSize;
	u64 freeSize;

	if(cellFsGetFreeSize(dev_name, &blockSize, &freeSize)  == CELL_FS_SUCCEEDED) return (freeSize * blockSize);
	return 0;
*/
}
