#ifdef ARTEMIS_PRX
	if(islike(param, "/artemis.ps3"))
	{
		// /artemis.ps3                        start artermis & open codelist file
		// /artemis.ps3<ncl-file>              start artermis & open codelist file (up to 1300 bytes)
		// /artemis.ps3?f=<ncl-file>&t=<code>  start artermis & saves codelist to file

		if(param[12] == '/')
		{
			force_copy(param + 12, (char*)ARTEMIS_CODES_FILE);
		}

		sprintf(param, "%s%s", "/edit.ps3", ARTEMIS_CODES_FILE);

		start_artemis();

		is_popup = 1;
		goto html_response;
	}
#endif