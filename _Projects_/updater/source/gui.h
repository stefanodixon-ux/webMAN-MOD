#include <tiny3d.h>
#include <libfont.h>

// font 0: 224 chr from 32 to 255, 16 x 32 pix 2 bit depth
#include "font.h"

void cls(void)
{
	tiny3d_Clear(0x00000000, TINY3D_CLEAR_ALL);

	// Enable alpha Test
	tiny3d_AlphaTest(1, 0x10, TINY3D_ALPHA_FUNC_GEQUAL);

   // Enable alpha blending.
	tiny3d_BlendFunc(1, TINY3D_BLEND_FUNC_SRC_RGB_SRC_ALPHA | TINY3D_BLEND_FUNC_SRC_ALPHA_SRC_ALPHA,
		TINY3D_BLEND_FUNC_DST_RGB_ONE_MINUS_SRC_ALPHA | TINY3D_BLEND_FUNC_DST_ALPHA_ZERO,
		TINY3D_BLEND_RGB_FUNC_ADD | TINY3D_BLEND_ALPHA_FUNC_ADD);

	SetFontColor(0xffffffff, 0x10000000);
	SetCurrentFont(0);

	SetFontSize(12, 24);
	SetFontAutoCenter(1);
}

void Init_Graph()
{
	tiny3d_Init(1024*1024);
	tiny3d_Project2D();

	u32 * texture_mem = tiny3d_AllocTexture(64*1024*1024); // alloc 64MB of space for textures (this pointer can be global)

	u32 * texture_pointer; // use to asign texture space without changes texture_mem

	if(!texture_mem) exit(0); // fail!

	texture_pointer = texture_mem;

	ResetFont();
	texture_pointer = (u32 *) AddFontFromBitmapArray((u8 *) font  , (u8 *) texture_pointer, 32, 255, 16, 32, 2, BIT0_FIRST_PIXEL);


	int videoscale_x = 0;
	int videoscale_y = 0;

	double sx = (double) Video_Resolution.width;
	double sy = (double) Video_Resolution.height;
	double psx = (double) (1000 + videoscale_x)/1000.0;
	double psy = (double) (1000 + videoscale_y)/1000.0;

	tiny3d_UserViewport(1,
		(float) ((sx - sx * psx) / 2.0), // 2D position
		(float) ((sy - sy * psy) / 2.0),
		(float) ((sx * psx) / 848.0),	// 2D scale
		(float) ((sy * psy) / 512.0),
		(float) ((sx / 1920.0) * psx),  // 3D scale
		(float) ((sy / 1080.0) * psy));
}
