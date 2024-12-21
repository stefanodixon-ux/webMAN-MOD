#ifdef FPS_OVERLAY
#define OVERLAY_ADDR		0x8000000000700000ULL

static u8 overlay_enabled = 0;
static u8 overlay_info = 0;

enum show_progress_options
{
	OV_CLEAR,
	OV_SCAN,
	OV_COPY,
	OV_FIX,
	OV_DELETE,
	OV_DUMP,
	OV_FIND,
	OV_SHOW
};

static void show_progress(const char *path, u8 oper)
{
	if(!overlay_enabled) return;
	if(syscalls_removed || CFW_SYSCALLS_REMOVED(TOC)) return;

	char data[0x80];
	u64 *data2 = (u64 *)data;

	if(oper == OV_CLEAR)
		{_memset(data, sizeof(data)); overlay_info = 0;}
	else if(oper == OV_SCAN)
		snprintf(data, sizeof(data), "\n%s:\n%s", STR_SCAN2, path);
	else if(oper == OV_COPY)
		snprintf(data, sizeof(data), "\n%s:\n%s", STR_COPYING, path);
	else if(oper == OV_FIX)
		snprintf(data, sizeof(data), "\n%s:\n%s", STR_FIXING, path);
	else if(oper == OV_DELETE)
		snprintf(data, sizeof(data), "\n%s:\n%s", STR_DELETE, path);
	else if(oper == OV_DUMP)
		snprintf(data, sizeof(data), "\n%s:\n%s", "Dumping", path);
	else if(oper == OV_FIND)
		snprintf(data, sizeof(data), "\n%s:\n%s", "Searching", path);
	else // if(oper == OV_SHOW)
		strncopy(data, sizeof(data), path);

	// fix degree character
	if(overlay_info)
	{
		replace_char(data, 0x80, '\n'); // •
		replace_char(data, 0xC2, 0x9F); // °
		for(int c = 0xA0; c <= 0xFF; c++)
			replace_char(data, c, 0x20);
	}

	u8 len = strlen(data); _memset(data + len, sizeof(data) - len);

	u64 addr = OVERLAY_ADDR; len = (oper == OV_CLEAR) ? 0xF : len / 8;
	for(u8 n = 0; n <= len; n++, addr += 8)
	{
		pokeq(addr, data2[n]);
	}
}

static void disable_progress(void)
{
	show_progress("", OV_CLEAR);
}
#else
#define show_progress(path, oper)
#define disable_progress()
#endif // FPS_OVERLAY

static void play_rco_sound(const char *sound)
{
	const char *system_plugin = (char*)"system_plugin";
	char *sep = strchr(sound, '|'); if(sep) {*sep = NULL, system_plugin = sep + 1;}

	u32 plugin = View_Find(system_plugin);
	if(plugin)
		PlayRCOSound(plugin, sound, 1, 0);
}

static void play_sound_id(u8 value)
{
	if(value >= '0') value -= '0';

	if(value == 1) { BEEP1 }
	if(value == 2) { BEEP2 }
	if(value == 3) { BEEP3 }

	if(value == 0) { play_rco_sound("snd_cancel"); }
	if(value == 4) { play_rco_sound("snd_cursor"); }
	if(value == 5) { play_rco_sound("snd_trophy"); }
	if(value == 6) { play_rco_sound("snd_decide"); }
	if(value == 7) { play_rco_sound("snd_option"); }
	if(value == 8) { play_rco_sound("snd_system_ok"); }
	if(value == 9) { play_rco_sound("snd_system_ng"); }
}

#ifdef PKG_HANDLER
static int LoadPluginByName(char *plugin_name);
#endif

//------------
/* Based on PHTNC's code to write VSH Notify notifications with icons */

enum rco_icons
{
	ICON_CAUTION	 = 3,
	ICON_NETWORK	 = 5,
	ICON_PSN		 = 18,
	ICON_CHECK		 = 22,
	ICON_EXCLAMATION = 23,
	ICON_NOTIFY		 = 29,
	ICON_ERROR		 = 35,
	ICON_MUSIC		 = 30,
	ICON_VIDEO		 = 32,
	ICON_GAME		 = 33,
	ICON_PS2_DISC	 = 43,
	ICON_PSP_UMD	 = 48,
	ICON_WAIT		 = 49,
	ICON_MOUNT		 = 50,
};

#define MAX_RCO_IMAGES	51

static s32 show_msg_with_icon(u8 icon_id, const char *msg)
{
	const char *rco_images[MAX_RCO_IMAGES] = {
								// system_plugin icons
								"tex_notification_info",			//0
								"tex_notification_friend",			//1
								"tex_notification_headset",			//2
								"tex_notification_caution",			//3
								"tex_notification_keypad",			//4
								"tex_notification_mediasever",		//5
								"tex_temporary_icon",				//6
								"tex_notification_psbutton_insensitive",	//7
								"tex_notification_settings",		//8
								"tex_notification_trophy_bronze",	//9
								"tex_notification_trophy_silver",	//10
								"tex_notification_trophy_gold",		//11
								"tex_notification_trophy_platinum",	//12
								"tex_pointer_hand",					//13
								"tex_pointer_pen",					//14
								"tex_pointer_arrow",				//15
								"tex_pointer_grab",					//16
								"tex_arrow_right",					//17
								// explore_plugin icons
								"tex_psn_big",						//18
								"tex_psplus_icon",					//19
								"tex_Signing_In",					//20
								"tex_new_ws",						//21
								"tex_check_ws",						//22
								"tex_urgent_ws",					//23
								"item_tex_cam_icon",				//24
								"item_tex_Profile_LevelIcon",		//25
								"item_tex_ps_store",				//26
								"tex_album_icon",					//27
								"item_tex_Players",					//28
								"tex_indi_NewRoom",					//29
								"tex_music",						//30
								"tex_photo",						//31
								"tex_video",						//32
								"tex_game",							//33
								"tex_lock_icon",					//34
								"tex_indi_Sign_out",				//35-Error icon
								"tex_indi_Message",					//36
								"tex_Message_Sent",					//37
								"item_tex_CardBallon",				//38
								"tex_loading_icon",					//39
								"tex_Avatar_Default",				//40
								"item_tex_disc_bd",					//41-PS3
								"item_tex_disc_icon",				//42-CD
								"item_tex_disc_cd_ps2",				//43-PS2
								"item_tex_disc_ps1",				//44-PSX
								"item_tex_disc_bd_contents",		//45-BD
								"item_tex_disc_dvd",				//46-DVD
								"game_tex_disc_unknown",			//47
								"item_tex_psp_icon",				//48
								"tex_indi_AFK",						//49
								"tex_go_game",						//50
							};

	char rco[64], texture[64];
	const char *plugin = "explore_plugin";
	const char *tex = texture;
	if(icon_id >= MAX_RCO_IMAGES) icon_id = 0;

	// custom textures
	char *pos = strstr(msg, "&icon=");
	if(pos)
	{
		icon_id = get_valuen(pos, "&icon=", 0, 50);

		if(icon_id || pos[6] == '0')
		{
			tex = rco_images[icon_id];
		}
		else if(get_param("&icon=", texture, pos, 63))
		{
			icon_id = MAX_RCO_IMAGES;

			// example: /popup.ps3?message&icon=item_tex_cam_facebook&rco=explore_plugin
			if(get_param("&rco=", rco, pos, 63))
			{
				plugin = rco;
			}
			else if(islike(texture, "tex_notification"))
				plugin = "system_plugin";
		}
		*pos = NULL;
	}
	else
		tex = rco_images[icon_id];

	if(icon_id < 18) plugin = "system_plugin";

	pos = strstr(msg, "&rco=");
	if(pos)
	{
		// show notification with RCO text
		int teximg, dummy = 0, rco_plugin = View_Find(plugin), tex_plugin = rco_plugin;

		// example: /popup.ps3?msg_psn&rco=explore_plugin&icon=5
		if(get_param("&rco=", rco, pos, 63))
		{
			rco_plugin = View_Find(rco);
			#ifdef PKG_HANDLER
			if(!rco_plugin)
			{
				LoadPluginByName(rco);
				rco_plugin = View_Find(rco);
			}
			#endif

			if(!rco_plugin) return vshtask_notify(msg);
		}

		LoadRCOTexture(&teximg, tex_plugin, tex); *pos = NULL;
		return NotifyWithTexture(0, tex, 0, &teximg, &dummy, "", "", 0, LoadRCOText(rco_plugin, msg), 0, 0, 0);
	}

	if(IS_INGAME || webman_config->msg_icon)
		return vshtask_notify(msg);

	u32 _plugin = View_Find(plugin);
	if (_plugin <= 0)
		return FAILED;

	int len = strlen(msg); if(len >= 200) len = 198;

	wchar_t message[len + 2];

	mbstowcs(message, msg, len + 1);  //size_t stdc_FCAC2E8E(wchar_t *dest, const char *src, size_t max)

	int teximg, dummy = 0;
	LoadRCOTexture(&teximg, _plugin, tex);
	return NotifyWithTexture(0, tex, 0, &teximg, &dummy, "", "", 0, message, 0, 0, 0);
}
//------------

static void show_msg(const char *text)
{
	//if(!vshtask_notify)
	//	vshtask_notify = getNIDfunc("vshtask", 0xA02D46E7, 0);
	//if(!vshtask_notify) return;

	char msg[240]; strncopy(msg, sizeof(msg), text);

	char *snd = strstr(msg, "&snd=");
	if(snd)
	{
		if(ISDIGIT(snd[5]))
			play_sound_id(snd[5]);
		else
			play_rco_sound(snd + 5);

		*snd = NULL;
	}

	if(strstr(msg, "&icon=") || strstr(msg, "&rco="))
		show_msg_with_icon(0, msg);
	else
		vshtask_notify(msg);
}

static void show_msg2(const char *text1, const char *text2)
{
	char msg[200];
	snprintf(msg, sizeof(msg), "%s %s", text1, text2);
	vshtask_notify(msg);
}

static void show_status(const char *label, u8 status)
{
	show_msg2(label, status ? STR_ENABLED : STR_DISABLED);
}

static void show_error(const char *text)
{
	char msg[200];
	if(IS(text, "[nonCobra]"))
		concat_text(msg, "Cobra", STR_DISABLED);
	else
		snprintf(msg, sizeof(msg), "%s %s", STR_ERROR, text);
	
	play_sound_id(9); show_msg_with_icon(ICON_ERROR, msg);
}
