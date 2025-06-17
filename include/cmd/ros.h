#ifdef PATCH_ROS
	if(islike(param, "/ros.ps3"))
	{
		// /ros.ps3                             <= show info only
		// /ros.ps3<path-nofsm-patch>           <= minimal patch ros0 & ros1 (5.5MB + 5.5MB)
		// /ros.ps3<path-nofsm-patch>?ros0      <= minimal patch ros0
		// /ros.ps3<path-nofsm-patch>?ros1      <= minimal patch ros1
		// /ros.ps3<path-nofsm-patch>?full      <= full patch ros0 & ros1 + verify data (7MB + 7MB)
		// /ros.ps3<path-nofsm-patch>?ros0?full <= full patch ros0 only   + verify data
		// /ros.ps3<path-nofsm-patch>?ros1?full <= full patch ros0 only   + verify data
		// /ros.ps3<path-nofsm-patch>?cfw       <= force patch on cfw + minimal patch ros0 & ros1
		// /ros.ps3<path-nofsm-patch>?doit      <= force patch on cfw + if md5 not match + minimal patch ros0 & ros1
		// /ros.ps3<path-nofsm-patch>?doit?full <= force patch on cfw + if md5 not match +full patch ros0 & ros1 + verify data

		// Verifies: 
		// 1. NAND / NOR flash type
		// 2. Applicable Version must be 3.56 or lower
		// 3. Should not be CFW (can be skipped with ?cfw or ?doit)
		// 4. nofsm patch file must exist with a size of 7MB (7340000 bytes) / detect it in root of storage devices
		// 5. MD5 of nofsm patch file must match current FW version (can be skipped with ?doit)
		// 6. Ensures that metldr is not metldr.2
		// 7. Writes minimal patch by default (5.5MB = 0x2C80 sectors) (can write full ros with ?full)
		// 8. Verifies that current ROS is valid
		// 9. Verifies written data (full mode only)
		// 10. Both ROS are written (can be forced to a single ROS using ?ros0 or ?ros1)

		char *filename = param + 8;
		if(IS(filename, "?stop"))
			stop_ros = ros_patching; // true
		else
		{
			sprintf(header, HTML_REFRESH "0;URL=/patchros.ps3%s\">"
							"<h1>Patching ROS...please wait", filename); strcpy(param, header);
		}
		keep_alive = http_response(conn_s, header, "/patchros.ps3", CODE_HTTP_OK, param);
		goto exit_handleclient_www;
	}
	else if(islike(param, "/patchros.ps3"))
	{
		// /patchros.ps3 <= use same parameters as /ros.ps3

		char *filename = param + 13, *buffer = header;

		bool full = false;
		if(get_flag(filename, "?full")) full = true;

		u8 ros = 2; // patch both ros by default
		if(get_flag(filename, "?ros0")) ros = 0;
		if(get_flag(filename, "?ros1")) ros = 1;

		bool review = false;
		bool doit = get_flag(filename, "?doit");
		bool allow_cfw = get_flag(filename, "?cfw") || payload_ps3hen || doit;

		int len = sprintf(buffer, "%s ", IS_NAND ? "NAND" : "NOR");
		get_sys_info(buffer + len, 30, false); strcat(buffer, "<p>");

		u8 data[0x20];
		_memset(data, sizeof(data));

		int ret = GetApplicableVersion(data);
		sprintf(buffer + strlen(buffer), "Minimum Downgrade: %x.%02x", data[1], data[3]);

		int fw_ver = (int)(c_firmware * 100);

		// find default nofsm_patch for current firmware version
		if(*filename != '/')
		{
			sprintf(filename, "/nofsm_patch_%i.bin", fw_ver);

			check_path_alias(filename); review = true;
		}

		// check if nofsm_patch exists
		if(not_exists(filename))
			sprintf(filename, "  %s", STR_NOTFOUND);

		// show file path of nofsm_patch
		add_breadcrumb_trail2(buffer, "<p>Patch:", filename);
		strcat(buffer, "<p>"); 

		if(ret)
			sprintf(param, "Applicable Version failed: %x\n", ret);
		else if((data[1] >= 0x04) || (data[1] == 0x03 && data[3] > 0x56) || (BETWEEN(480, fw_ver, 492) == false))
			sprintf(param, "CFW not supported!");
		else if(!allow_cfw)
			sprintf(param, "Already on CFW!");
		else if(sys_admin)
		{
			ret = FAILED;

			// calculate md5 of nofsm_patch
			char md5[33];
			calc_md5(filename, md5);
			u64 sz = file_size(filename);

			// abort if nofsm_patch.bin size is not 7MB
			if(sz != 7340000)
				doit = false;
			else if(!doit)
			{
				const char *nofsm_md5[13] = {
					"d8bb229d2d802acccc0345031ee01d83", // nofsm_patch_480.bin
					"0bbb87121684b43e6727a459365f04ff", // nofsm_patch_481.bin
					"32b8c0f25884933a00d1e924f84b2a68", // nofsm_patch_482.bin
					"2284cfdb950d14ca199ad9469aaa1ec4", // nofsm_patch_483.bin
					"ed1f431ac996d2ab06ea40a7193d4912", // nofsm_patch_484.bin
					"99ffe0a761bee3e0668ba919cbfe4cf4", // nofsm_patch_485.bin
					"0669179877ae23f5c918e6e62e930bac", // nofsm_patch_486.bin
					"1efb54b005b09a10e436f9c76e51d870", // nofsm_patch_487.bin
					"ea9f81a031a7b6701560b3501712df1a", // nofsm_patch_488.bin
					"b9feb4432adb086d890472ab29b663a9", // nofsm_patch_489.bin
					"8cf9701297129151e22eda39c1d2670f", // nofsm_patch_490.bin
					"3c19a6855490b5ed471764962e9fcd83", // nofsm_patch_491.bin
					"36bd44795f06b59eecbdaad6982be426", // nofsm_patch_492.bin
				};
				if(BETWEEN(480, fw_ver, 492))
					doit = IS(md5, nofsm_md5[fw_ver - 480]);
				else
					doit = false;
			}

			sys_addr_t sysmem =  doit ? sys_mem_allocate(_128KB_) : NULL;

			// show info
			char *html = sysmem ? (char *)sysmem : param;
			sprintf(html, "%s<p>MD5: %s<p>Size: %'llu bytes", buffer, md5, sz);
			keep_alive = http_response(conn_s, header, "/ros.ps3", CODE_HTTP_OK, html);

			if(sysmem)
			{
				if(review)
				{
					// show link if enough memory was allocated
					len = sprintf(header, "<h1>[<a href=\"/ros.ps3%s\">Patch</a>]", filename);
					send(conn_s, header, len, 0);
				}
				else
				{
					// patch ros
					u8 *mem = (u8 *)sysmem;
					ret = patch_ros(filename, mem, conn_s, ros, full);
				}

				// free memory buffer
				sys_memory_free(sysmem);
			}

			// show patch result
			if(!review)
				send(conn_s, ret ? "<h1>FAILED!" : "<h1>PATCHED", 11, 0);
			send(conn_s, " [<a href=\"/dump.ps3?flash\">Dump Flash</a>]", 45, 0);

			// close connection & exit
			sclose(&conn_s); loading_html--;
			goto exit_handleclient_www;
		}
		else
			concat_text(param, "ADMIN", STR_DISABLED);

		// show error
		strcat(buffer, param); strcpy(param, buffer);
		keep_alive = http_response(conn_s, header, "/patchros.ps3", CODE_HTTP_OK, param);
		goto exit_handleclient_www;
	}
#endif
