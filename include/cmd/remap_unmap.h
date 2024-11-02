#ifdef COBRA_ONLY
	if(islike(param, "/remap.ps3") || islike(param, "/remap_ps3") || islike(param, "/unmap.ps3"))
	{
		// /remap.ps3<path1>&to=<path2>       files on path1 are accessed from path2
		// /remap.ps3?src=<path1>&to=<path2>  use syscall35
		// /remap_ps3<path1>&to=<path2>       files on path1 are accessed from path2 (no check if folder exists)
		// /remap_ps3?src=<path1>&to=<path2>  files on path1 are accessed from path2 (no check if folder exists / sc35)
		// /unmap.ps3<path>                   unmap path
		// /unmap.ps3?src=<path>              use syscall35

		if(!cobra_version || syscalls_removed) goto exit_nocobra_error;

		char *path1 = header, *path2 = header + MAX_PATH_LEN, *url = header + 2 * MAX_PATH_LEN, *title = header + 2 * MAX_PATH_LEN;

		_memset(header, sizeof(header));

		#ifdef ALLOW_DISABLE_MAP_PATH
		// /remap.ps3?status                  status of map path
		// /remap.ps3?disable                 disable map path
		// /remap.ps3?enable                  restore map path after disable
		if(param[10] == '?' && (param[11] == 'e' || param[11] == 'd'))
		{
			disable_map_path(param[11] == 'd');
		}
		else if(param[10] == '?' && (param[11] == 's'))
		{
			u64 open_hook_symbol = (dex_mode) ? open_hook_dex : open_hook_cex;

			bool map_path_enabled = (peekq(open_hook_symbol) != original_open_hook);
			concat_text(param, "map_path", map_path_enabled ? STR_ENABLED : STR_DISABLED );
		}
		else
		#endif
		{
			bool use_sc35 = false;
			bool isremap = (param[1] == 'r');
			bool nocheck = (param[6] == '_');

			disable_map_path(false);

			if(param[10] == '/') get_value(path1, param + 10, MAX_PATH_LEN); else
			if(param[11] == '/') get_value(path1, param + 11, MAX_PATH_LEN); else
			{
				get_param("src=", path1, param, MAX_PATH_LEN);
				#ifdef PS3MAPI
				use_sc35 = !is_syscall_disabled(35);
				#else
				use_sc35 = true;
				#endif
			}

			if(isremap)
			{
				strcopy(path2, "/dev_bdvd");
				get_param("to=", path2, param, MAX_PATH_LEN);
			}

			check_path_tags(path1);
			check_path_tags(path2);

			check_path_alias(path1);
			check_path_alias(path2);

			if(param[10] == 0)
			{
				sys_map_path(NULL, NULL);
				apply_remaps();
			}
			else if(*path1 && path1[1] && (nocheck || file_exists(isremap ? path2 : path1)))
			{
				if(use_sc35)
					sys_map_path2(path1, path2);
				else
					sys_map_path(path1, path2);

				htmlenc(url, path2, 1); urlenc(url, path1); htmlenc(title, path1, 0);

				if(isremap && *path2)
				{
					htmlenc(path1, path2, 0);
					sprintf(param,  "Remap: " HTML_URL "<br>"
									"To: "    HTML_URL "<p>"
									"Unmap: " HTML_URL2, url, title, path1, path2, "/unmap.ps3", url, title);
				}
				else
				{
					sprintf(param, "Unmap: " HTML_URL, url, title);
				}
			}
			else
				sprintf(param, "%s: %s %s", STR_ERROR, (isremap ? path2 : path1), STR_NOTFOUND);
		}

		if(!mc) keep_alive = http_response(conn_s, header, param, CODE_HTTP_OK, param);

		goto exit_handleclient_www;
	}
	else if(islike(param, "/app_home.ps3"))
	{
		if(!cobra_version || syscalls_removed) goto exit_nocobra_error;

		char *path = param + 13; if(*path == '?') ++path;

		if(*path == '/')
		{
			check_path_alias(path);
			set_app_home(path);
		}
		else
		{
			unmap_app_home();
			strcpy(path, "/app_home");
		}

		sprintf(header, HTML_URL, "/app_home" , "/app_home"); strcat(header, " => ");
		if(!mc) keep_alive = http_response(conn_s, header, path, CODE_PREVIEW_FILE, path);

		goto exit_handleclient_www;
	}
#endif // #ifdef COBRA_ONLY
