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

static void get_ps_titleid_from_path(char *title_id, const char *_path)
{
	// returns title_id as "SCES00000"

	if(!title_id) return;

	if(!_path || *_path != '/') {*title_id = NULL; return;}

	char *path = strrchr(_path, '/');
	char *game_id = strstr(path, " [S"); // title id enclosed in square brackets

	if(game_id) 
		path = game_id + 2;
	else
	{
		game_id = strstr(path, "[S"); // title id enclosed in square brackets
		if(game_id)
			path = game_id + 1;
		else
		{
			game_id = strstr(path, "(S"); // title id enclosed in round brackets
			if(game_id) path = game_id + 1;
		}
	}

	// SLES 5052 games
	// SLUS 2325 games
	// SLPS 2119 games
	// SLPM 3488 games
	// SLKA  481 games
	// SLAJ  175 games

	// SCES 5003 games
	// SCPS  735 games
	// SCUS  604 games
	// SCAJ  397 games
	// SCKA  151 games

	// TCPS  178 games

		game_id = strstr(path, "SL"); // 13640 games SLES/SLUS/SLPM/SLPS/SLAJ/SLKA
	if(!game_id)
		game_id = strstr(path, "SC"); // 6890 games SCES/SCUS/SCPS/SCAJ/SCKA
	if(!game_id)
		game_id = strstr(path, "TCPS"); // 178 games

	u16 len = 0; for(;game_id[len] && ++len < 12;);

	if(!game_id || len < 9 || game_id[5] < '0' || game_id[5] > '9' || !strchr("EUPKA", game_id[2]))
		*title_id = NULL;
	else if(game_id[4] == '_' && len >= 11)
		sprintf(title_id, "%.4s%.3s%.2s", game_id, game_id + 5, game_id + 9); // SLxS_000.00
	else if(game_id[4] == '-' && len >= 10)
		sprintf(title_id, "%.4s%.5s", game_id, game_id + 5); // SLxS-00000
	else
		sprintf(title_id, "%.9s", game_id); // SLxS00000
}

static inline void copy_ps2config(char *temp, const char *_path)
{
	size_t len = sprintf(temp, "%s.CONFIG", _path);
	if(file_exists(temp) == false && len > 15) strcpy(temp + len - 15, ".CONFIG\0"); // remove .BIN.ENC
	if(file_exists(temp) == false)
	{
		char title_id[TITLE_ID_LEN + 1];
		get_ps_titleid_from_path(title_id, _path);

		if(strlen(title_id) == TITLE_ID_LEN)
		{
			sprintf(temp, "%s/%.4s_%3s.%.2s.ENC", "/dev_hdd0/game/PS2CONFIG/USRDIR/CONFIG/ENC",
							title_id,      // SLES, SLUS, SLPM, SLPS, SCES, SCUS, SCPS
							title_id + 4,  // _000.00
							title_id + 7); // SLxS00000
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