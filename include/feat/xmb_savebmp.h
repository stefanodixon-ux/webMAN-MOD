/***********************************************************************
* pause/continue rsx fifo
*
* u8 pause    = pause fifo (1), continue fifo (0)
***********************************************************************/
#ifndef LITE_EDITION

#if defined(PS3_BROWSER) || defined(XMB_SCREENSHOT)
static s32 rsx_fifo_pause(u8 pause)
{
	// lv2 sys_rsx_context_attribute(u32 rsx_ctx_id, u64 arg_2, u64 arg_3); // syscall 674
	system_call_6(0x2A2, 0x55555555ULL, (u64)(pause ? 2 : 3), 0, 0, 0, 0);

	return (s32)p1;
}
#endif

#ifdef XMB_SCREENSHOT

// canvas constants
#define BASE          0xC0000000UL     // local memory base ea

// get pixel offset into framebuffer by x coordinates
#define OFFSET(x) (u32)(offset + (x<<2))

#define _ES32(v)((u32)(((((u32)v) & 0xFF000000) >> 24) | \
		               ((((u32)v) & 0x00FF0000) >> 8 ) | \
		               ((((u32)v) & 0x0000FF00) << 8 ) | \
		               ((((u32)v) & 0x000000FF) << 24)))

// display values
static u32 offset = 0, pitch = 0;
static u32 h = 0, w = 0;

//static DrawCtx ctx;                                 // drawing context

/***********************************************************************
*
***********************************************************************/
static void init_graphic(void)
{
	// get current display values
	h = getDisplayHeight();                     // display height
	w = getDisplayWidth();                      // display width
	getDisplayPitch(&pitch, &offset); pitch /= 4; // framebuffer pitch size
	offset = BASE + INT32(0x60201104);          // start offset of current framebuffer
}

static void saveBMP(char *path, bool notify_bmp, bool small)
{
	if((*path != '/') || !is_ext(path, ".bmp"))
	{
		// current date/time
		CellRtcDateTime t;
		cellRtcGetCurrentClockLocalTime(&t);

		// build file path
		if(*path != '/')
			strcpy(path, drives[0]);
		else if(!islike(path, "/dev_"))
		{
			prepend(path, drives[0], 9);
		}

		sprintf(path + strlen(path), "/screenshot_%04d_%02d_%02d_%02d_%02d_%02d.bmp", t.year, t.month, t.day, t.hour, t.minute, t.second);
	}

	filepath_check(path);

	// create bmp file
	int fd; mkdir_tree(path);
	if(IS_INGAME || cellFsOpen(path, CELL_FS_O_CREAT | CELL_FS_O_WRONLY | CELL_FS_O_TRUNC, &fd, NULL, 0) != CELL_FS_SUCCEEDED) { BEEP3 ; return;}

	// max frame line size = 1920 pixel * 4(byte per pixel) = 7680 byte = 8 KB
	// max bmp buffer size = 1920 pixel * 3(byte per pixel) = 5760 byte = 6 KB

	sys_addr_t sysmem = sys_mem_allocate(_64KB_);
	if(!sysmem) {cellFsClose(fd); return;}

	rsx_fifo_pause(1);

	// initialize graphic
	init_graphic();

	u16 c, y, x, idx;
	u16 rr = small ? 2 : 1; w /= rr, h /= rr; // resize bmp image if small flag is true

	u16 margin_w = small ? 80 : 0, margin_h = small ? 30 : 0;

	w -= 2 * margin_w, h -= 2 * margin_h; margin_w *= rr;

	// calc buffer sizes
	u32 line_frame_size = (w * 4); // ABGR

	// alloc buffers
	u8 *tmp_buf = (u8*)sysmem;
	u8 *bmp_buf = tmp_buf + (4 * line_frame_size); // start offset: 30 KB

	#define bmp_header			tmp_buf
	#define bmp_header_size		0x36

	_memset(bmp_header, bmp_header_size);
	bmp_header[0x00] = 0x42;
	bmp_header[0x01] = 0x4D;
	bmp_header[0x0A] = bmp_header_size;
	bmp_header[0x0E] = 0x28;
	bmp_header[0x1A] = 0x01;
	bmp_header[0x1C] = 0x18;
	bmp_header[0x26] = bmp_header[0x2A] = 0x12;
	bmp_header[0x27] = bmp_header[0x2B] = 0x1B;

	// set bmp header
	u32 tmp;
	tmp = _ES32(w * h * 3 + bmp_header_size);
	memcpy(bmp_header + 0x02 , &tmp, 4);    // file size
	tmp = _ES32(w);
	memcpy(bmp_header + 0x12, &tmp, 4);     // bmp width
	tmp = _ES32(h);
	memcpy(bmp_header + 0x16, &tmp, 4);     // bmp height
	tmp = _ES32(w * h * 3);
	memcpy(bmp_header + 0x22, &tmp, 4);     // bmp data size

	// write bmp header
	cellFsWrite(fd, (void *)bmp_header, bmp_header_size, NULL);

	// dump...
	#define bottom_margin h * rr
	u32 px = 4 * rr; line_frame_size *= rr; c = idx = 0;
	for(y = bottom_margin; y > 0; y -= rr)
	{
		tmp = (y * pitch) + margin_w;
		tmp_buf = (u8*)(u64*)(OFFSET(tmp));

		// convert line from ABGR to RGB
		for(x = 0; x < line_frame_size; x += px, idx += 3)
		{
			bmp_buf[idx]   = tmp_buf[x + 3];  // R
			bmp_buf[idx+1] = tmp_buf[x + 2];  // G
			bmp_buf[idx+2] = tmp_buf[x + 1];  // B
		}

		// write bmp data
		if(++c >= 4)
		{
			cellFsWrite(fd, bmp_buf, idx, NULL);
			c = idx = 0;
		}
	}

	// continue rsx rendering
	rsx_fifo_pause(0);

	cellFsClose(fd);
	sys_memory_free(sysmem);

	if(notify_bmp) show_msg(path);
}

/*
#include "../vsh/system_plugin.h"

static void saveBMP2(char *path, bool notify_bmp)
{
	if(IS_ON_XMB) //XMB
	{
		char bmp[0x50];
		if(!is_ext(path, ".bmp"))
		{
			path = bmp;

			CellRtcDateTime t;
			cellRtcGetCurrentClockLocalTime(&t);

			sprintf(path, "/dev_hdd0/screenshot_%02d_%02d_%02d_%02d_%02d_%02d.bmp", t.year, t.month, t.day, t.hour, t.minute, t.second);
		}

		filepath_check(path);

		system_interface = (system_plugin_interface *)plugin_GetInterface(View_Find("system_plugin"),1); // 1=regular xmb, 3=ingame xmb (doesnt work)

		rsx_fifo_pause(1);

		system_interface->saveBMP(path);

		rsx_fifo_pause(0);

		if(notify_bmp) show_msg(path);
	}
}
*/

#endif // #ifdef XMB_SCREENSHOT

#endif // #ifndef LITE_EDITION
