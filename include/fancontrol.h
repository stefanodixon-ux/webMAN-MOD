#define MIN_FANSPEED			(20) /* % */
#define DEFAULT_MIN_FANSPEED	(25) /* % */
#define MAX_FANSPEED			(0xF4)
#define MAX_TEMPERATURE			(85) /* °C */
#define MY_TEMP 				(68) /* °C */

#define FAN_AUTO 				(0)

#define PERCENT_TO_8BIT(val)	((val * 255) / 100)

#define DISABLED		(0)
#define ENABLED			(1)
#define TOGGLE_MODE		(2)
#define ENABLE_SC8		(3)
#define PS2_MODE_OFF	(4)

#define SET_PS2_MODE	(1)
#define SYSCON_MODE		(0)

static u8 fan_speed = 0x33;
static u8 old_fan = 0x33;
static u32 max_temp = 0; //syscon

#define SC_SET_FAN_POLICY				(389)
#define SC_GET_FAN_POLICY				(409)
#define SC_GET_TEMPERATURE				(383)

u64 get_fan_policy_offset = 0;
u64 set_fan_policy_offset = 0;

static bool fan_ps2_mode = false; // temporary disable dynamic fan control

static void get_temperature(u32 _dev, u32 *_temp)
{
	system_call_2(SC_GET_TEMPERATURE, (u64)(u32) _dev, (u64)(u32) _temp); *_temp >>= 24; // return °C
}

static int sys_sm_set_fan_policy(u8 unknown , u8 fan_mode, u8 fan_speed)
{
	// syscon mode: 0, 1, 0x0
	// manual mode: 0, 2, fan_speed (0x33 - 0xFF)

	u64 restore_set_fan_policy = peekq(set_fan_policy_offset); // sys 389 get_fan_policy
	u64 enable_set_fan_policy = 0x3860000100000000ULL | (restore_set_fan_policy & 0xffffffffULL);

	lv2_poke_fan(set_fan_policy_offset, enable_set_fan_policy);
	system_call_3(SC_SET_FAN_POLICY, (u64) unknown, (u64) fan_mode, (u64) fan_speed);
	lv2_poke_fan(set_fan_policy_offset, restore_set_fan_policy);

	return_to_user_prog(int);
}

static int sys_sm_get_fan_policy(u8 id, u8 *st, u8 *mode, u8 *speed, u8 *unknown)
{
	u64 restore_get_fan_policy = peekq(get_fan_policy_offset); // sys 409 get_fan_policy
	u64 enable_get_fan_policy = 0x3860000100000000ULL | (restore_get_fan_policy & 0xffffffffULL);

	lv2_poke_fan(get_fan_policy_offset, enable_get_fan_policy);
	system_call_5(SC_GET_FAN_POLICY, (u64) id, (u64)(u32) st, (u64)(u32) mode, (u64)(u32) speed, (u64)(u32) unknown);
	lv2_poke_fan(get_fan_policy_offset, restore_get_fan_policy);

	return_to_user_prog(int);
}

static void fan_control(u8 set_fanspeed, u8 init)
{
	if(fan_ps2_mode) return; //do not change fan settings while PS2 game is mounted

	if(get_fan_policy_offset)
	{
		{ PS3MAPI_ENABLE_ACCESS_SYSCALL8 }

		u8 min_fan_speed = PERCENT_TO_8BIT(webman_config->minfan);

		if(!init)
		{
			//backup[3]=peekq(syscall_base + (u64) (130 * 8));
			//backup[4]=peekq(syscall_base + (u64) (138 * 8));
			//backup[5]=peekq(syscall_base + (u64) (379 * 8));

			sys_sm_set_fan_policy(0, 2, min_fan_speed);
		}

		if(set_fanspeed < 0x33)
		{
			u8 st, mode, unknown;
			u8 fan_speed8 = 0;
			sys_sm_get_fan_policy(0, &st, &mode, &fan_speed8, &unknown);
			fan_speed = RANGE(fan_speed8, min_fan_speed , 0xFC);
		}
		else
			fan_speed = RANGE(set_fanspeed, min_fan_speed , 0xFC);

		old_fan = fan_speed;
		sys_sm_set_fan_policy(0, 2, fan_speed);

		{ PS3MAPI_DISABLE_ACCESS_SYSCALL8 }
	}
}

static void restore_fan(u8 set_ps2_temp)
{
	if(get_fan_policy_offset > 0)
	{
		//pokeq(syscall_base + (u64) (130 * 8), backup[3]);
		//pokeq(syscall_base + (u64) (138 * 8), backup[4]);
		//pokeq(syscall_base + (u64) (379 * 8), backup[5]);

		{ PS3MAPI_ENABLE_ACCESS_SYSCALL8 }

		if(set_ps2_temp)
		{
			webman_config->ps2temp = RANGE(webman_config->ps2temp, 20, 99); //%
			sys_sm_set_fan_policy(0, 2, PERCENT_TO_8BIT(webman_config->ps2temp));
			fan_ps2_mode = true;
		}
		else sys_sm_set_fan_policy(0, 1, 0x0); //syscon

		{ PS3MAPI_DISABLE_ACCESS_SYSCALL8 }
	}
}

static void enable_fan_control(u8 enable, char *msg)
{
	if(enable == PS2_MODE_OFF) fan_ps2_mode = false;		else
	if(enable == ENABLE_SC8) webman_config->fanc = ENABLED;	else
	if(enable <= ENABLED)	 webman_config->fanc = enable;	else
							 webman_config->fanc = (webman_config->fanc ? DISABLED : ENABLED);

	max_temp = 0;
	if(webman_config->fanc)
	{
		if(webman_config->temp0 == FAN_AUTO) max_temp = webman_config->temp1;
		fan_control(webman_config->temp0, false);
		sprintf(msg, "%s %s", STR_FANCTRL3, STR_ENABLED);
	}
	else
	{
		restore_fan(SYSCON_MODE); //syscon
		sprintf(msg, "%s %s", STR_FANCTRL3, STR_DISABLED);
	}
	save_settings();
	if(enable != PS2_MODE_OFF) show_msg(msg);

	if(enable == ENABLE_SC8) { PS3MAPI_ENABLE_ACCESS_SYSCALL8 }
}

static void reset_fan_mode(void)
{
	fan_ps2_mode = false;

	webman_config->temp0 = (u8)(((float)(webman_config->manu + 1) * 255.f) / 100.f); // manual fan speed
	webman_config->temp0 = RANGE(webman_config->temp0, 0x33, MAX_FANSPEED);
	fan_control(webman_config->temp0, false);

	if(max_temp) webman_config->temp0 = FAN_AUTO; // enable dynamic fan mode
}
