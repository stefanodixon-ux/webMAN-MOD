	if(islike(param, "/reloadxmb.ps3") && refreshing_xml == 0)
	{
		reload_xmb();
		sprintf(param, "/index.ps3");
	}
