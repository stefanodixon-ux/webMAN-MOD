#ifdef COBRA_ONLY
#define SYSCALL8_OPCODE_ENABLE_PS2NETEMU	0x1ee9	/* Cobra 7.50 */
#define PS2NETEMU_GET_STATUS				2

static int get_cobra_ps2netemu_status(void)
{
	system_call_2(SC_COBRA_SYSCALL8, (u64) SYSCALL8_OPCODE_ENABLE_PS2NETEMU, (u64) PS2NETEMU_GET_STATUS);
	return (int)p1;
}

static void enable_ps2netemu_cobra(int param)
{
#ifdef SPOOF_CONSOLEID
	if ((eid0_idps[0] & 0x00000000000000FF) > 0x04) return; // 0x01 = CECH-A*, 0x02 = CECH-B, 0x03 = CECH-C, 0x04 = CECH-E
#endif
	int status = get_cobra_ps2netemu_status();

	if(status < 0 || status == param) return;

	system_call_2(SC_COBRA_SYSCALL8, (u64) SYSCALL8_OPCODE_ENABLE_PS2NETEMU, (u64) param);

	if(pad_data.len > 0)
	{
		char msg[100];
		sprintf(msg, "%s %s", webman_config->ps2emu ? "ps2_netemu.self" : "ps2_emu.self", STR_ENABLED);
		show_msg(msg);
	}
}
#endif

// called from mount_misc.h
static inline void copy_ps2icon(char *temp, const char *_path)
{
	if(file_exists(PS2_CLASSIC_ISO_ICON ".bak") == false)
		_file_copy((char*)PS2_CLASSIC_ISO_ICON, (char*)(PS2_CLASSIC_ISO_ICON ".bak"));

	size_t len = sprintf(temp, "%s.png", _path);
	if(file_exists(temp) == false) sprintf(temp, "%s.PNG", _path);
	if(file_exists(temp) == false && len > 12) sprintf(temp + len - 12, ".png"); // remove .BIN.ENC
	if(file_exists(temp) == false && len > 12) sprintf(temp + len - 12, ".PNG");

	cellFsUnlink(PS2_CLASSIC_ISO_ICON);
	if(file_exists(temp))
		_file_copy(temp, (char*)PS2_CLASSIC_ISO_ICON);
	else
		_file_copy((char*)(PS2_CLASSIC_ISO_ICON ".bak"), (char*)PS2_CLASSIC_ISO_ICON);
}

static inline void copy_ps2config(char *temp, const char *_path)
{
	size_t len = sprintf(temp, "%s.CONFIG", _path);
	if(file_exists(temp) == false && len > 15) strcpy(temp + len - 15, ".CONFIG\0"); // remove .BIN.ENC
	if(file_exists(temp) == false)
	{
		char *game_id = strstr(_path, "[SL"); 
		if(!game_id)
			  game_id = strstr(_path, "[SC");
		if(game_id)
		{
			if(game_id[5] == '_')
				sprintf(temp, "%s/%.11s.ENC", "/dev_hdd0/game/PS2CONFIG/USRDIR/CONFIG/ENC", game_id + 1); //[SLxS_000.00]
			else
				sprintf(temp, "%s/%.4s_%3s.%.2s.ENC", "/dev_hdd0/game/PS2CONFIG/USRDIR/CONFIG/ENC",
								game_id + 1,  // SLES, SLUS, SLPM, SLPS, SCES, SCUS, SCPS
								game_id + 5,  // _000.00
								game_id + 8); // [SLxS00000]
		}
	}

	cellFsUnlink(PS2_CLASSIC_ISO_CONFIG);
	_file_copy(temp, (char*)PS2_CLASSIC_ISO_CONFIG);
}

#ifndef LITE_EDITION

static void enable_classic_ps2_mode(void)
{
	save_file(PS2_CLASSIC_TOGGLER, NULL, 0);
}

static void disable_classic_ps2_mode(void)
{
	cellFsUnlink(PS2_CLASSIC_TOGGLER);
}

#endif