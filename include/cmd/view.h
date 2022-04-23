#ifdef VIEW_PARAM_SFO
			if(islike(param, "/view.ps3"))
			{
				char *filename = param + 9;
				char *sfo = strstr(filename, ".SFO");
				char *patch = NULL; if(sfo) {if((patch = strchr(sfo, '?'))) *patch++ = NULL;}
				if(is_ext(filename, ".SFO"))
				{
					char param_sfo[_4KB_], out[_8KB_];
					u16 sfo_size = read_sfo(filename, param_sfo);

					// backup original PARAM.SFO
					if(patch)
						{sprintf(out, "%s.bak", filename); save_file(out, param_sfo, sfo_size);}

					// show & patch PARAM.SFO
					sprintf(out, "<style>a{%s;cursor:pointer}</style>", HTML_URL_STYLE);
					add_breadcrumb_trail(out, filename); concat(out, "<hr>");
					get_param_sfo((unsigned char *)param_sfo, patch, out + strlen(out), sfo_size);

					// save new PARAM.SFO
					if(patch)
						save_file(filename, param_sfo, sfo_size);

					// show PARAM.SFO
					keep_alive = http_response(conn_s, header, param, CODE_HTTP_NOCSS, out);
					goto exit_handleclient_www;
				}
				else
					{strcpy(header, filename); sprintf(param, "%s%s", "/hexview.ps3", header);}
			}
#endif
