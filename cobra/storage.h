#ifndef __STORAGE_H__
#define __STORAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/syscall.h>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "scsi.h"
#include "syscall8.h"

#define MAX_PATH		0x420

#define ATA_HDD				0x101000000000007ULL
#define BDVD_DRIVE			0x101000000000006ULL
#define PATA0_HDD_DRIVE		0x101000000000008ULL
#define PATA0_BDVD_DRIVE	BDVD_DRIVE
#define PATA1_HDD_DRIVE		ATA_HDD
#define PATA1_BDVD_DRIVE	0x101000000000009ULL
#define BUILTIN_FLASH		0x100000000000001ULL
#define MEMORY_STICK		0x103000000000010ULL
#define SD_CARD				0x103000100000010ULL
#define COMPACT_FLASH		0x103000200000010ULL

#define USB_MASS_STORAGE_1(n)	(0x10300000000000AULL+n) /* For 0-5 */
#define USB_MASS_STORAGE_2(n)	(0x10300000000001FULL+(n-6)) /* For 6-127 */

#define	HDD_PARTITION(n)	(ATA_HDD | ((uint64_t)n<<32))
#define FLASH_PARTITION(n)	(BUILTIN_FLASH | ((uint64_t)n<<32))

#define DEVICE_TYPE_PS3_DVD	0xFF70
#define DEVICE_TYPE_PS3_BD	0xFF71
#define DEVICE_TYPE_PS2_CD	0xFF60
#define DEVICE_TYPE_PS2_DVD	0xFF61
#define DEVICE_TYPE_PSX_CD	0xFF50
#define DEVICE_TYPE_BDROM	0x40
#define DEVICE_TYPE_BDMR_SR	0x41 /* Sequential record */
#define DEVICE_TYPE_BDMR_RR 0x42 /* Random record */
#define DEVICE_TYPE_BDMRE	0x43
#define DEVICE_TYPE_DVD		0x10 /* DVD-ROM, DVD+-R, DVD+-RW etc, they are differenced by booktype field in some scsi command */
#define DEVICE_TYPE_CD		0x08 /* CD-ROM, CD-DA, CD-R, CD-RW, etc, they are differenced somehow with scsi commands */

#define STORAGE_COMMAND_NATIVE		0x01
#define STORAGE_COMMAND_GET_DEVICE_SIZE	0x10
#define STORAGE_COMMAND_GET_DEVICE_TYPE	0x11

typedef uint32_t sys_device_handle_t;

typedef struct
{
	char label[32];
	uint32_t unk_08;
	uint32_t unk_0C;
	uint64_t sector_count;
	uint32_t sector_size;
	uint32_t unk_30;
	uint8_t unk_40[8];
} __attribute__((packed)) sys_device_info_t;

/*
typedef struct
{
	uint32_t inlen;
	uint32_t unk1;
	uint32_t outlen;
	uint32_t unk2;
	uint32_t unk3;
} __attribute__((packed)) StorageCmdScsiData;
*/

/* Payload storage extensions */

typedef struct
{
	int size;
	int disc_emulation;
	char firstfile_path[MAX_PATH];
} __attribute__((packed)) sys_emu_state_t;

int sys_storage_open(uint64_t device_id, uint64_t unk, sys_device_handle_t *device_handle, uint64_t unk2);
int sys_storage_read(sys_device_handle_t device_handle, uint64_t mode, uint64_t start_sector, uint32_t sector_count, void *buf, uint32_t *nread, uint64_t flags);
int sys_storage_close(sys_device_handle_t device_handle);

#ifdef COBRA_ONLY

int sys_storage_get_device_info(uint64_t device_id, sys_device_info_t *device_info);
int sys_storage_ext_fake_storage_event(uint64_t event, uint64_t param, uint64_t device);
int sys_storage_ext_get_disc_type(unsigned int *real_disctype, unsigned int *effective_disctype, unsigned int *fake_disctype);
int sys_storage_ext_umount_discfile(void);
int sys_map_path(const char *oldpath, const char *newpath);

#endif //#ifdef COBRA_ONLY

#ifdef __cplusplus
}
#endif

#endif /* __STORAGE_H__ */
