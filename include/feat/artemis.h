#ifdef ARTEMIS_PRX

#define THREAD_NAME_ART "artt"
#define uint			unsigned int
#define process_id_t	u32
#define sys_prx_id_t	int32_t

#define ISVALID_CHAR(c)	(ISDIGIT(c) || (c >= 'A' && c <= 'F') || (c == ' ') || (c == '\r') || (c == '\n'))

static sys_addr_t sysmem_art = NULL;
static sys_addr_t typeA_Copy = NULL;

// redefinition of pad bit flags
#define	PAD_SELECT			(1<<0)
//#define	PAD_L3			(1<<1)
//#define	PAD_R3			(1<<2)
#define	PAD_START			(1<<3)
//#define	PAD_UP			(1<<4)
//#define	PAD_RIGHT		(1<<5)
//#define	PAD_DOWN		(1<<6)
//#define	PAD_LEFT		(1<<7)
//#define	PAD_L2			(1<<8)
//#define	PAD_R2			(1<<9)
//#define	PAD_L1			(1<<10)
//#define	PAD_R1			(1<<11)
//#define	PAD_TRIANGLE	(1<<12)
//#define	PAD_CIRCLE		(1<<13)
//#define	PAD_CROSS		(1<<14)
//#define	PAD_SQUARE		(1<<15)

static int doForceWrite = 0;
static int isConstantWrite = 0;
static process_id_t attachedPID = 0;

//Config
//static u8 isDEX = 0;
//static u8 isCCAPI = 0;
#define get_process_mem		ps3mapi_get_process_mem
#define set_process_mem		ps3mapi_set_process_mem

static char *userCodes = NULL;

static u8 h2b(char hex)
{
	char c = hex | 0x20;
	if(c >= 0 && c <= 9)
		c -= '0';
	else if(c >= 'a' && c <= 'f')
		c -= 'W';
	return c;
}

static double ___atof(const char *s)
{
	double d, ret = 0.0, sign = 1.0;
	int e = 0, esign = 1, flags = 0, i;

	/* remove leading white spaces. */
	for (; ISSPACE(*s); ) ++s;
	if (*s == '-') {
		/* negative value. */
		sign = -1.0;
		++s;
	}
	else if (*s == '+') ++s;
	for (; ISDIGIT(*s); ++s) {
		/* process digits before decimal point. */
		flags |= 1;
		ret *= 10.0;
		ret += (double)(int)(*s - '0');
	}
	if (*s == '.') {
		for (d = 0.1, ++s; ISDIGIT(*s); ++s) {
			/* process digits after decimal point. */
			flags |= 2;
			ret += (d * (double)(int)(*s - '0'));
			d *= 0.1;
		}
	}
	if (flags) {
		/* test for exponent token. */
		if ((*s == 'e') || (*s == 'E')) {
			++s;
			if (*s == '-') {
				/* negative exponent. */
				esign = -1;
				++s;
			}
			else if (*s == '+') ++s;
			if (ISDIGIT(*s)) {
				for (; ISDIGIT(*s); ++s) {
					/* process exponent digits. */
					e *= 10;
					e += (int)(*s - '0');
				}
				if (esign >= 0) for (i = 0; i < e; ++i) ret *= 10.0;
				else for (i = 0; i < e; ++i) ret *= 0.1;
			}
		}
	}

	return (ret * sign);
}

static int ps3mapi_get_process_mem(process_id_t pid, u64 addr, char *buf, int size)
{
	system_call_6(8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_PROC_MEM, (u64)pid, (u64)addr, (u64)((u32)buf), (u64)size);
	return_to_user_prog(int);
}

static int ps3mapi_set_process_mem(process_id_t pid, u64 addr, char *buf, int size )
{
	system_call_6(8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_SET_PROC_MEM, (u64)pid, (u64)addr, (u64)((u32)buf), (u64)size);
	return_to_user_prog(int);
}
/*
static int dex_get_process_mem(process_id_t pid, u64 addr, char *buf, int size)
{
	system_call_4(904, (u64)pid, (u64)addr, (u64)size, (u64)((u32)buf));
	return_to_user_prog(int);
}

static int dex_set_process_mem(process_id_t pid, u64 addr, char *buf, int size )
{
	system_call_4(905, (u64)pid, (u64)addr, (u64)size, (u64)((u32)buf));
	return_to_user_prog(int);
}

static int ccapi_set_process_mem(process_id_t pid, u64 addr, char *buf, int size )
{
	system_call_4(201, (u64)pid, (u64)addr, (u64)size, (u64)((u32)buf));
	return_to_user_prog(int);
}

static int ccapi_get_process_mem(process_id_t pid, u64 addr, char *buf, int size)
{
	system_call_4(200, (u64)pid, (u64)addr, (u64)size, (u64)((u32)buf));
	return_to_user_prog(int);
}

static int get_process_mem(process_id_t pid, u64 addr, char *buf, int size)
{
	if (isDEX)
	{
		return dex_get_process_mem(pid, addr, buf, size);
	}
	else if (isCCAPI)
	{
		return ccapi_get_process_mem(pid, addr, buf, size);
	}

	return ps3mapi_get_process_mem(pid, addr, buf, size);
}

static int set_process_mem(process_id_t pid, u64 addr, char * buf, int size)
{
	if (isDEX)
	{
		return dex_set_process_mem(pid, addr, buf, size);
	}
	else if (isCCAPI)
	{
		return ccapi_set_process_mem(pid, addr, buf, size);
	}

	return ps3mapi_set_process_mem(pid, addr, buf, size);
}
*/

/*
 * Code Parsing Functions
 */

/*
 * Function:		ReadHexPartial()
 * File:			main.c
 * Project:			ArtemisPS3-PRX
 * Description:		Converts a hex string into an array of bytes
 *					In cases where the len is less than 4, it will shift the value over such that something like "011" will be 0x11
 * Arguments:
 *	read:			buffer containing string
 *	start:			start index of conversion within buffer
 *	len:			length of hex string
 *	buf:			buffer that will store the resulting byte/char array after conversion
 *	bufSize:		allocated size of buf
 * Return:			Returns pointer to buf
 */
static char * ReadHexPartial(char * read, int start, int len, char * buf, int bufSize)
{
	_memset(buf, bufSize);
	int str = (start + len - 1);
	int cnt = 0;
	for (int x = str; x >= start; x--, cnt++)
	{
		while ((read[x] == ' ' || read[x] == '\r' || read[x] == '\n') && x >= start)
			x--;
		if (x < start)
			break;

		unsigned char c = h2b(read[x]);

		int bufOff = ((bufSize < 4) ? 4 : bufSize) - (cnt/2) - 1;

		if (cnt & 1)
			buf[bufOff] |= (unsigned char)c;
		else
			buf[bufOff] |= (unsigned char)(c << 4);
	}

	return buf;
}

/*
 * Function:		ReadHex()
 * File:			main.c
 * Project:			ArtemisPS3-PRX
 * Description:		Converts a hex string into an array of bytes
 *					In cases where the len is less than 4, it will NOT (unlike ReadHexPartial) shift the value over such that something like "011" will be 0x01100000
 * Arguments:()A
 *	read:			buffer containing string
 *	start:			start index of conversion within buffer
 *	len:			length of hex string
 *	buf:			buffer that will store the resulting byte/char array after conversion
 *	bufSize:		allocated size of buf
 * Return:			Returns pointer to buf
 */
static const char *ReadHex(const char *read, int start, int len, char *buf, int bufSize)
{
	for (int x = start, n = 0; n < len; n++, x += 2)
	{
		buf[n] = (h2b(read[x])<<4) | h2b(read[x + 1]);
	}
	return buf;
}

/*
 * Function:		WriteMem()
 * File:			main.c
 * Project:			ArtemisPS3-PRX
 * Description:		Writes the contents of buf to addr
 * Arguments:
 *	pid:			process id of process to write to
 *	addr:			address to write to
 *	buf:			bytes to write to addr
 *	size:			number of bytes to write from buf
 * Return:			Returns the result of set_process_mem (>= 0 is a success)
 */
static int WriteMem(unsigned long pid, u64 addr, char * buf, int size)
{
//	char bufTemp[4];
	int ret1 = 0;

/*	if (size > 4 && 0) //Can't seem to write too many bytes at once
	{
		for (int z = 0; z < size; z += 4)
		{
			int zl = (size - 4);
			if (zl > 4)
				zl = 4;

			strncpy(bufTemp, buf + z, 4);
			if (addr)
			{
				ret1 = set_process_mem(pid, addr + z, bufTemp, zl);
				//if (ret1)
				//	printf("Writing %d bytes to 0x%08X return %d (== ENOSYS = %d)\n", zl, (uint)(addr + z), ret1, ret1 == ENOSYS);
			}
		}
	}
	else */
	{
		if (addr)
		{
			ret1 = set_process_mem(pid, addr, buf, size);
		}
	}

	return ret1;
}

/*
 * Function:		CompareMemory()
 * File:			main.c
 * Project:			ArtemisPS3-PRX
 * Description:		Compares the bytes at addr to buf (equal to)
 * Arguments:
 *	pid:			process id of process to write to
 *	addr:			address to read from
 *	buf:			bytes to compare with that from addr
 *	bufLen:			number of bytes to compare
 * Return:			Returns 0 if not equal, 1 if equal
 */
static int CompareMemory(unsigned long pid, u64 addr, char *buf, int bufLen)
{
	char cmp[1];

	for (int x = 0; x < bufLen; x++)
	{
		if (get_process_mem(pid, addr + x, cmp, 1) < 0)
			return 0;
		if (buf[x] != cmp[0])
			return 0;
	}

	return 1;
}

/*
 * Function:		CompareMemoryBuffered()
 * File:			main.c
 * Project:			ArtemisPS3-PRX
 * Description:		Compares the bytes at source to buf (equal to)
 * Arguments:
 *	source:			buffer to compare with
 *	sourceOff:		offset within source buffer to start comparing with
 *	buf:			bytes to compare with that from source
 *	bufLen:			number of bytes to compare
 * Return:			Returns 0 if not equal, 1 if equal
 */
static int CompareMemoryBuffered(char *source, int sourceOff, char *buf, int bufLen)
{
	for (int x = 0; x < bufLen; x++)
	{
		if (buf[x] != source[sourceOff + x])
			return 0;
	}

	return 1;
}

/*
 * Function:		CompareMemoryAnd()
 * File:			main.c
 * Project:			ArtemisPS3-PRX
 * Description:		Compares the bytes at addr to buf (and equal to)
 * Arguments:
 *	pid:			process id of process to write to
 *	addr:			address to read from
 *	buf:			bytes to compare with that from addr
 *	bufLen:			number of bytes to compare
 * Return:			Returns 0 if not equal, 1 if equal
 */
static int CompareMemoryAnd(unsigned long pid, u64 addr, char *buf, int bufLen)
{
	char cmp[bufLen];
	if (get_process_mem(pid, addr, cmp, bufLen) < 0)
		return 0;

	for (int x = 0; x < bufLen; x++)
	{
		cmp[x] &= buf[x];
		if (cmp[x] != buf[x])
			return 0;
	}

	return 1;
}

/*
 * Function:		isCodeLineValid()
 * File:			main.c
 * Project:			ArtemisPS3-PRX
 * Description:		Checks if the line is a valid code
 * Arguments:
 *	line:			buffer holding line
 * Return:			Returns 0 if not valid, 1 if valid
 */
static int isCodeLineValid(char * line)
{
	int lineLen = strlen(line);
	int spaceCnt = 0;
	for (int x = 0; x < lineLen; x++)
		if (line[x] == ' ')
			spaceCnt++;

	int isText = 0, isFloat = 0;
	if (line[0] == '1' && line[1] == ' ')
		isText = 1;
	else if (line[0] == '2' && line[1] == ' ')
		isFloat = 1;

	if (!isText && !isFloat)
	{
		//Check number of spaces
		if (spaceCnt != 2)
			return 0;

		//Check hex
		for (int z = 0; z < lineLen; z++)
		{
			char c = line[z];
			if (c >= 'a')
				c -= 0x20;

			if(!ISVALID_CHAR(c))
			{
				return 0;
			}
		}
	}
	else if (isText && !isFloat)
	{
		//Check number of spaces
		if (spaceCnt < 2)
			return 0;

		//Check hex
		int textSpaceCnt = 0;
		for (int z = 0; z < lineLen; z++)
		{
			char c = line[z];
			if (c == ' ')
				textSpaceCnt++;
			if (c >= 'a')
				c -= 0x20;

			if (textSpaceCnt >= 2)
				break;

			if (!ISVALID_CHAR(c))
			{
				return 0;
			}
		}
	}
	else if (!isText && isFloat)
	{
		//Check number of spaces
		if (spaceCnt != 2)
			return 0;

		//Check hex
		for (int z = 0; z < lineLen; z++)
		{
			char c = line[z];
			if (c >= 'a')
				c -= 0x20;

			if (!ISVALID_CHAR(c))
			{
				return 0;
			}
		}
	}
	else
		return 0;

	return 1;
}

/*
 * Function:		ParseLine()
 * File:			main.c
 * Project:			ArtemisPS3-PRX
 * Description:		The core of the Artemis PS3 Engine
 *					Parses the line at start and performs the code operation on it appropriately
 * Arguments:
 *	pid:			process id of process to write to
 *	lines:			buffer containing entire code list
 *	start:			start position of current line
 *	linesLen:		strlen of lines (pre-calculated for speed)
 *	skip:			pointer to an int that holds the number of lines to skip (used with multiline codes and conditionals)
 *	doForceWrite:	whether the user has pressed the activation combo or the code is constant writing
 * Return:			Returns index of next line (within lines)
 */
static u64 ptrAddr = 0;
static uint typeA_Size = 0;
static int ParseLine(unsigned long pid, char * lines, int start, int linesLen, int * skip, int doForceWrite)
{
	if (pid == NULL || pid == 0)
		return start;

	char cType = lines[start];
	int lineLen = 0, arg2Len = 0, arg1Off = 0, totalLenRead = 0, arg0 = 0;
	int startPos, arg0_2 = 0, buf0Len;

	for (lineLen = start; lineLen < linesLen; lineLen++)
	{
		if (lines[lineLen] == '\n' || lines[lineLen] == '\r')
			break;
	}
	lineLen -= start;
	int arg3Off = start + lineLen, arg4Off = arg3Off + 1, arg4Len = arg4Off;

	totalLenRead = lineLen;
	if (skip[0] <= 0)
	{
		skip[0] = 0;

		//Parse first line
		while (lines[(start + lineLen) - arg2Len] != ' ')
			arg2Len++;
		while (lines[start + arg1Off] != ' ')
			arg1Off++;

		char addrBuf[4]; memset(addrBuf, 0, 4);

		ReadHexPartial(lines, start + 1, (arg1Off - 1), addrBuf, (arg1Off - 1)/2);
		arg0 = (int)(*((u32*)addrBuf));

		ReadHex(lines, start + arg1Off + 1, 8, addrBuf, 4);
		u64 addr = (u64)(*((u32*)addrBuf));
		if (ptrAddr)
		{
			addr = ptrAddr;
			ptrAddr = 0;
		}

		buf0Len = arg2Len / 2;
		char buf0[buf0Len];
		char arg2Temp[arg2Len - 1];

		if (arg0 < 0)
			arg0 = 0;
		char buf1[arg0];

		//Check if theres a second line
		if ((start + lineLen + 1) < linesLen)
		{
			//Parse second line vars (for codes that need the second line
			//Get next code arguments
			while (arg3Off < linesLen && lines[arg3Off] != ' ')
				arg3Off++;
			arg4Off = arg3Off + 1;
			while (arg4Off < linesLen && lines[arg4Off] != ' ')
				arg4Off++;
			arg4Len = arg4Off + 1;
			while (arg4Len < linesLen && lines[arg4Len] != '\r' && lines[arg4Len] != '\n')
				arg4Len++;
			arg4Len -= arg4Off;
		}
		else
			arg4Len = 0;

		char buf0_2[arg4Len/2];
		char buf1_2[(arg4Off - arg3Off)/2];

		if (arg4Len == 0)
			goto skipSecondLine;

		//Get args for second line
		ReadHexPartial(lines, start + lineLen + 2, (arg3Off) - (start + lineLen + 2), addrBuf, ((arg3Off) - (start + lineLen + 2))/2);
		arg0_2 = (uint)(*((u32*)addrBuf));

		//Get address for second line
		ReadHexPartial(lines, arg3Off + 1, arg4Off - arg3Off - 1, buf0_2, (arg4Off - arg3Off - 1)/2);

		//Get value for second line
		ReadHexPartial(lines, arg4Off + 1, arg4Len - 1, buf1_2, (arg4Len - 1)/2);
		skipSecondLine: ;

		startPos = (start + lineLen) - arg2Len + 1;
		switch (cType)
		{
			case '0': ; //Write bytes (1=OR,2=AND,3=XOR,rest=write)
				ReadHex(lines, startPos, arg2Len - 1, buf0, 4);
				//Get source bytes
				if(arg0) get_process_mem(pid, addr, buf1, buf0Len);
				switch (arg0)
				{
					case 1: //OR
						for (uint cnt0 = 0; cnt0 < (uint)buf0Len; cnt0++)
							buf0[cnt0] |= buf1[cnt0];
						break;
					case 2: //AND
						for (uint cnt0 = 0; cnt0 < (uint)buf0Len; cnt0++)
							buf0[cnt0] &= buf1[cnt0];
						break;
					case 3: //XOR
						for (uint cnt0 = 0; cnt0 < (uint)buf0Len; cnt0++)
							buf0[cnt0] ^= buf1[cnt0];
						break;
				}
				//Write bytes to dest
				WriteMem(pid, addr, buf0, buf0Len);
				break;
			case '1': //Write text
				//Get total text write size
				arg1Off++;
				while (lines[start + arg1Off] != ' ')
					arg1Off++;

				arg2Len = lineLen - arg1Off;
				strncpy(buf0, lines + startPos, arg2Len - 1);
				buf0[arg2Len-1] = '\0';
				WriteMem(pid, addr, buf0, arg2Len - 1);
				break;
			case '2': //Write float
				strncpy(buf0, lines + startPos, arg2Len - 1);
				float buf2Flt = (float)___atof(buf0); //atof(buf2);
				WriteMem(pid, addr, (char*)&buf2Flt, arg2Len - 1);
				totalLenRead = lineLen;
				break;
			case '4': ; //Write condensed
				//Get count
				uint count = (uint)(*((u32*)buf1_2));

				//Get increment
				u64 inc = (u64)(*((u32*)buf0_2));

				//Get write
				ReadHex(lines, startPos, arg2Len - 1, buf0, buf0Len);

				for (uint cnt4 = 0; cnt4 < count; cnt4++)
				{
					WriteMem(pid, addr + (u64)(cnt4 * inc), buf0, buf0Len);
					if (arg0_2)
						*(uint*)buf0 += (arg0_2 << ((buf0Len % 4) * 8));
				}

				skip[0]++;
				break;
			case '6': //Write pointer

				//Get offset
				ReadHexPartial(lines, startPos, arg2Len - 1, buf0, (arg2Len - 1)/2);
				u64 offset = (u64)(*((u32*)buf0));

				//Get address at pointer
				get_process_mem(pid, addr, buf0, 4);
				ptrAddr = (u64)(*((u32*)buf0));

				ptrAddr += offset;

				break;
			case 'A': //Copy paste

				switch (arg0)
				{
					case 1:
						//Get count
						ReadHexPartial(lines, startPos, arg2Len - 1, buf0, (arg2Len - 1)/2);
						uint count = (uint)(*((u32*)buf0));

						if(count > _64KB_) break;

						if(!typeA_Copy)
							typeA_Copy = sys_mem_allocate(_64KB_);

						if(typeA_Copy)
						{
							typeA_Size = count;
							get_process_mem(pid, addr, (char *)typeA_Copy, 4);

							memcpy64(arg2Temp, lines + (startPos), arg2Len - 1);
						}
						else
						{
							typeA_Size = 0;
							typeA_Copy = NULL;
						}
						break;
					case 2:
						if (!typeA_Copy || typeA_Size <= 0)
							break;

						WriteMem(pid, addr, (char *)typeA_Copy, typeA_Size);
						break;
				}

				break;
			case 'B': //Find Replace
				//Only work when doForceWrite is true (1) which means everytime the user activates Artemis from the in game XMB
				//Don't want to waste time constantly searching

				if (!doForceWrite)
					break;

				//Get end addr
				ReadHex(lines, startPos, arg2Len - 1, addrBuf, 4);
				u64 endAddr = (u64)(*((u32*)addrBuf));

				//new (COP) length
				uint binc = arg4Len/2;

				//original (OGP) length
				uint cmpSize = (arg4Off - arg3Off)/2;

				//Flip addresses
				u64 temp = 0;
				if (endAddr < addr) { temp = addr; addr = endAddr; endAddr = temp; }

				const size_t scanInc = _64KB_;
				sys_addr_t sysmem = sys_mem_allocate(scanInc);

				if(sysmem)
				{
					for (u64 curAddr = addr; curAddr < endAddr; curAddr += (scanInc - cmpSize))
					{
						if (get_process_mem(pid, curAddr, (char *)sysmem, scanInc) >= 0)
						{
							//So we stop it each loop before (scanInc - cmpSize) in the instance that
							//the result is the last 2 bytes, for instance, and the compare is actually 4 bytes (so it won't result even though it should)
							//This fixes that
							for (u64 boff = 0; boff < (scanInc - cmpSize); boff++)
							{
								//Break if count reached
								if (arg0 > 0 && temp >= (u64)arg0)
									break;
								if ((curAddr + boff) >= endAddr)
									break;

								if (CompareMemoryBuffered((char *)sysmem, boff, buf0_2, cmpSize))
								{
									//printf ("Instance found at 0x%08x, writing 0x%x (%d)\n", curAddr + boff, *(uint*)buf1_2, binc);
									WriteMem(pid, curAddr + boff, buf1_2, binc);
									//Just skip in case the replacement has, within itself, the ogp
									//We subtract one because it gets added back at the top of the loop
									boff += binc - 1;
									temp++;
								}
							}
						}
					}
					sys_memory_free(sysmem);
				}

				skip[0]++;
				totalLenRead = lineLen;
				break;
			case 'D': //Write conditional
				ReadHex(lines, startPos, arg2Len - 1, buf0, 4);
				int DisCond = CompareMemory(pid, addr, buf0, buf0Len);

				if (!DisCond)
				{
					skip[0] += arg0;
				}

				break;
			case 'E': //Write conditional (bitwise)
				ReadHex(lines, startPos, arg2Len - 1, buf0, 4);
				int EisCond = CompareMemoryAnd(pid, addr, buf0, buf0Len);

				if (!EisCond)
				{
					skip[0] += arg0;
				}

				break;
			case 'F': //Copy bytes

				//Get destination
				ReadHex(lines, startPos, arg2Len - 1, buf0, 4);
				u64 dest = (u64)(*((u32*)buf0));

				//Get source bytes
				get_process_mem(pid, addr, buf1, arg0);
				//Write bytes to dest
				WriteMem(pid, dest, buf1, arg0);

				break;

		}
	}
	else
		skip[0]--;

	return start + totalLenRead;
}

/*
 * Function:		check_syscall_api
 * File:			main.c
 * Project:			ArtemisPS3-PRX
 * Description:		Checks syscalls to determine which API to use
 * Arguments:
 *  void
 * Return:			void
 */
/*static void check_syscall_api(void)
{
	char check[4];
	isDEX = dex_get_process_mem(attachedPID, 0x10000, check, 4) != ENOSYS;
	if (!(isDEX && check[0] == 0x7F && check[1] == 'E' && check[2] == 'L' && check[3] == 'F'))
	{
		isDEX = 0;
	}
	if (!isDEX)
	{
		isCCAPI = ccapi_get_process_mem(attachedPID, 0x10000, check, 4) != ENOSYS;
		if (!(isCCAPI && check[0] == 0x7F && check[1] == 'E' && check[2] == 'L' && check[3] == 'F'))
		{
			isCCAPI = 0;
		}
	}
}*/

/*
 * Function:		ConvertCodes()
 * File:			main.c
 * Project:			ArtemisPS3-PRX
 * Description:		Loops through each line of the code list and interprets the codes
 *					Calls ParseLine() to use the code
 * Arguments:
 *	pid:			process id of process to write to
 *	ncCodes:		buffer containing entire code list
 * Return:			void
 */
static void ConvertCodes(unsigned long pid, char * ncCodes)
{
	char lineBuf[100];

	int totalLen = strlen(ncCodes);

	int lineNum = 0, codeNum = 0, lineCharInd = 0;
	int skip[1]; skip[0] = 0;

	//check_syscall_api();

	for (int x = 0; x < totalLen; x++)
	{
		if (ncCodes[x] != '#')
		{
			if (ncCodes[x] == '\n')
			{
				lineNum++;
				lineCharInd = -1;
			}
			else
			{
				if (lineCharInd == 0)
				{
					int zz;
					for (zz = x; zz < totalLen; zz++)
					{
						if (ncCodes[zz] == '\n' || ncCodes[zz] == '\r')
							break;
					}
					memcpy64(lineBuf, ncCodes + x, zz - x);
					lineBuf[(zz - x)] = 0;
				}

				switch (lineNum)
				{
					case 1: //cWrite
						if (*lineBuf == 0)
						{
							lineNum--;
						}
						else
						{
							if (lineCharInd == 0)
								isConstantWrite = ncCodes[x] == '1' || ncCodes[x] == 'T';

							if (!isConstantWrite && !doForceWrite) //skip this code if not constant or force write
							{


								while (ncCodes[x] != '#' && x < totalLen)
									x++;
								x--;
							}
						}
						break;
					case 0: //Name
						break;

					default: //codes

						if (lineNum > 1 && isCodeLineValid(lineBuf))
						{
							if (isConstantWrite || doForceWrite)
								x = ParseLine(pid, ncCodes, x, totalLen, skip, doForceWrite);
							else
							{
								skip[0]++;
								x = ParseLine(pid, ncCodes, x, totalLen, skip, doForceWrite);
							}
						}
						break;
				}
			}
			lineCharInd++;
		}
		else
		{
			lineNum = -1;
			isConstantWrite = 0;
			skip[0] = 0;
			codeNum++;
			lineCharInd = 0;

			while ((ncCodes[x] == '\n') && x < totalLen)
				x++;
		}
	}
}

static void release_art(void)
{
	if(sysmem_art)
		{sys_memory_free(sysmem_art); sysmem_art = NULL;}
	if(typeA_Copy)
		{sys_memory_free(typeA_Copy); typeA_Copy = NULL;}
	attachedPID = 0;
	userCodes = NULL;
}

/*
 * Function:		art_process()
 * File:			main.c
 * Project:			ArtemisPS3-PRX
 * Description:		Processes an entire codelist once
 * Arguments:
 *	forceWrite:		whether the user activated Artemis (1) or a constant write (0)
 * Return:			void
 */
static void art_process(int forceWrite)
{
	if (attachedPID)
	{
		doForceWrite = forceWrite;

		if (!userCodes || forceWrite)
		{
			release_art();

			size_t mem_size = _64KB_;
			size_t fileSize = file_size("/dev_hdd0/tmp/art.txt");

			if(fileSize)
			{
				if(fileSize >= _128KB_) mem_size = _192KB_; else
				if(fileSize >= _64KB_)  mem_size = _128KB_;

				if((fileSize + 1) < mem_size)
					sysmem_art = sys_mem_allocate(mem_size);
			}

			if(sysmem_art)
			{
				show_msg("Artemis PS3\nAttached");

				userCodes = (char *)sysmem_art;
				if(read_file("/dev_hdd0/tmp/art.txt", userCodes, mem_size, 0))
				{
					userCodes[mem_size] = '\0';
					userCodes[fileSize] = '\n';
					userCodes[fileSize+1] = '\0';
				}
				else
					release_art();
			}

			if(!sysmem_art)
			{
				show_status(STR_ERROR, "Artemis PS3\nFailed to Attach");
				release_art();
			}
		}

		if (userCodes && (attachedPID == GetGameProcessID()))
		{
			ConvertCodes(attachedPID, userCodes);
			sys_timer_usleep(100000);
		}
	}

	doForceWrite = 0;
}

/*
 * Function:		art_thread()
 * File:			main.c
 * Project:			ArtemisPS3-PRX
 * Description:		Artemis PRX Thread start
 *					Interprets user input and calls art_process()
 * Arguments:
 *	arg:
 * Return:			void
 */
static void art_thread(u64 arg)
{
	int GameProcessID = 0, lastGameProcessID = 0;

	sys_timer_sleep(10);
	sys_ppu_thread_yield();

	CellPadData data;
	CellPadInfo2 info;

	while (working)
	{
		GameProcessID = GetGameProcessID();

		if (GameProcessID)
		{
			if (GameProcessID != lastGameProcessID)
			{
				for (int x = 0; x < 1000; x++) //5 seconds delay
				{
					sys_timer_usleep(5000);
					sys_ppu_thread_yield();
				}
				show_msg("Artemis PS3\nStart To Attach");
			}

			cellPadGetInfo2(&info);
			if (info.port_status[0] && (cellPadGetData(0, &data) | 1) && data.len > 0)
			{
				u32 pad = data.button[2] | (data.button[3] << 8);
				if (attachedPID) // Run codes
				{
					art_process(0);
				}

				if (pad & PAD_START)
				{
					attachedPID = GameProcessID;

					if (attachedPID)
					{
						art_process(1);
					}
					else
					{
						show_status(STR_ERROR, "Artemis PS3\nFailed to Attach");
					}

					// wait for release START button
					while ((cellPadGetData(0, &data) | 1) && data.len > 0)
					{
						if (!((data.button[2] | (data.button[3] << 8)) & PAD_START))
							break;

						sys_ppu_thread_sleep(1);
					}
				}
				else if ((pad & PAD_SELECT) && attachedPID)
				{
					show_msg("Artemis PS3\nDetached");
					release_art();
				}
			}

			sys_timer_usleep(100000); //0.1 second delay
		}
		else break; // exit on XMB
		// else if (attachedPID) // Disconnect
		// {
		// 	//printf("Artemis PS3 :::: Process Exited\n");
		// 	release_art();
		// }
		// else
		// 	sys_ppu_thread_sleep(3); //3 second delay

		lastGameProcessID = GameProcessID;
		sys_timer_usleep(1668);
		sys_ppu_thread_yield();
	}

	if(working)
		thread_id_art = SYS_PPU_THREAD_NONE; // allow restart artemis

	release_art();
	sys_ppu_thread_exit(0);
}

#endif // #ifdef ARTEMIS_PRX