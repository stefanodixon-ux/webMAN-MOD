bool file_exists( char* path )
{
	struct stat st;
	return ps3ntfs_stat(path, &st) >= SUCCESS;
}

bool not_exists( char* path )
{
	struct stat st;
	return ps3ntfs_stat(path, &st) < SUCCESS;
}

int SaveFile(char *path, char *mem, int file_size)
{
	sysLv2FsUnlink((char*)path);

	int fd = ps3ntfs_open(path, O_CREAT | O_WRONLY | O_TRUNC, 0);
	if(fd >= 0)
	{
		if(file_size != ps3ntfs_write(fd, mem, file_size))
		{
			ps3ntfs_close(fd);
			return FAILED;
		}
		ps3ntfs_close(fd);
	}
	else
		return FAILED;
/*
	FILE *fp;

	fp = fopen(path, "wb");

	if (fp)
	{
		if(file_size != fwrite((void *) mem, 1, file_size, fp))
		{
			fclose(fp);
			return FAILED;
		}
		fclose(fp);
	}
	else
		return FAILED;
*/
	sysLv2FsChmod(path, FS_S_IFMT | 0777);

	return SUCCESS;
}

u64 get_filesize(char *path)
{
	struct stat st;
	if (ps3ntfs_stat(path, &st) < 0) return 0ULL;
	return st.st_size;
}

int readfile_ntfs(char *file, char *data, u64 size)
{
	if(size==0) return FAILED;

	int fd = ps3ntfs_open(file, O_RDONLY, 0);
	if(fd >= 0)
	{
		int re = ps3ntfs_read(fd, (void *)data, size);
		ps3ntfs_close(fd);

		return (re == size) ? SUCCESS : FAILED;
	}

	return FAILED;
}

int copy_file(char *src_file, char *out_file)
{
	u64 size=get_filesize(src_file);
	if(size==0) return FAILED;

	int fd = ps3ntfs_open(src_file, O_RDONLY, 0);
	if(fd >= 0)
	{
		char *mem = NULL;

		if((mem = malloc(size)) != NULL)
		{
			ps3ntfs_seek64(fd, 0, SEEK_SET);
			int re = ps3ntfs_read(fd, (void *) mem, size);
			ps3ntfs_close(fd);

			SaveFile(out_file, mem, re);

			free(mem);

			return (re == size) ? SUCCESS : FAILED;
		}
		else
			ps3ntfs_close(fd);
	}
	return FAILED;
}

/* 	By Evilnat

	If disckey ('.key' file) exists, copy it to CACHE_PATH to
	decrypt on-the-fly with Cobra when the ISO is mounted

	If dkey ('.dkey' file) exists, we will transform it to disckey and
	copy it to CACHE_PATH
*/
static void convert_dkey_to_key(uint8_t disckey[0x10], char dkey[0x20])
{
	for (int i = 0; i < 0x10; i++)
	{
		char byte[3];
		strncpy(byte, &dkey[i * 2], 2);
		byte[2] = 0;
		disckey[i] = strtol(byte, NULL, 16);
	}
}

static void cache_disckey(char *ext, char *full_path, char *direntry, char *filename)
{
	if(strcasestr(ext, ".key") || strcasestr(ext - 1, ".dkey"))
	{
		char *key_path = image_file;

		sprintf(key_path, "%s/%s", full_path, direntry);
		int wlen = snprintf(wm_path, 255, CACHE_PATH "/%s", filename);

		if(strcasestr(ext - 1, ".dkey"))
		{
			char dkey[0x20];
			if(!readfile_ntfs(key_path, dkey, 0x20))
			{
				strcpy(wm_path + wlen - 5, ".key");

				uint8_t disckey[0x10];
				convert_dkey_to_key(disckey, dkey);
				SaveFile(wm_path, (char *)disckey, 0x10);
			}
		}
		else
			copy_file(key_path, wm_path);
	}
}
