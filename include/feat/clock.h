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

static void update_clock(char *header);
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
	else if((newDate[0] == '2') && (newDate[1] == '0') && (newDate[4] == '-') && (newDate[7] == '-')) // 2024-12-24 hh:mm:ss
	{
		newDate[4] = newDate[7] = newDate[10] = '\0';
		rDate.year = val(newDate); ndays = 120 + (int)((rDate.year - 2001) / 4); // leap days
		rDate.month = val(newDate + 5); for(u8 i = 0; i < rDate.month - 1; i++) ndays +=  mdays[i]; // year days
		rDate.day = val(newDate + 8); 
		patchedDate = ((rDate.year * 365) + rDate.day + ndays) * 86400000000ULL;
		if((newDate[13] == ':') && (newDate[16] == ':'))
		{
			newDate[13] = newDate[16] = newDate[20] = '\0';
			rDate.hour   = val(newDate + 11); patchedDate += rDate.hour * 3600000000ULL; patchedDate -= (20) * 3600000000ULL;
			rDate.minute = val(newDate + 14); patchedDate += rDate.minute * 60000000ULL;
			rDate.second = val(newDate + 17); patchedDate += rDate.second *  1000000ULL;
		}
	}
	else
	{
		patchedDate = convertH(newDate);
		if(patchedDate < DATE_2000_01_01) {patchedDate = DATE_2024_12_24; newDate = NULL;}
	}
/*
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
*/

//	u64 a1, a2;
//	{ system_call_4(0x362, 0x3002, 0, (u64)(u32)&a1, (u64)(u32)&a2); }
	_cellRtcGetCurrentTick(&currentTick);

	if((currentTick < DATE_2024_12_24) || newDate)
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

static void update_clock(char *header)
{
	strcopy(header, STR_ERROR);

	const char *hostname = "ps3.aldostools.org";
	const u16  port = 80;

	int g_socket = connect_to_server(hostname, port);
	if(g_socket >= 0)
	{
		int req_len = snprintf(header, 250,
					 "GET /date.php HTTP/1.1\r\n"
					 "Host: %s:%i\r\n"
					 "Connection: close\r\n"
					 "User-Agent: webMAN/1.0\r\n"
					 "\r\n",
					 hostname, port);

		if(send(g_socket, header, req_len, 0) == req_len)
		{
			req_len = recv(g_socket, header, 250, MSG_WAITALL);

			char *date = header, newdate[24];
			char *pos = strstr(date, "\r\n\r\n");
			if(pos) {sprintf(date, "%.20s", pos + 8); strcpy(newdate, date); Fix_Clock(newdate);}
		}
	}
}
#endif