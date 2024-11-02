#ifdef SYS_BGM
	if(islike(param, "/sysbgm.ps3"))
	{
		// /sysbgm.ps3          toggle in-game background music flag
		// /sysbgm.ps3?         show status of in-game background music flag
		// /sysbgm.ps3?1        enable in-game background music flag
		// /sysbgm.ps3?enable   enable in-game background music flag
		// /sysbgm.ps3?0        disable in-game background music flag
		// /sysbgm.ps3?disable  disable in-game background music flag

		if(param[11] == '?')
		{
			if( param[12] & 1) system_bgm = 0; else //enable
			if(~param[12] & 1) system_bgm = 1;      //disable
		}

		if(param[12] != 's')
		{
			int * arg2;
			if(system_bgm)
			{
				u32 (*BgmPlaybackDisable)(int, void *);
				BgmPlaybackDisable = getNIDfunc("vshmain", 0xEDAB5E5E, 17*2);
				BgmPlaybackDisable(0, &arg2); system_bgm = 0;
			}
			else
			{
				u32 (*BgmPlaybackEnable)(int, void *);
				BgmPlaybackEnable  = getNIDfunc("vshmain", 0xEDAB5E5E, 16*2);
				BgmPlaybackEnable(0, &arg2);  system_bgm = 1;
			}
		}

		concat_text(templn, "System BGM:", system_bgm ? STR_ENABLED : STR_DISABLED);
		_concat(&sbuffer, templn);
		show_msg(templn);
	}
	else
#endif
