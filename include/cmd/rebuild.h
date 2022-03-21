	if(islike(param, "/rebuild.ps3"))
	{
		// /rebuild.ps3  reboots & start rebuilding file system

		cmd[0] = cmd[1] = 0; cmd[2] = 0x03; cmd[3] = 0xE9; // 00 00 03 E9
		save_file("/dev_hdd0/mms/db.err", cmd, 4);
		goto reboot; // hard reboot
	}
