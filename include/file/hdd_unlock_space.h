/*
* Copyright (c) 2021 by picard(aka 3141card)
* This file is released under the GPLv2.
*/
#ifndef LITE_EDITION

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
static u32 hdd_unlock_space(char unlock, u8 opt)
{
	u64 sb_addr = get_ufs_sb_addr();

	if(sb_addr == 0)
	{
		BEEP3;  // fail
		return 0;
	}

	u32 minfree;
	u32 optim;

	if(unlock == 'e') unlock = 1; else // enable
	if(unlock == 'd') unlock = 0; else // disable
	if(ISDIGIT(unlock)) unlock -= '0'; else // 0-9
	{
		minfree = lv2_peek_32(sb_addr + 0x3C);
		optim   = lv2_peek_32(sb_addr + 0x80);
		unlock  = (minfree == 8) && (optim == 0); // toggle
	}

	// toggle: original / new
	if(unlock)
	{
		minfree = unlock;
		optim   = opt;
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

	return minfree | (optim<<8);
}

#endif //#ifndef LITE_EDITION
