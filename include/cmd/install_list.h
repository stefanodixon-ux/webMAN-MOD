#ifdef PKG_HANDLER
	if(islike(param, "/install.ps3") || islike(param, "/install_ps3"))
	{
		char *pkg_file = param + 12;
		check_path_alias(pkg_file);

		add_url(buffer, "Install PKG: "
						"<select autofocus onchange=\"$$('wmsg').style.display='block';"
						"window.location='/install.ps3", pkg_file, "/'+this.value;\">"
						"<option>");

		int fd, len;
		if(cellFsOpendir(pkg_file, &fd) == CELL_FS_SUCCEEDED)
		{
			CellFsDirectoryEntry entry; size_t read_e;
			while(working)
			{
				if(cellFsGetDirectoryEntries(fd, &entry, sizeof(entry), &read_e) || !read_e) break;
				len = entry.entry_name.d_namlen -4; if(len < 0) continue;
				if(_IS(entry.entry_name.d_name + len, ".pkg") || _IS(entry.entry_name.d_name + len, ".p3t"))
				{
					concat(buffer, "<option>");
					concat(buffer, entry.entry_name.d_name);
				}
			}
			cellFsClosedir(fd);
		}
		concat(buffer, "</select>");
	}
	else
#endif
