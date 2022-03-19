#ifdef LOAD_PRX
	if(islike(param, "/loadprx.ps3") || islike(param, "/unloadprx.ps3"))
	{
		// /loadprx.ps3<path-sprx>
		// /loadprx.ps3?prx=<path-sprx>
		// /loadprx.ps3?prx=<path-sprx>&slot=<slot>
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

		*templn = 0;
		if(param[12] == '/') sprintf(templn, "%s", param + 12); else
		if(param[14] == '/') sprintf(templn, "%s", param + 14); else
		{
			get_param("prx=", templn, param, MAX_PATH_LEN);
		}

		if(*templn == '/')
			check_path_alias(templn);
		prx_found = file_exists(templn);

		if(prx_found || (*templn != '/'))
		{
			if(*templn)
			{
				slot = ps3mapi_get_vsh_plugin_slot_by_name(templn, false);
				if(islike(param, "/unloadprx.ps3")) prx_found = false;
			}
			if((slot < 1) || (slot > 6))
			{
				slot = get_valuen(param, "slot=", 0, 6);
				if(!slot) slot = get_free_slot(); // find first free slot if slot == 0
			}

			if(prx_found)
				sprintf(param, "slot: %i<br>load prx: %s%s", slot, templn, HTML_BODY_END);
			else
				sprintf(param, "unload slot: %i%s", slot, HTML_BODY_END);

			_concat(&sbuffer, param);

			if(slot < 7)
			{
				cobra_unload_vsh_plugin(slot);

				if(prx_found)
					{cobra_load_vsh_plugin(slot, templn, NULL, 0); if(strstr(templn, "/webftp_server")) goto quit;}
			}
		}
		else
			_concat(&sbuffer, STR_NOTFOUND);
	}
	else
#endif // #ifdef LOAD_PRX
