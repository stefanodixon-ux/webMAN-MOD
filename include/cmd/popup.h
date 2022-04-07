	if(islike(param, "/popup.ps3"))
	{
		// /popup.ps3	- show info once
		// /popup.ps3@	- show overlay text using VshFpsCounter.sprx
		// /popup.ps3$	- show persistent info ON
		// /popup.ps3*	- show persistent info OFF
		// /popup.ps3?	- show webman version
		// /popup.ps3&snd=<id>
		// /popup.ps3?<msg>
		// /popup.ps3?<msg>&icon=<id>
		// /popup.ps3*<msg>
		// /popup.ps3?<msg>&snd=<id>
		// /popup.ps3*<msg>&snd=<id>
		// /popup.ps3?<msg>&icon=<id>&snd=<id>
		// /popup.ps3?<msg>&icon=<rsc_icon> (e.g. /popup.ps3?Hello&icon=item_tex_cam_facebook)
		// /popup.ps3?<msg>&icon=<rsc_icon>&rco=<plugin_name> (e.g. /popup.ps3?Hello&icon=item_tex_NewAvc&rco=explore_plugin)

		if(param[10] == '\0')
			{show_info_popup = true;}						// show info once
		else if(param[10] == '$' && param[11] == '\0')
			{BEEP1; show_persistent_popup = PERSIST, show_info_popup = true;}	// show persistent info ON
		else if(param[10] == '*' && param[11] == '\0')
			{if(show_persistent_popup) BEEP2; show_persistent_popup = 0;}		// show persistent info OFF
		else if(param[10] == '?' && param[11] == '\0')
			{show_wm_version(param);}						// show webman version
		else
			{is_popup = 1;}									// show message

		goto html_response;
	}
