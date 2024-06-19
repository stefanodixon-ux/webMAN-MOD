	if(islike(param, "/reloadxmb.ps3") && refreshing_xml == 0)
	{
		reload_xmb(param[14]);
		sprintf(param, "/index.ps3");
	}
