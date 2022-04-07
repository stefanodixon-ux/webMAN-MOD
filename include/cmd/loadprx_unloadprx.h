#ifdef LOAD_PRX
	if(islike(param, "/loadprx.ps3") || islike(param, "/unloadprx.ps3"))
	{
		// /loadprx.ps3<path-sprx>
		// /loadprx.ps3?prx=<path-sprx>
		// /loadprx.ps3?prx=<path-sprx>&slot=<slot>
		// /unloadprx.ps3<path-sprx>
		// /unloadprx.ps3?prx=<path-sprx>
		// /unloadprx.ps3?slot=<slot>
		// /unloadprx.ps3?id=<id>

		if(!cobra_version || syscalls_removed) goto exit_nocobra_error;

		unsigned int slot = 7; bool prx_found;

		#ifdef PKG_HANDLER
		int id = get_valuen32(param, "id=");
		if(id)
		{
			UnloadPluginById(id);
			keep_alive = http_response(conn_s, header, param, CODE_HTTP_OK, param); is_busy = false;
			goto exit_handleclient_www;
		}
		#endif

		char *sprx_path = (char *)templn;
		if(param[12] == '/') strcpy(sprx_path, param + 12); else
		if(param[14] == '/') strcpy(sprx_path, param + 14); else
		{
			get_param("prx=", sprx_path, param, MAX_PATH_LEN);
		}

		if(*sprx_path == '/')
			check_path_alias(sprx_path);
		prx_found = file_exists(sprx_path);

		if(prx_found || (*sprx_path != '/'))
		{
			if(*sprx_path)
			{
				slot = ps3mapi_get_vsh_plugin_slot_by_name(sprx_path, 0);
				if(islike(param, "/unloadprx.ps3") || (slot < 7)) prx_found = false;
			}
			if((slot < 1) || (slot > 6))
			{
				slot = get_valuen(param, "slot=", 0, 6);
				if(!slot) slot = get_free_slot(); // find first free slot if slot == 0
			}

			if(prx_found)
				sprintf(param, "slot: %i<br>load prx: %s%s", slot, sprx_path, HTML_BODY_END);
			else
				sprintf(param, "unload slot: %i%s", slot, HTML_BODY_END);

			_concat(&sbuffer, param);

			if(slot < 7)
			{
				char *tmp_name = templn + 512;
				char *tmp_filename = templn + 550;
				ps3mapi_check_unload(slot, tmp_name, tmp_filename);

				cobra_unload_vsh_plugin(slot);

				if(prx_found)
				{
					cobra_load_vsh_plugin(slot, sprx_path, NULL, 0);
					if(strstr(sprx_path, "/webftp_server")) goto quit;
				}

				if(strstr(sprx_path, "/VshFpsCounter")) overlay_enabled = prx_found;
			}
		}
		else
			_concat(&sbuffer, STR_NOTFOUND);
	}
	else
#endif // #ifdef LOAD_PRX
