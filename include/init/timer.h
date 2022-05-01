static void sys_ppu_thread_sleep(u32 seconds)
{
	sys_ppu_thread_yield();

	seconds *= 8;
	for(u32 n = 0; n < seconds; n++)
	{
		if(!working) break;
		sys_timer_usleep(125000);
	}
}

#define sys_ppu_thread_usleep    sys_timer_usleep
