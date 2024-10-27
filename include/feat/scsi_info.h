#ifdef BDINFO
struct lv2_atapi_cmnd_block {
	uint8_t pkt[0x20]; // packet command block
	uint32_t pktlen;
	uint32_t blocks;
	uint32_t block_size;
	uint32_t proto;    // transfer mode
	uint32_t in_out;   // transfer direction
	uint32_t unknown;
} __attribute__((packed));

static int sys_storage_send_atapi_command(uint32_t fd, struct lv2_atapi_cmnd_block *atapi_cmnd, uint8_t *buffer)
{
	uint64_t tag;
	system_call_7(0x25C, fd, 1, (uint32_t) atapi_cmnd , sizeof (struct lv2_atapi_cmnd_block), (uint32_t) buffer, atapi_cmnd->block_size, (uint32_t) &tag);
	return_to_user_prog(int);
}

static void init_atapi_cmnd_block( struct lv2_atapi_cmnd_block *atapi_cmnd, uint32_t block_size, uint32_t proto, uint32_t type) {
	_memset(atapi_cmnd, sizeof(struct lv2_atapi_cmnd_block));
	atapi_cmnd->pktlen = 12; // 0xC
	atapi_cmnd->blocks = 1;
	atapi_cmnd->block_size = block_size; /* transfer size is block_size * blocks */
	atapi_cmnd->proto = proto;
	atapi_cmnd->in_out = type;
}

static int ps3rom_lv2_get_inquiry(int fd, uint8_t *buffer) {
	struct lv2_atapi_cmnd_block atapi_cmnd;

	init_atapi_cmnd_block(&atapi_cmnd, 0x3C, 1, 1);
	atapi_cmnd.pkt[0] = SCSI_CMD_FORMAT_INQUIRY; // 0x12
	atapi_cmnd.pkt[1] = 0;
	atapi_cmnd.pkt[2] = 0;
	atapi_cmnd.pkt[3] = 0;
	atapi_cmnd.pkt[4] = 0x3C;

	return sys_storage_send_atapi_command(fd, &atapi_cmnd, buffer);
}

static void get_bdvd_info(const char *dump_path, char *buffer)
{
	// open Blu Ray Drive
	uint32_t fd;
	uint64_t device = BDVD_DRIVE;
	char *pos = strchr(dump_path, '?'); if(pos) {device = val(pos + 1); *pos = 0;}

	int ret = sys_storage_open(device, 0, &fd, 0);
	if(ret != 0)
	{
		sprintf(buffer, "sys_storage_open failed (0x%x)", ret);
		return;
	}

	uint8_t buf[0x38]; _memset(buf, sizeof(buf));

	// inquiry command
	ret = ps3rom_lv2_get_inquiry(fd, buf);
	if(ret != 0)
	{
		sprintf(buffer, "sys_storage_send_atapi_command failed (0x%x)", ret);
		return;
	}

	// close device
	sys_storage_close(fd);

	// display bd manufacturer + product identification
	sprintf(buffer, "BD info: %.48s", buf + 8);

	// dump result to file
	if(*dump_path == '/')
		save_file(dump_path, (const char *)(buf+8), 0x30);
}
#endif
