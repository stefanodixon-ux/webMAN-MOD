#ifdef SWAP_KERNEL
static void swap_kernel(const char *source, char *target, char *tempstr)
{
	u64 size = file_size(source);

	if(size)
	{
		enable_dev_blind(source);

		// for cobra req: /dev_flash/sys/stage2.bin & /dev_flash/sys/lv2_self
		strcopy(target, SYS_COBRA_PATH "stage2.bin");
		if(isDir("/dev_flash/rebug/cobra"))
		{
			const char *ext = get_ext(source);
			if(IS(ext, ".dex"))
				concat_path(target, "/dev_flash/rebug/cobra", "stage2.dex");
			else if(IS(ext, ".cex"))
				concat_path(target, "/dev_flash/rebug/cobra", "stage2.cex");
		}

		if(not_exists(target))
		{
			strcopy(tempstr, source);
			strcopy(get_filename(tempstr), "/stage2.bin");
			if(file_exists(tempstr)) force_copy(tempstr, target);
		}

		// copy: /dev_flash/sys/lv2_self
		strcopy(target, "/dev_blind/sys/lv2_self");
		if(file_size(target) != size)
			force_copy(source, target);

		if(file_size(target) == size)
		{
			#define FLH_OS		0x2F666C682F6F732FULL
			#define OS_LV2		0x2F6F732F6C76325FULL

			#define LOCAL_S		0x2F6C6F63616C5F73ULL
			#define YS0_SYS		0x7973302F7379732FULL
			#define LV2_SELF	0x6C76325F73656C66ULL

			u64 lv2_offset = 0x15DE78; // 4.xx CFW LV1 memory location for: /flh/os/lv2_kernel.self
			if(peek_lv1(lv2_offset) != FLH_OS)
				for(u64 addr = 0x100000ULL; addr < 0xFFFFF8ULL; addr += 4) // Find in 16MB
					if(peek_lv1(addr) == OS_LV2)      // /os/lv2_
					{
						lv2_offset = addr - 4; break; // 0x12A2C0 on 3.55
					}

			if(peek_lv1(lv2_offset) == FLH_OS)  // Original: /flh/os/lv2_kernel.self
			{
				poke_lv1(lv2_offset + 0x00, LOCAL_S); // replace -> /local_sys0/sys/lv2_self
				poke_lv1(lv2_offset + 0x08, YS0_SYS);
				poke_lv1(lv2_offset + 0x10, LV2_SELF);

				working = 0;
				del_turnoff(0); // no beep
				create_file(WM_NOSCAN_FILE);
				{system_call_3(SC_SYS_POWER, SYS_REBOOT, NULL, 0);} /*load LPAR id 1*/
				sys_ppu_thread_exit(0);
			}
		}
	}
	else
		strcpy(target, STR_ERROR);
}
#endif

#ifdef COBRA_NON_LITE
static void swap_file(const char *path, const char *curfile, const char *rento, const char *newfile)
{
	char file1[64], file2[64];

	concat3(file1, "/dev_flash", path + 10, newfile);

	if(file_exists(file1))
	{
		mount_device("/dev_blind", NULL, NULL);
		concat2(file1, path, curfile);

		concat2(file2, path, rento);
		cellFsRename(file1, file2);

		concat2(file2, path, newfile);
		cellFsRename(file2, file1);
	}
}
#endif // #ifdef COBRA_NON_LITE

#ifdef REX_ONLY
static bool toggle_rebug_mode(void)
{
	enable_dev_blind("REBUG Mode Switcher activated!");

	if(file_exists(VSH_MODULE_PATH "vsh.self.swp"))
	{
		show_msg("Normal Mode detected!\n"
				 "Switch to REBUG Mode Debug XMB...");
		sys_ppu_thread_sleep(3);

		swap_file(VSH_ETC_PATH, "index.dat", "index.dat.nrm", "index.dat.swp");
		swap_file(VSH_ETC_PATH, "version.txt", "version.txt.nrm", "version.txt.swp");
		swap_file(VSH_MODULE_PATH, "vsh.self", "vsh.self.nrm", "vsh.self.swp");

		return true; // vsh reboot
	}
	else
	if((file_exists(VSH_MODULE_PATH "vsh.self.nrm"))
	&& (file_exists(VSH_MODULE_PATH "vsh.self.cexsp")))
	{
		show_msg("REBUG Mode Debug XMB detected!\n"
				 "Switch to Retail XMB...");
		sys_ppu_thread_sleep(3);

		swap_file(VSH_MODULE_PATH, "vsh.self", "vsh.self.dexsp", "vsh.self.cexsp");

		return true; // vsh reboot
	}
	else
	if(file_exists(VSH_MODULE_PATH "vsh.self.dexsp"))
	{
		show_msg("REBUG Mode Retail XMB detected!\n"
				 "Switch to Debug XMB...");
		sys_ppu_thread_sleep(3);

		swap_file(VSH_MODULE_PATH, "vsh.self", "vsh.self.cexsp", "vsh.self.dexsp");

		return true; // vsh reboot
	}
	return false;
}

static bool toggle_normal_mode(void)
{
	enable_dev_blind("Normal Mode Switcher activated!");

	if((file_exists(VSH_MODULE_PATH "vsh.self.nrm"))
	&& (file_exists(VSH_MODULE_PATH "vsh.self.cexsp")))
	{
		show_msg("REBUG Mode Debug XMB detected!\n"
				 "Switch to Normal Mode...");

		swap_file(VSH_ETC_PATH, "index.dat", "index.dat.swp", "index.dat.nrm");
		swap_file(VSH_ETC_PATH, "version.txt", "version.txt.swp", "version.txt.nrm");
		swap_file(VSH_MODULE_PATH, "vsh.self", "vsh.self.swp", "vsh.self.nrm");

		return true; // vsh reboot
	}
	else
	if(file_exists(VSH_MODULE_PATH "vsh.self.dexsp"))
	{
		show_msg("REBUG Mode Retail XMB detected!\n"
				 "Switch to Normal Mode...");

		swap_file(VSH_ETC_PATH, "index.dat", "index.dat.swp", "index.dat.nrm");
		swap_file(VSH_ETC_PATH, "version.txt", "version.txt.swp", "version.txt.nrm");
		swap_file(VSH_MODULE_PATH, "vsh.self", "vsh.self.cexsp", "vsh.self.nrm");

		cellFsRename(VSH_MODULE_PATH "vsh.self.dexsp", VSH_MODULE_PATH "vsh.self.swp");

		return true; // vsh reboot
	}
	else
	if(file_exists(VSH_MODULE_PATH "vsh.self.swp"))
	{
		show_msg("Normal Mode detected!\n"
				 "No need to switch!");
		sys_ppu_thread_sleep(3);
		disable_dev_blind();
	}
	return false;
}

static void toggle_debug_menu(void)
{
	enable_dev_blind("Debug Menu Switcher activated!");

	if(file_exists(VSH_MODULE_PATH "sysconf_plugin.sprx.dex"))
	{
		show_msg("CEX QA Menu is active!\n"
				 "Switch to DEX Debug Menu...");

		swap_file(VSH_MODULE_PATH, "sysconf_plugin.sprx", "sysconf_plugin.sprx.cex", "sysconf_plugin.sprx.dex");
	}
	else
	if(file_exists(VSH_MODULE_PATH "sysconf_plugin.sprx.cex"))
	{
		show_msg("DEX Debug Menu is active!\n"
				 "Switch to CEX QA Menu...");

		swap_file(VSH_MODULE_PATH, "sysconf_plugin.sprx", "sysconf_plugin.sprx.dex", "sysconf_plugin.sprx.cex");
	}
	sys_ppu_thread_sleep(1);
	disable_dev_blind();
}
#endif //#ifdef REX_ONLY

#ifdef COBRA_NON_LITE
static bool toggle_cobra(void)
{
	enable_dev_blind("COBRA Toggle activated!");
	#ifdef REX_ONLY
	if( (file_exists(REBUG_COBRA_PATH "stage2.cex")) /* &&
		(file_exists(REBUG_COBRA_PATH "stage2.dex")) */)
	{
		show_msg("COBRA is active!\n"
				 "Deactivating COBRA...");

		save_file(TMP_DIR "/loadoptical", "SCE\0", 4); // Force load PS2 discs on BC consoles with Cobra 8.x

		cellFsRename(REBUG_COBRA_PATH "stage2.cex", REBUG_COBRA_PATH "stage2.cex.bak");
		cellFsRename(REBUG_COBRA_PATH "stage2.dex", REBUG_COBRA_PATH "stage2.dex.bak");

		{system_call_3(SC_SYS_POWER, SYS_HARD_REBOOT, NULL, 0);} // hard reboot
		return true;
	}
	else if((file_exists(REBUG_COBRA_PATH "stage2.cex.bak")) /* &&
			(file_exists(REBUG_COBRA_PATH "stage2.dex.bak")) */)
	{
		show_msg("COBRA is inactive!\n"
				 "Activating COBRA...");

		cellFsRename(REBUG_COBRA_PATH "stage2.cex.bak", REBUG_COBRA_PATH "stage2.cex");
		cellFsRename(REBUG_COBRA_PATH "stage2.dex.bak", REBUG_COBRA_PATH "stage2.dex");
		return true; // vsh reboot
	}
	#endif //#ifdef REX_ONLY
	if(file_exists(HABIB_COBRA_PATH "stage2.cex"))
	{
		show_msg("COBRA is active!\n"
				 "Deactivating COBRA...");

		cellFsRename(HABIB_COBRA_PATH "stage2.cex", HABIB_COBRA_PATH "stage2_disabled.cex");

		return true; // vsh reboot
	}
	else if(file_exists(HABIB_COBRA_PATH "stage2_disabled.cex"))
	{
		show_msg("COBRA is inactive!\n"
				 "Activating COBRA...");

		cellFsRename(HABIB_COBRA_PATH "stage2_disabled.cex", HABIB_COBRA_PATH "stage2.cex");

		return true; // vsh reboot
	}

	if(file_exists(SYS_COBRA_PATH "stage2.bin"))
	{
		show_msg("COBRA is active!\n"
				 "Deactivating COBRA...");

		cellFsRename(SYS_COBRA_PATH "stage2.bin", SYS_COBRA_PATH "stage2_disabled.bin");

		swap_file(COLDBOOT_PATH, "", ".cobra", ".normal");

		return true; // vsh reboot
	}
	else if(file_exists(SYS_COBRA_PATH "stage2_disabled.bin"))
	{
		show_msg("COBRA is inactive!\n"
				 "Activating COBRA...");

		cellFsRename(SYS_COBRA_PATH "stage2_disabled.bin", SYS_COBRA_PATH "stage2.bin");

		swap_file(COLDBOOT_PATH, "", ".normal", ".cobra");

		return true; // vsh reboot
	}
	return false;
}

static void toggle_ps2emu(void)
{
	enable_dev_blind("Swapping ps2emu activated!");
	#ifdef REX_ONLY
	if(file_exists(REBUG_TOOLBOX "ps2_netemu.self"))
	{
		struct CellFsStat s;
		u64 size1, size2;

		// ---- Backup PS2Emus to Rebug Toolbox folder ----
		if(not_exists(REBUG_TOOLBOX "ps2_netemu.self.cobra"))
			 force_copy(PS2_EMU_PATH  "ps2_netemu.self",
						(char*)REBUG_TOOLBOX "ps2_netemu.self.cobra");

		if(not_exists(REBUG_TOOLBOX "ps2_gxemu.self.cobra"))
			 force_copy(PS2_EMU_PATH  "ps2_gxemu.self",
						(char*)REBUG_TOOLBOX "ps2_gxemu.self.cobra");

		if(not_exists(REBUG_TOOLBOX "ps2_emu.self.cobra"))
			 force_copy(PS2_EMU_PATH  "ps2_emu.self",
						(char*)REBUG_TOOLBOX "ps2_emu.self.cobra");

		// ---- Swap ps2_netemu.self ----
		size1 = size2 = 0;
		if( cellFsStat(PS2_EMU_PATH  "ps2_netemu.self", &s) == CELL_FS_SUCCEEDED ) size1 = s.st_size;
		if( cellFsStat(REBUG_TOOLBOX "ps2_netemu.self", &s) == CELL_FS_SUCCEEDED ) size2 = s.st_size;

		show_msg((size1 == size2) ? (char*)"Restoring original Cobra ps2emu..." :
									(char*)"Switching to custom ps2emu...");

		if((size1 > 0) && (size2 > 0))
			force_copy((size1==size2) ? (char*)REBUG_TOOLBOX "ps2_netemu.self.cobra" :
										(char*)REBUG_TOOLBOX "ps2_netemu.self",
										(char*)PS2_EMU_PATH  "ps2_netemu.self");

		// ---- Swap ps2_gxemu.self ----
		size1 = size2 = 0;
		if( cellFsStat(PS2_EMU_PATH  "ps2_gxemu.self", &s) == CELL_FS_SUCCEEDED ) size1 = s.st_size;
		if( cellFsStat(REBUG_TOOLBOX "ps2_gxemu.self", &s) == CELL_FS_SUCCEEDED ) size2 = s.st_size;

		if((size1 > 0) && (size2 > 0))
			force_copy((size1==size2) ? (char*)REBUG_TOOLBOX "ps2_gxemu.self.cobra" :
										(char*)REBUG_TOOLBOX "ps2_gxemu.self",
										(char*)PS2_EMU_PATH  "ps2_gxemu.self");

		// ---- Swap ps2_emu.self ----
		size1 = size2 = 0;
		if( cellFsStat(PS2_EMU_PATH  "ps2_emu.self", &s) == CELL_FS_SUCCEEDED ) size1 = s.st_size;
		if( cellFsStat(REBUG_TOOLBOX "ps2_emu.self", &s) == CELL_FS_SUCCEEDED ) size2 = s.st_size;

		if((size1 > 0) && (size2 > 0))
			force_copy((size1==size2) ? (char*)REBUG_TOOLBOX "ps2_emu.self.cobra" :
										(char*)REBUG_TOOLBOX "ps2_emu.self",
										(char*)PS2_EMU_PATH  "ps2_emu.self");
	}
	else
	#endif //#ifdef REX_ONLY
	if(file_exists(PS2_EMU_PATH "ps2_netemu.self.swap"))
	{
		show_msg("Switching to custom ps2emu...");

		swap_file(PS2_EMU_PATH, "ps2_netemu.self", "ps2_netemu.tmp", "ps2_netemu.self.swap");
		swap_file(PS2_EMU_PATH, "ps2_gxemu.self",  "ps2_gxemu.tmp",  "ps2_gxemu.self.swap");
		swap_file(PS2_EMU_PATH, "ps2_emu.self",    "ps2_emu.tmp",    "ps2_emu.self.swap");
	}
	else if(file_exists(PS2_EMU_PATH "ps2_netemu.self.sp"))
	{
		show_msg("Switching to custom ps2emu...");

		swap_file(PS2_EMU_PATH, "ps2_netemu.self", "ps2_netemu.tmp", "ps2_netemu.self.sp");
		swap_file(PS2_EMU_PATH, "ps2_gxemu.self",  "ps2_gxemu.tmp",  "ps2_gxemu.self.sp");
		swap_file(PS2_EMU_PATH, "ps2_emu.self",    "ps2_emu.tmp",    "ps2_emu.self.sp");
	}
	else if(file_exists(PS2_EMU_PATH "ps2_netemu.tmp"))
	{
		show_msg("Restoring original ps2emu...");

		if(c_firmware >= 4.65f)
		{
			swap_file(PS2_EMU_PATH, "ps2_netemu.self", "ps2_netemu.self.swap", "ps2_netemu.tmp");
			swap_file(PS2_EMU_PATH, "ps2_gxemu.self",  "ps2_gxemu.self.swap",  "ps2_gxemu.tmp");
			swap_file(PS2_EMU_PATH, "ps2_emu.self",    "ps2_emu.self.swap",    "ps2_emu.tmp");
		}
		else
		{
			swap_file(PS2_EMU_PATH, "ps2_netemu.self", "ps2_netemu.self.sp", "ps2_netemu.tmp");
			swap_file(PS2_EMU_PATH, "ps2_gxemu.self",  "ps2_gxemu.self.sp",  "ps2_gxemu.tmp");
			swap_file(PS2_EMU_PATH, "ps2_emu.self",    "ps2_emu.self.sp",    "ps2_emu.tmp");
		}
	}
}
#endif //#ifdef COBRA_NON_LITE
