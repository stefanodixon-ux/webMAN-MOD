#ifndef LITE_EDITION
	if(is_popup)
	{
		if(islike(param, "/edit.ps3"))
		{
			// /edit.ps3<file>              open text file (up to 2000 bytes)
			// /edit.ps3?f=<file>&t=<txt>   saves text to file

			char *filename = templn, *txt = buffer + BUFFER_SIZE_HTML - _6KB_, *backup = txt; memset(txt, 0, _2KB_); *filename = 0;

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

			// show text box
			sprintf(tempstr,"<form action=\"/edit.ps3\">"
							"<input type=hidden name=\"f\" value=\"%s\">"
							"<textarea name=\"t\" maxlength=%i style=\"width:800px;height:400px;\">%s</textarea><br>"
							"<input accesskey=\"S\" class=\"bs\" type=\"submit\" value=\" %s \">",
							filename, MAX_TEXT_LEN, txt, STR_SAVE); _concat(&sbuffer, tempstr);

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
			char *msg = (param + 11); // /popup.ps3?<msg>

			if(param[10] == '*')
				show_msg2(msg);
			else if(param[10] == '@')
			{
				overlay = 1;
				show_progress(msg, OV_SHOW);
			}
			else
				show_msg(msg);

			sprintf(templn, "Message sent: %s", msg); _concat(&sbuffer, templn);
		}

		loading_html = keep_alive = is_popup = 0; goto send_response;
	}
#endif // #ifndef LITE_EDITION
