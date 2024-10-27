#ifdef VIEW_PARAM_SFO
			if(islike(param, "/view.ps3"))
			{
				const char *filename = param + 9;
				const char *sfo = strstr(filename, ".SFO");
				if(sfo)
				{
					char *patch = strchr(sfo, '?'); if(patch) *patch++ = NULL;

					char param_sfo[_4KB_], out[_8KB_], *tmp = header;
					u16 sfo_size = read_sfo(filename, param_sfo);

					// backup original PARAM.SFO
					if(patch)
						{concat2(out, filename, ".bak"); save_file(out, param_sfo, sfo_size);}

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
				else if(isDir(filename))
					strcpy(param, filename);
				else
					prepend(param, "/hexview.ps3", 3);
			}
#endif
