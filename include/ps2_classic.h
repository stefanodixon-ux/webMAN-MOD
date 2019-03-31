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
	get_eid0_idps();
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