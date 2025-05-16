	if(is_setup)
	{
		// /setup.ps3?          reset webman settings
		// /setup.ps3?<params>  save settings

		if(strchr(param, '&') == NULL)
		{
			reset_settings();
		}
		if(save_settings() == CELL_FS_SUCCEEDED)
		{
			_concat2(&sbuffer, "<br>", STR_SETTINGSUPD);
		}
		else
			_concat(&sbuffer, STR_ERROR);
	}
	else
	if(islike(param, "/setup.ps3"))
	{
		// /setup.ps3    setup form with webman settings
		// /setup.ps3!   reload webman settings and show setup form
		// /setup.ps3@<pos>=<value>|<pos>:<text>|...  set configuration and show setup form
		#ifndef LITE_EDITION
		if(param[10] == '!')
		{
			read_settings();
		}
		if(param[10] == '@')
		{
			char *sep, *pos = param + 11; u16 offset; bool alpha;
			while(*pos)
			{
				offset = (u16)val(pos);
				sep = strchr(pos, '='); alpha = !sep;
				if(alpha) sep = strchr(pos, ':'); if(!sep) break; pos = sep;
				sep = strchr(++pos, '|'); if(sep) *sep = NULL;

				if(alpha)
					strcpy((char*)(wmconfig) + offset, pos);
				else
				{
					wmconfig[offset] = (u8)val(pos);
					if(offset == 595) profile = wmconfig[offset];
				}

				if(!sep) break; pos = sep + 1;
			}
			save_settings();
		}
		#endif
		setup_form(pbuffer, templn);
	}
	else