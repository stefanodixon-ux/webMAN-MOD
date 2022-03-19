#ifdef PKG_HANDLER
	if(islike(param, "/install.ps3") || islike(param, "/install_ps3"))
	{
		char *pkg_file = param + 12;
		check_path_alias(pkg_file);

		strcat(buffer, "Install PKG: <select autofocus onchange=\"$('wmsg').style.display='block';window.location='/install.ps3");
		strcat(buffer, pkg_file);
		strcat(buffer, "/'+this.value;\"><option>");

		int fd, len;
		if(cellFsOpendir(pkg_file, &fd) == CELL_FS_SUCCEEDED)
		{
			CellFsDirectoryEntry entry; size_t read_e;
			while(working)
			{
				if(cellFsGetDirectoryEntries(fd, &entry, sizeof(entry), &read_e) || !read_e) break;
				len = entry.entry_name.d_namlen -4; if(len < 0) continue;
				if(!strcmp(entry.entry_name.d_name + len, ".pkg") || !strcmp(entry.entry_name.d_name + len, ".p3t"))
				{
					strcat(buffer, "<option>");
					strcat(buffer, entry.entry_name.d_name);
				}
			}
			cellFsClosedir(fd);
		}
		strcat(buffer, "</select>");
	}
	else
#endif
