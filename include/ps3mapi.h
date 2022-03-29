///////////// PS3MAPI BEGIN //////////////

#define SYSCALL8_OPCODE_PS3MAPI						0x7777

#define PS3MAPI_SERVER_VERSION						0x0120
#define PS3MAPI_SERVER_MINVERSION					0x0120

#define PS3MAPI_WEBUI_VERSION						0x0121
#define PS3MAPI_WEBUI_MINVERSION					0x0120

#define PS3MAPI_CORE_MINVERSION						0x0111

#define PS3MAPI_OPCODE_GET_CORE_VERSION				0x0011
#define PS3MAPI_OPCODE_GET_CORE_MINVERSION			0x0012
#define PS3MAPI_OPCODE_GET_FW_TYPE					0x0013
#define PS3MAPI_OPCODE_GET_FW_VERSION				0x0014
#define PS3MAPI_OPCODE_GET_ALL_PROC_PID				0x0021
#define PS3MAPI_OPCODE_GET_PROC_NAME_BY_PID			0x0022
#define PS3MAPI_OPCODE_GET_PROC_BY_PID				0x0023
#define PS3MAPI_OPCODE_GET_CURRENT_PROC				0x0024
#define PS3MAPI_OPCODE_GET_CURRENT_PROC_CRIT		0x0025
#define PS3MAPI_OPCODE_GET_PROC_MEM					0x0031
#define PS3MAPI_OPCODE_SET_PROC_MEM					0x0032
#define PS3MAPI_OPCODE_GET_ALL_PROC_MODULE_PID		0x0041
#define PS3MAPI_OPCODE_GET_PROC_MODULE_NAME			0x0042
#define PS3MAPI_OPCODE_GET_PROC_MODULE_FILENAME		0x0043
#define PS3MAPI_OPCODE_LOAD_PROC_MODULE				0x0044
#define PS3MAPI_OPCODE_UNLOAD_PROC_MODULE			0x0045
#define PS3MAPI_OPCODE_UNLOAD_VSH_PLUGIN			0x0046
#define PS3MAPI_OPCODE_GET_VSH_PLUGIN_INFO			0x0047
#define PS3MAPI_OPCODE_GET_PROC_MODULE_SEGMENTS		0x0048 // TheRouletteBoi
#define PS3MAPI_OPCODE_GET_VSH_PLUGIN_BY_NAME		0x004F

#define PS3MAPI_OPCODE_GET_IDPS 					0x0081
#define PS3MAPI_OPCODE_SET_IDPS 					0x0082
#define PS3MAPI_OPCODE_GET_PSID 					0x0083
#define PS3MAPI_OPCODE_SET_PSID						0x0084
#define PS3MAPI_OPCODE_CHECK_SYSCALL				0x0091
#define PS3MAPI_OPCODE_DISABLE_SYSCALL				0x0092
#define PS3MAPI_OPCODE_PDISABLE_SYSCALL8 			0x0093
#define PS3MAPI_OPCODE_PCHECK_SYSCALL8 				0x0094
#define PS3MAPI_OPCODE_RENABLE_SYSCALLS				0x0095
#define PS3MAPI_OPCODE_REMOVE_HOOK					0x0101

#define PS3MAPI_OPCODE_SUPPORT_SC8_PEEK_POKE		0x1000
//#define PS3MAPI_OPCODE_LV2_PEEK					0x1006
//#define PS3MAPI_OPCODE_LV2_POKE					0x1007
//#define PS3MAPI_OPCODE_LV1_PEEK					0x1008
//#define PS3MAPI_OPCODE_LV1_POKE					0x1009

#define PS3MAPI_OPCODE_SET_ACCESS_KEY				0x2000
#define PS3MAPI_OPCODE_REQUEST_ACCESS				0x2001

#define PS3MAPI_OPCODE_PHOTO_GUI					0x2222

#define PS3MAPI_FIND_FREE_SLOT						NULL

#define PS3MAPI_ENABLED								1	// R2+TRIANGLE - CFW syscalls partially disabled - keep syscall 8 (PS3MAPI enabled)
#define PS3MAPI_DISABLED							4	// R2+TRIANGLE - CFW syscalls fully disabled - remove syscall 8 (PS3MAPI disabled)

#define unload_vsh_plugin(a) ps3mapi_get_vsh_plugin_slot_by_name(a, true)
#define get_free_slot(a)	 ps3mapi_get_vsh_plugin_slot_by_name(PS3MAPI_FIND_FREE_SLOT, false)

///////////// PS3MAPI END //////////////

#define HOME_PS3MAPI	"<a href='/home.ps3mapi'>PS3MAPI</a> --> "

#if defined(REMOVE_SYSCALLS) || defined(PS3MAPI)
#define CFW_SYSCALLS 16
static u16 sc_disable[CFW_SYSCALLS] = {200, 201, 202, 203, 204, 1022, 6, 7, 10, 11, 15, 20, 35, 36, 38, 9};
#else
#define CFW_SYSCALLS 17
static u16 sc_disable[CFW_SYSCALLS] = {200, 201, 202, 203, 204, 1022, 6, 7, 10, 11, 15, 20, 35, 36, 38, 8, 9};
#endif

#ifdef COBRA_ONLY

#ifdef REMOVE_SYSCALLS
static void disable_signin_dialog(void);
static void enable_signin_dialog(void);
#endif

static void ps3mapi_get_vsh_plugin_info(unsigned int slot, char *tmp_name, char *tmp_filename)
{
	memset(tmp_name, 0, 30);
	memset(tmp_filename, 0, STD_PATH_LEN);
	system_call_5(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_VSH_PLUGIN_INFO, (u64)slot, (u64)(u32)tmp_name, (u64)(u32)tmp_filename);
}

static unsigned int ps3mapi_get_vsh_plugin_slot_by_name(const char *name, bool unload)
{
	char tmp_name[30];
	char tmp_filename[STD_PATH_LEN];

	bool find_free_slot = (!name || (*name == PS3MAPI_FIND_FREE_SLOT));

	unsigned int slot;
	for (slot = 1; slot < 7; slot++)
	{
		ps3mapi_get_vsh_plugin_info(slot, tmp_name, tmp_filename);

		if(find_free_slot) {if(*tmp_name) continue; break;} else
		if(IS(tmp_name, name) || strstr(tmp_filename, name)) {if(unload) cobra_unload_vsh_plugin(slot); break;}
	}
	return slot;
}

static void unload_vsh_gui(void)
{
	unload_vsh_plugin("VSH_MENU"); // unload vsh menu
	unload_vsh_plugin("sLaunch");  // unload sLaunch
}

static void start_vsh_gui(bool vsh_menu)
{
	unsigned int slot;
	slot = unload_vsh_plugin(vsh_menu ? "VSH_MENU" : "sLaunch");
	if(slot < 7) return; unload_vsh_gui();
	slot = get_free_slot(); char arg[2] = {1, 0};
	if(slot < 7) cobra_load_vsh_plugin(slot, vsh_menu ? WM_RES_PATH "/wm_vsh_menu.sprx" : WM_RES_PATH "/slaunch.sprx", (u8*)arg, 1);
}
#endif
///////////////////////////////

#ifdef PS3MAPI

static void ps3mapi_syscall8(char *buffer, char *templn, const char *param);
static void ps3mapi_setmem(char *buffer, char *templn, const char *param);

static u32 found_offset = 0;
static u8 ps3mapi_working = 0;

static int is_syscall_disabled(u32 sc)
{
	int ret_val = NONE;
	{ system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_CHECK_SYSCALL, sc); ret_val = (int)p1;}
	if(ret_val<0) {u64 sc_null = peekq(SYSCALL_TABLE); ret_val = (peekq(SYSCALL_PTR(sc)) == sc_null);}

	return ret_val;
}

static void add_sound_list(char *buffer, const char *param)
{
	add_option_item(1, "Simple", IS_MARKED("snd=1"), buffer);
	add_option_item(2, "Double", IS_MARKED("snd=2"), buffer);
	add_option_item(3, "Triple", IS_MARKED("snd=3"), buffer);
	add_option_item(0, "snd_cancel", IS_MARKED("snd=0"), buffer);
	add_option_item(4, "snd_cursor", IS_MARKED("snd=4"), buffer);
	add_option_item(5, "snd_trophy", IS_MARKED("snd=5"), buffer);
	add_option_item(6, "snd_decide", IS_MARKED("snd=6"), buffer);
	add_option_item(7, "snd_option", IS_MARKED("snd=7"), buffer);
	add_option_item(8, "snd_system_ok", IS_MARKED("snd=8"), buffer);
	add_option_item(9, "snd_system_ng", IS_MARKED("snd=9"), buffer);
}

static void ps3mapi_buzzer(char *buffer, char *templn, const char *param)
{
	bool is_ps3mapi_home = (*param == ' ');

	if(islike(param, "/buzzer.ps3mapi") && param[15] == '?')
	{
		u8 value = get_valuen(param, "snd=", 0, 9) | get_valuen(param, "mode=", 0, 9);
		play_sound_id(value);
	}
	else if(islike(param, "/beep.ps3"))
	{
		play_sound_id(param[10]);
	}

	if(!is_ps3mapi_home)
		sprintf(templn, "<b>%s%s</b>"
						HTML_BLU_SEPARATOR,
						HOME_PS3MAPI, "Buzzer");
	else
		sprintf(templn, "<td width=\"260\" class=\"la\"><u>%s:</u><br>", "Buzzer");
	concat(buffer, templn);

	sprintf(templn, "<form id=\"buzzer\" action=\"/buzzer%s<br>"
					"<b>%s:</b>  <select name=\"snd\">", HTML_FORM_METHOD, "Sound");
	concat(buffer, templn);

	add_sound_list(buffer, param);

	sprintf(templn, "</select>   <input type=\"submit\" value=\" %s \"/></td></form><br>", "Ring");

	if(!is_ps3mapi_home) strcat(templn, HTML_RED_SEPARATOR); else strcat(templn, "</td>");
	concat(buffer, templn);
}

static void ps3mapi_led(char *buffer, char *templn, const char *param)
{
	bool is_ps3mapi_home = (*param == ' ');

	if(islike(param, "/led.ps3mapi") && param[12] == '?')
	{
		int color = get_valuen(param, "color=", 0, 2);
		int mode  = get_valuen(param, "mode=", 0, 6);
		int mode2 = mode; if(mode >= 4) color = 2;
		if(mode == 4) {mode = 1, mode2 = 2;}
		if(mode == 5) {mode = 2, mode2 = 3;}
		if(mode == 6) {mode = 3, mode2 = 2;}

		if((color == 0) || (color == 2)) { system_call_2(SC_SYS_CONTROL_LED, RED, mode); }
		if((color == 1) || (color == 2)) { system_call_2(SC_SYS_CONTROL_LED, GREEN, mode2); }
	}

	if(!is_ps3mapi_home)
		sprintf(templn, "<b>%s%s</b>"
						HTML_BLU_SEPARATOR,
						HOME_PS3MAPI, "Led");
	else
		sprintf(templn, "<td width=\"260\" class=\"la\"><u>%s:</u><br>", "Led");

	concat(buffer, templn);

	sprintf(templn, "<form id=\"led\" action=\"/led%s<br>"
					"<b>%s:</b>  <select name=\"color\">", HTML_FORM_METHOD,  "Color"); concat(buffer, templn);

	add_option_item(0, "Red",				 IS_MARKED("color=0"), buffer);
	add_option_item(1, "Green",				 IS_MARKED("color=1"), buffer);
	add_option_item(2, "Yellow (Red+Green)", IS_MARKED("color=2"), buffer);

	sprintf(templn, "</select>   <b>%s:</b>  <select name=\"mode\">", "Mode"); concat(buffer, templn);

	add_option_item(0, "Off",		 IS_MARKED("mode=0"), buffer);
	add_option_item(1, "On",		 IS_MARKED("mode=1"), buffer);
	add_option_item(2, "Blink fast", IS_MARKED("mode=2"), buffer);
	add_option_item(3, "Blink slow", IS_MARKED("mode=3"), buffer);
	add_option_item(4, "Blink alt1", IS_MARKED("mode=4"), buffer);
	add_option_item(5, "Blink alt2", IS_MARKED("mode=5"), buffer);
	add_option_item(6, "Blink alt3", IS_MARKED("mode=6"), buffer);

	sprintf(templn, "</select>   <input type=\"submit\" value=\" %s \"/></form><br>", "Set");
	if(!is_ps3mapi_home) strcat(templn, HTML_RED_SEPARATOR); else strcat(templn, "</table></td>");
	concat(buffer, templn);
}

static void ps3mapi_notify(char *buffer, char *templn, const char *param)
{
	bool is_ps3mapi_home = (*param == ' ');

	u8 icon_id = (u8)get_valuen32(param, "&icon=");

	char *snd = strstr(param, "&snd=");

	if(snd && ISDIGIT(snd[5])) play_sound_id(snd[5]);

	char msg[200]; strcpy(msg, "Hello :)");
	if(get_param("?msg=", msg, param, 199))
	{
		if(icon_id) webman_config->msg_icon = 0; // enable icons
		vshNotify_WithIcon(icon_id, msg);
	}

	if(!is_ps3mapi_home)
		sprintf(templn, "<b>%s%s</b>"
						HTML_BLU_SEPARATOR,
						HOME_PS3MAPI, "Notify");
	else
		sprintf(templn, "<tr><td class=\"la\"><br><u>%s:</u><br><br>", "Notify");

	concat(buffer, templn);

	sprintf(templn, HTML_FORM_METHOD_FMT("/notify")
					"<table width=\"800\">"
					"<tr><td class=\"la\"><textarea name=\"msg\" cols=\"111\" rows=\"2\" maxlength=\"199\">%s</textarea>"
					"<br>Icon (0-50): " HTML_NUMBER("icon", "%i", "0", "50")
					" Sound: <select name=\"snd\"><option value=''>No Sound", HTML_FORM_METHOD, msg, icon_id);
	concat(buffer, templn);

	add_sound_list(buffer, param);

	sprintf(templn, "</td></tr>"
					"<tr><td class=\"ra\">"
					"<input class=\"bs\" type=\"submit\" value=\" %s \"/></td></tr>"
					"</table></form>", "Send");

	if(!is_ps3mapi_home) strcat(templn, HTML_RED_SEPARATOR); else strcat(templn, "</td>");
	concat(buffer, templn);
}

static bool add_sc_checkbox(int sc, const char *id, const char *label, char *buffer)
{
	bool disabled = is_syscall_disabled(sc);
	add_check_box(id, disabled, label, _BR_, (!disabled), buffer);
	return disabled;
}

static void ps3mapi_syscall(char *buffer, char *templn, const char *param)
{
	bool is_ps3mapi_home = (*param == ' ');

	u64 sc_null = peekq(SYSCALL_TABLE);

	if(strstr(param, ".ps3mapi?"))
	{
		for(u8 sc = 0; sc < CFW_SYSCALLS; sc++)
		{
			sprintf(templn, "sc%i=1", sc_disable[sc]);
			if(IS_MARKED(templn))
			{
				if(sc_disable[sc] == SC_PEEK_LV2) {peekq = lv2_peek_ps3mapi;}
				if(sc_disable[sc] == SC_POKE_LV2) {pokeq = lv2_poke_ps3mapi, lv2_poke_fan = (payload_ps3hen) ? lv2_poke_fan_hen : lv2_poke_ps3mapi;}
				if(sc_disable[sc] == SC_POKE_LV1) {poke_lv1 = lv1_poke_ps3mapi;}

				pokeq(SYSCALL_PTR(sc_disable[sc]), sc_null);
				system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_DISABLE_SYSCALL, (u64)sc_disable[sc]);
			}
		}

#ifdef REMOVE_SYSCALLS
		if(IS_MARKED("sce=1"))  { restore_cfw_syscalls(); } else
		if(IS_MARKED("scd=1"))  { remove_cfw_syscalls(webman_config->keep_ccapi); }
#endif
	}

	if(!is_ps3mapi_home)
		sprintf(templn, "<b>%s%s</b>"
						HTML_BLU_SEPARATOR
						"<table width=\"800\">", HOME_PS3MAPI, "CFW syscall");
	else
		sprintf(templn, "<table width=\"800\">"
						"<tr><td class=\"la\"><u>%s:</u><br><br></td></tr>", "CFW syscall");

	concat(buffer, templn);

	sprintf(templn, "<form id=\"syscall\" action=\"/syscall%s"
					"<br><tr><td width=\"260\" class=\"la\">",
					HTML_FORM_METHOD); concat(buffer, templn);

	u8 sc_count = 0;

	if(add_sc_checkbox(6, "sc6", "[6]LV2 Peek", buffer)) sc_count++;
	if(add_sc_checkbox(7, "sc7", "[7]LV2 Poke", buffer)) sc_count++;
	if(add_sc_checkbox(9, "sc9", "[9]LV1 Poke", buffer)) sc_count++;
	add_sc_checkbox(10, "sc10", "[10]LV1 Call", buffer);
	add_sc_checkbox(15, "sc15", "[15]LV2 Call", buffer);
	add_sc_checkbox(11, "sc11", "[11]LV1 Peek", buffer);

	concat(buffer, "</td><td  width=\"260\"  valign=\"top\" class=\"la\">");

	add_sc_checkbox(35, "sc35", "[35]Map Path", buffer);
	add_sc_checkbox(36, "sc36", "[36]Map Game", buffer);
	add_sc_checkbox(38, "sc38", "[38]New sk1e", buffer);
	add_sc_checkbox(1022, "sc1022", "[1022]PRX Loader", buffer);

	concat(buffer, "</td><td  width=\"260\"  valign=\"top\" class=\"la\">");

	add_sc_checkbox(200, "sc200", "[200]sys_dbg_read_process_memory", buffer);
	add_sc_checkbox(201, "sc201", "[201]sys_dbg_write_process_memory", buffer);
	add_sc_checkbox(202, "sc202", "[202]Free - Payloader3", buffer);
	add_sc_checkbox(203, "sc203", "[203]LV2 Peek CCAPI", buffer);
	add_sc_checkbox(204, "sc204", "[204]LV2 Poke CCAPI", buffer);

#ifdef REMOVE_SYSCALLS
	concat(buffer, "<br>");
	if(sc_count)  add_checkbox("sce\" onclick=\"b.value=(this.checked)?' Enable ':'Disable';", "Re-Enable Syscalls & Unlock syscall 8", _BR_, false, buffer);
	else		 _add_checkbox("scd", "Disable Syscalls & Lock syscall 8", false, buffer);
#endif

	sprintf(templn, "</td></tr><tr><td class=\"ra\"><br><input class=\"bs\" id=\"b\" type=\"submit\" value=\" %s \"/></td></tr></form></table><br>", "Disable");
	concat(buffer, templn);

	if(!is_ps3mapi_home && islike(param, "/syscall.ps3mapi")) {ps3mapi_syscall8(buffer, templn, param);}
}

static void ps3mapi_syscall8(char *buffer, char *templn, const char *param)
{
	bool is_ps3mapi_home = (*param == ' ');
	int ret_val = NONE;
	int disable_cobra = 0, disabled_cobra = 0;

	if(strstr(param, ".ps3mapi?"))
	{
		u8 mode = get_valuen(param, "mode=", 0, 5);

		#ifdef REMOVE_SYSCALLS
		if(mode) enable_signin_dialog();
		#endif

		{ system_call_2(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_DISABLE_COBRA, (mode == 5)); }

		if( mode <= 3 ) { system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_PDISABLE_SYSCALL8, (u64)mode); }
		if( mode == 4 ) { system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_DISABLE_SYSCALL, SC_COBRA_SYSCALL8); }
		if( mode == 5 )	{ system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_PDISABLE_SYSCALL8, 3); }

		webman_config->sc8mode = (mode == 4) ? PS3MAPI_DISABLED : PS3MAPI_ENABLED; // fully disabled : Partially disabled
	}

	if(!is_ps3mapi_home && islike(param, "/syscall8.ps3mapi")) {ps3mapi_syscall(buffer, templn, param);}

	{ system_call_2(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_DISABLE_COBRA, SYSCALL8_DISABLE_COBRA_CAPABILITY); disable_cobra = (int)p1;}
	if(disable_cobra == SYSCALL8_DISABLE_COBRA_OK)
	{
		{ system_call_2(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_DISABLE_COBRA, SYSCALL8_DISABLE_COBRA_STATUS); disabled_cobra = (int)p1;}
		if(disabled_cobra) ret_val = 3;
	}

	sprintf(templn, "<b>%s%s</b>"
					HTML_BLU_SEPARATOR
					"<table width=\"800\">"
					"<form id=\"syscall8\" action=\"/syscall8%s"
					"<br><tr><td class=\"la\">",
					is_ps3mapi_home ? "" : HOME_PS3MAPI, "CFW syscall 8", HTML_FORM_METHOD); concat(buffer, templn);

	{ system_call_2(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_PCHECK_SYSCALL8); ret_val = (int)p1;}

	syscalls_removed = (ret_val != 0); peek_lv1 = (syscalls_removed) ? lv1_peek_ps3mapi : lv1_peek_cfw;
	if(!syscalls_removed) disable_signin_dialog();

	#ifdef REMOVE_SYSCALLS
	if(!syscalls_removed) disable_signin_dialog();
	#endif

	if(ret_val < 0)
	{
		add_radio_button("mode\" disabled", 0, "sc8_0", "Fully enabled", _BR_, false, buffer);
		add_radio_button("mode\" disabled", 1, "sc8_1", "Partially disabled : Keep only COBRA/MAMBA/PS3MAPI features", _BR_, false, buffer);
		add_radio_button("mode\" disabled", 2, "sc8_2", "Partially disabled : Keep only PS3MAPI features", _BR_, false, buffer);
		add_radio_button("mode\" disabled", 3, "sc8_3", "Fake disabled (can be re-enabled)", _BR_, false, buffer);
	}
	else
	{
		if(syscalls_removed && (ret_val == 0)) ret_val = 1; if(!c_firmware) ret_val = 4;

		add_radio_button("mode", 0, "sc8_0", "Fully enabled", _BR_, (ret_val == 0), buffer);
		add_radio_button("mode", 1, "sc8_1", "Partially disabled : Keep only COBRA/MAMBA/PS3MAPI features", _BR_, (ret_val == 1), buffer);
		add_radio_button("mode", 2, "sc8_2", "Partially disabled : Keep only PS3MAPI features", _BR_, (ret_val == 2), buffer);

		if(disable_cobra == SYSCALL8_DISABLE_COBRA_OK)
			add_radio_button("mode", 5, "sc8_5", "Disable COBRA/MAMBA/PS3MAPI features / keep lv1_peek (can be re-enabled)", _BR_, disabled_cobra, buffer);

		add_radio_button("mode", 3, "sc8_3", "Fake disabled (can be re-enabled)", _BR_, (ret_val == 3 && !disabled_cobra), buffer);
	}

	if(ret_val < 0 || ret_val == 3)
		add_radio_button("mode\" disabled=\"disabled", 4, "sc8_4", "Fully disabled (can't be re-enabled)", _BR_, (ret_val < 0), buffer);
	else
		add_radio_button("mode", 4, "sc8_4", "Fully disabled (can't be re-enabled)", _BR_, false, buffer);

	sprintf(templn, "</td></tr><tr><td class=\"ra\"><br><input class=\"bs\" type=\"submit\" value=\" %s \"/></td></tr></form></table><br>", "Set");

	if(!is_ps3mapi_home) strcat(templn, HTML_RED_SEPARATOR);
	concat(buffer, templn);
}

static void ps3mapi_get_process_name_by_id(u32 pid, char *name, u16 size)
{
	memset(name, 0, size);
	system_call_4(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_PROC_NAME_BY_PID, (u64)pid, (u64)(u32)name);
}

static u32 get_current_pid(void)
{
	if(IS_INGAME)
		return GetGameProcessID();

	u32 pid_list[16];
	{system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_ALL_PROC_PID, (u64)(u32)pid_list); }

	for(int i = 0; i < 16; i++)
	{
		if(pid_list[i] > 2)
		{
			return pid_list[i];
		}
	}
	return 0;
}

static u8 add_proc_list(char *buffer, char *templn, u32 *proc_id, u8 src)
{
	u8 is_vsh = 0;
	u32 pid = *proc_id;

	if(pid == 0)
	{
		concat(buffer, "<select name=\"proc\">");
		u32 pid_list[16];
		{system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_ALL_PROC_PID, (u64)(u32)pid_list); }
		for(int i = 0; i < 16; i++)
		{
			if(1 < pid_list[i])
			{
				ps3mapi_get_process_name_by_id(pid_list[i], templn, MAX_LINE_LEN);
				if(*templn) add_option_item(pid_list[i], templn, (i == 0), buffer);
			}
		}
		if(src < 3)
		{
			if(!payload_ps3hen)
				add_option_item(LV1, "LV1 Memory", false, buffer);
				add_option_item(LV2, "LV2 Memory", false, buffer);
			if(src == 1)
			{
				add_option_item(FLASH, "Flash", false, buffer);
				add_option_item(HDD0,  drives[0], false, buffer);
				add_option_item(USB0, drives[1], false, buffer);
				add_option_item(USB1, drives[2], false, buffer);
			}
		}
		concat(buffer, "</select> ");
	}
	else
	{
		sprintf(templn, "<a href=\"%s?proc=%i\">", (src == 3) ? "/getmem.ps3mapi" : "/gameplugin.ps3mapi", pid); concat(buffer, templn);

		if(pid == LV1)
			strcpy(templn, "LV1 Memory");
		else if(pid == LV2)
			strcpy(templn, "LV2 Memory");
		else if(pid == FLASH)
			strcpy(templn, "Flash");
		else if(pid == HDD0)
			strcpy(templn, drives[0]);
		else if(pid >= USB0 && pid < PID)
			strcpy(templn, drives[pid - USB0 + 1]);
		else
			ps3mapi_get_process_name_by_id(pid, templn, MAX_LINE_LEN);

		concat(buffer, templn); concat(buffer, "</a>");

		is_vsh = (strstr(templn, "_main_vsh.self") != NULL);

		if(pid >= PID)
		{
			if(IS_INGAME)
			{
				sprintf(templn, HTML_BUTTON_FMT, HTML_BUTTON, "Exit", HTML_ONCLICK, "/xmb.ps3$exit");
				concat(buffer, templn);

				sprintf(templn, HTML_BUTTON_FMT2, HTML_BUTTON, "Reload", HTML_ONCLICK2, "/xmb.ps3$reloadgame", HTML_SEND_CMD);
				concat(buffer, templn);
			}

			sprintf(templn, HTML_BUTTON_FMT2, HTML_BUTTON, "Pause", HTML_ONCLICK2, "/xmb.ps3$rsx_pause", HTML_SEND_CMD);
			concat(buffer, templn);

			sprintf(templn, HTML_BUTTON_FMT2, HTML_BUTTON, "Continue", HTML_ONCLICK2, "/xmb.ps3$rsx_continue", HTML_SEND_CMD);
			concat(buffer, templn);
		}

		sprintf(templn, "<input name=\"proc\" type=\"hidden\" value=\"%u\"><br><br>", pid);
		concat(buffer, templn);

		add_game_info(buffer, templn, src);

		*proc_id = pid;
	}

	return is_vsh;
}

static u32 ps3mapi_find_offset(u32 pid, u32 address, u32 stop, u8 step, const char *sfind, u8 len, const char *mask, u32 fallback)
{
	int retval = NONE;
	found_offset = fallback;

	char mem[0x200]; int m = sizeof(mem) - len; u8 gap = len + 0x10 - (len % 0x10);
	for(; address < stop; address += sizeof(mem) - gap)
	{
		retval = ps3mapi_get_memory(pid, address, mem, sizeof(mem));
		if(retval < 0) break;

		for(int offset = 0; offset < m; offset += step)
		{
			if( !bcompare(mem + offset, sfind, len, mask) )
			{
				found_offset = (address + offset);
				return found_offset;
			}
		}
	}
	return found_offset;
}

static int ps3mapi_patch_process(u32 pid, u32 address, const char *new_value, int size)
{
	if(pid == LV1)
	{
		poke_chunk_lv1(address, size, (u8*)new_value);
	}
	else if(pid == LV2)
	{
		poke_chunk_lv2(address, size, (u8*)new_value);
	}
	else
	{
		system_call_6(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_SET_PROC_MEM, (u64)pid, (u64)address, (u64)(u32)new_value, (u64)size);
		return (int)p1;
	}
	return 0;
}

#define BINDATA_SIZE	0x100
#define HEXDATA_SIZE	0x200
#define BINDATA_LEN		"256"
#define HEXDATA_LEN		"512"

static void ps3mapi_getmem(char *buffer, char *templn, const char *param)
{
	u32 pid = get_valuen32(param, "proc=");
	u32 address = 0;
	int length = BINDATA_SIZE;
	found_offset = 0;
	int hilite = 0;

	bool is_read_only = (pid >= FLASH) && (pid < PID);
	bool is_ps3mapi_home = (*param == ' ');
	bool not_found = false;

	char *find = (char*)"";
	const char *dump_file = (char*)"/dev_hdd0/mem_dump.bin";
	const char *dump_size = (char*)"&dump=400000";

	if(strstr(param, ".ps3mapi?"))
	{
		char addr_tmp[0x60];
		if(get_param("addr=", addr_tmp, param, 0x10))
		{
			address = convertH(addr_tmp);

			length = (int)get_valuen32(param, "len=");
			if(length == 0) length = BINDATA_SIZE;
			length = RANGE(length, 1, BINDATA_SIZE);
		}

		if(!pid) pid = get_current_pid();

		if(pid >= PID) address = MAX(address, 0x10000);

		if(get_param("find=", addr_tmp, param, 0x60))
		{
			find = strstr(param, "find=") + 5;
			char sfind[0x60], *mask = addr_tmp;
			u8 len = snprintf(sfind, 0x60, "%s", addr_tmp);

			// search hex: 0xAABBCC112233
			if(isHEX(addr_tmp))
			{
				len = Hex2Bin(addr_tmp, sfind);
				for(u8 i = 0, n = 0; i < len; i++, n+=2) mask[i] = addr_tmp[n]; sfind[len] = mask[len] = 0;
			}
			else if(strstr(param, "&exact"))
				memset(mask, 0, len);

			if(address == 0) address = 4;

			u32 addr = address;
			u32 stop = 0; char *pos = strstr(param, "&stop=");
			if(pos) stop = convertH(pos + 6); if(stop < address) stop = (address + 0x1000000ULL);
			u8  step = get_valuen(param, "step=", 0, 0xE0); if(step < 1) step = 4;
			u8  rep  = get_valuen(param, "rep=", 1, 0xFF);

			address += step;

			while(rep--)
			{
				address = ps3mapi_find_offset(pid, address, stop, step, sfind, len, mask, addr);
				if(rep)
				{
					if(address == addr) break;
					address += step;
				}
			}
			if(address == addr)
				not_found = true;
			else
				hilite = len;
		}

		if(pid == FLASH)
		{
			if(!address)
				dump_size = (char*)"&dump=1000000";
			dump_file = (char*)"/dev_hdd0/dump_flash.bin";
		}

		if(!not_found) // ignore dump / patch_process if find returned not found
		{
			int offset = (int)get_valuen64(param, "&offset="); address += offset;

			if(get_param("dump=", addr_tmp, param, 16))
			{
				u32 size = convertH(addr_tmp);
				if(size <= 256) size *= _1MB_;
				if(size >= _64KB_) ps3mapi_dump_process(dump_file, pid, address, size);
			}

			if(strstr(param, "&val="))
			{
				char value[BINDATA_SIZE + 1];
				char val_tmp[HEXDATA_SIZE + 1];
				char *new_value = val_tmp;

				ps3mapi_get_memory(pid, address, value, BINDATA_SIZE);

				hilite = length = get_param("val=", new_value, param, HEXDATA_SIZE);
				if(isHEX(val_tmp))
					{hilite = length = Hex2Bin(val_tmp, value); new_value = (char*)value;}

				if(length) {ps3mapi_patch_process(pid, address, new_value, length);}
				length = BINDATA_SIZE;
			}
		}
	}

	if(!is_ps3mapi_home)
		sprintf(templn, "<b>%s%s</b>"
						HTML_BLU_SEPARATOR,
						HOME_PS3MAPI, "Get process memory");
	else
		sprintf(templn, "<b>%s</b>"
						HTML_BLU_SEPARATOR,
						"Processes Commands");

	concat(buffer, templn);

	sprintf(templn, HTML_FORM_METHOD_FMT("/getmem")
					"<b><u>%s:</u></b>  ", HTML_FORM_METHOD, "Process"); concat(buffer, templn); memset(templn, 0, MAX_LINE_LEN);

	add_proc_list(buffer, templn, &pid, 1);

	sprintf(templn, "<b><u>%s:</u></b> " HTML_INPUT("addr", "%X", "16", "18")
					" <b><u>%s:</u></b> <input name=\"len\" type=\"number\" value=\"%i\" min=\"1\" max=\"" BINDATA_LEN "\">"
					" <input class=\"bs\" type=\"submit\" value=\" %s \"/></form>", "Address", address, "Length", length, "Get");
	concat(buffer, templn);

	if((pid != 0) && (length > 0))
	{
		sprintf(templn, "<br><b><u>%s:</u></b>", "Output");
		concat(buffer, templn);

		sprintf(templn, " <a id=\"pblk\" href=\"/getmem.ps3mapi?proc=%u&addr=%x\">&lt;&lt;</a> <a id=\"back\" href=\"/getmem.ps3mapi?proc=%u&addr=%x\">&lt;Back</a>", pid, address - 0x2000, pid, address - BINDATA_SIZE); buffer += concat(buffer, templn);
		sprintf(templn, " <a id=\"next\" href=\"/getmem.ps3mapi?proc=%u&addr=%x\">Next&gt;</a> <a id=\"nblk\" href=\"/getmem.ps3mapi?proc=%u&addr=%x\">&gt;&gt;</a>", pid, address + BINDATA_SIZE, pid, address + 0x2000); buffer += concat(buffer, templn);

		if(file_exists(dump_file)) {add_breadcrumb_trail2(buffer, " [", dump_file); concat(buffer, " ]");}

		if(!strstr(param, "dump=")) sprintf(templn, " [<a href=\"%s%s\">%s</a>]", param, dump_size, "Dump Process");
		concat(buffer, templn);

		char *pos = strstr(param, "&addr="); if(pos) *pos = 0;
		sprintf(templn, " [<a href=\"javascript:void(location.href='%s&addr=%x&find='+prompt('%s','%s').toString());\">%s</a>] %s%s%s", param, address, "Find", find, "Find", "<font color=#ff0>", not_found ? "Not found!" : "", "</font><hr>");
		concat(buffer, templn); if(pos) *pos = '&';
		char buffer_tmp[length + 1];
		memset(buffer_tmp, 0, sizeof(buffer_tmp));
		int retval = NONE;
		retval = ps3mapi_get_memory(pid, address, buffer_tmp, length);
		if(0 <= retval)
		{
			// show hex dump
			u8 byte = 0, p = 0;
			u16 num_bytes = MIN(0x200, ((u16)((length + 15) / 0x10) * 0x10));
			for(u16 i = 0, n = 0; i < num_bytes; i++)
			{
				if(!p)
				{
					sprintf(templn, "%08X  ", (int)((address & 0xFFFFFFFFULL) + i));
					buffer += concat(buffer, templn);
				}

				if(i >= length) buffer += concat(buffer, "&nbsp;&nbsp; ");
				else
				{
					if(hilite && (p == 0))
						 buffer += concat(buffer, "<font color=#ff0>");

					byte = (u8)buffer_tmp[i];

					sprintf(templn, "%02X ", byte); buffer += concat(buffer, templn);

					if(hilite && ((hilite == (p + 1)) || (p == 0xF)))
						 buffer += concat(buffer, "</font>");
				}

				if(p == 0xF)
				{
					buffer += concat(buffer, " ");
					if(hilite)
						 buffer += concat(buffer, "<font color=#ff0>");
					for(u8 c = 0; c < 0x10; c++, n++)
					{
						if(n >= length) break;
						byte = (u8)buffer_tmp[n];
						if(byte<32 || byte>=127) byte='.';

						if(byte==0x3C)
							buffer += concat(buffer, "&lt;");
						else if(byte==0x3E)
							buffer += concat(buffer, "&gt;");
						else
							{sprintf(templn,"%c", byte); buffer += concat(buffer, templn);}
						if(hilite)
						{
							hilite--; if(!hilite) buffer += concat(buffer, "</font>");
						}
					}
					if(hilite) buffer += concat(buffer, "</font>");
					buffer += concat(buffer, "<br>");
				}

				p++; if(p >= 0x10) p=0;
			}
			//

			// add navigation with left/right keys
			concat(buffer,  "<script>"
							"document.addEventListener('keydown',kd,false);"
							"function kd(e){"
							"if(typeof document.activeElement.name!='undefined')return;"
							"e=e||window.event;var kc=e.keyCode;"
							"if(kc==37){e.ctrlKey?pblk.click():back.click();}"
							"if(kc==39){e.ctrlKey?nblk.click():next.click();}}"
							"</script>");

			concat(buffer, "<textarea id=\"output\" style=\"display:none\">");

			for(int i = 0; i < length; i++)
			{
				sprintf(templn, "%02X", (u8)buffer_tmp[i]);
				concat(buffer, templn);
			}
		}
		else {sprintf(templn, "%s: %i", "Error", retval); concat(buffer, templn);}
		concat(buffer, "</textarea>");
	}

	concat(buffer, "<br>");

#ifdef DEBUG_MEM
	concat(buffer, "Dump: [<a href=\"/dump.ps3?mem\">Full Memory</a>] [<a href=\"/dump.ps3?flash\">Flash</a>] [<a href=\"/dump.ps3?rsx\">RSX</a>] [<a href=\"/dump.ps3?vsh\">VSH</a>] [<a href=\"/dump.ps3?lv1\">LV1</a>] [<a href=\"/dump.ps3?lv2\">LV2</a>]");
	sprintf(templn, " [<a href=\"/dump.ps3?%x\">LV1 Dump 0x%x</a>] [<a href=\"/peek.lv1?%x\">LV1 Peek</a>] [<a href=\"/peek.lv2?%x\">LV2 Peek</a>]", address, address, address, address); concat(buffer, templn);
#endif
	concat(buffer, "<p>");

	if(is_read_only) return;

	if(!is_ps3mapi_home && islike(param, "/getmem.ps3mapi")) ps3mapi_setmem(buffer, templn, param);
}

static void ps3mapi_setmem(char *buffer, char *templn, const char *param)
{
	bool is_ps3mapi_home = (*param == ' ');

	u32 pid = 0;
	u32 address = found_offset;
	int length = 0;
	char value[BINDATA_SIZE + 1];
	char val_tmp[HEXDATA_SIZE + 1];

	if(strstr(param, ".ps3mapi?"))
	{
		char addr_tmp[17];
		if(get_param("addr=", addr_tmp, param, 16))
		{
			address = (u32)convertH(addr_tmp);

			if(get_param("val=", val_tmp, param, HEXDATA_SIZE))
			{
				if(isHEX(val_tmp))
					length = Hex2Bin(val_tmp, value);
				else
					length = sprintf(value, "%s", val_tmp);
			}
		}
		else
			pid = get_current_pid();

		if(!pid) pid = get_valuen32(param, "proc=");
	}

	if(found_offset) address = found_offset; found_offset = 0;

	if(!is_ps3mapi_home && islike(param, "/setmem.ps3mapi")) ps3mapi_getmem(buffer, templn, param);

	if(!is_ps3mapi_home)
		sprintf(templn, "<b>%s%s</b>"
						HTML_BLU_SEPARATOR,
						HOME_PS3MAPI, "Set process memory");
	else
		sprintf(templn, "<u>%s:</u>", "Set process memory");

	concat(buffer, templn);

	sprintf(templn, HTML_FORM_METHOD_FMT("/setmem")
					"<b><u>%s:</u></b>  ", HTML_FORM_METHOD, "Process"); concat(buffer, templn); memset(templn, 0, MAX_LINE_LEN);

	add_proc_list(buffer, templn, &pid, 2);

	if(*val_tmp == 0) sprintf(val_tmp, "00");

	sprintf(templn, "<b><u>%s:</u></b> "  HTML_INPUT("addr", "%X", "16", "18")
					"<br><br><b><u>%s:</u></b><br>"
					"<table width=\"800\">"
					"<tr><td class=\"la\">"
					"<textarea accesskey=\"v\" id=\"val\" name=\"val\" cols=\"103\" rows=\"5\" maxlength=\"" HEXDATA_LEN "\">%s</textarea></td></tr>"
					"<tr><td class=\"ra\"><br>"
					"<input class=\"bs\" type=\"submit\" accesskey=\"s\" value=\" %s \"/></td></tr></table></form>", "Address", address, "Value", val_tmp, "Set");
	concat(buffer, templn);

	if((pid != 0) && (length > 0))
	{
		int retval = ps3mapi_patch_process(pid, address, value, length);
		if(retval < 0)
			sprintf(templn, "<br><b><u>%s: %i</u></b>", "Error", retval);
		else
			sprintf(templn, "<br><b><u>%s!</u></b>", "Done");
		concat(buffer, templn);
	}

	if(length == 0) concat(buffer, "<script>val.value=output.value</script>");

	if(!is_ps3mapi_home) concat(buffer, "<br>" HTML_RED_SEPARATOR); else concat(buffer, "<br>");
}

static void ps3mapi_setidps(char *buffer, char *templn, const char *param)
{
	bool is_ps3mapi_home = (*param == ' ');

	//{system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_IDPS, (u64)IDPS);}
	//{system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_PSID, (u64)PSID);}

	u64 _new_IDPS[2] = { IDPS[0], IDPS[1]};
	u64 _new_PSID[2] = { PSID[0], PSID[1]};

	if(islike(param, "/setidps.ps3mapi") && param[16] == '?')
	{
		char tmp_value[17];
		if(get_param("idps1=", tmp_value, param, 16))
		{
			_new_IDPS[0] = convertH(tmp_value);

			if(get_param("idps2=", tmp_value, param, 16))
			{
				_new_IDPS[1] = convertH(tmp_value);

				{system_call_4(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_SET_IDPS, (u64)_new_IDPS[0], (u64)_new_IDPS[1]);}
			}
		}

		if(get_param("psid1=", tmp_value, param, 16))
		{
			_new_PSID[0] = convertH(tmp_value);

			if(get_param("psid2=", tmp_value, param, 16))
			{
				_new_PSID[1] = convertH(tmp_value);

				{system_call_4(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_SET_PSID, (u64)_new_PSID[0], (u64)_new_PSID[1]);}
			}
		}
	}

	get_idps_psid();

	sprintf(templn, "<b>%s%s</b>"
					HTML_BLU_SEPARATOR
					HTML_FORM_METHOD_FMT("/setidps")
					"<table id='ht' width=\"800\">"
					"<tr><td width=\"400\" class=\"la\">"
					"<br><b><u>%s:</u></b><br>" HTML_INPUT("idps1", "%016llX", "16", "18") HTML_INPUT("idps2", "%016llX", "16", "18") "</td>"
					"<td class=\"la\">"
					"<br><b><u>%s:</u></b><br>" HTML_INPUT("psid1", "%016llX", "16", "18") HTML_INPUT("psid2", "%016llX", "16", "18") "</td></tr>"
					"<tr><td class=\"ra\"><br><button class=\"bs\">%s</button>",
					is_ps3mapi_home ? "" : HOME_PS3MAPI, "Set IDPS/PSID",
					HTML_FORM_METHOD, "IDPS", _new_IDPS[0], _new_IDPS[1], "PSID", _new_PSID[0], _new_PSID[1], "Set");

	concat(buffer, templn);

	concat(buffer,	"<style>.ht{-webkit-text-security:disc}</style>"
					"<script>var t='th';function h(){var e=document.getElementById('ht').getElementsByTagName('INPUT');t=t.split('').reverse().join('');for(var n=0;n<e.length;n++)e[n].className=t;}h();</script>"
					" <button onclick='h();return false;'>&#x1F453;</button>"
					"</td></tr>"
					"</table></form><br>");

	if(!is_ps3mapi_home) concat(buffer,	HTML_RED_SEPARATOR);
}

static void add_plugins_list(char *buffer, char *templn, u8 is_vsh)
{
	if(!strstr(buffer, "<datalist id=\"plugins\">"))
	{
		concat(buffer, "<div style=\"display:none\"><datalist id=\"plugins\">");
		int fd, cnt = 0;

		const char *vsh_modules[3] = {
									"/dev_flash/vsh/module",
									"/dev_flash/sys/internal",
									"/dev_flash/sys/external"
								};

		const char *paths[5] = {
									"/dev_hdd0/plugins",
									"/dev_hdd0/plugins/ps3xpad",
									"/dev_hdd0/plugins/ps3_menu",
									"/dev_hdd0/plugins/PS3Lock",
									WM_RES_PATH
								};

		#define PLUGINS_PATH	is_vsh ? vsh_modules[i] : (i < 3) ? drives[i] : paths[i - 3]

		u8 count = is_vsh ? 3 : 8;

		for(u8 i = 0; i < count; i++)
		if(cellFsOpendir(PLUGINS_PATH, &fd) == CELL_FS_SUCCEEDED)
		{
			CellFsDirectoryEntry dir; u32 read_e;
			char *entry_name = dir.entry_name.d_name;

			u16 plen = sprintf(templn, "<option>%s/", PLUGINS_PATH);

			while(working && (!cellFsGetDirectoryEntries(fd, &dir, sizeof(dir), &read_e) && read_e))
			{
				if(!extcmp(entry_name, ".sprx", 5))
				{
					sprintf(templn + plen, "%s</option>", entry_name); buffer += concat(buffer, templn); if(++cnt > 450) break;
				}
			}
			cellFsClosedir(fd);
		}

		concat(buffer, "</datalist></div>");
	}
}

static void ps3mapi_vshplugin(char *buffer, char *templn, const char *param)
{
	bool is_ps3mapi_home = (*param == ' ');

	char tmp_name[30];
	char tmp_filename[STD_PATH_LEN];

	if(islike(param, "/vshplugin.ps3mapi") && param[18] == '?')
	{
		unsigned int uslot = 99;

		char *pos = strstr(param + 18, "?s=");
		if(pos)
		{
			u8 boot_mode = get_valuen(pos, "?s=", 0, 4);
			sprintf(tmp_filename, "/dev_hdd0/boot_plugins.txt"); if(IS_DEX) sprintf(tmp_filename + 22, "_dex.txt");
			switch (boot_mode)
			{
				case 1: sprintf(tmp_filename + 10, "mamba_plugins.txt"); break;
				case 2: sprintf(tmp_filename + 10, "prx_plugins.txt");   break;
				case 3: sprintf(tmp_filename + 10, "game/PRXLOADER/USRDIR/plugins.txt"); break;
				case 4: sprintf(tmp_filename + 22, "_nocobra.txt"); if(IS_DEX) sprintf(tmp_filename + 30, "_dex.txt"); break;
			}

			sprintf(templn, "<p><a href=\"%s\" style=\"padding:8px;background:#900;border-radius:8px;\">%s</a><p>", tmp_filename, tmp_filename); concat(buffer, templn);

			save_file(tmp_filename, "", SAVE_ALL);
			for (unsigned int slot = 1; slot < 7; slot++)
			{
				ps3mapi_get_vsh_plugin_info(slot, tmp_name, templn);

				if(*templn)
				{
					save_file(tmp_filename, templn, APPEND_TEXT);
				}
			}
		}
		else
		{
			uslot = get_valuen(param, "load_slot=", 0, 6);

			if(strstr(param, "unload_slot="))
			{
				if ( uslot ) {system_call_2(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_UNLOAD_VSH_PLUGIN, (u64)uslot);}
			}
			else
			{
				char prx_path[STD_PATH_LEN];
				if(get_param("prx=", prx_path, param, STD_PATH_LEN))
				{
					if (!uslot ) uslot = get_free_slot(); // find free slot if slot == 0

					check_path_alias(prx_path);
					if ( uslot ) {{system_call_5(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_LOAD_VSH_PLUGIN, (u64)uslot, (u64)(u32)prx_path, NULL, 0);}}
				}
			}
		}
	}

	sprintf(templn, "<b>%s%s</b>"
					HTML_BLU_SEPARATOR "<br>"
					"<table>"
					"<tr><td width=\"75\" class=\"la\">%s</td>"
					"<td width=\"120\" class=\"la\">%s</td>"
					"<td width=\"500\" class=\"la\">%s</td>"
					"<td width=\"125\" class=\"ra\"> </td></tr>",
					is_ps3mapi_home ? "" : HOME_PS3MAPI, "VSH Plugins", "Slot", "Name", "File name");

	buffer += concat(buffer, templn);
	for (unsigned int slot = 0; slot < 7; slot++)
	{
		ps3mapi_get_vsh_plugin_info(slot, tmp_name, tmp_filename);

		if(*tmp_filename)
		{
			sprintf(templn, "<tr><td width=\"75\" class=\"la\">%i</td>"
							"<td width=\"120\" class=\"la\">%s</td>"
							"<td width=\"500\" class=\"la\">",
							slot, tmp_name); buffer += concat(buffer, templn);

			buffer += add_breadcrumb_trail(buffer, tmp_filename);

			sprintf(templn, "</td>"
							"<td width=\"100\" class=\"ra\">"
							HTML_FORM_METHOD_FMT("/vshplugin")
							"<input name=\"unload_slot\" type=\"hidden\" value=\"%i\"><input type=\"submit\" %s/></form></td></tr>",
							HTML_FORM_METHOD, slot, (slot) ? "value=\" Unload \"" : "value=\" Reserved \" disabled" );
		}
		else
 		{
			sprintf(templn, "<tr><td width=\"75\" class=\"la\">%i</td>"
							"<td width=\"120\" class=\"la\">%s</td>"
							HTML_FORM_METHOD_FMT("/vshplugin")
							"<td width=\"500\" class=\"la\">"
							HTML_INPUT("prx\" style=\"width:555px\" list=\"plugins", "", "128", "75") "<input name=\"load_slot\" type=\"hidden\" value=\"%i\"></td>"
							"<td width=\"100\" class=\"ra\"><input type=\"submit\" %s/></td></form></tr>",
							slot, "NULL",
							HTML_FORM_METHOD, slot, (slot) ? "value=\" Load \"" : "value=\" Reserved \" disabled" );
		}
			buffer += concat(buffer, templn);
	}

	sprintf(templn, "<tr><td colspan=4><p>%s > "	HTML_BUTTON_FMT
													HTML_BUTTON_FMT
													HTML_BUTTON_FMT
													HTML_BUTTON_FMT
													HTML_BUTTON_FMT "</td></tr>", STR_SAVE,
		HTML_BUTTON, dex_mode ?
					"boot_plugins_dex.txt" :
					"boot_plugins.txt",				HTML_ONCLICK, "/vshplugin.ps3mapi?s=0",
		HTML_BUTTON, dex_mode ?
					"boot_plugins_nocobra_dex.txt" :
					"boot_plugins_nocobra.txt",		HTML_ONCLICK, "/vshplugin.ps3mapi?s=4",
		HTML_BUTTON, "mamba_plugins.txt",			HTML_ONCLICK, "/vshplugin.ps3mapi?s=1",
		HTML_BUTTON, "prx_plugins.txt",				HTML_ONCLICK, "/vshplugin.ps3mapi?s=2",
		HTML_BUTTON, "plugins.txt",					HTML_ONCLICK, "/vshplugin.ps3mapi?s=3"); concat(buffer, templn);

	add_plugins_list(buffer, templn, 0);

	sprintf(templn, "</table><br>");

	if(!is_ps3mapi_home) strcat(templn, HTML_RED_SEPARATOR);
	concat(buffer, templn);
}

static sys_prx_id_t load_start(const char *path)
{
	int modres, res;
	sys_prx_id_t id;
	id = sys_prx_load_module(path, 0, NULL);
	if (id < CELL_OK)
	{
		BEEP3;
		return id;
	}
	res = sys_prx_start_module(id, 0, NULL, &modres, 0, NULL);
	if (res < CELL_OK)
	{
		BEEP3;
		return res;
	}
	else
	{
		BEEP1;
		return id;
	}
}

static sys_prx_id_t stop_unload(sys_prx_id_t id)
{
	int modres, res;
	res = sys_prx_stop_module(id, 0, NULL, &modres, 0, NULL);
	if (res < CELL_OK)
	{
		BEEP3;
		return res;
	}
	res = sys_prx_unload_module(id, 0, NULL);
	if (res < CELL_OK)
	{
		BEEP3;
		return res;
	}
	else
	{
		BEEP1;
		return id;
	}
}

static void ps3mapi_gameplugin(char *buffer, char *templn, const char *param)
{
	bool is_ps3mapi_home = (*param == ' ');

	u32 pid = 0;

	if(islike(param, "/gameplugin.ps3mapi") && param[19] == '?')
	{
		pid = get_valuen32(param, "proc=");
		if(pid)
		{
			if(pid <= LV2) {ps3mapi_vshplugin(buffer, templn, param); return;}

			char *pos = strstr(param, "unload_slot=");
			if(pos)
			{
				unsigned int prx_id = get_valuen32(pos, "unload_slot=");
				if(get_valuen32(param, "sys="))
					stop_unload(prx_id); // <- unload system modules
				else
					{system_call_4(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_UNLOAD_PROC_MODULE, (u64)pid, (u64)prx_id); }
			}
			else
			{
				char prx_path[STD_PATH_LEN];
				if(get_param("prx=", prx_path, param, STD_PATH_LEN))
				{
					check_path_alias(prx_path);
					if(strstr(prx_path, "/dev_flash"))
						load_start(prx_path); // <- load system modules from flash to process
					else
						{system_call_6(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_LOAD_PROC_MODULE, (u64)pid, (u64)(u32)prx_path, NULL, 0); } // <- load custom modules to process
				}
			}
		}
	}

	if(!is_ps3mapi_home)
		sprintf(templn, "<b>%s%s</b>"
						HTML_BLU_SEPARATOR,
						HOME_PS3MAPI, "Game Plugins");
	else
		sprintf(templn, "<b>%s</b>"
						HTML_BLU_SEPARATOR,
						"Game Plugins");

	concat(buffer, templn);

	sprintf(templn, HTML_FORM_METHOD_FMT("/gameplugin")
					"<b><u>%s:</u></b>  ", HTML_FORM_METHOD, "Process"); concat(buffer, templn);

	memset(templn, 0, MAX_LINE_LEN);

	u8 is_vsh = add_proc_list(buffer, templn, &pid, 3);

	if(is_ps3mapi_home || !pid)
		sprintf(templn, "<input class=\"bs\" type=\"submit\" value=\" Set \" /></form>");
	else
		sprintf(templn, "</form>");

	concat(buffer, templn);

	if(pid)
	{
		sprintf(templn,
					"<table>"
					 "<tr>"
					  "<td width=\"75\" class=\"la\">%s</td>"
					  "<td width=\"300\" class=\"la\">%s</td>"
					  "<td width=\"500\" class=\"la\">%s</td>"
					  "<td width=\"125\" class=\"ra\"> </td>"
					 "</tr>",
					"Slot", "Name", "File name"); buffer += concat(buffer, templn);

		#define MAX_SLOTS	61

		char tmp_name[30];
		char tmp_filename[STD_PATH_LEN];
		u32 mod_list[MAX_SLOTS];
		memset(mod_list, 0, sizeof(mod_list));
		{system_call_4(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_ALL_PROC_MODULE_PID, (u64)pid, (u64)(u32)mod_list);}

		for(unsigned int slot = 0; slot < MAX_SLOTS; slot++)
		{
			memset(tmp_name, 0, sizeof(tmp_name));
			memset(tmp_filename, 0, sizeof(tmp_filename));
			if(1 < mod_list[slot])
			{
				{system_call_5(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_PROC_MODULE_NAME, (u64)pid, (u64)mod_list[slot], (u64)(u32)tmp_name);}
				{system_call_5(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_PROC_MODULE_FILENAME, (u64)pid, (u64)mod_list[slot], (u64)(u32)tmp_filename);}
			}
			if(*tmp_filename)
			{
				sprintf(templn,
						"<tr>"
						 "<td width=\"75\" class=\"la\">%i</td>"
						 "<td width=\"300\" class=\"la\">%s</td>"
						 "<td width=\"500\" class=\"la\">",
						slot, tmp_name); buffer += concat(buffer, templn);

				buffer += add_breadcrumb_trail(buffer, tmp_filename);

				sprintf(templn, "</td>"
						 "<td width=\"100\" class=\"ra\">"
						  HTML_FORM_METHOD_FMT("/gameplugin")
						  "<input name=\"proc\" type=\"hidden\" value=\"%u\">"
						  "<input name=\"unload_slot\" type=\"hidden\" value=\"%i\">"
						  "<input name=\"sys\" type=\"hidden\" value=\"%u\">"
						  "<input type=\"submit\" value=\" Unload \" title=\"id=%i\">"
						  "</form>"
						 "</td>"
						"</tr>", HTML_FORM_METHOD,  pid, mod_list[slot], islike(tmp_filename, "/dev_flash"), mod_list[slot]);
			}
			else
			{
				sprintf(tmp_name, "NULL");
				//sprintf(tmp_filename, "/dev_hdd0/tmp/my_plugin_%i.sprx", slot);
				sprintf(templn,
						"<tr>"
						 "<td width=\"75\" class=\"la\">%i</td>"
						 "<td width=\"300\" class=\"la\">%s</td>"
						 "<td width=\"100\" class=\"ra\">"
						  HTML_FORM_METHOD_FMT("/gameplugin")
						   "<td width=\"500\" class=\"la\">"
							 "<input name=\"proc\" type=\"hidden\" value=\"%u\">"
							 HTML_INPUT("prx\" list=\"plugins", "", "128", "75")
							 "<input name=\"load_slot\" type=\"hidden\" value=\"%i\">"
							 "<input type=\"submit\" value=\" Load \">"
						   "</td>"
						  "</form>"
						 "</td>"
						"</tr>",
						slot, tmp_name,
						HTML_FORM_METHOD, pid, slot);
			}
			buffer += concat(buffer, templn);
		}

		add_plugins_list(buffer, templn, is_vsh);
	}

	sprintf(templn, "</table><br>");

	if(!is_ps3mapi_home) strcat(templn, HTML_RED_SEPARATOR);
	concat(buffer, templn);
}

static void ps3mapi_home(char *buffer, char *templn)
{
	int syscall8_state = NONE;
	{system_call_2(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_PCHECK_SYSCALL8); syscall8_state = (int)p1;}
	int core_version = 0;
	if(syscall8_state>=0) {system_call_2(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_CORE_VERSION); core_version = (int)(p1);}
	int versionfw = 0;
	if(syscall8_state>=0) {system_call_2(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_FW_VERSION); versionfw = (int)(p1);}
	char fwtype[32]; memset(fwtype, 0, 32);
	if(syscall8_state>=0) {system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_FW_TYPE, (u64)(u32)fwtype);}

	if(!versionfw)
		syscall8_state = NONE;

	//---------------------------------------------
	//PS3 Commands---------------------------------
	//---------------------------------------------
	sprintf(templn, "<b>%s</b>"
					HTML_BLU_SEPARATOR
					"<table width=\"800\"><tr>", "PS3 Commands");
	concat(buffer, templn);

	//RingBuzzer
	ps3mapi_buzzer(buffer, templn, (char*)" ");

	//LedRed
	ps3mapi_led(buffer, templn, (char*)" ");

	sprintf(templn, "</td>");
	concat(buffer, templn);

	//Notify
	ps3mapi_notify(buffer, templn, (char*)" ");

	if (syscall8_state >= 0 && syscall8_state < 3)
	{
		//---------------------------------------------
		//Process Commands
		//---------------------------------------------
		//GetMem
		ps3mapi_getmem(buffer, templn, (char*)" ");

		//SetMem
		ps3mapi_setmem(buffer, templn, (char*)" ");

		//---------------------------------------------
		//Game Plugin
		//---------------------------------------------
		ps3mapi_gameplugin(buffer, templn, (char*)" ");

		//---------------------------------------------
		//VSH Plugin
		//---------------------------------------------
		ps3mapi_vshplugin(buffer, templn, (char*)" ");

		//---------------------------------------------
		//IDPS/PSID
		//---------------------------------------------
		if(core_version >= 0x0120)
		{
			ps3mapi_setidps(buffer, templn, (char*)" ");
		}

		//---------------------------------------------
		//CFW Syscall
		//---------------------------------------------
		ps3mapi_syscall(buffer, templn, (char*)" ");
		//CFW Syscall8
		ps3mapi_syscall8(buffer, templn, (char*)" ");

		sprintf(templn, HTML_RED_SEPARATOR
						"%s: %X %s | PS3MAPI: webUI v%X, Server v%X, Core v%X | By NzV, modified by OsirisX", STR_FIRMWARE, versionfw, fwtype, PS3MAPI_WEBUI_VERSION, PS3MAPI_SERVER_VERSION, core_version);
		concat(buffer, templn);
	}
	else
	{
		//CFW Syscall8
		ps3mapi_syscall8(buffer, templn, (char*)" ");

		sprintf(templn, "</table><br>" HTML_RED_SEPARATOR
						"[SYSCALL8 %sDISABLED] | PS3MAPI: webUI v%X, Server v%X | By NzV", (syscall8_state == 3) ? "PARTIALLY ":"", PS3MAPI_WEBUI_VERSION, PS3MAPI_SERVER_VERSION);
		concat(buffer, templn);
	}
}
#endif // #ifdef PS3MAPI

////////////////////////////////////////
///////////// PS3MAPI END //////////////
////////////////////////////////////////
