#ifndef __NETISO_H__
#define __NETISO_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NETISO_PORT	38008
/*
typedef struct
{
	char server[0x40];
	char path[0x420];
	u32 emu_mode;
	u32 numtracks;
	u16 port;
	u8 pad[6];
	ScsiTrackDescriptor tracks[32];
} __attribute__((packed)) netiso_args;
*/
enum NETISO_CMD
{
	/* Closes the active ro file (if any) and open/stat a new one */
	NETISO_CMD_OPEN_FILE = 0x1224,
	/* Reads the active ro file. Offsets and sizes in bytes. If file read fails, client is exited. Only read data is returned. */
	NETISO_CMD_READ_FILE_CRITICAL,
	/* Reads 2048 sectors in a 2352 sectors iso.
	 * Offsets and sizes in sectors. If file read fails, client is exited */
	NETISO_CMD_READ_CD_2048_CRITICAL,
	/* Reads the active ro file. Offsets and sizes in bytes. It returns number of bytes read to client, -1 on error, and after that, the data read (if any)
	   Only up to the BUFFER_SIZE used by server can be red at one time*/
	NETISO_CMD_READ_FILE,
	/* Closes the active wo file (if any) and opens+truncates or creates a new one */
	NETISO_CMD_CREATE_FILE,
	/* Writes to the active wo file. After command, data is sent. It returns number of bytes written to client, -1 on erro.
	   If more than BUFFER_SIZE used by server is specified in command, connection is aborted. */
	NETISO_CMD_WRITE_FILE,
	/* Closes the active directory (if any) and opens a new one */
	NETISO_CMD_OPEN_DIR,
	/* Reads a directory entry. and returns result. If no more entries or an error happens, the directory is automatically closed. . and .. are automatically ignored */
	NETISO_CMD_READ_DIR_ENTRY,
	/* Deletes a file. */
	NETISO_CMD_DELETE_FILE,
	/* Creates a directory */
	NETISO_CMD_MKDIR,
	/* Removes a directory (if empty) */
	NETISO_CMD_RMDIR,
	/* Reads a directory entry (v2). and returns result. If no more entries or an error happens, the directory is automatically closed. . and .. are automatically ignored */
	NETISO_CMD_READ_DIR_ENTRY_V2,
	/* Stats a file or directory */
	NETISO_CMD_STAT_FILE,
	/* Gets a directory size */
	NETISO_CMD_GET_DIR_SIZE,

	/* Get complete directory contents */
	NETISO_CMD_READ_DIR,

	/* Replace this with any custom command */
	NETISO_CMD_CUSTOM_0 = 0x2412,
};

typedef struct _netiso_cmd
{
	u16 opcode;
	u8 data[14];
} __attribute__((packed)) netiso_cmd;

typedef struct _netiso_stat_cmd
{
	u16 opcode;
	u16 fp_len;
	u8 pad[12];
} __attribute__((packed)) netiso_stat_cmd;

typedef struct _netiso_stat_result
{
	s64 file_size; // Files: file size, directories: 0, error: -1
	u64 mtime;
	u64 ctime;
	u64 atime;
	s8 is_directory; // Files: 0, directory: 1, error: shouldn't be read.
} __attribute__((packed)) netiso_stat_result;

typedef struct _netiso_read_file_cmd
{
	u16 opcode;
	u16 pad;
	u32 num_bytes;
	u64 offset;
} __attribute__((packed)) netiso_read_file_cmd;

typedef struct _netiso_read_file_result
{
	s32 bytes_read;
} __attribute__((packed)) netiso_read_file_result;

typedef struct _netiso_open_cmd
{
	u16 opcode;
	u16 fp_len;
	u8 pad[12];
} __attribute__((packed)) netiso_open_cmd;

typedef struct _netiso_open_result
{
	s64 file_size; // -1 on error
	u64 mtime;
} __attribute__((packed)) netiso_open_result;

typedef struct _netiso_open_dir_cmd
{
	u16 opcode;
	u16 dp_len;
	u8 pad[12];
} __attribute__((packed)) netiso_open_dir_cmd;

typedef struct _netiso_open_dir_result
{
	s32 open_result; // 0 success, -1 error
} __attribute__((packed)) netiso_open_dir_result;

typedef struct _netiso_read_dir_entry_cmd
{
	u16 opcode;
	u8 pad[14];
} __attribute__((packed)) netiso_read_dir_entry_cmd;

typedef struct _netiso_read_dir_entry_result
{
	s64 file_size; // Files: file size, directories: 0, error or no more entries: -1
	u16 fn_len;
	s8 is_directory; // Files: 0, directory: 1, error: shouldn't be read.
} __attribute__((packed)) netiso_read_dir_entry_result;

typedef struct _netiso_read_dir_result
{
	s64 dir_size;
} __attribute__((packed)) netiso_read_dir_result;

typedef struct _netiso_read_dir_result_data
{
	s64 file_size;
	u64 mtime;
	s8 is_directory;
	char name[512];
} __attribute__((packed)) netiso_read_dir_result_data;

typedef struct _netiso_read_file_critical_cmd
{
	u16 opcode;
	u16 pad;
	u32 num_bytes;
	u64 offset;
} __attribute__((packed)) netiso_read_file_critical_cmd;

typedef struct _netiso_read_cd_2048_critical_cmd
{
	u16 opcode;
	u16 pad;
	u32 start_sector;
	u32 sector_count;
	u32 pad2;
} __attribute__((packed)) netiso_read_cd_2048_critical_cmd;

#ifdef __BIG_ENDIAN__

static inline u16 BE16(u16 x)
{
	return x;
}

static inline u32 BE32(u32 x)
{
	return x;
}

static inline u64 BE64(u64 x)
{
	return x;
}

#else

static inline u16 BE16(u16 x)
{
	u16 ret  = ((x<<8)&0xFF00);
		ret |= ((x>>8)&0xFF);

	return ret;
}

static inline u32 BE32(u32 x)
{
	u32 ret  = (((x) & 0xff) << 24);
		ret |= (((x) & 0xff00) << 8);
		ret |= (((x) & 0xff0000) >> 8);
		ret |= (((x) >> 24) & 0xff);

	return ret;
}

static inline u64 BE64(u64 x)
{
	u64 ret  = ((x << 56) & 0xff00000000000000ULL);
		ret |= ((x << 40) & 0x00ff000000000000ULL);
		ret |= ((x << 24) & 0x0000ff0000000000ULL);
		ret |= ((x <<  8) & 0x000000ff00000000ULL);
		ret |= ((x >>  8) & 0x00000000ff000000ULL);
		ret |= ((x >> 24) & 0x0000000000ff0000ULL);
		ret |= ((x >> 40) & 0x000000000000ff00ULL);
		ret |= ((x >> 56) & 0x00000000000000ffULL);

	return ret;
}

#endif

#ifdef __cplusplus
}
#endif

#endif
