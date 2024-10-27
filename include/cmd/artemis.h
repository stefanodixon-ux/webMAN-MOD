#ifdef ARTEMIS_PRX
	if(islike(param, "/artemis.ps3"))
	{
		// /artemis.ps3                        start artermis & open codelist file
		// /artemis.ps3<ncl-file>              start artermis & open codelist file (up to 1300 bytes)
		// /artemis.ps3?f=<ncl-file>&t=<code>  start artermis & saves codelist to file
		// /artemis.ps3?attach                 start artermis & attach game process
		// /artemis.ps3?detach                 start artermis & detach game process

		const char *code_list = param + 12;

		if(*code_list == '?')
			art_cmd = (param[13] == 'a') ? 1 :
					  (param[13] == 'd') ? 2 :  0;
		else if(*code_list == '/')
		{
			if(!islike(code_list, ARTEMIS_CODES))
				init_codelist(param + 12);
		}

		if((*code_list != '/') || strstr(code_list, ".ncl"))
			code_list = ARTEMIS_CODES_FILE;

		concat2(param, "/edit.ps3", code_list);

		start_artemis(); sys_ppu_thread_sleep(1);

		is_popup = 1;
		goto html_response;
	}
#endif