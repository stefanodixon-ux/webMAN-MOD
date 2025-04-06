#ifdef FIX_CLOCK
/*
static int sysSetCurrentTime(u64 sec, u64 nsec)
{
	system_call_2(146, (u32)sec, (u32)nsec);
	return_to_user_prog(int);
}

static int sys_ss_secure_rtc(u64 time)
{
	system_call_4(0x362, 0x3003, time / 1000000, 0, 0);
	return_to_user_prog(int);
}
*/
static int sysGetCurrentTime(u64 *sec, u64 *nsec)
{
	system_call_2(145,(u32)sec, (u32)nsec);
	return_to_user_prog(int);
}

static int sys_time_get_rtc(u64 *real_time_clock)
{
	system_call_1(119, (u32)real_time_clock);
	return_to_user_prog(int);
}

static void Fix_Clock(char *newDate)
{
	#define DATE_2000_01_01	0x00E01D003A63A000ULL
	//#define DATE_1970_01_01	0x00DCBFFEFF2BC000ULL
	//#define DATE_2023_11_23	0x00E2CAD2ECB78000ULL
	#define DATE_2024_12_24	0x00E2EA0BE8548000ULL

	u64 clock, diff;
	u64 sec, nsec;
	u64 currentTick;

	//u64 timedata = 0x00E2CABECEE02000ULL - DATE_2000_01_01;
	u64 patchedDate;

	static int (*_cellRtcGetCurrentTick)(u64 *pTick) = NULL;
	static int (*_cellRtcSetCurrentTick)(u64 *pTick) = NULL;
	static xsetting_8B69F85A_class*(*xSettingDateGetInterface)(void) = NULL;

	_cellRtcGetCurrentTick = getNIDfunc("cellRtc", 0x9DAFC0D9, 0);
	_cellRtcSetCurrentTick = getNIDfunc("cellRtc", 0xEB22BB86, 0);
	xSettingDateGetInterface = getNIDfunc("xsetting", 0x8B69F85A, 0);

	CellRtcDateTime rDate;

	u8 mdays[12] = {31,28,31,30,31,30,31,31,30,31,30,31}; u16 ndays;

	if(!newDate)
	{
		patchedDate = DATE_2024_12_24;
	}
	else if(newDate[0] == '2' && newDate[1] == '0' && (newDate[4] == '-') && (newDate[7] == '-')) // 2024-12-24
	{
		newDate[4] = newDate[7] = newDate[10] = '\0';
		rDate.year = val(newDate); ndays = 120 + (int)((rDate.year - 2001) / 4); // leap days
		rDate.month = val(newDate + 5); for(u8 i = 0; i < rDate.month - 1; i++) ndays +=  mdays[i]; // year days
		rDate.day = val(newDate + 8); 
		patchedDate = ((rDate.year * 365) + rDate.day + ndays) * 86400000000;
	}
	else
	{
		patchedDate = convertH(newDate);
		if(patchedDate < DATE_2000_01_01) {patchedDate = DATE_2024_12_24; newDate = NULL;}
	}

	if(!newDate)
	{
		struct CellFsStat buf;
		cellFsStat(WM_CONFIG_FILE, &buf);
		cellRtcSetTime_t(&rDate, buf.st_mtime);

		// use last wm_config date if it's later than the default patch date
		ndays = 120 + (int)((rDate.year - 2001) / 4); // leap days
		for(u8 i = 0; i < rDate.month - 1; i++) ndays +=  mdays[i]; // year days
		u64 configDate = ((rDate.year * 365) + rDate.day + ndays) * 86400000000; if(configDate > patchedDate) patchedDate = configDate;
	}

//	u64 a1, a2;
//	{ system_call_4(0x362, 0x3002, 0, (u64)(u32)&a1, (u64)(u32)&a2); }
	_cellRtcGetCurrentTick(&currentTick);

	if(currentTick < DATE_2024_12_24 || newDate)
	{
		_cellRtcSetCurrentTick(&patchedDate);
		sysGetCurrentTime(&sec, &nsec);
		sys_time_get_rtc(&clock);
		diff = sec - clock;
		xSettingDateGetInterface()->SaveDiffTime(diff);
	}
/*
	if(!a1)
		sys_ss_secure_rtc(timedata);

	{ system_call_4(0x362, 0x3002, 0, (u64)(u32)&a1, (u64)(u32)&a2); }
	_cellRtcGetCurrentTick(&currentTick);
	u64 result_time2 = (currentTick - DATE_2000_01_01);

	u64 rtc_clock = a1 * 1000000 + DATE_2000_01_01;

	if(rtc_clock < currentTick)
	{
		sys_ss_secure_rtc(result_time2);
	}
	else if(rtc_clock > currentTick)
	{
		sysSetCurrentTime(((rtc_clock - DATE_1970_01_01) / 1000000), 0);
		sysGetCurrentTime(&sec, &nsec);
		sys_time_get_rtc(&clock);
		diff = sec - clock;
		xSettingDateGetInterface()->SaveDiffTime(diff);
	}
*/
}
#endif