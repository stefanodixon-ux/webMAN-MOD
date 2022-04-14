	if(islike(param, "/eject.ps3"))
	{
		// /eject.ps3   eject physical disc from bd drive
		eject_insert(1, 0);
		sprintf(templn, HTML_REDIRECT_TO_URL, "javascript:history.back();", HTML_REDIRECT_WAIT);
		_concat2(&sbuffer, STR_EJECTED, templn);
	}
	else
	if(islike(param, "/insert.ps3"))
	{
		// /insert.ps3   insert physical disc into bd drive
		eject_insert(0, 1);
		if(!isDir("/dev_bdvd")) eject_insert(1, 1);
		_concat(&sbuffer, STR_LOADED);
		if(isDir("/dev_bdvd")) {sprintf(templn, HTML_REDIRECT_TO_URL, "/dev_bdvd", HTML_REDIRECT_WAIT); _concat(&sbuffer, templn);}
	}
	else
