/*
* Copyright (c) 2021 by picard(aka 3141card)
* This file is released under the GPLv2.
*/

#define LV2_START    0x8000000000000000ULL
#define LV2_END      0x8000000000800000ULL
#define UFS2_MAGIC   0x19540119UL

/***********************************************************************
* search ufs superblock by picard(aka 3141card)
***********************************************************************/
static u64 get_ufs_sb_addr(void)
{
	u64 addr = (LV2_END - 0xA8);

	while(addr > LV2_START)
	{
		if((u32)(peekq(addr)) == UFS2_MAGIC)
			return (u64)(addr - 0x558);
		addr -= 0x100;
	}

	return 0;
}

/***********************************************************************
* unlock hdd by picard(aka 3141card)
***********************************************************************/
static void hdd_unlock_space(void)
{
	u64 sb_addr = get_ufs_sb_addr();

	if(sb_addr == 0)
	{
		BEEP3;  // fail
		return;
	}

	u32 minfree = lv2_peek_32(sb_addr + 0x3C);
	u32 optim   = lv2_peek_32(sb_addr + 0x80);

	// toggle: original / new
	if((minfree == 8) && (optim == 0))
	{
		minfree = 1;
		optim   = 1;
		BEEP1;  // success
	}
	else
	{
		minfree = 8;
		optim   = 0;
		BEEP2;  // success
	}

	// write patch
	lv2_poke_32(sb_addr + 0x3C, minfree);
	lv2_poke_32(sb_addr + 0x80, optim);
}
