	if(islike(param, "/edit.ps3"))
	{
		// /edit.ps3<file>              open text file (up to 2000 bytes)
		// /edit.ps3?f=<file>&t=<txt>   saves text to file

		is_popup = 1;
		goto html_response;
	}
