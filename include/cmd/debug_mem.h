#ifdef DEBUG_MEM
	if(islike(param, "/dump.ps3"))
	{
		// /dump.ps3?lv1
		// /dump.ps3?lv2
		// /dump.ps3?rsx
		// /dump.ps3?mem
		// /dump.ps3?full
		// /dump.ps3?flash
		// /dump.ps3?<start-address>
		// /dump.ps3?<start-address>&size=<mb>

		ps3mapi_mem_dump(pbuffer, templn, param);
	}
	else
	if(islike(param, "/peek.lv") || islike(param, "/poke.lv") || islike(param, "/find.lv") || islike(param, "/hexview.ps3/"))
	{
		// /peek.lv1?<address>
		// /poke.lv1?<address>=<value>
		// /find.lv1?<value>
		// /find.lv1?<start-address>=<hex or text value>
		// /peek.lv2?<address>
		// /poke.lv2?<address>=<value>
		// /find.lv2?<value>
		// /find.lv2?<start-address>=<hex or text value>
		// /hexview.ps3<file-path>
		// /hexview.ps3<file-path>&offset=<value>
		// /hexview.ps3<file-path>&offset=<value>&data=<new-value>

		ps3mapi_find_peek_poke_hexview(pbuffer, templn, param);
	}
	else
#endif
