	if(islike(param, "/recovery.ps3"))
	{
		// /recovery.ps3  reboots in pseudo-recovery mode

		#define SC_UPDATE_MANAGER_IF				863
		#define UPDATE_MGR_PACKET_ID_READ_EPROM		0x600B
		#define UPDATE_MGR_PACKET_ID_WRITE_EPROM	0x600C
		#define RECOVER_MODE_FLAG_OFFSET			0x48C61

		{system_call_7(SC_UPDATE_MANAGER_IF, UPDATE_MGR_PACKET_ID_WRITE_EPROM, RECOVER_MODE_FLAG_OFFSET, 0x00, 0, 0, 0, 0);} // set recovery mode
		goto reboot; // hard reboot
	}
