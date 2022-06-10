#ifdef USE_INTERNAL_NET_PLUGIN
static int read_remote_file_critical(u64 offset, void *buf, u32 size)
{
	netiso_read_file_critical_cmd cmd;

	_memset(&cmd, sizeof(cmd));
	cmd.opcode = NETISO_CMD_READ_FILE_CRITICAL;
	cmd.num_bytes = size;
	cmd.offset = offset;

	if(send(g_socket, &cmd, sizeof(cmd), 0) != sizeof(cmd))
	{
		//DPRINTF("send failed (read file) (errno=%d)!\n", sys_net_errno);
		return FAILED;
	}

	if(recv(g_socket, buf, size, MSG_WAITALL) != (int)size)
	{
		//DPRINTF("recv failed (recv file)  (errno=%d)!\n", sys_net_errno);
		return FAILED;
	}

	return CELL_OK;
}

static int process_read_cd_2048_cmd(u8 *buf, u32 start_sector, u32 sector_count)
{
	netiso_read_cd_2048_critical_cmd cmd;

	_memset(&cmd, sizeof(cmd));
	cmd.opcode = NETISO_CMD_READ_CD_2048_CRITICAL;
	cmd.start_sector = start_sector;
	cmd.sector_count = sector_count;

	if(send(g_socket, &cmd, sizeof(cmd), 0) != sizeof(cmd))
	{
		//DPRINTF("send failed (read 2048) (errno=%d)!\n", sys_net_errno);
		return FAILED;
	}

	if(recv(g_socket, buf, sector_count * CD_SECTOR_SIZE_2048, MSG_WAITALL) != (int)(sector_count * CD_SECTOR_SIZE_2048))
	{
		//DPRINTF("recv failed (read 2048)  (errno=%d)!\n", sys_net_errno);
		return FAILED;
	}

	return CELL_OK;
}

static int process_read_iso_cmd(u8 *buf, u64 offset, u32 size)
{
	u64 read_end;

	//DPRINTF("read iso: %p %lx %x\n", buf, offset, size);
	read_end = offset + size;

	if(read_end >= discsize)
	{
		//DPRINTF("Read beyond limits: %llx %x (discsize=%llx)!\n", offset, size, discsize);

		if(offset >= discsize)
		{
			_memset(buf, size);
			return CELL_OK;
		}

		_memset(buf + (discsize - offset), read_end - discsize);
		size = discsize - offset;
	}

	return read_remote_file_critical(offset, buf, size);
}

static int process_read_cd_2352_cmd(u8 *buf, u32 sector, u32 remaining)
{
	int cache = 0;

	if(remaining <= CD_CACHE_SIZE)
	{
		int dif = (int)cached_cd_sector - sector;

		if(ABS(dif) < CD_CACHE_SIZE)
		{
			u8 *copy_ptr = NULL;
			u32 copy_offset = 0;
			u32 copy_size = 0;

			if(dif > 0)
			{
				if(dif < (int)remaining)
				{
					copy_ptr = cd_cache;
					copy_offset = dif;
					copy_size = remaining - dif;
				}
			}
			else
			{
				copy_ptr = cd_cache + ((-dif) * CD_SECTOR_SIZE_2352);
				copy_size = MIN((int)remaining, CD_CACHE_SIZE + dif);
			}

			if(copy_ptr)
			{
				memcpy64(buf + (copy_offset * CD_SECTOR_SIZE_2352), copy_ptr, copy_size * CD_SECTOR_SIZE_2352);

				if(remaining == copy_size)
				{
					return CELL_OK;
				}

				remaining -= copy_size;

				if(dif <= 0)
				{
					u32 newsector = cached_cd_sector + CD_CACHE_SIZE;
					buf += ((newsector-sector) * CD_SECTOR_SIZE_2352);
					sector = newsector;
				}
			}
		}

		cache = 1;
	}

	if(!cache)
	{
		return process_read_iso_cmd(buf, sector * CD_SECTOR_SIZE_2352, remaining * CD_SECTOR_SIZE_2352);
	}

	if(!cd_cache)
	{
		sys_addr_t addr = sys_mem_allocate(_128KB_);
		if(!addr)
		{
			//DPRINTF("sys_memory_allocate failed: %x\n", ret);
			return FAILED;
		}

		cd_cache = (u8 *)addr;
	}

	if(process_read_iso_cmd(cd_cache, sector * CD_SECTOR_SIZE_2352, CD_CACHE_SIZE * CD_SECTOR_SIZE_2352) != 0)
		return FAILED;

	memcpy64(buf, cd_cache, remaining * CD_SECTOR_SIZE_2352);
	cached_cd_sector = sector;

	return CELL_OK;
}

static u8 netiso_loaded = 0;
static sys_event_queue_t command_queue_net = NONE;
static sys_ppu_thread_t thread_id_net = SYS_PPU_THREAD_NONE;

static void netiso_thread(__attribute__((unused)) u64 arg)
{
	unsigned int real_disctype;
	ScsiTrackDescriptor *tracks;
	int emu_mode, num_tracks;
	sys_event_port_t result_port = (sys_event_port_t)(NONE);

	emu_mode = netiso_args.emu_mode & 0xF;
	CD_SECTOR_SIZE_2352 = 2352;

	//DPRINTF("Hello VSH\n");

	g_socket = connect_to_server(netiso_args.server, netiso_args.port);
	if(g_socket < 0 && !IS(webman_config->allow_ip, netiso_args.server))
	{
		// retry using ip of the remote connection
		g_socket = connect_to_server(webman_config->allow_ip, netiso_args.port);
	}

	if(g_socket < 0)
	{
		goto exit_netiso;
	}

	int ret = emu_mode;

	s64 size = open_remote_file(g_socket, netiso_args.path, &ret);
	if(size < 0)
	{
		goto exit_netiso;
	}

	discsize = (u64)size;

	ret = sys_event_port_create(&result_port, 1, SYS_EVENT_PORT_NO_NAME);
	if(ret != CELL_OK)
	{
		//DPRINTF("sys_event_port_create failed: %x\n", ret);
		goto exit_netiso;
	}

	sys_event_queue_attribute_t queue_attr;
	sys_event_queue_attribute_initialize(queue_attr);
	ret = sys_event_queue_create(&command_queue_net, &queue_attr, 0, 1);
	if(ret != CELL_OK)
	{
		//DPRINTF("sys_event_queue_create failed: %x\n", ret);
		goto exit_netiso;
	}

	unsigned int cd_sector_size_param = 0;

	if(emu_mode == EMU_PSX)
	{
		tracks = netiso_args.tracks;
		num_tracks = MIN(netiso_args.num_tracks, MAX_TRACKS);

		is_cd2352 = 1;

		if(discsize % CD_SECTOR_SIZE_2352)
		{
			discsize -= (discsize % CD_SECTOR_SIZE_2352);
		}

		if(CD_SECTOR_SIZE_2352 & 0xf) cd_sector_size_param = CD_SECTOR_SIZE_2352<<8;
		else if(CD_SECTOR_SIZE_2352 != 2352) cd_sector_size_param = CD_SECTOR_SIZE_2352<<4;
	}
	else
	{
		num_tracks = 0;
		tracks = NULL;
		is_cd2352 = 0;
	}

	sys_storage_ext_get_disc_type(&real_disctype, NULL, NULL);

	if(real_disctype != DISC_TYPE_NONE)
	{
		fake_eject_event(BDVD_DRIVE);
	}

	ret = sys_storage_ext_mount_discfile_proxy(result_port, command_queue_net, emu_mode, discsize, _256KB_, (num_tracks | cd_sector_size_param), tracks);
	//DPRINTF("mount = %x\n", ret);

	fake_insert_event(BDVD_DRIVE, real_disctype);

	if(ret != CELL_OK)
	{
		sys_event_port_destroy(result_port);
		goto exit_netiso;
	}

	netiso_loaded = 1;

	while(netiso_loaded)
	{
		sys_event_t event;

		ret = sys_event_queue_receive(command_queue_net, &event, 0);
		if(ret != CELL_OK)
		{
			//DPRINTF("sys_event_queue_receive failed: %x\n", ret);
			break;
		}

		if(!netiso_loaded) break;

		void *buf = (void *)(u32)(event.data3>>32ULL);
		u64 offset = event.data2;
		u32 size = event.data3&0xFFFFFFFF;

		switch(event.data1)
		{
			case CMD_READ_ISO:
			{
				if(is_cd2352)
				{
					ret = process_read_cd_2048_cmd(buf, offset / CD_SECTOR_SIZE_2048, size / CD_SECTOR_SIZE_2048);
				}
				else
				{
					ret = process_read_iso_cmd(buf, offset, size);
				}
			}
			break;

			case CMD_READ_CD_ISO_2352:
			{
				ret = process_read_cd_2352_cmd(buf, offset / CD_SECTOR_SIZE_2352, size / CD_SECTOR_SIZE_2352);
			}
			break;
		}

		while(netiso_loaded)
		{
			ret = sys_event_port_send(result_port, ret, 0, 0);
			if(ret == CELL_OK) break;

			if(ret == (int) 0x8001000A)
			{   // EBUSY
				sys_ppu_thread_usleep(100000);
				continue;
			}

			break;
		}

		//DPRINTF("sys_event_port_send failed: %x\n", ret);
		if(ret) break;
	}

exit_netiso:

	//if(args) sys_memory_free((sys_addr_t)args);

	if(command_queue_net != SYS_EVENT_QUEUE_NONE)
	{
		sys_event_queue_destroy(command_queue_net, SYS_EVENT_QUEUE_DESTROY_FORCE);
	}

	sys_storage_ext_get_disc_type(&real_disctype, NULL, NULL);
	fake_eject_event(BDVD_DRIVE);
	sys_storage_ext_umount_discfile();

	if(real_disctype != DISC_TYPE_NONE)
	{
		fake_insert_event(BDVD_DRIVE, real_disctype);
	}

	if(cd_cache)
	{
		sys_memory_free((sys_addr_t)cd_cache);
	}

	if(g_socket >= 0)
	{
		sclose(&g_socket);
	}

	sys_event_port_disconnect(result_port);

	if(sys_event_port_destroy(result_port) != CELL_OK)
	{
		//DPRINTF("Error destroyng result_port\n");
	}

	//DPRINTF("Exiting main thread!\n");
	netiso_loaded = 0;
	netiso_svrid = NONE;

	sys_ppu_thread_exit(0);
}

static void netiso_stop_thread(__attribute__((unused)) u64 arg)
{
	netiso_loaded = 0;
	netiso_svrid = NONE;

	if(g_socket >= 0)
	{
		sclose(&g_socket);
	}

	if(command_queue_net != SYS_EVENT_QUEUE_NONE)
	{
		if(sys_event_queue_destroy(command_queue_net, SYS_EVENT_QUEUE_DESTROY_FORCE))
		{
			//DPRINTF("Failed in destroying command_queue_net\n");
		}
	}

	thread_join(thread_id_net);

	sys_ppu_thread_exit(0);
}
#endif // #ifdef USE_INTERNAL_NET_PLUGIN