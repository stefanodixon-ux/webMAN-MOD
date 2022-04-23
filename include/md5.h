#ifdef CALC_MD5

#include <cell/hash/libmd5.h>

static void calc_md5(const char *filename, char *md5)
{
	u64 _md5[2] = {0, 0};

	size_t buffer_size = _256KB_; sys_addr_t sysmem = sys_mem_allocate(buffer_size);
	if(!sysmem) {buffer_size = _128KB_; sysmem = sys_mem_allocate(buffer_size);}
	if(!sysmem) {buffer_size =  _64KB_; sysmem = sys_mem_allocate(buffer_size);}
	if( sysmem)
	{
		int fd;

		if(cellFsOpen(filename, CELL_FS_O_RDONLY, &fd, NULL, 0) == CELL_FS_SUCCEEDED)
		{
			CellMd5WorkArea workarea;

			cellMd5BlockInit(&workarea);

			u8 *buf = (u8 *)sysmem;

			for(u64 nread = buffer_size; nread > 0; )
			{
				cellFsRead(fd, buf, buffer_size, &nread);

				cellMd5BlockUpdate(&workarea, buf, nread);
			}

			cellFsClose(fd);
			cellMd5BlockResult(&workarea, (u8*)_md5);
		}

		sys_memory_free(sysmem);
	}

	// return md5 hash as a string
	sprintf(md5, "%016llx%016llx", _md5[0], _md5[1]);
}

#endif
