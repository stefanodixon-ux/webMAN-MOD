		{
			char cpursx[32]; get_cpursx(cpursx);

			sprintf(html, " [<a href=\"%s\">"
							// prevents flickering but cause error 80710336 in ps3 browser (silk mode)
							//"<span id=\"lbl_cpursx\">%s</span></a>]<iframe src=\"/cpursx_ps3\" style=\"display:none;\"></iframe>"
							"<span id=\"err\" style=\"display:none\">%s&nbsp;</span>%s</a>]"
							"<script>function no_error(o){try{var doc=o.contentDocument||o.contentWindow.document;}catch(e){err.style.display='inline-block';o.style.display='none';}}</script>"
							//
							"<hr width=\"100%%\">"
							"<div id=\"rxml\"><H1>%s XML ...</H1></div>"
							"<div id=\"rhtm\"><H1>%s HTML ...</H1></div>"
			#ifdef COPY_PS3
							"<div id=\"rcpy\"><H1><a href=\"/copy.ps3$abort\">&#9746;</a> %s ...</H1></div>"
							//"<form action=\"\">", cpursx, STR_REFRESH, STR_REFRESH, STR_COPYING); _concat(&sbuffer, html);
							"<form action=\"\">", "/cpursx.ps3", cpursx, is_ps3_http ? cpursx : "<iframe src=\"/cpursx_ps3\" style=\"border:0;overflow:hidden;\" width=\"230\" height=\"23\" frameborder=\"0\" scrolling=\"no\" onload=\"no_error(this)\"></iframe>", STR_REFRESH, STR_REFRESH, STR_COPYING);
			#else
							//"<form action=\"\">", cpursx, STR_REFRESH, STR_REFRESH); _concat(&sbuffer, html);
							"<form action=\"\">", "/cpursx.ps3", cpursx, is_ps3_http ? cpursx : "<iframe src=\"/cpursx_ps3\" style=\"border:0;overflow:hidden;\" width=\"230\" height=\"23\" frameborder=\"0\" scrolling=\"no\" onload=\"no_error(this)\"></iframe>", STR_REFRESH, STR_REFRESH);
			#endif
			_concat(&sbuffer, html);
		}

skip_code1:
		if((webman_config->homeb) && (webman_config->home_url[0] > 0))
		{sprintf(html, HTML_BUTTON_FMT, HTML_BUTTON, STR_HOME, HTML_ONCLICK, webman_config->home_url); _concat(&sbuffer, html);}

		sprintf(html, HTML_BUTTON_FMT2
						HTML_BUTTON_FMT

						#ifdef EXT_GDATA
						HTML_BUTTON_FMT
						#endif

						, HTML_BUTTON, "Stat", HTML_ONCLICK, "/stat.ps3", param
						, HTML_BUTTON, STR_UNMOUNT, HTML_ONCLICK, "/mount.ps3/unmount"

						#ifdef EXT_GDATA
						, HTML_BUTTON, "gameDATA", HTML_ONCLICK, "/extgd.ps3"
						#endif
		);

		_concat(&sbuffer, html);

skip_code2:
		#ifdef COPY_PS3
		if(((islike(param, "/dev_") && strlen(param) > 12 && !strstr(param,"?")) || islike(param, "/dev_bdvd")) && !strstr(param,".ps3/") && !strstr(param,".ps3?"))
		{
			if(copy_in_progress)
				sprintf(html, "%s&#9746; %s\" %s'%s';\">", HTML_BUTTON, STR_COPY, HTML_ONCLICK, "/copy.ps3$abort");
			else
				sprintf(html, "%s%s\" onclick='rcpy.style.display=\"block\";location.href=\"/copy.ps3%s\";'\">", HTML_BUTTON, STR_COPY, param);

			_concat(&sbuffer, html);
		}

		if((islike(param, "/dev_") && !strstr(param,"?")) && !islike(param,"/dev_flash") && !strstr(param,".ps3/") && !strstr(param,".ps3?"))
		{	// add buttons + javascript code to handle delete / cut / copy / paste (requires fm.js)
			if(file_exists(FM_SCRIPT_JS))
			{
				sprintf(html, "%s%s\" id=\"%s\" onclick=\"tg(this,'%s','%s','#%x');\">", HTML_BUTTON, STR_DELETE, "bDel", "/delete.ps3", STR_DELETE, ORANGE); _concat(&sbuffer, html);
				sprintf(html, "%s%s\" id=\"%s\" onclick=\"tg(this,'%s','%s','#%x');\">", HTML_BUTTON, "Cut", "bCut", "/cut.ps3", "Cut", MAGENTA); _concat(&sbuffer, html);
				sprintf(html, "%s%s\" id=\"%s\" onclick=\"tg(this,'%s','%s','#%x');\">", HTML_BUTTON, "Copy", "bCpy", "/cpy.ps3", "Copy", CYAN); _concat(&sbuffer, html);

				if(cp_mode) {char *url = tempstr, *title = tempstr + MAX_PATH_LEN; urlenc(url, param); htmlenc(title, cp_path, 0); sprintf(html, "%s%s\" id=\"bPst\" %s'/paste.ps3%s'\" title=\"%s\">", HTML_BUTTON, "Paste", HTML_ONCLICK, url, title); _concat(&sbuffer, html);}
			}
		}
		#endif // #ifdef COPY_PS3

		if(webman_config->sman || strstr(param, "/sman.ps3")) {_concat(&sbuffer, "</div>"); goto continue_rendering;}

		sprintf(html, "%s%s XML%s\" %s'%s';\"> "
					  "%s%s HTML%s\" %s'%s';\">",
					  HTML_BUTTON, STR_REFRESH, SUFIX2(profile), HTML_ONCLICK, "/refresh.ps3';$$('rxml').style.display='block",
					  HTML_BUTTON, STR_REFRESH, SUFIX2(profile), HTML_ONCLICK, "/index.ps3?html';$$('rhtm').style.display='block");

		_concat(&sbuffer, html);

		#ifdef SYS_ADMIN_MODE
		if(sys_admin)
		#endif
		{
			sprintf(html, HTML_BUTTON_FMT
						  HTML_BUTTON_FMT,
						  HTML_BUTTON, STR_SHUTDOWN, HTML_ONCLICK, "/shutdown.ps3",
						  HTML_BUTTON, STR_RESTART,  HTML_ONCLICK, "/restart.ps3");

			_concat(&sbuffer, html);
		}

		#ifndef LITE_EDITION
		if(!get_flag(param, "$nobypass")) { PS3MAPI_REENABLE_SYSCALL8 }
		#endif

		_concat(&sbuffer, "</form><hr>");

#ifndef LITE_EDITION
continue_rendering:
#endif
		#ifdef COPY_PS3
		if(copy_in_progress)
		{
			get_copy_stats(html + 100, "");
			sprintf(html, "%s<a href=\"%s$abort\">&#9746 %s</a> %s", "<div id=\"cps\"><font size=2>", "/copy.ps3", STR_COPYING, html + 100);
		}
		else if(fix_in_progress)
		{
			sprintf(html, "%s<a href=\"%s$abort\">&#9746 %s</a> %s (%i %s)", "<div id=\"cps\"><font size=2>", "/fixgame.ps3", STR_FIXING, current_file, fixed_count, STR_FILES);
		}
		if((copy_in_progress || fix_in_progress) && file_exists(current_file))
		{
			strcat(html, "</font><p></div><script>setTimeout(function(){cps.style.display='none'},15000);</script>"); _concat(&sbuffer, html);
		}
		#endif

		keep_alive = 0;
