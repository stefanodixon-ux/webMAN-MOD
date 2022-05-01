/*
//////////// Toggle DLNA /////////////
static u8* (*paf_AF58E756)(void);
static int (*paf_350B4536)(void *job, int(*handler1)(void), void * param1, int r6, int r7, u8(*handler2)(void));

static int handler1_enabled(void)
{
	return vshmain_5F5729FB(0xC);
}
static int handler1_disabled(void)
{
	return vshmain_74A54CBF(0xC);
}
static u8 handler2(void)
{
	return paf_AF58E756()[0x3C];
}
static int Job_start(void *job, int(*handler1)(void), void * param1, int r6, int r7, u8(*handler2)(void))
{
	paf_AF58E756 = getNIDfunc("paf", 0xAF58E756, 0);
	paf_350B4536 = getNIDfunc("paf", 0x350B4536, 0);

	return paf_350B4536(job, handler1, param1, r6, r7, handler2);
}

static void toggle_dlna(int dlna)
{
	if(dlna >= 2)
	{
		xregistry()->loadRegistryIntValue(0x72, &dlna); dlna ^= 1; // toggle setting
	}

	int ret = xregistry()->saveRegistryIntValue(0x72, dlna);
	if (ret == CELL_OK)
	{
		Job_start(0, dlna ? handler1_enabled : handler1_disabled, 0, -1, -1, handler2);
		if(get_explore_interface())
		{
			exec_xmb_command2("reload_category %s", "photo");
			exec_xmb_command2("reload_category %s", "music");
			exec_xmb_command2("reload_category %s", "video");
		}
	}
}
/////////////////////////////////////
*/