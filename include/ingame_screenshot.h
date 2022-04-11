#define ENABLE_INGAME_SCREENSHOT	((int*)getNIDfunc("vshmain",0x981D7E9F,0))[0] -= 0x2C;

#ifndef LITE_EDITION

#ifdef PS3_BROWSER
static int (*vshmain_is_ss_enabled)(void) = NULL;
static int (*set_SSHT_)(int) = NULL;

static int opd[2] = {0, 0};

static void enable_ingame_screenshot(void)
{
	vshmain_is_ss_enabled = getNIDfunc("vshmain", 0x981D7E9F, 0); //is screenshot enabled?

	if(vshmain_is_ss_enabled() == 0)
	{
		set_SSHT_ = (void*)&opd;
		memcpy(set_SSHT_, vshmain_is_ss_enabled, 8);
		opd[0] -= 0x2C; // Sub before vshmain_981D7E9F sets Screenshot Flag
		set_SSHT_(1);	// enable screenshot

		show_msg("Screenshot enabled");
		sys_ppu_thread_sleep(2);
	}
}
#endif // #ifdef PS3_BROWSER

#endif // #ifndef LITE_EDITION