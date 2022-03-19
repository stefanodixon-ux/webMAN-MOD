#ifdef USE_NTFS

#define DEV_NTFS	"/dev_nt"

static bool is_ntfs_path(const char *path)
{
	return islike(path, DEV_NTFS);
}

static bool is_ntfs_path2(const char *path)
{
	return islike(path, DEV_NTFS) || islike(path, "ntfs");
}

static const char *ntfs_path(const char *path)
{
	return is_ntfs_path(path) ? path + 5 : path;
}

static void unmount_all_ntfs_volumes(void)
{
	if(mounts && (mountCount > 0))
		for(u8 u = 0; u < mountCount; u++) ntfsUnmount(mounts[u].name, 1);

	if(mounts) free(mounts);
}

static void mount_all_ntfs_volumes(void)
{
	unmount_all_ntfs_volumes();
	mountCount = ntfsMountAll(&mounts, NTFS_SU | NTFS_FORCE );
	if(mountCount <= 0) {mountCount = NTFS_UNMOUNTED;}
}

static u32 ftp_ntfs_transfer_in_progress = 0;

static void check_ntfs_volumes(void)
{
	root_check = false;

	if((mountCount > 0) && (!ftp_ntfs_transfer_in_progress))
	{
		DIR_ITER *pdir; char path[40];
		for(int i = 0; i < mountCount; i++)
		{
			snprintf(path, sizeof(path), "%s:/", mounts[i].name);
			pdir = ps3ntfs_diropen(path);
			if(pdir) ps3ntfs_dirclose(pdir); else { mountCount = NTFS_UNMOUNTED; break; }
		}
	}

	if(mountCount <= 0)
		for(u8 retry = 0; retry < 2; retry++)
		{
			mount_all_ntfs_volumes();
			if(mountCount > 0) break;
			sys_ppu_thread_sleep(2);
		}
}

static DIR_ITER *ps3ntfs_opendir(const char *full_path)
{
	if(mountCount <= 0) mount_all_ntfs_volumes();

	char *path = (char *)ntfs_path(full_path);

	path[5] = ':';
	if(path[6] != '/') {path[6] = '/', path[7] = 0;}

	return ps3ntfs_diropen(path);  // /dev_ntfs1v -> ntfs1:
}
#endif