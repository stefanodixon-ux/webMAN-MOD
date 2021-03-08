#ifdef GET_KLICENSEE

static char *hex_dump(char *buffer, int offset, int size)
{
	for (int k = 0; k < size ; k++)
	{
		sprintf(&buffer[2 * k],"%02X", (unsigned int)(((unsigned char*)offset)[k]));
	}
	return buffer;
}

#endif

#ifdef DEBUG_MEM

#define LV1		1
#define LV2		2

#define LV1_UPPER_MEMORY	0x8000000010000000ULL
#define LV2_UPPER_MEMORY	0x8000000000800000ULL

static void poke_chunk_lv1(u32 start, int size, u8 *buffer)
{
	for(int offset = 0; offset < size; offset += 8)
		poke_lv1((start + offset) | 0x8000000000000000ULL, *((u64*)(buffer + offset)));
}

static void poke_chunk_lv2(u32 start, int size, u8 *buffer)
{
	for(int offset = 0; offset < size; offset += 8)
		pokeq((start + offset) | 0x8000000000000000ULL, *((u64*)(buffer + offset)));
}

static void peek_chunk_lv1(u64 start, u64 size, u8 *buffer) // read from lv1
{
	for(u64 t, i = 0; i < size; i += 8)
	{
		t = peek_lv1(start + i); memcpy(buffer + i, &t, 8);
	}
}

static void peek_chunk_lv2(u64 start, u64 size, u8 *buffer) // read from lv1
{
	for(u64 t, i = 0; i < size; i += 8)
	{
		t = peekq(start + i); memcpy(buffer + i, &t, 8);
	}
}

static void dump_mem(char *file, u64 start, u32 dump_size)
{
	{ PS3MAPI_ENABLE_ACCESS_SYSCALL8 }

	int fd;
	u32 mem_size = _128KB_, addr;
	sys_addr_t sys_mem = NULL;

	if(start < 0x0000028080000000ULL) start |= 0x8000000000000000ULL;

	vshNotify_WithIcon(49, "Dumping memory...");

	if(sys_memory_allocate(mem_size, SYS_MEMORY_PAGE_SIZE_64K, &sys_mem) == CELL_OK)
	{
		u8 *mem_buf = (u8*)sys_mem;

		if(cellFsOpen(file, CELL_FS_O_CREAT | CELL_FS_O_TRUNC | CELL_FS_O_WRONLY, &fd, NULL, 0) == CELL_FS_SUCCEEDED)
		{
			for(addr = 0; addr < dump_size; addr += mem_size)
			{
				peek_chunk_lv1(start + addr, mem_size, mem_buf);
				cellFsWrite(fd, mem_buf, mem_size, NULL);
			}
			cellFsClose(fd);
		}
		sys_memory_free(sys_mem);
		show_msg("Memory dump completed!");
	}

	{ BEEP2 }

	{ PS3MAPI_DISABLE_ACCESS_SYSCALL8 }
}

static void ps3mapi_mem_dump(char *buffer, char *templn, char *param)
{
	char dump_file[MAX_PATH_LEN]; u64 start=0; u32 size=8;
	strcat(buffer, "Dump: [<a href=\"/dump.ps3?mem\">Full Memory</a>] [<a href=\"/dump.ps3?rsx\">RSX</a>] [<a href=\"/dump.ps3?vsh\">VSH</a>] [<a href=\"/dump.ps3?lv1\">LV1</a>] [<a href=\"/dump.ps3?lv2\">LV2</a>]");
	strcat(buffer, "<hr>");

	if(param[9] == '?' && param[10] >= '0')
	{
		if(islike(param + 10, "lv1")) {size = 16;} else
		if(islike(param + 10, "lv2")) {start = LV2_OFFSET_ON_LV1;} else
		if(param[10] == 'v' /*vsh */) {start = 0x910000;}  else
		if(param[10] == 'r' /*rsx */) {start = 0x0000028080000000ULL; size=256;}  else
		if(param[10] == 'm' /*mem */) {size = IS_DEH ? 512 : 256;} else
		if(param[10] == 'f' /*full*/) {size = IS_DEH ? 512 : 256;} else
		{
			start = convertH(param + 10);
			if(start >= LV1_UPPER_MEMORY - ((u64)(size * _1MB_))) start = LV1_UPPER_MEMORY - ((u64)(size * _1MB_));
		}

		char *pos = strstr(param, "&size=");
		if(pos) size = convertH(pos + 6);

		sprintf(dump_file, "/dev_hdd0/dump_%s.bin", param + 10);
		dump_mem(dump_file, start, (size * _1MB_));
		add_breadcrumb_trail2(buffer, "<p>Dumped:", dump_file);
		sprintf(templn, " [" HTML_URL2 "]", "/delete.ps3", dump_file, STR_DELETE); strcat(buffer, templn);
	}
}

#define HEXVIEW_SIZE	0x200

static void ps3mapi_find_peek_poke_hexview(char *buffer, char *templn, char *param)
{
	u64 address = 0, addr, byte_addr, value = 0, upper_memory = LV1_UPPER_MEMORY, found_address=0, step = 1;
	u8 byte = 0, p = 0, lv1 = 0, rep = 1;
	bool found = false, not_found = false;
	int flen = 0, hilite;
	char *v;

	u8 data[HEXVIEW_SIZE]; struct CellFsStat s;
	bool is_file = islike(param, "/hexview.ps3/");
	if(is_file)
	{
		char *fname = param + 12;

		char sfind[65];
		flen = get_param("&find=", sfind, fname, 64);

		char *pos = strstr(fname, "&rep="); if(pos) {rep = (u8)val(pos + 5); *pos = NULL;}

		address = (u64)get_valuen64(fname, "&offset=");
		pos = strstr(fname, "&data=");

		get_flag(fname, "&"); // file name
		check_path_alias(fname);

		// find data in file
		if(flen)
		{
			if(isHEX(sfind))
				{strcpy(templn, sfind); flen = Hex2Bin(templn, sfind);}

			int i, n = (0x400 - flen); u64 addr = address;
			while(read_file(fname, templn, 0x400, addr))
			{
				for(i = 0; i < n; i++) if(!bcompare(templn + i, sfind, flen, sfind) && !(--rep)) break;
				addr += i; if(i < n) {address = found_address = addr; found = true; break;}
			}
			if(rep) not_found = true;
		}

		if(pos)
		{
			pos += 6;

			// data write to write
			if(isHEX(pos))
				flen = Hex2Bin(pos, templn);
			else
			{
				flen = sprintf(templn, "%s", pos);
				// convert pipes to line breaks
				for(pos = templn; *pos; ++pos) if(*pos == '|') *pos = '\n';
			}

			found_address = address, found = true;
			write_file(fname, CELL_FS_O_WRONLY, templn, address, flen, false);
			goto view_file;
		}
	}

	if(is_file) {get_flag(param + 10, "&"); goto view_file;}
	address = convertH(param + 10);

	v = strstr(param + 10, "=");
	if(v)
	{
		flen = strlen(v + 1);
		for(p = 1; p <= flen; p++) if(!memcmp(v + p, " ", 1)) byte++; //ignore spaces
		flen -= byte; byte = p = 0;
	}

	buffer += concat(buffer, "<pre>");

	address|=0x8000000000000000ULL;

	lv1 = strstr(param,".lv1?") ? 1 : 0;
#ifdef COBRA_ONLY
	if(lv1) { system_call_1(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_DISABLE_COBRA); }
#endif
	if(islike(param, "/find.lv"))
	{
		char *pos = strstr(param, "&rep="); if(pos) {rep = (u8)val(pos + 5); *pos = NULL;}

		pos = strchr(param, '#'); if(!pos) pos = strstr(param, "&align");
		if(pos) {*pos = NULL, step = 4, address &= 0x80000000FFFFFFFCULL;} // find using aligned memory address (4X faster) e.g. /find.lv2?3000=3940ffff#

		pos = strstr(param, "&stop=");
		if(pos)
			{upper_memory = convertH(pos + 6) | 0x8000000000000000ULL; *pos = NULL;}
		else
			{upper_memory = (lv1 ? LV1_UPPER_MEMORY : LV2_UPPER_MEMORY) - 8;}
	}
	else if(strstr(param, "#"))
		upper_memory = 0x8FFFFFFFFFFFFFF8ULL; // use # to peek/poke any memory address
	else
		upper_memory = (lv1 ? LV1_UPPER_MEMORY : LV2_UPPER_MEMORY) - 8; // safe memory adddress

	if(address > upper_memory) address = upper_memory - HEXVIEW_SIZE;

	if((v == NULL) || (address > upper_memory)) { /* ignore find/poke if value is not provided or invalid address */ }
	else
	if(islike(param, "/find.lv"))
	{
		char sfind[33], tfind[33];
		if(isHEX(v + 1))
			flen = Hex2Bin(v + 1, sfind);
		else
			flen = sprintf(sfind, "%s", v + 1);

		u64 (*peek_mem)(u64) = lv1 ? peek_lv1 : peekq;

		for(addr = address; addr < upper_memory; addr += step)
		{
			value = peek_mem(addr); memcpy(tfind, (char*)&value, 8);
			if(flen >  8) {value = peek_mem(addr +  8); memcpy(tfind +  8, (char*)&value, 8);}
			if(flen > 16) {value = peek_mem(addr + 16); memcpy(tfind + 16, (char*)&value, 8);}
			if(flen > 24) {value = peek_mem(addr + 24); memcpy(tfind + 24, (char*)&value, 8);}
			if(!bcompare(tfind, sfind, flen, sfind) && !(--rep)) {address = found_address = addr; found = true; break;}
		}

		if(!found)
		{
			sprintf(templn, "<font color=#ff0>%s</font><p>", STR_NOTFOUND); buffer += concat(buffer, templn);
		}
		else
		{
			found_address = address = addr;
			sprintf(templn, "Offset: 0x%08X<br><br>", (u32)address); buffer += concat(buffer, templn);
		}
	}
	else
	if(islike(param, "/poke.lv"))
	{
		char *value = v + 1;

		if(lv1)
			peek_chunk_lv1(address, 0x400, (u8 *)templn);
		else
			peek_chunk_lv2(address, 0x400, (u8 *)templn);

		flen = strlen(value); flen = MIN(flen, 0x400);

		if(isHEX(value))
			flen = Hex2Bin(value, templn);
		else
			memcpy(templn, value, flen);

		if(lv1)
			poke_chunk_lv1(address, flen, (u8 *)templn);
		else
			poke_chunk_lv2(address, flen, (u8 *)templn);

		found_address = address; found = true;
	}

view_file:

	address &= 0xFFFFFFFFFFFFFFF0ULL;
	addr = address;

	if(is_file)
	{
		param += 12; check_path_alias(param); cellFsStat(param, &s);
		read_file(param, (void*)data, 0x200, address);
		add_breadcrumb_trail(buffer, param);

		// MD5
		buffer += concat(buffer, ": [");
		sprintf(templn, HTML_URL2, "/md5.ps3", param, "MD5"); buffer += concat(buffer, templn);
		buffer += concat(buffer, "]");

		// file navigation
		u64 max = s.st_size < HEXVIEW_SIZE ? 0 : s.st_size - HEXVIEW_SIZE;
		sprintf(templn, "<span style='float:right'><a id=\"pblk\" href=\"/hexview.ps3%s\">&lt;&lt;</a> <a id=\"back\" href=\"/hexview.ps3%s&offset=%lli\">&lt;Back</a>", param, param, (address < HEXVIEW_SIZE) ? 0ULL : (address - HEXVIEW_SIZE)); buffer += concat(buffer, templn);
		sprintf(templn, " <a id=\"next\" href=\"/hexview.ps3%s&offset=%lli\">Next&gt;</a> <a id=\"nblk\" href=\"/hexview.ps3%s&offset=%lli\">&gt;&gt;</a></span>", param, MIN(address + HEXVIEW_SIZE, max), param, max);
		buffer += concat(buffer, templn);

		char *pos = strstr(param, "&find="); if(pos) *pos = 0;
		sprintf(templn, " [<a href=\"javascript:void(location.href='http://'+location.hostname+'%s&find='+window.prompt('%s'));\">%s</a>] %s%s%s", param - 12, "Find", "Find", "<font color=#ff0>", not_found ? "Not found!" : "", "</font><hr>");
		buffer += concat(buffer, templn);
	}

	////////////////////////////////
	// show memory address in hex //
	////////////////////////////////

	//if(address + HEXVIEW_SIZE > (upper_memory + 8)) address = 0;

#ifdef COBRA_ONLY
	if(lv1) { system_call_1(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_DISABLE_COBRA); }
#endif
	for(u16 i = 0; i < HEXVIEW_SIZE; i++)
	{
		if(!p)
		{
			sprintf(templn, "%08X  ", (int)((address & 0xFFFFFFFFULL) + i));
			buffer += concat(buffer, templn);
		}

		byte_addr = address + i;

		if(is_file)
			byte = data[i];
		else
			byte = (u8)((lv1 ? peek_lv1(byte_addr) : peekq(byte_addr)) >> 56);

		hilite = found && (byte_addr >= found_address) && (byte_addr < (found_address + flen));
		if(hilite) buffer += concat(buffer, "<font color=#ff0>");
		sprintf(templn, "%02X ", byte); buffer += concat(buffer, templn);
		if(hilite) buffer += concat(buffer, "</font>");

		if(p == 0xF)
		{
			buffer += concat(buffer, " ");
			for(u8 c = 0; c < 0x10; c++, addr++)
			{
				if(is_file)
					byte = data[(0x10 * (i / 0x10)) + c];
				else
					byte = (u8)((lv1 ? peek_lv1(addr) : peekq(addr)) >> 56);
				if(byte<32 || byte>=127) byte='.';

				hilite = (found && addr >= found_address) && (addr < (found_address + flen));
				if(hilite) buffer += concat(buffer, "<font color=#ff0>");
				if(byte==0x3C)
					buffer += concat(buffer, "&lt;");
				else if(byte==0x3E)
					buffer += concat(buffer, "&gt;");
				else
					{sprintf(templn,"%c", byte); buffer += concat(buffer, templn);}

				if(hilite) buffer += concat(buffer, "</font>");
			}
			buffer += concat(buffer, "<br>");
		}

		p++; if(p>=0x10) p=0;
	}

#ifdef COBRA_ONLY
	if(lv1) { system_call_1(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_ENABLE_COBRA); }
#endif
	// footer

	if(!is_file)
	{
		buffer += concat(buffer, "<hr>Dump: [<a href=\"/dump.ps3?mem\">Full Memory</a>] [<a href=\"/dump.ps3?lv1\">LV1</a>] [<a href=\"/dump.ps3?lv2\">LV2</a>]");
		sprintf(templn, " [<a href=\"/dump.ps3?%llx\">Dump 0x%llx</a>]", lv1?address:address + LV2_OFFSET_ON_LV1, lv1?address:address + LV2_OFFSET_ON_LV1); buffer += concat(buffer, templn);
		sprintf(templn, " <a id=\"pblk\" href=\"/peek.lv%i?%llx\">&lt;&lt;</a> <a id=\"back\" href=\"/peek.lv%i?%llx\">&lt;Back</a>", lv1?1:2, ((int)(address-0x1000)>=0)?(address-0x1000):0, lv1?1:2, ((int)(address-HEXVIEW_SIZE)>=0)?(address-HEXVIEW_SIZE):0); buffer += concat(buffer, templn);
		sprintf(templn, " <a id=\"next\" href=\"/peek.lv%i?%llx\">Next&gt;</a> <a id=\"nblk\" href=\"/peek.lv%i?%llx\">&gt;&gt;</a></pre>", lv1?1:2, ((int)(address+0x400)<(int)upper_memory)?(address+HEXVIEW_SIZE):(upper_memory-HEXVIEW_SIZE), lv1?1:2, ((int)(lv1+0x1200)<(int)upper_memory)?(address+0x1000):(upper_memory-HEXVIEW_SIZE)); buffer += concat(buffer, templn);
	}

	// add navigation with left/right keys
	concat(buffer,  "<script>"
					"document.addEventListener('keydown',kd,false);"
					"function kd(e){"
					"if(typeof document.activeElement.name!='undefined')return;"
					"e=e||window.event;var kc=e.keyCode;"
					"if(kc==37){e.ctrlKey?pblk.click():back.click();}"
					"if(kc==39){e.ctrlKey?nblk.click():next.click();}}"
					"</script>");
}

#endif

