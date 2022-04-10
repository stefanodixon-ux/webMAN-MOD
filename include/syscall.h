#ifndef LITE_EDITION
static u16 parse_syscall(char *params, u64 sp[], u8 *num)
{
	char *p; u8 n;
	u16 sc = (u16)val(params);
	for(n = 0; n <= 8; n++)
	{
		sp[n] = 0, p = strchr(params, '|'); if(!p) break;
		params = p + 1, *p = NULL;
		sp[n] = (u64)val(params);
		if(!sp[n] && (*params != '0'))
			sp[n] = (u64)(u32)(params);
	}
	*num = n;
	return sc;
}

static u64 call_syscall(u16 sc, u64 sp[], u8 num)
{
	switch (num)
	{
		case 0: {system_call_0(sc); return p1;}
		case 1: {system_call_1(sc, sp[0]); return p1;}
		case 2: {system_call_2(sc, sp[0], sp[1]); return p1;}
		case 3: {system_call_3(sc, sp[0], sp[1], sp[2]); return p1;}
		case 4: {system_call_4(sc, sp[0], sp[1], sp[2], sp[3]); return p1;}
		case 5: {system_call_5(sc, sp[0], sp[1], sp[2], sp[3], sp[4]); return p1;}
		case 6: {system_call_6(sc, sp[0], sp[1], sp[2], sp[3], sp[4], sp[5]); return p1;}
		case 7: {system_call_7(sc, sp[0], sp[1], sp[2], sp[3], sp[4], sp[5], sp[6]); return p1;}
		case 8: {system_call_8(sc, sp[0], sp[1], sp[2], sp[3], sp[4], sp[5], sp[6], sp[7]); return p1;}
	}
	return FAILED;
}
#endif
