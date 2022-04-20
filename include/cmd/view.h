#ifdef VIEW_PARAM_SFO
			if(islike(param, "/view.ps3"))
			{
				char *filename = param + 9;
				if(is_ext(filename, ".SFO"))
				{
					char param_sfo[_4KB_], out[_6KB_];
					unsigned char *mem = (u8*)param_sfo;
					u16 sfo_size = read_sfo(filename, param_sfo);
					get_param_sfo(mem, NULL, out, 0, sfo_size);
					keep_alive = http_response(conn_s, header, param, CODE_HTTP_NOCSS, out);
					goto exit_handleclient_www;
				}
				else
					{strcpy(header, filename); sprintf(param, "%s%s", "/hexview.ps3", header);}
			}
#endif
