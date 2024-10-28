#ifdef WEB_CHAT

#define WMCHATFILE			"/dev_hdd0/tmp/wmtmp/wmchat.htm"

#ifndef LITE_EDITION
static void webchat(char *buffer, char *html, char *param, char *tempstr, sys_net_sockinfo_t conn_info_main)
{
	struct CellFsStat buf;

	int fd, size = 0;

	// truncate msg log
	if(cellFsStat(WMCHATFILE, &buf) != CELL_FS_SUCCEEDED || buf.st_size > _32KB_ || buf.st_size == 0)
	{
		_memset(tempstr, _4KB_);

		if(buf.st_size > _32KB_)
		{
			read_file(WMCHATFILE, tempstr, 4080, (buf.st_size - 4080);
		}

		if(cellFsOpen(WMCHATFILE, CELL_FS_O_CREAT | CELL_FS_O_WRONLY | CELL_FS_O_TRUNC | CELL_FS_O_APPEND, &fd, NULL, 0) == CELL_FS_SUCCEEDED)
		{
			size = sprintf(html, "%s10\">"
								 "<body bgcolor=\"#101010\" text=\"#c0c0c0\">"
								 "<script>window.onload=toBottom;function toBottom(){window.scrollTo(0, document.body.scrollHeight);}"
								 "</script>\0", HTML_REFRESH);

			if(*tempstr) {strcat(html, "<!--"); size += 4;}

			cellFsWrite(fd, html, size, NULL);
			cellFsWrite(fd, tempstr, size, NULL);
			cellFsClose(fd);
		}
	}

	// append msg
	char msg[200], user[20]="guest\0", *pos; *msg = NULL;
	if(conn_info_main.remote_adr.s_addr==0x7F000001) strcpy(user,"console\0");
	if(islike(param, "/chat.ps3?"))
	{
		pos = strstr(param, "u="); if(pos) get_value(user, pos + 2, 20);
		pos = strstr(param, "m="); if(pos) get_value(msg , pos + 2, 200);

		size = sprintf(html, "<font color=\"red%s\"><b>%s</b></font><br>%s<br><!---->", user, user, msg);

		if(cellFsOpen(WMCHATFILE, CELL_FS_O_RDWR | CELL_FS_O_CREAT | CELL_FS_O_APPEND, &fd, NULL, 0) == CELL_OK)
		{
			cellFsWrite(fd, html, size, NULL);
		}
		cellFsClose(fd);

		if(conn_info_main.remote_adr.s_addr != 0x7F000001) show_msg(msg);
	}

	// show msg log
	sprintf(html, "<iframe src=\"%s\" width=\"99%%\" height=\"300\"></iframe>", WMCHATFILE); strcat(buffer, html);

	// prompt msg
	sprintf(html, "<hr>"
					"<form name=\"f\" action=\"\">"
					HTML_INPUT("u", "%s", "10", "5") ":" HTML_INPUT("m", "", "500", "80")
					"<input type=submit value=\"send\">"
					"</form><script>f.m.focus();</script>", user); strcat(buffer, html);
}

#endif
#endif
