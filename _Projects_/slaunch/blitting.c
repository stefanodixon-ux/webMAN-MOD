#include "include/blitting.h"
#include "include/argb_dec.h"
#include "include/png_dec.h"
#include "include/jpg_dec.h"
#include "include/misc.h"
#include "include/mem.h"

#define FAILED -1

//#include <cell/rtc.h>
#include <cell/rtc.h>
extern uint32_t disp_w, disp_h;
extern uint32_t gpp;

// display values
static uint32_t BASE_offset = 0;

static Bitmap *bitmap = NULL;					   // font glyph cache

static const CellFontLibrary* font_lib_ptr = NULL;  // font library pointer
static uint32_t vsh_fonts[16] = {};				 // addresses of the 16 system font slots

int32_t LINE_HEIGHT = 0;

static uint32_t (*fn_offset)(uint32_t x, uint32_t y) = NULL;

static uint32_t offset_1080(uint32_t x, uint32_t y) {return OFFSET_1080(x, y);}
static uint32_t offset_720p(uint32_t x, uint32_t y) {return OFFSET_720p(x, y);}
static uint32_t offset_576p(uint32_t x, uint32_t y) {return OFFSET_576p(x, y);}
static uint32_t offset_480p(uint32_t x, uint32_t y) {return OFFSET_480p(x, y);}

/***********************************************************************
* get font object
***********************************************************************/
static int32_t get_font_object(void)
{
	int32_t i, font_obj = 0;
	int32_t pm_start = 0x10000;
	uint64_t pat[2] = {0x3800001090810080ULL, 0x90A100849161008CULL};

	while(pm_start < 0x700000)
	{
		if((*(uint64_t*)pm_start == pat[0]) && (*(uint64_t*)(pm_start+8) == pat[1]))
		{
			// get font object
			font_obj = (int32_t)((int32_t)((*(int32_t*)(pm_start + 0x4C) & 0x0000FFFF) <<16) +
					   (int16_t)( *(int32_t*)(pm_start + 0x54) & 0x0000FFFF));

			// get font library pointer
			font_lib_ptr = (void*)(*(int32_t*)font_obj);

			// get addresses of loaded sys fonts
			for(i = 0; i < 16; i++)
				vsh_fonts[i] = (font_obj + 0x14 + (i * 0x100));

			return 0;
		}

		pm_start += 4;
	}

	return FAILED;
}

/***********************************************************************
* set font with default settings
***********************************************************************/
static void set_font_default(void)
{
	int32_t i;
	bitmap = mem_alloc(sizeof(Bitmap)); if(!bitmap) return;
	memset(bitmap, 0, sizeof(Bitmap));

	// set font
	FontSetScalePixel(&ctx.font, FONT_W, FONT_H);
	FontSetEffectWeight(&ctx.font, FONT_WEIGHT);

	FontGetHorizontalLayout(&ctx.font, &bitmap->horizontal_layout);
	LINE_HEIGHT = bitmap->horizontal_layout.lineHeight;

	bitmap->max	= FONT_CACHE_MAX;
	bitmap->count  = 0;
	bitmap->font_w = FONT_W;
	bitmap->font_h = FONT_H;
	bitmap->weight = FONT_WEIGHT;

	for(i = 0; i < FONT_CACHE_MAX; i++)
		bitmap->glyph[i].image = (uint8_t *)ctx.font_cache + (i * 0x400);
}

static int get_theme_font(int user_id)
{
	int reg = -1;
	int reg_value = -1;
	uint16_t off_string, len_data, len_string;
	uint64_t r;
	char string[256];

	if(cellFsOpen("/dev_flash2/etc/xRegistry.sys", CELL_FS_O_RDONLY, &reg, NULL, 0) != CELL_FS_SUCCEEDED || reg == -1)
	{
		return reg_value;
	}

	CellFsStat stat;
	cellFsStat("/dev_flash2/etc/xRegistry.sys", &stat);
	uint64_t entry_offset = 0x10000;

	char key[40]; sprintf(key, "/setting/user/%08i/theme/font", user_id);

	for(;;)
	{
	//// Data entries ////
		//unk
		entry_offset += 2;

		//relative entry offset
		cellFsReadWithOffset(reg, entry_offset, &off_string, 2, &r);
		entry_offset += 4;

		//data lenght
		cellFsReadWithOffset(reg, entry_offset, &len_data, 2, &r);
		entry_offset += 3;

	//// String Entries ////
		off_string += 0x12;

		//string length
		cellFsReadWithOffset(reg, off_string, &len_string, 2, &r);
		off_string += 3;

		//string
		memset(string, 0, sizeof(string));
		cellFsReadWithOffset(reg, off_string, string, len_string, &r);

		//Find key
		if(!strcmp(string, key))
		{
			if(len_data == 4)
				cellFsReadWithOffset(reg, entry_offset, &reg_value, 4, &r);
			break;
		}

		entry_offset += len_data + 1;

		if(off_string == 0xCCDD || entry_offset >= stat.st_size) break;
	}

	cellFsClose(reg);

	return reg_value;
}

/***********************************************************************
* unbind and destroy renderer, close font instance
***********************************************************************/
static void font_init(void)
{
	uint32_t user_id = 0; int sysfont = 0;
	CellFontRendererConfig rd_cfg;
	CellFont *opened_font = NULL;

	if(get_font_object() == FAILED) return;

	// get id of current logged in user for the xRegistry query we do next
	user_id = xsetting_CC56EB2D()->GetCurrentUserNumber();
	if(user_id > 255) user_id = 1;

	// get current font style for the current logged in user
	xsetting_CC56EB2D()->GetRegistryValue(user_id, 0x5C, &sysfont);

	// fix font selection
	int lang_id = 1;
	xsetting_0AF1F161()->GetSystemLanguage(&lang_id);
	if(((lang_id >= 9) && (lang_id <= 11)) || (lang_id == 16) || (lang_id == 19)) //kor / chi-tra / chi-sim / pol
		sysfont = 0;
	if(lang_id == 7) // rus
		sysfont = 4;
	if(lang_id == 8) // jap
		sysfont = get_theme_font(user_id);

	// get sysfont
	switch(sysfont)
	{
		case 0:   // original
		  opened_font = (void*)(vsh_fonts[5]);
		  break;
		case 1:   // rounded
		  opened_font = (void*)(vsh_fonts[8]);
		  break;
		case 3:   // pop
		  opened_font = (void*)(vsh_fonts[10]);
		  break;
		case 4:   // russian
		  opened_font = (void*)(vsh_fonts[1]);
		  break;
		default:  // better than nothing
		  opened_font = (void*)(vsh_fonts[9]);
		  break;
	}

	if(!opened_font) return;

	FontOpenFontInstance(opened_font, &ctx.font);

	memset(&rd_cfg, 0, sizeof(CellFontRendererConfig));
	FontCreateRenderer(font_lib_ptr, &rd_cfg, &ctx.renderer);

	FontBindRenderer(&ctx.font, &ctx.renderer);

	set_font_default();
}

/***********************************************************************
* unbind and destroy renderer, close font instance
***********************************************************************/
void font_finalize(void)
{
	FontUnbindRenderer(&ctx.font);
	FontDestroyRenderer(&ctx.renderer);
	FontCloseFont(&ctx.font);
}

/***********************************************************************
* render a char glyph bitmap into bitmap cache by index
*
* int32_t cache_idx  =  index into cache
* uint32_t code	  =  unicode of char glyph to render
***********************************************************************/
static void render_glyph(int32_t idx, uint32_t code)
{
	CellFontRenderSurface  surface;
	CellFontGlyphMetrics   metrics;
	CellFontImageTransInfo transinfo;
	int32_t i, k, x, y, w, h;
	int32_t ibw, k1, k2;


	// setup render settings
	FontSetupRenderScalePixel(&ctx.font, bitmap->font_w, bitmap->font_h);
	FontSetupRenderEffectWeight(&ctx.font, bitmap->weight);

	x = ((int32_t)bitmap->font_w) * 2;
	y = ((int32_t)bitmap->font_h) * 2;
	w = x * 2;
	h = y * 2;

	// set surface
	FontRenderSurfaceInit(&surface, NULL, w, 1, w, h);

	// set render surface scissor, (full area/no scissoring)
	FontRenderSurfaceSetScissor(&surface, 0, 0, w, h);

	bitmap->glyph[idx].code = code;

	FontRenderCharGlyphImage(&ctx.font, bitmap->glyph[idx].code, &surface,
							(float_t)x, (float_t)y, &metrics, &transinfo);

	bitmap->count++;

	ibw = transinfo.imageWidthByte;
	bitmap->glyph[idx].w = transinfo.imageWidth;	  // width of char image
	bitmap->glyph[idx].h = transinfo.imageHeight;	 // height of char image

	// copy glyph bitmap into cache
	for(k1 = k = 0; k < bitmap->glyph[idx].h; k++)
		for(k2 = k * ibw, i = 0; i < bitmap->glyph[idx].w; i++)
			bitmap->glyph[idx].image[k1++] =
			transinfo.Image[k2 + i];

	bitmap->glyph[idx].metrics = metrics;
}

/***********************************************************************
*
***********************************************************************/
static Glyph *get_glyph(uint32_t code)
{
	int32_t i, new;
	Glyph *glyph;

	// search glyph into cache
	for(i = 0; i < bitmap->count; i++)
	{
		glyph = &bitmap->glyph[i];

		if(glyph->code == code)
			return glyph;
	}

	// if glyph not into cache
	new = bitmap->count + 1;

	if(new >= bitmap->max)	   // if cache full
	  bitmap->count = new = 0;   // reset

	// render glyph
	render_glyph(new, code);
	glyph = &bitmap->glyph[new];

	return glyph;
}

/***********************************************************************
* get ucs4 code from utf8 sequence
*
* uint8_t *utf8   =  utf8 string
* uint32_t *ucs4  =  variable to hold ucs4 code
***********************************************************************/
static int32_t utf8_to_ucs4(uint8_t *utf8, uint32_t *ucs4)
{
	uint32_t c1 = 0, c2 = 0, c3 = 0, c4 = 0;

	c1 = (uint32_t)*utf8; utf8++;

	if(c1 <= 0x7F)						// 1 byte sequence, ascii
	{
		*ucs4 = c1; return 1;
	}
	else if((c1 & 0xE0) == 0xC0)		  // 2 byte sequence
	{
		c2 = (uint32_t)*utf8;

		if((c2 & 0xC0) == 0x80)
		{
			*ucs4 = ((c1  & 0x1F) << 6) | (c2 & 0x3F); return 2;
		}
	}
	else if((c1 & 0xF0) == 0xE0)		  // 3 bytes sequence
	{
		c2 = (uint32_t)*utf8; utf8++;

		if((c2 & 0xC0) == 0x80)
		{
			c3 = (uint32_t)*utf8;

			if((c3 & 0xC0) == 0x80)
			{
				*ucs4 = ((c1  & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F); return 3;
			}
		}
	}
	else if((c1 & 0xF8) == 0xF0)		  // 4 bytes sequence
	{
		c2 = (uint32_t)*utf8; utf8++;

		if((c2 & 0xC0) == 0x80)
		{
			c3 = (uint32_t)*utf8; utf8++;

			if((c3 & 0xC0) == 0x80)
			{
				c4 = (uint32_t)*utf8;

				if((c4 & 0xC0) == 0x80)
				{
					*ucs4 = ((c1  & 0x07) << 18) | ((c2 & 0x3F) << 12) | ((c3 & 0x3F) <<  6) | (c4 & 0x3F); return 4;
				}
			}
		}
	}

	*ucs4 = 0; return 0;
}

/*
void dim_img(float dim)
{
	uint32_t i, k, CANVAS_WW = CANVAS_W/2;
	uint64_t *canvas = (uint64_t*)ctx.canvas;
	uint64_t new_pixel = 0;
	uint64_t new_pixel_R0, new_pixel_G0, new_pixel_B0, new_pixel_R1, new_pixel_G1, new_pixel_B1;

	for(i = 0; i < CANVAS_H ; i++)
		for(k = 0; k < CANVAS_WW; k++)
		{
			new_pixel = canvas[k + i * CANVAS_WW];
			new_pixel_B0 = (uint8_t)((float)((new_pixel)	 & 0xff)*dim);
			new_pixel_G0 = (uint8_t)((float)((new_pixel>> 8) & 0xff)*dim);
			new_pixel_R0 = (uint8_t)((float)((new_pixel>>16) & 0xff)*dim);
			new_pixel_B1 = (uint8_t)((float)((new_pixel>>32) & 0xff)*dim);
			new_pixel_G1 = (uint8_t)((float)((new_pixel>>40) & 0xff)*dim);
			new_pixel_R1 = (uint8_t)((float)((new_pixel>>48) & 0xff)*dim);
			canvas[k + i * CANVAS_WW] = new_pixel_R1<<48 | new_pixel_G1<<40 | new_pixel_B1<<32 | new_pixel_R0<<16 | new_pixel_G0<< 8 | new_pixel_B0;
		}
}
*/

void dim_bg(float ds, float de)
{
	uint32_t i, k, m;

	for(i = 0; i <= CANVAS_H/2 ; i++)
	{
		for(m = (CANVAS_H-i-1), k = 0; k < CANVAS_W; k += 2)
		{
			*(uint64_t*)(OFFSET(k, i)) = 0;
			*(uint64_t*)(OFFSET(k, m)) = 0;
		}
		sys_timer_usleep(250);
	}
}

/***********************************************************************
* dump background
***********************************************************************/
void dump_bg(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
	uint32_t i, k, kk, m, CANVAS_WW = w/2;
	uint64_t *bg = (uint64_t*)ctx.canvas;

	for(i = 0; i < h; i++, y++)
		for(m = i * CANVAS_WW, k = 0, kk = x; k < CANVAS_WW; k++, kk += 2)
			bg[m++] = *(uint64_t*)(OFFSET(kk, y));
}

/***********************************************************************
*
***********************************************************************/
void init_graphic()
{
	memset(&ctx, 0, sizeof(DrawCtx));

	// set drawing context
	ctx.canvas	   = mem_alloc(CANVAS_W * CANVAS_H * 4);	// background buffer
	ctx.menu	   = mem_alloc(CANVAS_W * 96 * 4);			// info bar
	ctx.font_cache = mem_alloc(FONT_CACHE_MAX * 32 * 32);	// glyph bitmap cache
	ctx.imgs	   = mem_alloc(MAX_WH4);					// images (actually just 1 image with max 384x384 resolution)
	ctx.side	   = mem_alloc(SM_M);						// side menu
	ctx.bg_color   = 0xFF000000;							// black, opaque
	ctx.fg_color   = 0xFFFFFFFF;							// white, opaque

	font_init();

	// get current display values
	BASE_offset = (*(uint32_t*)0x60201104) + BASE;	  // start offset of current framebuffer

	if(disp_h == 1080)	fn_offset = offset_1080; else
	if(disp_h == 720)	fn_offset = offset_720p; else
	if(disp_h == 576)	fn_offset = offset_576p; else
						fn_offset = offset_480p;

	flip_frame();

	//getDisplayPitch(&pitch, &unk1);	   // framebuffer pitch size
	//h = getDisplayHeight();			   // display height
	//w = getDisplayWidth();				// display width
}

extern char wm_icons[7][56];

/***********************************************************************
* load an image file
*
* int32_t idx	  = index of img, max 11 (0 - 10)
* const char *path = path to img file
***********************************************************************/
int32_t load_img_bitmap(int32_t idx, char *path, const char *default_img)
{
	if(!path) return FAILED;
	if(*path != '/') strcpy(path, default_img);

	if(idx > IMG_MAX) return FAILED;
	uint32_t *buf = ctx.canvas;
	if(idx) buf = ctx.imgs;

	if(!buf) return FAILED;

	bool use_default = (*default_img == '/');

	char *ext = NULL, *dot = strchr(path, '.'); while(dot) {ext = dot++, dot = strchr(dot, '.');}

	if(!ext || not_exists(path))
	{
		if(!default_img || not_exists(default_img)) goto blank_image;

		strcpy(path, default_img);
		use_default = false;
	}

retry:
	if(!ext)
		goto blank_image;
	if(!strcasecmp(ext, ".png"))
		ctx.img[idx] = load_png(path, buf);
	else if(!strcasecmp(ext, ".argb"))
		ctx.img[idx] = load_argb(path, buf);
	else
		ctx.img[idx] = load_jpg(path, buf);

	if(!ctx.img[idx].w || !ctx.img[idx].h)
	{
		if(use_default)
		{
			strcpy(path, default_img); use_default = false;
			goto retry;
		}

blank_image:

		if(gpp == 10)
		{
			ctx.img[idx].w = 260;
			ctx.img[idx].h = 300;
		}
		else // if(gpp == 40)
		{
			ctx.img[idx].w = 120;
			ctx.img[idx].h = 160;
		}
		//memset(buf, 0x80, ctx.img[idx].w * ctx.img[idx].h * 4);
		memset32(buf, 0x80808080, ctx.img[idx].w * ctx.img[idx].h);
	}

	if(disp_h < 720) return 0;

	if(gpp == 10 && ctx.img[idx].w <= (MAX_W / 2) && ctx.img[idx].h <= (MAX_H / 2))
	{
		//upscale x2
		uint32_t w2 = ctx.img[idx].w << 1;
		uint32_t pixel, y1, y2, x, x1, x2, ww;

		for(int32_t y = ctx.img[idx].h-1; y >= 0; y--)
		{
			ww = y * ctx.img[idx].w;
			x1 = y1 = y * w2 << 1, x2 = y2 = y1 + w2;
			for(x = 0; x < ctx.img[idx].w; x++)
			{
				pixel = buf[x + ww];
				buf[x1++] = pixel;
				buf[x1++] = pixel;
				buf[x2++] = pixel;
				buf[x2++] = pixel;
			}
		}

		ctx.img[idx].w <<= 1;
		ctx.img[idx].h <<= 1;
	}

	if(gpp == 40 && idx && (ctx.img[idx].w > 168 || ctx.img[idx].h > 168))
	{
		//downscale x2
		uint32_t tw, ww, mm = 2;
downscale:
		tw = ctx.img[idx].w / mm;

		if(tw > 168)
		{
			mm <<= 1;
			ctx.img[idx].w >>= 1;
			ctx.img[idx].h >>= 1;
			goto downscale;
		}

		for(uint32_t y = 0, yy = ww = 0; y < ctx.img[idx].h; y += mm, yy = y / mm * tw, ww = y * ctx.img[idx].w)
			for(uint32_t x = 0, xx = 0; x < ctx.img[idx].w; xx++, x += mm)
				buf[xx + yy] = buf[x + ww];

		ctx.img[idx].w >>= 1;
		ctx.img[idx].h >>= 1;

		if(ctx.img[idx].w > 168 || ctx.img[idx].h > 168) goto downscale;
	}

	return 0;
}

/***********************************************************************
* alpha blending (ARGB)
*
* uint32_t bg = background color
* uint32_t fg = foreground color
***********************************************************************/
static uint32_t mix_color(uint32_t bg, uint32_t fg)
{
  uint32_t a = fg >>24;

  if(a == 0) return bg; // fg is transparent

  uint32_t aa = a ^ 0xFF;

  uint32_t rb = (((fg & 0x00FF00FF) * a) + ((bg & 0x00FF00FF) * aa)) & 0xFF00FF00;
  uint32_t g  = (((fg & 0x0000FF00) * a) + ((bg & 0x0000FF00) * aa)) & 0x00FF0000;
  fg = a + ((bg >>24) * aa >>8);

  return (fg <<24) | ((rb | g) >>8);
}

static inline uint64_t mix_color64(uint64_t bg, uint64_t fg)
{
  return ((uint64_t)(mix_color(bg, fg>>32))<<32 | mix_color(bg, fg));
}

/*
******** 1920 0x2000 (8192) / 4 = 2048 pitch
** ** ** 1280 0x1400 (5120) / 4 = 1280
*  *  *   720 0x0C00 (3072) / 4 =  768
*/

/***********************************************************************
* flip finished frame into paused ps3-framebuffer
***********************************************************************/
void flip_frame(void)
{
	if(!ctx.canvas) return;

	uint64_t *canvas = (uint64_t *)ctx.canvas;
	uint32_t i, k, m, CANVAS_WW = CANVAS_W/2;

	for(i = 0; i < CANVAS_H; i++)
		for(m = i * CANVAS_WW, k = 0; k < CANVAS_W; k += 2)
			*(uint64_t*)(OFFSET(k, i)) = canvas[m++];
}

void set_texture_direct(uint32_t *texture, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	if(!texture) return;

	uint32_t i, k, m, _width = width>>1, ww = width + x;
	uint64_t *canvas = (uint64_t*)texture;
	for(i = 0; i < height; i++, y++)
		for(m = i * _width, k = x; k < ww; k += 2)
			*(uint64_t*)(OFFSET(k, y)) = canvas[m++];
}

void set_texture(uint8_t idx, uint32_t x, uint32_t y)
{
	if(!ctx.img[idx].addr) return;

	if (!(ctx.img[idx].w&3) && !ctx.img[idx].b) { // Use VMX SIMD
		uint32_t i, k, kk, m, _width = ctx.img[idx].w>>2;
		vec_uint4 *canvas = (vec_uint4*)ctx.img[idx].addr;

		for (i = 0; i < ctx.img[idx].h; i++, y++)
			for (m = i * _width, k = 0, kk = x; k < _width; k++, kk+=4)
				*(vec_uint4*)(OFFSET(kk, y)) = canvas[k + m];
	} else {
		uint32_t i, k, m, _width = ctx.img[idx].w>>1;
		uint64_t *canvas = (uint64_t*)ctx.img[idx].addr;
		uint32_t ww = ctx.img[idx].w + x;

		if(!ctx.img[idx].b)	// jpeg - no transparency
			for(i = 0; i < ctx.img[idx].h; i++, y++)
				for(m = i * _width, k = x; k < ww; k += 2, m++)
					*(uint64_t*)(OFFSET(k, y)) = canvas[m];
		else				// png - blend with 18% gray background
			for(i = 0; i < ctx.img[idx].h; i++, y++)
				for(m = i * _width, k = x; k < ww; k += 2, m++)
					*(uint64_t*)(OFFSET(k, y)) = mix_color64(0x80303030, canvas[m]);
	}
}

#define DIM_PIXEL(p)	new_pixel = canvas[p]; \
						pixel = (uint8_t*)&new_pixel; \
						pixel[1] -= (pixel[1]>>2); \
						pixel[2] -= (pixel[2]>>2); \
						pixel[3] -= (pixel[3]>>2); \
						pixel[5] -= (pixel[5]>>2); \
						pixel[6] -= (pixel[6]>>2); \
						pixel[7] -= (pixel[7]>>2); \
						new_pixel = *((uint64_t*)pixel);

void set_backdrop(uint8_t idx, uint8_t restore)
{
	if(!ctx.canvas) return;
	
	uint32_t i, k, kk, m, CANVAS_WW = CANVAS_W/2;
	uint64_t *canvas = (uint64_t*)ctx.canvas;
	uint64_t new_pixel; uint8_t *pixel;
	uint32_t ww = (ctx.img[idx].x + ctx.img[idx].w)/2;
	uint32_t hh = (ctx.img[idx].y + ctx.img[idx].h);

	for(i = (ctx.img[idx].y+16); i < (hh+16) ; i++)
		for(m = i * CANVAS_WW, k = ww, kk=ww<<1; k < ww + 8; k++, kk += 2)
		{
			DIM_PIXEL(k + m); //75%
			*(uint64_t*)(OFFSET(kk, i)) = new_pixel;
		}

	for(i = hh; i < (hh+16) ; i++)
		for(m = i * CANVAS_WW, kk = (ctx.img[idx].x+16), k = kk>>1; k < ww; k++, kk += 2)
		{
			DIM_PIXEL(k + m); //75%
			*(uint64_t*)(OFFSET(kk, i)) = new_pixel;
		}

	if(restore)
	{
		for(i = (ctx.img[idx].y-16); i < (hh+32) ; i++)
			for(m = i * CANVAS_WW, kk = (ctx.img[idx].x-16), k = kk>>1; k < (ctx.img[idx].x)/2; k++, kk += 2)
				*(uint64_t*)(OFFSET(kk, i)) = canvas[k + m];

		for(i = (ctx.img[idx].y-16); i < (ctx.img[idx].y) ; i++)
			for(m = i * CANVAS_WW, kk = (ctx.img[idx].x), k = kk>>1; k < (ctx.img[idx].x+ctx.img[idx].w+16)/2; k++, kk += 2)
				*(uint64_t*)(OFFSET(kk, i)) = canvas[k + m];

		for(i = (ctx.img[idx].y); i < (ctx.img[idx].y+16) ; i++)
			for(m = i * CANVAS_WW, kk = (ctx.img[idx].x+ctx.img[idx].w), k = kk>>1; k < (ctx.img[idx].x+ctx.img[idx].w+16)/2; k++, kk += 2)
				*(uint64_t*)(OFFSET(kk, i)) = canvas[k + m];

		for(i = hh; i < (hh+16) ; i++)
			for(m = i * CANVAS_WW, kk = (ctx.img[idx].x), k = kk>>1; k < (ctx.img[idx].x+16)/2; k++, kk += 2)
				*(uint64_t*)(OFFSET(kk, i)) = canvas[k + m];
	}
}

// draw color box on the frame-buffer
void set_textbox(uint64_t color, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	uint32_t i, k, _width = width + x;
	for(i = 0; i < height; i++, y++)
		for(k = (x&0xffe); k < _width; k += 2)
			*(uint64_t*)(OFFSET(k, y)) = color;
}

// draw colored selection frame around the #IDX on the frame-buffer
void set_frame(uint8_t idx, uint64_t color)
{
	set_textbox(color, ctx.img[idx].x - 16, ctx.img[idx].y - 16, ctx.img[idx].w + 32, 16);
	set_textbox(color, ctx.img[idx].x - 16, ctx.img[idx].y + ctx.img[idx].h, ctx.img[idx].w + 32, 16);

	set_textbox(color, ctx.img[idx].x - 16, ctx.img[idx].y, 16, ctx.img[idx].h);
	set_textbox(color, ctx.img[idx].x + ctx.img[idx].w, ctx.img[idx].y, 16, ctx.img[idx].h);
}

/***********************************************************************
* set new font values
*
* float_t font_w	=  char width
* float_t font_h	=  char height
* float_t weight	=  line weight
* int32_t distance  =  distance between chars
***********************************************************************/
void set_font(float_t font_w, float_t font_h, float_t weight, int32_t distance)
{
	// max size is 32 * 32 pixels
	if(font_w > 32.f) font_w = 32.f;
	if(font_h > 32.f) font_h = 32.f;

	// set font
	FontSetScalePixel(&ctx.font, font_w, font_h);
	FontSetEffectWeight(&ctx.font, weight);

	// get and set new line height
	FontGetHorizontalLayout(&ctx.font, &bitmap->horizontal_layout);
	LINE_HEIGHT = bitmap->horizontal_layout.lineHeight;

	bitmap->count    = 0;							 // reset font cache
	bitmap->font_w   = font_w;
	bitmap->font_h   = font_h;
	bitmap->weight   = weight;
	bitmap->distance = distance;
}

/***********************************************************************
* print text, (TTF)
*
* int32_t x	   = start x coordinate into canvas
* int32_t y	   = start y coordinate into canvas
* const char *str = string to print
***********************************************************************/
int32_t print_text(uint32_t *texture, uint32_t text_width, uint32_t x, uint32_t y, const char *str)
{
	uint32_t *canvas = texture;
	uint32_t i, k, m, len = 0;
	uint32_t code = 0;											  // char unicode
	uint32_t t_x = x, t_y = y;									   // temp x/y
	uint32_t o_x = x, o_y = y + bitmap->horizontal_layout.baseLineY; // origin x/y
	Glyph *glyph;												   // char glyph
	uint8_t *utf8 = (uint8_t*)str; if(!str) return x;
	uint32_t pixel, color = (disp_w == 1920 ? 0 : 0xff333333);

	memset(&glyph, 0, sizeof(Glyph));

	// center text (only 1 line)
	if(x == CENTER_TEXT)
	{
		for(;;) // get render length
		{
			utf8 += utf8_to_ucs4(utf8, &code);

			if(code == 0) break;

			glyph = get_glyph(code);
			len += glyph->metrics.Horizontal.advance + bitmap->distance;
		}

		o_x = t_x = (CANVAS_W - len - bitmap->distance) / 2;
		utf8 = (uint8_t*)str;
	}

	// render text
	for(;;)
	{
		utf8 += utf8_to_ucs4(utf8, &code);

		if(code == 0) break;

		if((code == '^') || ((code == '\n') && (x != CENTER_TEXT)))
		{
			o_x = x;
			o_y += bitmap->horizontal_layout.lineHeight;
			continue;
		}
		else
		{
			// get glyph to draw
			glyph = get_glyph(code);

			// get bitmap origin(x, y)
			t_x = o_x + glyph->metrics.Horizontal.bearingX;
			t_y = o_y - glyph->metrics.Horizontal.bearingY;

			// draw bitmap
			for(i = 0; i < glyph->h; i++)
			  if(t_y + i < CANVAS_H)
				  for(m = (t_y + i) * text_width, k = 0; k < glyph->w; k++)
					if((glyph->image[i * glyph->w + k]) && (t_x + k < text_width))
					{
						pixel = m + t_x + k;
						canvas[pixel + text_width + 1] = color;
						canvas[pixel] =
						mix_color(canvas[pixel],
								 ((uint32_t)glyph->image[i * glyph->w + k] <<24) |
								 (ctx.fg_color & 0x00FFFFFF));
					}

			// get origin-x for next char
			o_x += glyph->metrics.Horizontal.advance + bitmap->distance;
		}
	}

	return o_x;
}

/***********************************************************************
* draw png part into frame.
*
* int32_t can_x	=  start x coordinate into canvas
* int32_t can_y	=  start y coordinate into canvas
* int32_t png_x	=  start x coordinate into png
* int32_t png_y	=  start y coordinate into png
* int32_t w		=  width of png part to blit
* int32_t h		=  height of png part to blit
***********************************************************************/
/*
int32_t draw_png(int32_t idx, int32_t c_x, int32_t c_y, int32_t p_x, int32_t p_y, int32_t w, int32_t h)
{
	uint32_t i, k, m, hh = h, ww = w;

	const uint32_t CANVAS_WW = CANVAS_W - c_x, CANVAS_HH = CANVAS_H - c_y;

	if(ww > CANVAS_WW) ww = CANVAS_WW;
	if(hh > CANVAS_HH) hh = CANVAS_HH;

	uint32_t offset = p_x + p_y * ctx.img[idx].w;

	for(i = 0; i < hh; i++)
		for(m = (c_y + i) * CANVAS_W + c_x, k = 0; k < ww; k++)
			ctx.canvas[m + k] =
				mix_color(ctx.canvas[m + k],
				ctx.img[idx].addr[offset + (k + i * ctx.img[idx].w)]);

	return (c_x + w);
}
*/

// some primitives...
/***********************************************************************
* draw a single pixel,
*
* int32_t x  =  start x coordinate into frame
* int32_t y  =  start y coordinate into frame
**********************************************************************
void draw_pixel(int32_t x, int32_t y)
{
  if((x < CANVAS_W) && (y < CANVAS_H))
	  ctx.canvas[x + y * CANVAS_W] = ctx.fg_color;
}*/

/***********************************************************************
* draw a line,
*
* int32_t x   =  line start x coordinate into frame
* int32_t y   =  line start y coordinate into frame
* int32_t x2  =  line end x coordinate into frame
* int32_t y2  =  line end y coordinate into frame
**********************************************************************
void draw_line(int32_t x, int32_t y, int32_t x2, int32_t y2)
{
	int32_t i = 0, dx1 = 0, dy1 = 0, dx2 = 0, dy2 = 0;
	int32_t w = x2 - x;
	int32_t h = y2 - y;


	if(w < 0) dx1 = -1; else if(w > 0) dx1 = 1;
	if(h < 0) dy1 = -1; else if(h > 0) dy1 = 1;
	if(w < 0) dx2 = -1; else if(w > 0) dx2 = 1;

	int32_t l = abs(w);
	int32_t s = abs(h);

	if(!(l > s))
	{
		l = abs(h);
		s = abs(w);

		if(h < 0) dy2 = -1; else if(h > 0) dy2 = 1;

		dx2 = 0;
	}

	int32_t num = l >> 1;

	for(i = 0; i <= l; i++)
	{
		draw_pixel(x, y);
		num+=s;

		if(!(num < l))
		{
			num-=l;
			x+=dx1;
			y+=dy1;
		}
		else
		{
			x+=dx2;
			y+=dy2;
		}
	}
}*/

/***********************************************************************
* draw a rectangle,
*
* int32_t x  =  rectangle start x coordinate into frame
* int32_t y  =  rectangle start y coordinate into frame
* int32_t w  =  width of rectangle
* int32_t h  =  height of rectangle
**********************************************************************
void draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
	draw_line(x, y, x + w, y);
	draw_line(x + w, y, x + w, y + h);
	draw_line(x + w, y + h, x, y + h);
	draw_line(x, y + h, x, y);
}*/

/***********************************************************************
* circle helper function
*
* int32_t x_c  =  circle center x coordinate into frame
* int32_t y_c  =  circle center y coordinate into frame
* int32_t x	=  circle point x coordinate into frame
* int32_t y	=  circle point y coordinate into frame
**********************************************************************
static void circle_points(int32_t x_c, int32_t y_c, int32_t x, int32_t y)
{
	draw_pixel(x_c + x, y_c + y);
	draw_pixel(x_c - x, y_c + y);
	draw_pixel(x_c + x, y_c - y);
	draw_pixel(x_c - x, y_c - y);
	draw_pixel(x_c + y, y_c + x);
	draw_pixel(x_c - y, y_c + x);
	draw_pixel(x_c + y, y_c - x);
	draw_pixel(x_c - y, y_c - x);
}*/

/***********************************************************************
* draw a circle,
*
* int32_t x_c  =  circle center x coordinate into frame
* int32_t y_c  =  circle center y coordinate into frame
* int32_t r	=  circle radius
**********************************************************************
void draw_circle(int32_t x_c, int32_t y_c, int32_t r)
{
	int32_t x = 0;
	int32_t y = r;
	int32_t p = 1 - r;

	circle_points(x_c, y_c, x, y);

	while(x < y)
	{
		x++;

		if(p < 0)
		{
			p += 2 * x + 1;
		}
		else
		{
			y--;
			p += 2 * (x - y) + 1;
		}

		circle_points(x_c, y_c, x, y);
	}
}*/
