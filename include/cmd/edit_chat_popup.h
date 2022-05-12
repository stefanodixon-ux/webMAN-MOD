#ifndef LITE_EDITION
	if(is_popup)
	{
		if(islike(param, "/edit.ps3"))
		{
			// /edit.ps3<file>              open text file (up to 2000 bytes)
			// /edit.ps3?f=<file>&t=<txt>   saves text to file

			char *filename = templn, *txt = buffer + BUFFER_SIZE_HTML - _6KB_, *backup = txt; _memset(txt, _2KB_); *filename = 0;

			// get file name
			get_value(filename, param + ((param[9] == '/') ? 9 : 12), MAX_PATH_LEN); // /edit.ps3<file>  *or* /edit.ps3?f=<file>&t=<txt>

			filepath_check(filename);

			if(*filename != '/')
			{
				sprintf(filename, "/dev_hdd0/boot_plugins.txt"); // default file
			}

			char *pos = strstr(param, "&t=");
			if(pos)
			{
				// backup the original text file
				sprintf(backup, "%s.bak", filename);
				del(backup, 0); // delete previous backup
				rename_file(filename, backup);

				// save text file
				strcpy(txt, pos + 3);
				save_file(filename, txt, SAVE_ALL);
			}
			else
			{
				// load text file
				read_file(filename, txt, MAX_TEXT_LEN, 0);
			}

			#ifdef ARTEMIS_PRX
			if(islike(filename, ARTEMIS_CODES_FILE))
			{
				add_game_info(buffer, tempstr, 0);
			}
			#endif

			// show text box
			sprintf(tempstr,"<form action=\"/edit.ps3\">"
							"<input type=hidden name=\"f\" value=\"%s\">"
							"<textarea name=\"t\" maxlength=%i style=\"width:800px;height:400px;\">%s</textarea><br>"
							"<input accesskey=\"S\" class=\"bs\" type=\"submit\" value=\" %s \">",
							filename, MAX_TEXT_LEN, txt, STR_SAVE); _concat(&sbuffer, tempstr);

			#ifdef ARTEMIS_PRX
			if(islike(filename, ARTEMIS_CODES_FILE))
			{
				_concat(&sbuffer, " [<a href=\"/artemis.ps3?attach\">Attach</a>]"
								  " [<a href=\"/artemis.ps3?detach\">Detach</a>]");
				if(!artemis_working) _concat(&sbuffer, " NOT");
									 _concat(&sbuffer, " RUNNING");
				if(attachedPID) {sprintf(filename, " • ATTACHED"); _concat(&sbuffer, filename);}
				strcpy(filename, "/dev_hdd0/tmp/art.log");
			}
			#endif

			// show filename link
			char *p = get_filename(filename);
			if(p)
			{
				if(is_ext(p, ".bat")
				#ifdef WM_CUSTOM_COMBO
				|| islike(filename, WM_CUSTOM_COMBO)
				#endif
				)
					{sprintf(tempstr," [<a href=\"/play.ps3%s\">EXEC</a>]", filename); _concat(&sbuffer, tempstr);}
				strcpy(txt, p); *p = NULL; sprintf(tempstr," &nbsp; " HTML_URL HTML_URL2 "</form>", filename, filename, filename, txt, txt); _concat(&sbuffer, tempstr);
			}
		}
		else
		#ifdef WEB_CHAT
		if(islike(param, "/chat.ps3"))
		{
			// /chat.ps3    webchat

			webchat(buffer, templn, param, tempstr, conn_info);
		}
		else
		#endif

		{
			u8 mode = param[10];
			char *msg = (param + 11); // /popup.ps3?<msg>

			if(islike(param + 10, "@info")) --msg;
			u8 op = parse_tags(msg);

			if(mode == '*')
				show_msg2(msg);
			#ifdef FPS_OVERLAY
			else if(mode == '@')
			{
				if(op >= 10) overlay_info = op; // set persistent show

				if(*msg)
					show_progress(msg, OV_SHOW);
				else
					disable_progress();
			}
			#endif
			else
				show_msg(msg);

			if(op) _concat(&sbuffer, msg); else _concat2(&sbuffer, "Message sent: ", msg);

			if(mode=='=') sbuffer.size = sprintf(sbuffer.str, "%s", msg); // raw mode
		}

		loading_html = keep_alive = is_popup = 0; goto send_response;
	}
#endif // #ifndef LITE_EDITION
