		{
			char cpursx[32]; get_cpursx(cpursx);

			sprintf(templn, " [<a href=\"%s\">"
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
							//"<form action=\"\">", cpursx, STR_REFRESH, STR_REFRESH, STR_COPYING); _concat(&sbuffer, templn);
							"<form action=\"\">", "/cpursx.ps3", cpursx, is_ps3_http ? cpursx : "<iframe src=\"/cpursx_ps3\" style=\"border:0;overflow:hidden;\" width=\"230\" height=\"23\" frameborder=\"0\" scrolling=\"no\" onload=\"no_error(this)\"></iframe>", STR_REFRESH, STR_REFRESH, STR_COPYING);
			#else
							//"<form action=\"\">", cpursx, STR_REFRESH, STR_REFRESH); _concat(&sbuffer, templn);
							"<form action=\"\">", "/cpursx.ps3", cpursx, is_ps3_http ? cpursx : "<iframe src=\"/cpursx_ps3\" style=\"border:0;overflow:hidden;\" width=\"230\" height=\"23\" frameborder=\"0\" scrolling=\"no\" onload=\"no_error(this)\"></iframe>", STR_REFRESH, STR_REFRESH);
			#endif
			_concat(&sbuffer, templn);
		}

		if((webman_config->homeb) && (strlen(webman_config->home_url)>0))
		{sprintf(templn, HTML_BUTTON_FMT, HTML_BUTTON, STR_HOME, HTML_ONCLICK, webman_config->home_url); _concat(&sbuffer, templn);}

		sprintf(templn, HTML_BUTTON_FMT
						HTML_BUTTON_FMT
						HTML_BUTTON_FMT

						#ifdef EXT_GDATA
						HTML_BUTTON_FMT
						#endif

						, HTML_BUTTON, STR_EJECT, HTML_ONCLICK, "/eject.ps3"
						, HTML_BUTTON, STR_INSERT, HTML_ONCLICK, "/insert.ps3"
						, HTML_BUTTON, STR_UNMOUNT, HTML_ONCLICK, "/mount.ps3/unmount"

						#ifdef EXT_GDATA
						, HTML_BUTTON, "gameDATA", HTML_ONCLICK, "/extgd.ps3"
						#endif
		);

		_concat(&sbuffer, templn);

skip_code:
		#ifdef COPY_PS3
		if(((islike(param, "/dev_") && strlen(param) > 12 && !strstr(param,"?")) || islike(param, "/dev_bdvd")) && !strstr(param,".ps3/") && !strstr(param,".ps3?"))
		{
			if(copy_in_progress)
				sprintf(templn, "%s&#9746; %s\" %s'%s';\">", HTML_BUTTON, STR_COPY, HTML_ONCLICK, "/copy.ps3$abort");
			else
				sprintf(templn, "%s%s\" onclick='rcpy.style.display=\"block\";location.href=\"/copy.ps3%s\";'\">", HTML_BUTTON, STR_COPY, param);

			_concat(&sbuffer, templn);
		}

		if((islike(param, "/dev_") && !strstr(param,"?")) && !islike(param,"/dev_flash") && !strstr(param,".ps3/") && !strstr(param,".ps3?"))
		{	// add buttons + javascript code to handle delete / cut / copy / paste (requires fm.js)
			#ifdef EMBED_JS
			sprintf(templn, "<script>"
							"function tg(b,m,x,c){"
							"var i,p,o,h,l=document.querySelectorAll('.d,.w'),s=m.length,n=1;"
							"for(i=1;i<l.length;i++){o=l[i];"
							"h=o.href;p=h.indexOf('/cpy.ps3');if(p>0){n=0;s=8;bCpy.value='Copy';}"
							"if(p<1){p=h.indexOf('/cut.ps3');if(p>0){n=0;s=8;bCut.value='Cut';}}"
							"if(p<1){p=h.indexOf('/delete.ps3');if(p>0){n=0;s=11;bDel.value='%s';}}"
							"if(p>0){o.href=h.substring(p+s,h.length);o.style.color='#ccc';}"
							"else{p=h.indexOf('/',8);o.href=m+h.substring(p,h.length);o.style.color=c;}"
							"}if(n)b.value=(b.value == x)?x+' %s':x;"
							"}</script>", STR_DELETE, STR_ENABLED); _concat(&sbuffer, templn);
			#else
			if(file_exists(FM_SCRIPT_JS))
			#endif
			{
				sprintf(templn, "%s%s\" id=\"bDel\" onclick=\"tg(this,'%s','%s','red');\">", HTML_BUTTON, STR_DELETE, "/delete.ps3", STR_DELETE); _concat(&sbuffer, templn);
				sprintf(templn, "%s%s\" id=\"bCut\" onclick=\"tg(this,'%s','%s','magenta');\">", HTML_BUTTON, "Cut", "/cut.ps3", "Cut"); _concat(&sbuffer, templn);
				sprintf(templn, "%s%s\" id=\"bCpy\" onclick=\"tg(this,'%s','%s','blue');\">", HTML_BUTTON, "Copy", "/cpy.ps3", "Copy"); _concat(&sbuffer, templn);

				if(cp_mode) {char *url = tempstr, *title = tempstr + MAX_PATH_LEN; urlenc(url, param); htmlenc(title, cp_path, 0); sprintf(templn, "%s%s\" id=\"bPst\" %s'/paste.ps3%s'\" title=\"%s\">", HTML_BUTTON, "Paste", HTML_ONCLICK, url, title); _concat(&sbuffer, templn);}
			}
		}
		#endif // #ifdef COPY_PS3

		if(webman_config->sman || strstr(param, "/sman.ps3")) {_concat(&sbuffer, "</div>"); goto continue_rendering;}

		sprintf(templn,  "%s%s XML%s\" %s'%s';\"> "
						 "%s%s HTML%s\" %s'%s';\">",
						 HTML_BUTTON, STR_REFRESH, SUFIX2(profile), HTML_ONCLICK, "/refresh.ps3';document.getElementById('rxml').style.display='block",
						 HTML_BUTTON, STR_REFRESH, SUFIX2(profile), HTML_ONCLICK, "/index.ps3?html';document.getElementById('rhtm').style.display='block");

		_concat(&sbuffer, templn);

		#ifdef SYS_ADMIN_MODE
		if(sys_admin)
		#endif
		{
			sprintf(templn,  HTML_BUTTON_FMT
							 HTML_BUTTON_FMT,
							 HTML_BUTTON, STR_SHUTDOWN, HTML_ONCLICK, "/shutdown.ps3",
							 HTML_BUTTON, STR_RESTART,  HTML_ONCLICK, "/restart.ps3");

			_concat(&sbuffer, templn);
		}

		#ifndef LITE_EDITION
		char *nobypass = strstr(param, "$nobypass");
		if(!nobypass) { PS3MAPI_REENABLE_SYSCALL8 } else *nobypass = NULL;
		#endif

		sprintf( templn, "</form><hr>");
		_concat(&sbuffer, templn);

#ifndef LITE_EDITION
continue_rendering:
#endif
		#ifdef COPY_PS3
		if(copy_in_progress)
		{
			sprintf(templn, "%s<a href=\"%s$abort\">&#9746 %s</a> %s (%i %s)", "<div id=\"cps\"><font size=2>", "/copy.ps3", STR_COPYING, current_file, copied_count, STR_FILES);
		}
		else if(fix_in_progress)
		{
			sprintf(templn, "%s<a href=\"%s$abort\">&#9746 %s</a> %s (%i %s)", "<div id=\"cps\"><font size=2>", "/fixgame.ps3", STR_FIXING, current_file, fixed_count, STR_FILES);
		}
		if((copy_in_progress || fix_in_progress) && file_exists(current_file))
		{
			strcat(templn, "</font><p></div><script>setTimeout(function(){cps.style.display='none'},15000);</script>"); _concat(&sbuffer, templn);
		}
		#endif

		keep_alive = 0;
