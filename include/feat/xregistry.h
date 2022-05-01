#include "../../vsh/xregistry.h"

#ifndef LITE_EDITION

#ifdef DEBUG_XREGISTRY

static u32 get_xreg_value(const char *key, int new_value, char *str_value, bool read_only)
{
	int reg = NONE;
	int reg_value = new_value;
	u16 off_string, len_data, len_string;
	u64 r;
	char string[256];

	if(cellFsOpen("/dev_flash2/etc/xRegistry.sys", CELL_FS_O_RDONLY, &reg, NULL, 0) != CELL_FS_SUCCEEDED || reg == NONE)
	{
		return reg_value;
	}

	CellFsStat stat;
	cellFsStat("/dev_flash2/etc/xRegistry.sys", &stat);
	u64 entry_offset = 0x10000;

	while(true)
	{
	//// Data entries ////
		//unk
		entry_offset += 2;

		//relative entry offset
		cellFsReadWithOffset(reg, entry_offset, &off_string, 2, &r);
		entry_offset += 4;

		//data lenght
		cellFsReadWithOffset(reg, entry_offset, &len_data, 2, &r);
		entry_offset += 3;

	//// String Entries ////
		off_string += 0x12;

		//string length
		cellFsReadWithOffset(reg, off_string, &len_string, 2, &r);
		off_string += 3;

		//string
		_memset(string, sizeof(string));
		cellFsReadWithOffset(reg, off_string, string, len_string, &r);

		//Find key
		if(IS(string, key))
		{
			if(read_only)
			{
				if(len_data == 4)
					cellFsReadWithOffset(reg, entry_offset, &reg_value, 4, &r);
				else
					cellFsReadWithOffset(reg, entry_offset, str_value, len_data, &r);
			}
			else
			{
				cellFsClose(reg);
				if(cellFsOpen("/dev_flash2/etc/xRegistry.sys", CELL_FS_O_WRONLY, &reg, NULL, 0) == CELL_FS_SUCCEEDED)
				{
					if(len_data == 4)
						cellFsWriteWithOffset(reg, entry_offset, &new_value, 4, &r);
					else
						cellFsWriteWithOffset(reg, entry_offset, str_value, strlen(str_value), &r);
				}
			}
			break;
		}

		entry_offset += len_data + 1;

		if(off_string == 0xCCDD || entry_offset >= stat.st_size) break;
	}

	cellFsClose(reg);

	return reg_value;
}
#endif // #ifdef DEBUG_XREGISTRY

#ifdef PS3_BROWSER
static int get_xreg_entry_size(int id)
{
	if(id >= 0x28 && id < 0x3D)
		return 1;

	switch (id)
	{
		case 0x09: // "/setting/np/titleId"
		case 0x19: // "/setting/np/tppsProxyServer"
		case 0x1B: // "/setting/np/tppsProxyUserName"
		case 0x1C: // "/setting/np/tppsProxyPassword"
		case 0x20: // "/setting/system/hddSerial"
		case 0x26: // "/setting/system/updateServerUrl"
		case 0x2D: // "/setting/system/debugDirName"
		case 0x3D: // "/setting/system/bootMode"
		case 0x63: // "/setting/libad/adServerURL"
		case 0x64: // "/setting/libad/adCatalogVersion"
		case 0x6C: // "/setting/net/adhocSsidPrefix"
		case 0x7C: // "/setting/wboard/baseUri"
					return 0x80;
		case 0x5E:
		case 0x6E:
		case 0x78:
		case 0x79:
		case 0x82:
		case 0x83:
		case 0xC8:
		case 0xD2:
		case 0xD3:
		case 0xDC:
		case 0xDD:
					return 0x81;
		case 0x6F:
		case 0x96:
					return 1;
	}
	return 0;
}
#endif // #ifdef PS3_BROWSER

#endif // #ifndef LITE_EDITION
