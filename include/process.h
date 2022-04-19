#define SC_GET_PRX_MODULE_BY_ADDRESS	(461)
#define SC_STOP_PRX_MODULE 				(482)
#define SC_UNLOAD_PRX_MODULE 			(483)
#define SC_PPU_THREAD_EXIT				(41)

static inline sys_prx_id_t prx_get_module_id_by_address(void *addr)
{
	system_call_1(SC_GET_PRX_MODULE_BY_ADDRESS, (u64)(u32)addr);
	return (int)p1;
}
/*
static void unload_prx_module(void)
{
	sys_prx_id_t prx = prx_get_module_id_by_address(unload_prx_module);
	{system_call_3(SC_UNLOAD_PRX_MODULE, prx, 0, NULL);}
}
*/
static inline void _sys_ppu_thread_exit(u64 val)
{
	system_call_1(SC_PPU_THREAD_EXIT, val); // prxloader = mandatory; cobra = optional; ccapi = don't use !!!
}

static void finalize_module(void)
{
	u64 meminfo[5];

	meminfo[0] = 0x28;
	meminfo[1] = 2;
	meminfo[3] = 0;

	sys_prx_id_t prx = prx_get_module_id_by_address(finalize_module);

	{system_call_3(SC_STOP_PRX_MODULE, prx, 0, (u64)(u32)meminfo);}

	if(wm_unload_combo == 3) // L1+R1+TRIANGLE
	{
		char prx_id[20]; sprintf(prx_id, "%d", prx);
		save_file(WM_RELOAD_FILE, prx_id, 0); // save prx_id for unload this process from the other plugin
	}
	else if((wm_unload_combo == 4) || file_exists(WM_UNLOAD_FILE)) // force unload module
		{system_call_3(SC_UNLOAD_PRX_MODULE, prx, 0, NULL);}
}
