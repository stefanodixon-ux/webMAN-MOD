	#ifdef FIX_CLOCK
	if(islike(param, "/fixclock.ps3"))
	{
		char *param2 = param + 14;
		if(param[13] != '?') update_clock_from_server_time(param2); else fix_clock(param2);
		strcopy(param, "/cpursx.ps3");
	}
	#endif
	if(islike(param, "/cpursx_ps3"))
	{
		char *cpursx = header, *buffer = param; get_cpursx(cpursx);
 /*		// prevents flickering but cause error 80710336 in ps3 browser (silk mode)
		sprintf(buffer, "%s15;URL=%s\">"
						"<script>parent.document.getElementById('lbl_cpursx').innerHTML = \"%s\";</script>",
						HTML_REFRESH, "/cpursx_ps3", cpursx);
 */
		sprintf(buffer, "%s15;URL=%s\">"
						"%s"
						"<a href=\"%s\" target=\"_parent\" style=\"text-decoration:none;\">"
						"<font color=\"#fff\">%s</a>",
						HTML_REFRESH, "/cpursx_ps3",
						HTML_BODY, "/cpursx.ps3", cpursx);

		int buf_len = sprintf(header, HTML_RESPONSE_FMT,
									  CODE_HTTP_OK, "/cpursx_ps3", HTML_HEADER, buffer, HTML_BODY_END);

		send(conn_s, header, buf_len, 0); keep_alive = 0;

		goto exit_handleclient_www;
	}
	else if(islike(param, "/tempc.html") || islike(param, "/tempf.html"))
	{
		u8 t1 = 0, t2 = 0;
		get_temperature(0, &t1); // CPU // 3E030000 -> 3E.03°C -> 62.(03/256)°C
		get_temperature(1, &t2); // RSX

		u8 st, mode, unknown;
		sys_sm_get_fan_policy(0, &st, &mode, &fan_speed, &unknown);

		sprintf(header, "function setGaugeValues(){"
						"cpu=%i;"
						"rsx=%i;"
						"fan=%i;"
						"}", t1, t2, fan_speed * 100 / 255);

		save_file(HTML_BASE_PATH "/temp.js", header, SAVE_ALL);
	}
