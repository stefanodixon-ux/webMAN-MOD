#ifdef GET_KLICENSEE
	if(islike(param, "/klic.ps3"))
	{
		// /klic.ps3           Show status of auto log klicensees
		// /klic.ps3?log       Toggle auto log of klicensees
		// /klic.ps3?auto      Auto-Log: Started
		// /klic.ps3?off       Auto-Log: Stopped
		// /dev_hdd0/klic.log  Log file

		if(npklic_struct_offset == 0)
		{
			// get klicensee struct
			vshnet_5EE098BC = getNIDfunc("vshnet", 0x5EE098BC, 0);
			int* func_start = (int*)(*((int*)vshnet_5EE098BC));
			npklic_struct_offset = (((*func_start) & 0x0000FFFF) << 16) + ((*(func_start + 5)) & 0x0000FFFF) + 0xC;//8;
		}

		#define KL_OFF     0
		#define KL_GET     1
		#define KL_AUTO    2

		u8 klic_polling_status = klic_polling;

		if(param[10] == 'o') klic_polling = KL_OFF;  else
		if(param[10] == 'a') klic_polling = KL_AUTO; else
		if(param[10] == 'l') klic_polling = klic_polling ? KL_OFF : KL_GET; // toggle

		if((klic_polling_status == KL_OFF) && (klic_polling == KL_AUTO))
		{
			if(IS_ON_XMB) keep_alive = http_response(conn_s, header, param, CODE_HTTP_OK, (char*)"/KLIC: Waiting for game...");

			// wait until game start
			while((klic_polling == KL_AUTO) && IS_ON_XMB && working) sys_ppu_thread_usleep(500000);
		}

		char kl[0x120], prev[0x200], buffer[0x200]; memset(kl, 0, sizeof(kl));

		if(IS_INGAME)
		{
			#define KLICENSEE_SIZE          0x10
			#define KLICENSEE_OFFSET        (npklic_struct_offset)
			#define KLIC_PATH_OFFSET        (npklic_struct_offset+0x10)
			#define KLIC_CONTENT_ID_OFFSET  (npklic_struct_offset-0xA4)

			hex_dump(kl, KLICENSEE_OFFSET, KLICENSEE_SIZE);
			get_game_info(); sprintf(buffer, "%s %s</H2>"
											 "%s%s<br>"
											 "%s%s<br>"
											 "%s%s<p>",
											 _game_TitleID, _game_Title,
											 "KLicensee: ",  hex_dump(kl, KLICENSEE_OFFSET, KLICENSEE_SIZE),
											 "Content ID: ", (char*)(KLIC_CONTENT_ID_OFFSET),
											 "File: ",       (char*)(KLIC_PATH_OFFSET));
		}
		else
			{sprintf(buffer, "ERROR: <a style=\"%s\" href=\"play.ps3\">%s</a><p>", HTML_URL_STYLE, "KLIC: Not in-game!"); klic_polling = KL_OFF; vshNotify_WithIcon(ICON_EXCLAMATION, "KLIC: Not in-game!");}


		sprintf(prev, "%s", ((klic_polling_status) ? (klic_polling ? "Auto-Log: Running" : "Auto-Log: Stopped") :
													((klic_polling == KL_GET)  ? "Added to Log" :
													 (klic_polling == KL_AUTO) ? "Auto-Log: Started" : "Enable Auto-Log")));

		sprintf(header, "<a style=\"%s\" href=\"%s\">%s</a>", HTML_URL_STYLE,
					(klic_polling_status > 0 && klic_polling > 0) ? "klic.ps3?off"  :
					( (klic_polling_status | klic_polling)  == 0) ? "klic.ps3?auto" : "dev_hdd0/klic.log", prev); strcat(buffer, header);

		keep_alive = http_response(conn_s, header, param, CODE_HTTP_OK, buffer);

		if(*kl && (klic_polling != KL_OFF))
		{
			get_game_info(); sprintf(header, "%s [%s]", _game_Title, _game_TitleID);

			sprintf(buffer, "%s\n\n%s", header, (char*)(KLIC_PATH_OFFSET));
			show_msg(buffer);

			if(klic_polling == KL_GET)
			{
				sprintf(buffer, "%s%s\n"
								"%s%s", "KLicensee: ", kl, "Content ID: ", (char*)(KLIC_CONTENT_ID_OFFSET));
				show_msg(buffer);
			}

			if(klic_polling_status == KL_OFF)
			{
				while((klic_polling != KL_OFF) && IS_INGAME && working)
				{
					hex_dump(kl, (int)KLICENSEE_OFFSET, KLICENSEE_SIZE);
					sprintf(buffer, "%s %s %s %s", kl, (char*)(KLIC_CONTENT_ID_OFFSET), header, (char*)(KLIC_PATH_OFFSET));

					if(klic_polling == KL_AUTO && IS(buffer, prev)) {sys_ppu_thread_usleep(10000); continue;}

					save_file("/dev_hdd0/klic.log", buffer, APPEND_TEXT);

					if(klic_polling == KL_GET) break; strcpy(prev, buffer);
				}

				klic_polling = KL_OFF;
			}
		}
		goto exit_handleclient_www;
	}
#endif
