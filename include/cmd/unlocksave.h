#ifdef UNLOCK_SAVEDATA
	if(islike(param, "/unlocksave.ps3"))
	{
		// /unlocksave.ps3<path>  fix PARAM.SFO in savedata folder

		char *savedata_path = param + 15;
		if(*savedata_path != '/') sprintf(savedata_path, "%s/home/%08i", drives[0], xusers()->GetCurrentUserNumber());

		check_path_alias(savedata_path);

		if(file_exists(savedata_path))
		{
			Check_Overlay();
			scan(savedata_path, true, "/PARAM.SFO", SCAN_UNLOCK_SAVE, NULL);
			keep_alive = http_response(conn_s, header, param, CODE_BREADCRUMB_TRAIL, param);
		}
		else
			keep_alive = http_response(conn_s, header, param, CODE_PATH_NOT_FOUND, "404 Path not found");

		goto exit_handleclient_www;
	}
#endif
