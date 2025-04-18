if(islike(param, "/gpuclock.ps3") || islike(param, "/memclock.ps3"))
{
	bool gpu = (param[1] == 'g');

	if(param[13] == '?')
	{
		u16 mhz = (u16)val(param + 14); // new clock speed (300-1200)
		overclock(mhz, gpu);

		char *pos = strchr(param, '|');
		if(pos)
		{
			mhz = (u16)val(pos + 1); // new clock speed (300-1200)
			overclock(mhz, !gpu);
		}
	}

	clock_s clock1, clock2;
	clock1.value = lv1_peek_cobra(GPU_CORE_CLOCK); // GPU Core
	clock2.value = lv1_peek_cobra(GPU_VRAM_CLOCK); // GPU Memory

	sprintf(param, "GPU: %i Mhz | VRAM: %i Mhz", 50 * (int)clock1.mul, 25 * (int)clock2.mul);

	keep_alive = http_response(conn_s, header, "/gpuclock.ps3", CODE_HTTP_OK, param); show_msg(param);

	goto exit_handleclient_www;
}
