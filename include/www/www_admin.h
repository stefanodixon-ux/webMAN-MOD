#ifdef SYS_ADMIN_MODE
	if(!sys_admin)
	{
		bool accept = false;
		if(*param != '/') accept = false; else
		if(islike(param, "/admin.ps3")) accept = true; else
		{
			accept = ( IS(param, "/")

					|| islike(param, "/cpursx")

					|| islike(param, "/mount")
					|| islike(param, "/refresh.ps3")
					|| islike(param, "/reloadxmb.ps3")
					|| islike(param, "/index.ps3")
					|| islike(param, "/sman.ps3")
					|| islike(param, "/games.ps3")
					|| islike(param, "/play.ps3")

					|| islike(param, "/eject.ps3")
					|| islike(param, "/insert.ps3")
			#ifdef EXT_GDATA
					|| islike(param, "/extgd.ps3")
			#endif
			#ifdef NOBD_PATCH
					|| islike(param, "/nobd.ps3")
			#endif
			#ifdef WEB_CHAT
					|| islike(param, "/chat.ps3")
			#endif
			#ifndef LITE_EDITION
					|| islike(param, "/popup.ps3")
			#endif
					 );

			if(!accept)
			{
				#ifndef LITE_EDITION
				if(check_password(param)) {is_local = true, sys_admin = 1;}
				#endif

				if(is_local)
					accept = ( islike(param, "/setup.ps3")
					#ifdef PKG_HANDLER
							|| islike(param, "/install")
							|| islike(param, "/download.ps3")
					#endif
					#ifdef PS3_BROWSER
							|| islike(param, "/browser.ps3")
							|| islike(param, "/xmb.ps3")
					#endif
							|| islike(param, "/restart.ps3")
							|| islike(param, "/shutdown.ps3"));
			}

			if(!accept)
			{
				sys_admin = 0;
				const char *ext = get_ext(param);
				if(!accept && (islike(param, "/net") || file_exists(param)) && (_IS(ext, ".jpg") || _IS(ext, ".png") || _IS(ext, ".css") || _IS(ext, ".js") || _IS(ext, "html")) ) accept = true;
			}
		}

		if(!accept)
		{
			concat3(param, STR_ERROR, "\nADMIN ", STR_DISABLED);
			keep_alive = http_response(conn_s, header, param, CODE_BAD_REQUEST, param);
			goto exit_handleclient_www;
		}
	}
#endif // #ifdef SYS_ADMIN_MODE
