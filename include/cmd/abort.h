#if defined(FIX_GAME) || defined(COPY_PS3)
	if(strstr(param, ".ps3$abort"))
	{
		// /copy.ps3$abort
		// /fixgame.ps3$abort
		do_restart = false;

		if(copy_in_progress) {copy_aborted = true; show_msg_with_icon(ICON_EXCLAMATION, STR_CPYABORT);}   // /copy.ps3$abort
		#ifdef FIX_GAME
		if(fix_in_progress)  {fix_aborted  = true; show_msg_with_icon(ICON_EXCLAMATION, "Fix aborted!");} // /fixgame.ps3$abort
		#endif

		strcpy(param, "/");
	}
#endif
