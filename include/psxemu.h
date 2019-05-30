#ifdef COBRA_ONLY
static void select_ps1emu(const char *path)
{
	webman_config->ps1emu = pad_select_netemu(path, webman_config->ps1emu);

	sys_map_path("/dev_flash/ps1emu/ps1_netemu.self", NULL);
	sys_map_path("/dev_flash/ps1emu/ps1_emu.self"   , NULL);

	char msg[50];

	if(webman_config->ps1emu)
	{
		sys_map_path("/dev_flash/ps1emu/ps1_netemu.self", "///dev_flash/ps1emu/ps1_emu.self");
		sys_map_path("/dev_flash/ps1emu/ps1_emu.self"   , "///dev_flash/ps1emu/ps1_netemu.self");

		sprintf(msg, "%s %s", "ps1_netemu.self", STR_ENABLED);
	}
	else
	{
		sprintf(msg, "%s %s", "ps1_emu.self", STR_ENABLED);
	}

	if(pad_data.len > 0) show_msg(msg);
}
#endif
