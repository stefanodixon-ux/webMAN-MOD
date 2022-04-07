#define Check_Overlay()		overlay = (peekq(OVERLAY_ADDR) == 0)
#define OVERLAY_ADDR		0x8000000000700000ULL

static u8 overlay = 0;
static u8 overlay_enabled = 0;

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
	if(!overlay || syscalls_removed) return;

	char data[0x80];
	u64 *data2 = (u64 *)data;

	if(oper == OV_CLEAR)
		{memset(data, 0, 0x80); overlay = 0;}
	else if(oper == OV_SCAN)
		snprintf(data, 0x80, "\n%s:\n%s", STR_SCAN2, path);
	else if(oper == OV_COPY)
		snprintf(data, 0x80, "\n%s:\n%s", STR_COPYING, path);
	else if(oper == OV_FIX)
		snprintf(data, 0x80, "\n%s:\n%s", STR_FIXING, path);
	else if(oper == OV_DELETE)
		snprintf(data, 0x80, "\n%s:\n%s", STR_DELETE, path);
	else if(oper == OV_DUMP)
		snprintf(data, 0x80, "\n%s:\n%s", "Dumping", path);
	else if(oper == OV_FIND)
		snprintf(data, 0x80, "\n%s:\n%s", "Searching", path);
	else // if(oper == OV_SHOW)
		snprintf(data, 0x80, "%s", path);

	u8 len = strlen(data); memset(data + len, 0, 0x80 - len);

	u64 addr = OVERLAY_ADDR;
	for(u8 n = 0; n < 0x10; n++, addr += 8)
	{
		pokeq(addr, data2[n]);
	}
}

static void disable_progress(void)
{
	show_progress("", OV_CLEAR);
}

static void play_rco_sound(const char *sound)
{
	char *system_plugin = (char*)"system_plugin";
	char *sep = strchr(sound, '|'); if(sep) {*sep = NULL, system_plugin = sep + 1;}
	PlayRCOSound((View_Find(system_plugin)), sound, 1, 0);
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

static int32_t vshNotify_WithIcon(u8 icon_id, const char *msg)
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

	if(icon_id >= MAX_RCO_IMAGES) icon_id = 0;
	char *plugin = (char*)"explore_plugin";
	char *tex = (char*)rco_images[icon_id];

	// custom textures
	char rco[24], texture[64];
	char *pos = strstr(msg, "&icon=");
	if(pos)
	{
		icon_id = get_valuen(pos, "&icon=", 0, 50);

		if(icon_id || pos[6] == '0')
		{
			tex = (char*)rco_images[icon_id];
		}
		else if(get_param("&icon=", texture, pos, 63))
		{
			tex = texture, icon_id = MAX_RCO_IMAGES;

			if(get_param("&rco=", rco, pos, 23))
			{
				plugin = (char*)rco;
			}
			else if(islike(texture, "tex_notification"))
				plugin = (char*)"system_plugin";
		}
		*pos = NULL;
	}

	if(icon_id < 18) plugin = (char*)"system_plugin";

	if(IS_INGAME || webman_config->msg_icon)
		return vshtask_notify(msg);

	uint32_t _plugin = View_Find(plugin);
	if (_plugin <= 0)
		return FAILED;

	int len = strlen(msg); if(len >= 200) len = 198;

	wchar_t message[len + 2];

	mbstowcs((wchar_t *)message, (const char *)msg, len + 1);  //size_t stdc_FCAC2E8E(wchar_t *dest, const char *src, size_t max)

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

	char msg[240];
	snprintf(msg, 240, "%s", text);

	char *snd = strstr(msg, "&snd=");
	if(snd)
	{
		if(ISDIGIT(snd[5]))
			play_sound_id(snd[5]);
		else
			play_rco_sound(snd + 5);

		*snd = NULL;
	}

	if(strstr(msg, "&icon="))
		vshNotify_WithIcon(0, msg);
	else
		vshtask_notify(msg);
}

static void show_status(const char *label, const char *status)
{
	char msg[200];
	snprintf(msg, 200, "%s %s", label, status);
	if(IS(label, STR_ERROR))
		vshNotify_WithIcon(ICON_ERROR, msg);
	else
		vshtask_notify(msg);
}

