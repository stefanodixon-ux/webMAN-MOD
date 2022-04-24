#ifdef VIEW_PARAM_SFO
			if(islike(param, "/view.ps3"))
			{
				char *filename = param + 9;
				char *sfo = strstr(filename, ".SFO");
				char *patch = NULL; if(sfo) {if((patch = strchr(sfo, '?'))) *patch++ = NULL;}
				if(is_ext(filename, ".SFO"))
				{
					char param_sfo[_4KB_], out[_8KB_], *tmp = header;
					u16 sfo_size = read_sfo(filename, param_sfo);

					// backup original PARAM.SFO
					if(patch)
						{sprintf(out, "%s.bak", filename); save_file(out, param_sfo, sfo_size);}

					// add header & style sheet
					sprintf(tmp, "<style>a{%s;cursor:pointer}</style>", HTML_URL_STYLE);
					u16 len = sprintf(out, HTML_RESPONSE_FMT,
									  CODE_HTTP_OK, param, HTML_BODY, HTML_RESPONSE_TITLE, tmp);

					// add ICON0.PNG
					copy_path(tmp, filename);
					sprintf(out + len,	"<img src=\"%s/ICON0.PNG\" "
										"onerror=\"this.style.display='none';\"> ", tmp);

					// show & patch PARAM.SFO
					add_breadcrumb_trail(out, filename); concat(out, "<hr>");
					get_param_sfo((unsigned char *)param_sfo, patch, out + fast_concat.size, sfo_size);

					// save new PARAM.SFO
					if(patch)
						save_file(filename, param_sfo, sfo_size);

					// add footer
					sprintf(tmp, "<hr>" HTML_BUTTON_FMT "%s",
								 HTML_BUTTON, " &#9664;  ", HTML_ONCLICK, "javascript:history.back();", HTML_BODY_END);
					concat(out, tmp);

					// show PARAM.SFO
					keep_alive = http_response(conn_s, header, param, CODE_HTTP_NOCSS, out);
					goto exit_handleclient_www;
				}
				else
					{strcpy(header, filename); sprintf(param, "%s%s", "/hexview.ps3", header);}
			}
#endif
