#ifdef PATCH_ROS
static bool stop_ros = false;
static bool ros_patching = false;

static int sys_storage_write(int fd, u32 start_sector, u32 sectors, u8 *bounce_buf, u64 *sectors_read)
{
	system_call_7(603, fd, 0, start_sector, sectors, (u64)(u32) bounce_buf, (u64)(u32) sectors_read, FLASH_FLAGS);
	return (int)p1;
}

static int patch_ros(const char *patch_file, u8 *mem, int conn_s, u8 ros, bool full)
{
	if(ros_patching || file_size(patch_file) != 7340000) {BEEP3; return FAILED;}

	ros_patching = true; // prevent call this function while processing

	// open nofsm_patch.bin
	int fd, pos, ret = CELL_OK;
	if(cellFsOpen(patch_file, CELL_FS_O_RDONLY, &fd, NULL, 0) != CELL_FS_SUCCEEDED) {BEEP3; ros_patching = false; return FAILED;}

	// open flash NOR
	const u32 flags = 0x01000000;
	u32 offset1 = 0x10, offset2 = 0x10, metldr_sec = 0x4, r;
	u64 read, start_ros0 = 0x600, start_ros1 = 0x3E00; // NOR ros0 = 0xC0010, ros1 = 0x7C0010
	sys_device_handle_t flash_id;
	if(sys_storage_open(FLASH_DEVICE_NOR, 0, &flash_id, flags) != CELL_OK)
	{
		// open flash NAND
		if(sys_storage_open(FLASH_DEVICE_NAND, 0, &flash_id, flags) != CELL_OK) {BEEP3; cellFsClose(fd); ros_patching = false; return FAILED;}
		offset1 = 0x30, offset2 = 0x20;
		//start_ros0 = 0x600;  // NAND ros0 = 0xC0030  / 0x80030
		//start_ros1 = 0x3E00; //      ros1 = 0x7C0020 / 0x780020
		metldr_sec = 0x204;    //    metldr = 0x40820  / 0x820
	}

	const int sec_size = 0x200; // 512 bytes
	int seg_size = full ? 0x40 : 0x80;  // 64 sectors  / 128 sectors
	int buf_size = seg_size * sec_size; // 32KB (full) / 64KB (default)

	const int header_size = 0x4C0; // 3 sectors
	u32 patch_size = full ? 0x3800 : 0x2C80; // 7MB vs 5.5MB => 0x700000 vs 0x590000 // min clean: 0x2F80 =>0x5F0000
	bool write_data = true;

	u8 *backup  = mem; // full only
	u8 *sectors = mem + (full ? buf_size : 0);
	u8 *written = sectors    + buf_size;
	char *item_name = ((char *)sectors) + 0x20;

	u32 sec, ros_sec, patch_offset; u8 *sector = sectors;

retry_metldr:
	// abort if metldr is not found at 0x820 NOR / 0x40820 NAND (or if metldr.2 is found)
	sys_storage_read(flash_id, 0, metldr_sec, 1, sector, &r, FLASH_FLAGS);
	if(!IS(item_name, "metldr") || IS(item_name, "metldr.2"))
	{
		// try to find metldr in NOR offset
		if(metldr_sec == 0x204) {metldr_sec = 0x4; goto retry_metldr;}
		ret = FAILED; goto exit_ros;
	}

patch_ros:
	if(stop_ros) {ret = FAILED; goto exit_ros;}

	if(ros == 1)
	{
		offset1 = offset2;
		start_ros0 = start_ros1;

		// notify start patching ROS1
		send(conn_s, "<p>Patching ROS1:", 17, 0);
	}
	else // ros == 0 || ros >= 2
	{
		// notify start patching ROS0
		send(conn_s, "<p>Patching ROS0:", 17, 0);
	}
	BEEP1;

retry_ros:
	// read first 3 sectors of ros0 to get first 0x10 (NOR) or 0x30 bytes (NAND) and check for "sdk_version"
	sys_storage_read(flash_id, 0, start_ros0, 3, sectors, &r, FLASH_FLAGS);	

	// check for "sdk_version" to verify that it is a valid ros0 file table
	for(pos = offset1; pos < header_size; pos += 0x30)
		if(IS(item_name + pos, "sdk_version")) break;
	if(pos >= header_size)
	{
		// try to find ROS in 0x80030 / 0x780020 (NAND)
		if(start_ros0 == 0x600) {start_ros0 = 0x400, start_ros1 = 0x3C00; goto retry_ros;}
		ret = FAILED; goto exit_ros;
	}

	// backup first sector before write
	if(full) memcpy(backup, sector, sec_size);

	// read first sector nofsm_patch.bin & write to ros0 / ros1
	if(cellFsReadWithOffset(fd, 0, (void *)(sector + offset1), sec_size - offset1, &read) != CELL_FS_SUCCEEDED) {ret = FAILED; goto exit_ros;}
	sys_storage_write(flash_id, start_ros0, 1, sector, &read);

	// verify written data
	if(full)
	{
		sys_storage_read(flash_id, 0, start_ros0, 1, written, &r, FLASH_FLAGS);
		if(memcmp(sector, written, sec_size))
		{
			// restore original sector
			sys_storage_write(flash_id, start_ros0, 1, backup, &read);
			ret = FAILED; goto exit_ros;
		}
	}

	seg_size = 0x40;  // 64 sectors
	buf_size = seg_size * sec_size; // 32KB

	for(sec = 1; sec < patch_size; sec += seg_size)
	{
		if(stop_ros) {ret = FAILED; goto exit_ros;}

		// prevent cross ROS boundaries
		if(sec + seg_size >= 0x3800)
		{
			seg_size = (0x3800 - sec - 1);  // 62 sectors
			buf_size = seg_size * sec_size; // 62 * 0x200 bytes = 0x7C00 = bytes
		}

		patch_offset = (sec * sec_size) - offset1;
		ros_sec = start_ros0 + sec;

		// read nofsm patch
		if(cellFsReadWithOffset(fd, patch_offset, (void *)sectors, buf_size, &read) != CELL_FS_SUCCEEDED) {ret = FAILED; goto exit_ros;}

		// backup sectors before write
		if(full)
		{
			sys_storage_read(flash_id, 0, ros_sec, seg_size, backup, &r, FLASH_FLAGS);

			// verify if needs to write segment
			write_data = (memcmp(sectors, backup, buf_size) != CELL_OK);
		}

		if(write_data)
		{
			// write patch to ros0 / ros1
			sys_storage_write(flash_id, ros_sec, seg_size, sectors, &read);

			// verify written data
			if(full)
			{
				sys_storage_read(flash_id, 0, ros_sec, seg_size, written, &r, FLASH_FLAGS);
				if(memcmp(sectors, written, buf_size))
				{
					// restore original sectors
					sys_storage_write(flash_id, ros_sec, seg_size, backup, &read);
					ret = FAILED; goto exit_ros;
				}
			}
		}

		// show progress every 64KB
		if(sec % 80 == 1) send(conn_s, ".", 1, 0);
	}

	// write ros1 if ros0 succeeded and should write both ros areas
	if((ret == CELL_OK) && (start_ros0 != start_ros1) && (ros >= 2))
	{
		ros = 1;
		goto patch_ros;
	}

exit_ros:
	// close & exit
	sys_storage_close(flash_id);
	cellFsClose(fd);

	if(ret) {BEEP3} else {BEEP2}

	stop_ros = ros_patching = false;
	return ret;
}
#endif //#ifdef PATCH_ROS

