#ifdef VIDEO_REC
	if(islike(param, "/videorec.ps3"))
	{
		// /videorec.ps3
		// /videorec.ps3<path>
		// /videorec.ps3?<video-rec-params> {mp4, jpeg, psp, hd, avc, aac, pcm, 64, 128, 384, 512, 768, 1024, 1536, 2048}
		// /videorec.ps3?<path>&video=<format>&audio=<format>

		char *video_path = param[13] ? param + 13 : (char*)"/dev_hdd0/VIDEO";

		filepath_check(video_path);

		if(IS_INGAME)
			toggle_video_rec(video_path);

		use_open_path = true;
		add_breadcrumb_trail(buffer, video_path);
		sprintf(header, ":<p>Video recording: <a href=\"%s\">%s</a></p>", param, recording ? STR_ENABLED : STR_DISABLED);
		_concat(&sbuffer, header);

		if(!recording && recOpt) {sprintf(param, "<a class=\"f\" href=\"%s\">%s</a><br>", (char*)&recOpt[0x6], (char*)&recOpt[0x6]); _concat(&sbuffer, param);}
	}
	else
#endif
