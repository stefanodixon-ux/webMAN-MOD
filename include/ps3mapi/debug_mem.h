#define LV1		1
#define LV2		2
#define FLASH	3
#define HDD0	7
#define USB0	10
#define USB1	11
#define PID		14

#if defined(DEBUG_MEM) || defined(PS3MAPI)

#define LV1_UPPER_MEMORY	0x8000000010000000ULL
#define LV2_UPPER_MEMORY	0x8000000000800000ULL

static void poke_chunk_lv1(u64 start, int size, u8 *buffer, u8 oper)
{
	start |= BASE_MEMORY; u64 old_value, new_value;
	for(int offset = 0; offset < size; offset += 8)
	{
		new_value = *(u64*)(buffer + offset);
		if(oper)
		{
			old_value = peek_lv1(start + offset);
			new_value = update_value(old_value, new_value, oper);
		}
		poke_lv1(start + offset, new_value);
	}
}

static void poke_chunk_lv2(u64 start, int size, u8 *buffer, u8 oper)
{
	start |= BASE_MEMORY; u64 old_value, new_value;
	for(int offset = 0; offset < size; offset += 8)
	{
		new_value = *(u64*)(buffer + offset);
		if(oper)
		{
			old_value = peekq(start + offset);
			new_value = update_value(old_value, new_value, oper);
		}
		pokeq(start + offset, new_value);
	}
}

static void peek_chunk_lv1(u64 start, u64 size, u64 *buffer) // read from lv1
{
	for(u64 offset = 0; offset < size; offset += 8)
	{
		*(buffer++) = peek_lv1(start + offset);
	}
}

static void peek_chunk_lv2(u64 start, u64 size, u64 *buffer) // read from lv1
{
	for(u64 offset = 0; offset < size; offset += 8)
	{
		*(buffer++) = peekq(start + offset);
	}
}

static int peek_chunk_device(u64 device, u64 start_sector, u32 size, u64 *buffer) // read from lv1
{
	u32 read;
	sys_device_handle_t dev_id;
	if(sys_storage_open(device, 0, &dev_id, 0) == CELL_OK)
	{
		start_sector /= 0x200, size /= 0x200; // convert bytes to sector
		sys_storage_read(dev_id, 0, start_sector, MAX(size, 1), buffer, &read, 0);
		sys_storage_close(dev_id);
		return CELL_OK;
	}
	else
	{
		_memset((u8*)buffer, size);
		return FAILED;
	}
}

static void peek_chunk_flash(u64 start_sector, u32 size, u64 *buffer) // read from lv1
{
	if (peek_chunk_device(FLASH_DEVICE_NOR,  start_sector, size, buffer) == FAILED)
		peek_chunk_device(FLASH_DEVICE_NAND, start_sector, size, buffer);
}

static int ps3mapi_get_memory(u32 pid, u32 address, char *mem, u32 size)
{
	if(pid == LV1)
	{
		peek_chunk_lv1((address | BASE_MEMORY), size, (u64*)mem);
	}
	else if(pid == LV2)
	{
		peek_chunk_lv2((address | BASE_MEMORY), size, (u64*)mem);
	}
	else if(pid == FLASH)
	{
		peek_chunk_flash(address, size, (u64*)mem);
	}
	else if(pid < PID)
	{
		u64 device = (pid >= 10) ?  (0x103000000000000ULL | pid): // USB
									(0x101000000000000ULL | pid); // HDD
		peek_chunk_device(device, address, size, (u64*)mem);
	}
	#ifdef PS3MAPI
	else
	{
		#define PS3MAPI_OPCODE_GET_PROC_MEM				0x0031

		system_call_6(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_PROC_MEM, (u64)pid, (u64)address, (u64)(u32)mem, size);
		return (int)p1;
	}
	#endif
	return 0;
}

static void ps3mapi_dump_process(const char *dump_file, u32 pid, u32 address, u32 size)
{
	sys_addr_t sysmem = sys_mem_allocate(_64KB_);
	if(sysmem)
	{
		{ PS3MAPI_ENABLE_ACCESS_SYSCALL8 }

		int fd;
		char *mem_buf = (char*)sysmem;

		show_msg_with_icon(ICON_WAIT, "Dumping...");

		if(cellFsOpen(dump_file, CELL_FS_O_CREAT | CELL_FS_O_WRONLY | CELL_FS_O_TRUNC, &fd, NULL, 0) == CELL_FS_SUCCEEDED)
		{
			for(u32 addr = 0; addr < size; addr += _64KB_)
			{
				ps3mapi_get_memory(pid, (address + addr), mem_buf, _64KB_);
				cellFsWrite(fd, mem_buf, _64KB_, NULL);
				#ifdef FPS_OVERLAY
				if(overlay_enabled)
				{
					char label[20];
					sprintf(label, "0x%x", address + addr);
					show_progress(label, OV_DUMP);
				}
				#endif
			}
			cellFsClose(fd);
		}
		sys_memory_free(sysmem);

		show_msg("Dump completed!");

		play_rco_sound("snd_trophy");

		disable_progress();

		{ PS3MAPI_DISABLE_ACCESS_SYSCALL8 }
	}
}

#ifdef DEBUG_MEM
static void ps3mapi_mem_dump(char *buffer, char *html, char *param)
{
	char dump_file[MAX_PATH_LEN]; u64 start = 0; u32 size = 8, pid = LV1;
	strcat(buffer, "Dump: [<a href=\"/dump.ps3?mem\">Full Memory</a>] [<a href=\"/dump.ps3?flash\">Flash</a>] [<a href=\"/dump.ps3?rsx\">RSX</a>] [<a href=\"/dump.ps3?vsh\">VSH</a>] [<a href=\"/dump.ps3?lv1\">LV1</a>] [<a href=\"/dump.ps3?lv2\">LV2</a>]");
	strcat(buffer, "<hr>");

	if(param[9] == '?' && param[10] >= '0')
	{
		if(islike(param + 10, "lv1")) {size = 16;} else
		if(islike(param + 10, "lv2")) {pid = LV2;} else
		if(islike(param + 10, "fla"))
		{
			#ifdef SPOOF_CONSOLEID
			get_eid0_idps();
			pid = FLASH, size = IS_NAND ? 256 : 16;
			#else
			pid = FLASH, size = 16;
			#endif
		} else
		if(param[10] == 'v' /*vsh */) {start = 0x910000;}  else
		if(param[10] == 'r' /*rsx */) {start = 0x0000028080000000ULL; size=256;}  else
		if(param[10] == 'm' /*mem */) {size = IS_DEH ? 512 : 256;} else
		if(param[10] == 'f' /*full*/) {size = IS_DEH ? 512 : 256;} else
		{
			start = convertH(param + 10);
			if(start >= LV1_UPPER_MEMORY - ((u64)(size * _1MB_))) start = LV1_UPPER_MEMORY - ((u64)(size * _1MB_));
		}

		const char *pos = strstr(param, "&size=");  if(pos) size  = val(pos + 6);
					pos = strstr(param, "&start="); if(pos) start = val(pos + 7);

		if(start <= 0x100) start *= _1MB_;

		sprintf(dump_file, "/dev_hdd0/dump_%s.bin", param + 10);
		ps3mapi_dump_process(dump_file, pid, start, (size * _1MB_));
		add_breadcrumb_trail2(buffer, "<p>Dumped:", dump_file);
		sprintf(html, " [" HTML_URL2 "]", "/delete.ps3", dump_file, STR_DELETE); strcat(buffer, html);
	}
}
#endif //#ifdef DEBUG_MEM

#define HEXVIEW_SIZE	0x200

static void ps3mapi_find_peek_poke_hexview(char *buffer, char *templn, char *param)
{
	size_t size = 0;
	u64 address = 0, addr, byte_addr, upper_memory = 0x8FFFFFFFFFFFFFF8ULL, found_address = 0, step = 1;
	u8 byte = 0, p = 0, lv1 = 0, rep = 1, oper = 0; // replace value
	bool found = false, not_found = false;
	int flen = 0, hilite;
	char *v; char *html = templn;

	char sfind[0x60];
	u8 data[HEXVIEW_SIZE];

	bool is_find = islike(param, "/find.lv");
	bool is_file = islike(param, "/hexview.ps3/");
	bool check_address = strchr(param, '!')  ? 1 : 0;

	if(is_file)
	{
		char *fname = param + 12;

		flen = get_param("&find=", sfind, fname, 0x60);

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

			sys_addr_t sysmem = sys_mem_allocate(_64KB_);
			if(sysmem)
			{
				char *mem = (char*)sysmem, *mask = sfind;
				s32 i, n; u64 addr = address;
				u64 faster = faster_find(sfind, flen, mask);
				while((n = read_file(fname, mem, _64KB_, addr)))
				{
					n -= flen; if(n <= 0) break;
					for(i = 0; i < n; i++)
					{
						if(faster && (*(u64*)(mem + i) != faster)) continue;
						if(!bcompare(mem + i, sfind, flen, mask) && !(--rep)) break;
					}
					addr += i; if(i < n) {address = found_address = addr; found = true; break;}
				}
				sys_memory_free(sysmem);
				if(rep) not_found = true;
			}
			else
				not_found = true;
		}

		if(pos)
		{
			pos += 6;

			// data write to write
			if(isHEX(pos))
				flen = Hex2Bin(pos, templn);
			else
			{
				flen = strcopy(templn, pos);
				// convert pipes to line breaks
				for(pos = templn; *pos; ++pos) if(*pos == '|') *pos = '\n';
			}

			found_address = address, found = true;
			patch_file(fname, templn, address, flen);
			goto view_file;
		}
	}

	if(is_file) {get_flag(param + 10, "&"); goto view_file;}
	address = convertH(param + 10);

	v = strchr(param + 10, '=');
	if(v)
	{
		oper = get_operator(v, false); // =, |=, &=, ^=, @= (poke.lv1 / poke.lv2)
		flen = strlen(v + 1);
		for(p = 1; p <= flen; p++) if(!memcmp(v + p, " ", 1)) byte++; //ignore spaces
		flen -= byte; byte = p = 0;
	}

	concat(buffer, "<pre>");

	lv1 = strstr(param,".lv1?") ? 1 : 0;
	#ifdef COBRA_ONLY
	if(lv1) { system_call_1(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_DISABLE_COBRA); }
	#endif

	if(check_address || !lv1) address |= BASE_MEMORY;

	if(is_find)
	{
		char *pos = strstr(param, "&rep="); if(pos) {rep = (u8)val(pos + 5); *pos = NULL;}

		pos = strchr(param, '!'); if(!pos) pos = strstr(param, "&align");
		if(pos) {*pos = NULL, step = 4, address &= 0x80000000FFFFFFFCULL;} // find using aligned memory address (4X faster) e.g. /find.lv2?3000=3940ffff#

		pos = strstr(param, "&stop=");
		if(pos)
			{upper_memory = convertH(pos + 6) | BASE_MEMORY; *pos = NULL;}
		else
			{upper_memory = (lv1 ? LV1_UPPER_MEMORY : LV2_UPPER_MEMORY) - 8;}
	}
	else if(check_address)
	{
		upper_memory = (lv1 ? LV1_UPPER_MEMORY : LV2_UPPER_MEMORY) - 8; // safe memory adddress
		if(address > upper_memory) address = upper_memory - HEXVIEW_SIZE;
	}

	if((v == NULL) || (address > upper_memory)) { /* ignore find/poke if value is not provided or invalid address */ }
	else
	if(is_find)
	{
		char tfind[33], label[20];
		if(isHEX(v + 1))
			flen = Hex2Bin(v + 1, sfind);
		else
			flen = strcopy(sfind, v + 1);

		u64 (*peek_mem)(u64) = lv1 ? peek_lv1 : peekq;

		u64 _upper_memory = (upper_memory - flen + 8) & 0x8FFFFFFFFFFFFFF0ULL;
		u64 faster = faster_find(sfind, flen, sfind);
		for(addr = address; addr < _upper_memory; addr += step)
		{
			if(faster && (peek_mem(addr) != faster)) continue;

			for(u8 n = 0; n < flen; n += 8)
				*(u64*)(tfind + n) = peek_mem(addr +  n);

			if(!bcompare(tfind, sfind, flen, sfind) && !(--rep)) {address = found_address = addr; found = true; break;}

			if((addr & 0xF00) == 0) {sprintf(label, "0x%llx", address + addr); show_progress(label, OV_FIND);}
		}

		if(!found)
		{
			sprintf(html, "<font color=#ff0>%s</font><p>", STR_NOTFOUND); concat(buffer, html);
		}
		else
		{
			found_address = address = addr;
			sprintf(html, "Offset: 0x%08X<br><br>", (u32)address); concat(buffer, html);
		}
		disable_progress();
	}
	else
	if(islike(param, "/poke.lv"))
	{
		char *value = v + 1;

		if(lv1)
			peek_chunk_lv1(address, 0x400, (u64 *)templn);
		else
			peek_chunk_lv2(address, 0x400, (u64 *)templn);

		flen = strlen(value); flen = MIN(flen, 0x400);

		if(isHEX(value))
			flen = Hex2Bin(value, templn);
		else
			memcpy64(templn, value, flen);

		if(lv1)
			poke_chunk_lv1(address, flen, (u8 *)templn, oper);
		else
			poke_chunk_lv2(address, flen, (u8 *)templn, oper);

		found_address = address; found = true;
	}

view_file:

	address &= 0xFFFFFFFFFFFFFFF0ULL;
	addr = address;

	if(is_file)
	{
		param += 12; check_path_alias(param);
		read_file(param, (void*)data, 0x200, address);
		add_breadcrumb_trail(buffer, param);

		// MD5
		concat(buffer, ": [");
		sprintf(html, HTML_URL2, "/md5.ps3", param, "MD5"); concat(buffer, html);
		concat(buffer, "]");

		// file navigation
		size = file_size(param);
		u64 max = (size < HEXVIEW_SIZE) ? 0 : (size - HEXVIEW_SIZE);
		sprintf(html, "<span style='float:right'><a id=\"pblk\" href=\"/hexview.ps3%s\">&lt;&lt;</a> <a id=\"back\" href=\"/hexview.ps3%s&offset=0x%llx\">&lt;Back</a>", param, param, (address < HEXVIEW_SIZE) ? 0ULL : (address - HEXVIEW_SIZE)); concat(buffer, html);
		sprintf(html, " <a id=\"next\" href=\"/hexview.ps3%s&offset=0x%llx\">Next></a> <a id=\"nblk\" href=\"/hexview.ps3%s&offset=0x%llx\">>></a></span>", param, MIN(address + HEXVIEW_SIZE, max), param, max);
		concat(buffer, html);

		char *pos = strstr(param, "&find="); if(pos) *pos = NULL;
		sprintf(html, " [<a href=\"javascript:void(location.href='%s&offset=0x%llx&data='+prompt('%s').toString());\">%s</a>]", param - 12, address, "Write", "Write");
		concat(buffer, html);

		sprintf(html, " [<a href=\"javascript:void(location.href='%s&offset=0x%llx&find='+prompt('%s','%s').toString());\">%s</a>] %s%s%s", param - 12, address + 0x10, "Find", sfind, "Find", "<font color=#ff0>", not_found ? "Not found!" : "", "</font><hr>");
		concat(buffer, html);
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
			concat(buffer, templn);
		}

		byte_addr = address + i;

		if(is_file)
			byte = data[i];
		else
			byte = (u8)((lv1 ? peek_lv1(byte_addr) : peekq(byte_addr)) >> 56);

		hilite = found && (byte_addr >= found_address) && (byte_addr < (found_address + flen));
		if(hilite) concat(buffer, "<font color=#ff0>");
		sprintf(templn, "%02X ", byte); concat(buffer, templn);
		if(hilite) concat(buffer, "</font>");

		if(p == 0xF)
		{
			concat(buffer, " ");
			for(u8 c = 0; c < 0x10; c++, addr++)
			{
				if(is_file)
					byte = data[(0x10 * (i / 0x10)) + c];
				else
					byte = (u8)((lv1 ? peek_lv1(addr) : peekq(addr)) >> 56);
				if(byte<32 || byte>=127) byte='.';

				hilite = (found && addr >= found_address) && (addr < (found_address + flen));
				if(hilite) concat(buffer, "<font color=#ff0>");
				if(byte==0x3C)
					concat(buffer, "&lt;");
				else if(byte==0x3E)
					concat(buffer, ">");
				else
					{sprintf(templn,"%c", byte); concat(buffer, templn);}

				if(hilite) concat(buffer, "</font>");
			}
			concat(buffer, "<br>");

			if(is_file && (byte_addr >= size - 1)) break;
		}

		p++; if(p>=0x10) p=0;
	}

	#ifdef COBRA_ONLY
	if(lv1) { system_call_1(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_ENABLE_COBRA); }
	#endif

	// footer

	if(!is_file)
	{
		concat(buffer, "<hr>Dump: [<a href=\"/dump.ps3?mem\">Full Memory</a>] [<a href=\"/dump.ps3?lv1\">LV1</a>] [<a href=\"/dump.ps3?lv2\">LV2</a>]");
		sprintf(html, " [<a href=\"/dump.ps3?%llx\">Dump 0x%llx</a>]", lv1?address:address + LV2_OFFSET_ON_LV1, lv1?address:address + LV2_OFFSET_ON_LV1); concat(buffer, html);
		sprintf(html, " <a id=\"pblk\" href=\"/peek.lv%i?%llx\">&lt;&lt;</a> <a id=\"back\" href=\"/peek.lv%i?%llx\">&lt;Back</a>", lv1?1:2, ((int)(address-0x1000)>=0)?(address-0x1000):0, lv1?1:2, ((int)(address-HEXVIEW_SIZE)>=0)?(address-HEXVIEW_SIZE):0); concat(buffer, html);
		sprintf(html, " <a id=\"next\" href=\"/peek.lv%i?%llx\">Next></a> <a id=\"nblk\" href=\"/peek.lv%i?%llx\">>></a></pre>", lv1?1:2, ((int)(address+0x400)<(int)upper_memory)?(address+HEXVIEW_SIZE):(upper_memory-HEXVIEW_SIZE), lv1?1:2, ((int)(lv1+0x1200)<(int)upper_memory)?(address+0x1000):(upper_memory-HEXVIEW_SIZE)); concat(buffer, html);
	}

	// add navigation with left/right keys
	add_html(dat_DEBUG_MEM_KEYS, 0, buffer, html);
}

#endif // #if defined(DEBUG_MEM) || defined(PS3MAPI)
