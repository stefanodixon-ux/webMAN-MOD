static u16 plugin_active = 0;

#ifdef AUTO_POWER_OFF

#define AUTO_POWER_OFF_BACKUP_FILE   WMTMP "/auto_power_off"

static int AutoPowerOff[2] = {-1, -1};

#define AutoPowerOffGame	AutoPowerOff[0]
#define AutoPowerOffVideo	AutoPowerOff[1]

static void setAutoPowerOff(bool disable)
{
	if((c_firmware < 4.46f) || webman_config->auto_power_off) return;

	if(AutoPowerOffGame < 0)
	{
		xregistry()->loadRegistryIntValue(0x33, &AutoPowerOffGame);
		xregistry()->loadRegistryIntValue(0x32, &AutoPowerOffVideo);
	}

	xregistry()->saveRegistryIntValue(0x33, disable ? 0 : AutoPowerOffGame);
	xregistry()->saveRegistryIntValue(0x32, disable ? 0 : AutoPowerOffVideo);

	if(disable)
	{
		save_file(AUTO_POWER_OFF_BACKUP_FILE, (char *)&AutoPowerOff, 2 * sizeof(int));
	}
	else
		cellFsUnlink(AUTO_POWER_OFF_BACKUP_FILE);
}

static void restoreAutoPowerOff(void)
{
	if(read_file(AUTO_POWER_OFF_BACKUP_FILE, (char *)&AutoPowerOff, 2 * sizeof(int), 0))
	{
		setAutoPowerOff(false);
	}
}

static void setPluginActive(void)
{
	if(plugin_active == 0) setAutoPowerOff(true);
	plugin_active++;
}

static void setPluginInactive(void)
{
	if(plugin_active > 0)
	{
		plugin_active--;
		if(plugin_active == 0) setAutoPowerOff(false);
	}
}

#else
 #define setPluginActive()
 #define setPluginInactive()
#endif

static void setPluginExit(void)
{
	working = plugin_active = 0;

	#ifdef AUTO_POWER_OFF
	setAutoPowerOff(false);
	#endif

	del_turnoff(0);
}
