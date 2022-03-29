#ifdef USE_NTFS

#define DEV_NTFS	"/dev_nt"

static bool is_ntfs_path(const char *path)
{
	if(!path) return false;

	return islike(path, DEV_NTFS) || islike(path, "ntfs");
}

static const char *ntfs_path(const char *path)
{
	if(!path) return NULL;

	char *npath = (char*)path;
	if(islike(npath, DEV_NTFS))
	{
		npath[10] = ':'; // /dev_ntfs0:
		return npath + 5; //  => ntfs0:
	}
	if(islike(npath, "ntfs"))
	{
		npath[5] = ':';  // ntfs0:
	}
	return npath;
}

static void unmount_all_ntfs_volumes(void)
{
	if(mounts)
		for(int u = 0; u < mountCount; u++) ntfsUnmount(mounts[u].name, 1);

	if(mounts) free(mounts); mounts = NULL;
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
			if(pdir) ps3ntfs_dirclose(pdir); else mountCount = NTFS_UNMOUNTED;
		}
	}

	if(mountCount <= 0)
		mount_all_ntfs_volumes();
}

static DIR_ITER *ps3ntfs_opendir(const char *full_path)
{
	if(mountCount <= 0) mount_all_ntfs_volumes();

	char *path = (char *)ntfs_path(full_path);  // /dev_ntfs1v -> ntfs1:

	if(!path) return NULL;

	if(path[6] != '/') {path[6] = '/', path[7] = 0;}

	return ps3ntfs_diropen(path);
}
#endif