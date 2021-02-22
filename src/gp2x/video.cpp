#include "driver.h"
#include "gp2x_mame.h"
#include "minimal.h"
#include "cpuctrl.h"

#ifndef GP2X_BYTE_SIZE_SCREEN 
#define GP2X_BYTE_SIZE_SCREEN (640*480) 
#endif 

struct osd_bitmap *scrbitmap;
unsigned char current_palette[256][3];
unsigned char dirtycolor[256];
unsigned int dirtypalette;

/* Timing variables */
extern int frameskip;
#ifndef MAME4ALL_BENCH
int gp2x_frameskip=2;
int gp2x_frameskip_auto=1;
#else
int gp2x_frameskip=5;
int gp2x_frameskip_auto=0;
#endif
int gp2x_double_buffer=0;
#if !defined(DREAMCAST) || defined(MAME4ALL_BENCH)
int gp2x_vsync=0;
#else
int gp2x_vsync=1;
#endif

#if !defined(GP2X) && !defined(NO_SPLASH)
extern void load_bmp_8bpp(unsigned char *out, unsigned char *in);
extern unsigned char gp2xsplash_bmp[];
#endif

/* GP2X Graphic Display Variables */
int game_width;
int game_height;
int gp2x_xoffset;
int gp2x_yoffset;
int gp2x_xscreen;
int gp2x_yscreen;
#ifndef GP2X
int gp2x_xmax;
int gp2x_ymax;
#endif
int gp2x_width;
int gp2x_height;
#ifndef DREAMCAST
int gp2x_rotate=0;
#else
int gp2x_rotate=3;
#endif
int visible_game_width;
int visible_game_height;
int visible_game_width_offset;
int visible_game_height_offset;

static int frame_buffer=0;

#ifdef PSP_RES
int gp2x_text_width=480;
#else
int gp2x_text_width=320;
#endif

#if defined(GP2X) || defined(PSP)
int gp2x_showfps=0;
#endif

#ifdef GP2X
int gp2x_bordertvout=0;
int gp2x_bordertvout_width=6;
int gp2x_bordertvout_height=8;
#endif

/* GP2X Video Init */
void gp2x_video_init(void) {

	/* Video and Sound Init */
	gp2x_init(1000,8,DEFAULT_SAMPLE_RATE,16,0,60);
   	
   	/* Set MAME Palette */
   	gp2x_mame_palette();
}

void gp2x_mame_palette(void) {
	int i;

	/* Color #0 is black */
	for (i=0;i<256;i++)
		gp2x_video_color8(i,0,0,0);
	
	/* Color #1 is white */
	gp2x_video_color8(1,255,255,255);
	
	/* Update Palette */
	gp2x_video_setpalette();
}

void update_p_update_display_func(unsigned n);
/* Adjust Display: New Gfx Core */
void gp2x_adjust_display(void) {

#ifdef GP2X
	switch (gp2x_rotate) {
		case 0: gp2x_text_width=320;
			SetVideoScaling(320,320,240);
			break;
		case 1: gp2x_text_width=320;
			SetVideoScaling(320,320,240);
			break;
		case 2: gp2x_text_width=visible_game_width;
			SetVideoScaling(visible_game_width,visible_game_width,visible_game_height);
			break;
		case 3: gp2x_text_width=visible_game_height;
			SetVideoScaling(visible_game_height,visible_game_height,visible_game_width);
			break;
		default: break;
	}
	
	if (gp2x_rotate&1) {

		/* Image has to be rotated */	
		gp2x_xoffset=0;
		gp2x_yoffset=0;
		gp2x_xscreen=0;
		gp2x_yscreen=0;
		gp2x_width=240;
		gp2x_height=320;

		if (visible_game_width>240)
			gp2x_xoffset = (visible_game_width-240) / 2 ;
		else {
			gp2x_width = visible_game_width;
			gp2x_xscreen = (240-visible_game_width) / 2;
		}
		gp2x_xoffset+= visible_game_width_offset;

		if (visible_game_height>320) 
			gp2x_yoffset = (visible_game_height-320) / 2;
		else {
			gp2x_height = visible_game_height;
			gp2x_yscreen = (320-visible_game_height) / 2;
		}
		gp2x_yoffset+= visible_game_height_offset;
	}
	else
#else
	if (gp2x_rotate==1)
	{
// DIV2
#ifdef PSP_RES
		gp2x_xoffset=visible_game_width_offset;
		gp2x_yoffset=visible_game_height_offset;
		gp2x_xscreen=0;
		gp2x_yscreen=0;
		gp2x_width=visible_game_width;
		gp2x_height=visible_game_height;
		gp2x_xmax=960;
		gp2x_ymax=544;
		if (gp2x_xmax&15)
			gp2x_xmax+=16;
		gp2x_xmax&=0x07F0;
		(visible_game_height>544)?gp2x_xscreen=(visible_game_width-960)/2:gp2x_xscreen=(960-visible_game_width)/2;
#else
		gp2x_xoffset=0;
		gp2x_yoffset=0;
		gp2x_xscreen=0;
		gp2x_yscreen=0;
		gp2x_width=640;
		gp2x_height=480;
		if (visible_game_width>640) 
			gp2x_xoffset = (visible_game_width-640) / 2;
		else {
			gp2x_width = visible_game_width;
			gp2x_xscreen = (640-visible_game_width) / 2;
		}
		gp2x_xoffset+= visible_game_width_offset;

		if (visible_game_height>480)
			gp2x_yoffset = (visible_game_height-480) / 2;
		else {
			gp2x_height = visible_game_height;
			gp2x_yscreen = (480-visible_game_height) / 2;
		}
		gp2x_yoffset+= visible_game_height_offset;
		gp2x_width&=0x07E0;
#endif
	} else
#ifdef DREAMCAST
	if (gp2x_rotate==3)
	{
// HW SCALED
		gp2x_xoffset=visible_game_width_offset;
		gp2x_yoffset=visible_game_height_offset;
		gp2x_width=visible_game_width;
		if (gp2x_width>512)
			gp2x_width=512;
		gp2x_width&=0x7F0;
		gp2x_height=visible_game_height;
		if (gp2x_height>512)
			gp2x_height=512;
		if (Machine->orientation & ORIENTATION_SWAP_XY)
		{
			if (gp2x_width<gp2x_height)
			{
				gp2x_ymax=gp2x_height;
				gp2x_yscreen=0;
//				gp2x_xmax=(4*gp2x_height)/3;
//				gp2x_xmax=gp2x_height;
				gp2x_xmax=(23*gp2x_height)/20;
				if (gp2x_xmax>512)
					gp2x_xmax=512;
				gp2x_xscreen=(gp2x_xmax-gp2x_width)/2;
			}
			else
			{
				gp2x_xmax=gp2x_width;
				gp2x_xscreen=0;
				gp2x_ymax=(3*gp2x_width)/4;
				if (gp2x_ymax>512)
					gp2x_ymax=512;
				gp2x_yscreen=(gp2x_ymax-gp2x_height)/2;
			}
		}
		else
		{
			gp2x_xscreen=0;
			gp2x_xmax=gp2x_width;
			gp2x_yscreen=0;
			gp2x_ymax=gp2x_height;
		}
	}
	else
#endif
	if (gp2x_rotate)
	{
// SW SCALED
		gp2x_xoffset=visible_game_width_offset;
		gp2x_yoffset=visible_game_height_offset;
		gp2x_xscreen=0;
		gp2x_yscreen=0;
		gp2x_width=visible_game_width;
		gp2x_height=visible_game_height;
#ifdef PSP_RES
		gp2x_xmax=480;
#else
		gp2x_xmax=320;
#endif
		gp2x_ymax=(visible_game_height<<8)/visible_game_width;
#ifdef PSP_RES
		gp2x_ymax*=480;
#else
		gp2x_ymax*=320;
#endif
		gp2x_ymax>>=8;
//		if ((visible_game_width>visible_game_height)&&(gp2x_ymax<=240))
#ifdef PSP_RES
		if (gp2x_ymax<=272)
			gp2x_yscreen=((272-gp2x_ymax)/2)&0x00FE;
		else
		{
			gp2x_ymax=272;
			gp2x_xmax=(visible_game_width<<8)/visible_game_height;
			gp2x_xmax*=272;
			gp2x_xmax>>=8;
			if (gp2x_xmax>480)
				gp2x_xmax=480;
#else
		if (gp2x_ymax<=240)
			gp2x_yscreen=((240-gp2x_ymax)/2)&0x00FE;
		else
		{
			gp2x_ymax=240;
			gp2x_xmax=(visible_game_width<<8)/visible_game_height;
			gp2x_xmax*=240;
			gp2x_xmax>>=8;
			if (gp2x_xmax>320)
				gp2x_xmax=320;
#endif
			else
			if (gp2x_xmax&15)
				gp2x_xmax+=16;
			gp2x_xmax&=0x07F0;
#ifdef PSP_RES
			gp2x_xscreen=((480-gp2x_xmax)/2)&0x00FD;
#else
			gp2x_xscreen=((320-gp2x_xmax)/2)&0x00FD;
#endif
		}
	}
	else
#endif
	{
		/* Image not rotated */
#ifdef PSP_RES
		gp2x_xoffset=visible_game_width_offset;
		gp2x_yoffset=visible_game_height_offset;
		gp2x_xscreen=0;
		gp2x_yscreen=0;
		gp2x_width=visible_game_width;
		gp2x_height=visible_game_height;
		gp2x_xmax=480;
		gp2x_ymax=272;
		if (gp2x_xmax&15)
			gp2x_xmax+=16;
		gp2x_xmax&=0x07F0;
		if (gp2x_rotate!=3)
			(visible_game_width>480)?gp2x_xscreen=(visible_game_width-480)/2:gp2x_xscreen=(480-visible_game_width)/2;
#else
		gp2x_xoffset=0;
		gp2x_yoffset=0;
		gp2x_xscreen=0;
		gp2x_yscreen=0;
		gp2x_width=320;
		gp2x_height=240;

		if (visible_game_width>320) 
			gp2x_xoffset = (visible_game_width-320) / 2;
		else {
			gp2x_width = visible_game_width;
			gp2x_xscreen = (320-visible_game_width) / 2;
		}
		gp2x_xoffset+= visible_game_width_offset;
		
		if (visible_game_height>240) 
			gp2x_yoffset = (visible_game_height-240) / 2;
		else {
			gp2x_height = visible_game_height;		
			gp2x_yscreen = (240-visible_game_height) / 2;
		}
		gp2x_yoffset+= visible_game_height_offset;
#ifndef GP2X
		gp2x_width&=0x07F0;
#endif
#endif
	}
#ifndef GP2X
	update_p_update_display_func(gp2x_rotate&3);
#else
	update_p_update_display_func((gp2x_bordertvout<<2)+(gp2x_rotate&3));
#endif
}

static unsigned char fontdata8x8[] =
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x3C,0x42,0x99,0xBD,0xBD,0x99,0x42,0x3C,0x3C,0x42,0x81,0x81,0x81,0x81,0x42,0x3C,
	0xFE,0x82,0x8A,0xD2,0xA2,0x82,0xFE,0x00,0xFE,0x82,0x82,0x82,0x82,0x82,0xFE,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x38,0x64,0x74,0x7C,0x38,0x00,0x00,
	0x80,0xC0,0xF0,0xFC,0xF0,0xC0,0x80,0x00,0x01,0x03,0x0F,0x3F,0x0F,0x03,0x01,0x00,
	0x18,0x3C,0x7E,0x18,0x7E,0x3C,0x18,0x00,0xEE,0xEE,0xEE,0xCC,0x00,0xCC,0xCC,0x00,
	0x00,0x00,0x30,0x68,0x78,0x30,0x00,0x00,0x00,0x38,0x64,0x74,0x7C,0x38,0x00,0x00,
	0x3C,0x66,0x7A,0x7A,0x7E,0x7E,0x3C,0x00,0x0E,0x3E,0x3A,0x22,0x26,0x6E,0xE4,0x40,
	0x18,0x3C,0x7E,0x3C,0x3C,0x3C,0x3C,0x00,0x3C,0x3C,0x3C,0x3C,0x7E,0x3C,0x18,0x00,
	0x08,0x7C,0x7E,0x7E,0x7C,0x08,0x00,0x00,0x10,0x3E,0x7E,0x7E,0x3E,0x10,0x00,0x00,
	0x58,0x2A,0xDC,0xC8,0xDC,0x2A,0x58,0x00,0x24,0x66,0xFF,0xFF,0x66,0x24,0x00,0x00,
	0x00,0x10,0x10,0x38,0x38,0x7C,0xFE,0x00,0xFE,0x7C,0x38,0x38,0x10,0x10,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1C,0x1C,0x1C,0x18,0x00,0x18,0x18,0x00,
	0x6C,0x6C,0x24,0x00,0x00,0x00,0x00,0x00,0x00,0x28,0x7C,0x28,0x7C,0x28,0x00,0x00,
	0x10,0x38,0x60,0x38,0x0C,0x78,0x10,0x00,0x40,0xA4,0x48,0x10,0x24,0x4A,0x04,0x00,
	0x18,0x34,0x18,0x3A,0x6C,0x66,0x3A,0x00,0x18,0x18,0x20,0x00,0x00,0x00,0x00,0x00,
	0x30,0x60,0x60,0x60,0x60,0x60,0x30,0x00,0x0C,0x06,0x06,0x06,0x06,0x06,0x0C,0x00,
	0x10,0x54,0x38,0x7C,0x38,0x54,0x10,0x00,0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00,
	0x00,0x00,0x00,0x00,0x18,0x18,0x30,0x00,0x00,0x00,0x00,0x00,0x3E,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x04,0x08,0x10,0x20,0x40,0x00,0x00,
	0x38,0x4C,0xC6,0xC6,0xC6,0x64,0x38,0x00,0x18,0x38,0x18,0x18,0x18,0x18,0x7E,0x00,
	0x7C,0xC6,0x0E,0x3C,0x78,0xE0,0xFE,0x00,0x7E,0x0C,0x18,0x3C,0x06,0xC6,0x7C,0x00,
	0x1C,0x3C,0x6C,0xCC,0xFE,0x0C,0x0C,0x00,0xFC,0xC0,0xFC,0x06,0x06,0xC6,0x7C,0x00,
	0x3C,0x60,0xC0,0xFC,0xC6,0xC6,0x7C,0x00,0xFE,0xC6,0x0C,0x18,0x30,0x30,0x30,0x00,
	0x78,0xC4,0xE4,0x78,0x86,0x86,0x7C,0x00,0x7C,0xC6,0xC6,0x7E,0x06,0x0C,0x78,0x00,
	0x00,0x00,0x18,0x00,0x00,0x18,0x00,0x00,0x00,0x00,0x18,0x00,0x00,0x18,0x18,0x30,
	0x1C,0x38,0x70,0xE0,0x70,0x38,0x1C,0x00,0x00,0x7C,0x00,0x00,0x7C,0x00,0x00,0x00,
	0x70,0x38,0x1C,0x0E,0x1C,0x38,0x70,0x00,0x7C,0xC6,0xC6,0x1C,0x18,0x00,0x18,0x00,
	0x3C,0x42,0x99,0xA1,0xA5,0x99,0x42,0x3C,0x38,0x6C,0xC6,0xC6,0xFE,0xC6,0xC6,0x00,
	0xFC,0xC6,0xC6,0xFC,0xC6,0xC6,0xFC,0x00,0x3C,0x66,0xC0,0xC0,0xC0,0x66,0x3C,0x00,
	0xF8,0xCC,0xC6,0xC6,0xC6,0xCC,0xF8,0x00,0xFE,0xC0,0xC0,0xFC,0xC0,0xC0,0xFE,0x00,
	0xFE,0xC0,0xC0,0xFC,0xC0,0xC0,0xC0,0x00,0x3E,0x60,0xC0,0xCE,0xC6,0x66,0x3E,0x00,
	0xC6,0xC6,0xC6,0xFE,0xC6,0xC6,0xC6,0x00,0x7E,0x18,0x18,0x18,0x18,0x18,0x7E,0x00,
	0x06,0x06,0x06,0x06,0xC6,0xC6,0x7C,0x00,0xC6,0xCC,0xD8,0xF0,0xF8,0xDC,0xCE,0x00,
	0x60,0x60,0x60,0x60,0x60,0x60,0x7E,0x00,0xC6,0xEE,0xFE,0xFE,0xD6,0xC6,0xC6,0x00,
	0xC6,0xE6,0xF6,0xFE,0xDE,0xCE,0xC6,0x00,0x7C,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00,
	0xFC,0xC6,0xC6,0xC6,0xFC,0xC0,0xC0,0x00,0x7C,0xC6,0xC6,0xC6,0xDE,0xCC,0x7A,0x00,
	0xFC,0xC6,0xC6,0xCE,0xF8,0xDC,0xCE,0x00,0x78,0xCC,0xC0,0x7C,0x06,0xC6,0x7C,0x00,
	0x7E,0x18,0x18,0x18,0x18,0x18,0x18,0x00,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00,
	0xC6,0xC6,0xC6,0xEE,0x7C,0x38,0x10,0x00,0xC6,0xC6,0xD6,0xFE,0xFE,0xEE,0xC6,0x00,
	0xC6,0xEE,0x3C,0x38,0x7C,0xEE,0xC6,0x00,0x66,0x66,0x66,0x3C,0x18,0x18,0x18,0x00,
	0xFE,0x0E,0x1C,0x38,0x70,0xE0,0xFE,0x00,0x3C,0x30,0x30,0x30,0x30,0x30,0x3C,0x00,
	0x60,0x60,0x30,0x18,0x0C,0x06,0x06,0x00,0x3C,0x0C,0x0C,0x0C,0x0C,0x0C,0x3C,0x00,
	0x18,0x3C,0x66,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,
	0x30,0x30,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x3C,0x06,0x3E,0x66,0x66,0x3C,0x00,
	0x60,0x7C,0x66,0x66,0x66,0x66,0x7C,0x00,0x00,0x3C,0x66,0x60,0x60,0x66,0x3C,0x00,
	0x06,0x3E,0x66,0x66,0x66,0x66,0x3E,0x00,0x00,0x3C,0x66,0x66,0x7E,0x60,0x3C,0x00,
	0x1C,0x30,0x78,0x30,0x30,0x30,0x30,0x00,0x00,0x3E,0x66,0x66,0x66,0x3E,0x06,0x3C,
	0x60,0x7C,0x76,0x66,0x66,0x66,0x66,0x00,0x18,0x00,0x38,0x18,0x18,0x18,0x18,0x00,
	0x0C,0x00,0x1C,0x0C,0x0C,0x0C,0x0C,0x38,0x60,0x60,0x66,0x6C,0x78,0x6C,0x66,0x00,
	0x38,0x18,0x18,0x18,0x18,0x18,0x18,0x00,0x00,0xEC,0xFE,0xFE,0xFE,0xD6,0xC6,0x00,
	0x00,0x7C,0x76,0x66,0x66,0x66,0x66,0x00,0x00,0x3C,0x66,0x66,0x66,0x66,0x3C,0x00,
	0x00,0x7C,0x66,0x66,0x66,0x7C,0x60,0x60,0x00,0x3E,0x66,0x66,0x66,0x3E,0x06,0x06,
	0x00,0x7E,0x70,0x60,0x60,0x60,0x60,0x00,0x00,0x3C,0x60,0x3C,0x06,0x66,0x3C,0x00,
	0x30,0x78,0x30,0x30,0x30,0x30,0x1C,0x00,0x00,0x66,0x66,0x66,0x66,0x6E,0x3E,0x00,
	0x00,0x66,0x66,0x66,0x66,0x3C,0x18,0x00,0x00,0xC6,0xD6,0xFE,0xFE,0x7C,0x6C,0x00,
	0x00,0x66,0x3C,0x18,0x3C,0x66,0x66,0x00,0x00,0x66,0x66,0x66,0x66,0x3E,0x06,0x3C,
	0x00,0x7E,0x0C,0x18,0x30,0x60,0x7E,0x00,0x0E,0x18,0x0C,0x38,0x0C,0x18,0x0E,0x00,
	0x18,0x18,0x18,0x00,0x18,0x18,0x18,0x00,0x70,0x18,0x30,0x1C,0x30,0x18,0x70,0x00,
	0x00,0x00,0x76,0xDC,0x00,0x00,0x00,0x00,0x10,0x28,0x10,0x54,0xAA,0x44,0x00,0x00,
};

static void gp2x_text(unsigned char *screen, int x, int y, char *text, int color)
{
	unsigned int i,l;
	screen=screen+x+y*gp2x_text_width;

	for (i=0;i<strlen(text);i++)
	{
		for (l=0;l<8;l++)
		{
			screen[l*gp2x_text_width+0]=(fontdata8x8[((text[i])*8)+l]&0x80)?color:screen[l*gp2x_text_width+0];
			screen[l*gp2x_text_width+1]=(fontdata8x8[((text[i])*8)+l]&0x40)?color:screen[l*gp2x_text_width+1];
			screen[l*gp2x_text_width+2]=(fontdata8x8[((text[i])*8)+l]&0x20)?color:screen[l*gp2x_text_width+2];
			screen[l*gp2x_text_width+3]=(fontdata8x8[((text[i])*8)+l]&0x10)?color:screen[l*gp2x_text_width+3];
			screen[l*gp2x_text_width+4]=(fontdata8x8[((text[i])*8)+l]&0x08)?color:screen[l*gp2x_text_width+4];
			screen[l*gp2x_text_width+5]=(fontdata8x8[((text[i])*8)+l]&0x04)?color:screen[l*gp2x_text_width+5];
			screen[l*gp2x_text_width+6]=(fontdata8x8[((text[i])*8)+l]&0x02)?color:screen[l*gp2x_text_width+6];
			screen[l*gp2x_text_width+7]=(fontdata8x8[((text[i])*8)+l]&0x01)?color:screen[l*gp2x_text_width+7];
		}
		screen+=8;
	}

#ifdef GP2X
	if (gp2x_bordertvout)
	{
		screen+=((((gp2x_bordertvout_width*gp2x_text_width)/100)+7)&~7)/2;
		if (gp2x_rotate)
			screen+=(((((gp2x_bordertvout_height*(gp2x_rotate&1?visible_game_width:visible_game_height))/100)+7)&~7)/2)*gp2x_text_width;
		else
			screen+=(((((gp2x_bordertvout_height*240)/100)+7)&~7)/2)*gp2x_text_width;
	}
#endif

#if !defined(GP2X) && defined(GP2X_SDLWRAPPER_NODOUBLEBUFFER)
	if (gp2x_sdlwrapper_is_single)
		gp2x_video_flip_single();
#endif
}


/* Game List Text Out */
void gp2x_gamelist_text_out(int x, int y, char *eltexto) {
	char texto[33];
	strncpy(texto,eltexto,32);
	texto[32]=0;
	if (texto[0]!='-')
		gp2x_text(gp2x_screen8,x+1,y+1,texto,0);
	gp2x_text(gp2x_screen8,x,y,texto,255);
}

/* Variadic functions guide found at http://www.unixpapa.com/incnote/variadic.html */
void gp2x_gamelist_text_out_fmt(int x, int y, char* fmt, ...)
{
	char strOut[128];
	va_list marker;
	
	va_start(marker, fmt);
	vsprintf(strOut, fmt, marker);
	va_end(marker);	

	gp2x_gamelist_text_out(x, y, strOut);
}

/* Debug information function*/
void gp2x_text_out(int x, int y, char *texto) {
	gp2x_text(gp2x_screen8,x,y,texto,1);
#ifdef OUTPUT_VIDEO_TEXT
	puts(texto);
#endif
}

/* Debug information function*/
void gp2x_text_out_int(int x, int y, int entero) {
	char texto[50];
	sprintf(texto,"%d\n",entero);
	gp2x_text(gp2x_screen8,x,y,texto,1);
#ifdef OUTPUT_VIDEO_TEXT
	puts(texto);
#endif
}

static int log=0;

/* Set Debug Log start at row 0 */
void gp2x_gamelist_zero(void) {
	log=0;
}

/* Pause Text Message */
void gp2x_text_pause(void) {
	gp2x_video_flip_single();
#ifdef GP2X
	if (gp2x_showfps) {
		int offsetx=0;
		int offsety=0;
		if (gp2x_bordertvout) {
			offsetx+=((((gp2x_bordertvout_width*gp2x_text_width)/100)+7)&~7)/2;
			offsety+=((((gp2x_bordertvout_height*(gp2x_rotate&1?visible_game_width:visible_game_height))/100)+7)&~7)/2;
		}
		fast_memset(&gp2x_screen8[(0+offsety)*gp2x_text_width]+offsetx,0,48);
		fast_memset(&gp2x_screen8[(1+offsety)*gp2x_text_width]+offsetx,0,48);
		fast_memset(&gp2x_screen8[(2+offsety)*gp2x_text_width]+offsetx,0,48);
		fast_memset(&gp2x_screen8[(3+offsety)*gp2x_text_width]+offsetx,0,48);
		fast_memset(&gp2x_screen8[(4+offsety)*gp2x_text_width]+offsetx,0,48);
		fast_memset(&gp2x_screen8[(5+offsety)*gp2x_text_width]+offsetx,0,48);
		fast_memset(&gp2x_screen8[(6+offsety)*gp2x_text_width]+offsetx,0,48);
		fast_memset(&gp2x_screen8[(7+offsety)*gp2x_text_width]+offsetx,0,48);
	}
	gp2x_text(gp2x_screen8,1,1,"PAUSE\0",0);
	gp2x_text(gp2x_screen8,0,0,"PAUSE\0",1);
#else
#ifdef PSP_RES
	gp2x_text(gp2x_screen8,190,160,"Arcade  Paused!\0",0);
	gp2x_text(gp2x_screen8,190,160,"Arcade  Paused!\0",1);
	gp2x_text(gp2x_screen8,190,170,"L+R to Continue\0",0);
	gp2x_text(gp2x_screen8,190,170,"L+R to Continue\0",1);
	gp2x_text(gp2x_screen8,190,180,"or HOME to Quit\0",0);
	gp2x_text(gp2x_screen8,190,180,"or HOME to Quit\0",1);
#else
	gp2x_text(gp2x_screen8,141,111,"PAUSE\0",0);
	gp2x_text(gp2x_screen8,140,110,"PAUSE\0",1);
#endif
#endif
	gp2x_video_flip_single();
}

/* Debug Log information funcion */
void gp2x_text_log(char *texto) {
	if (!log)
	{
#if !defined(GP2X) && !defined(NO_SPLASH)
		load_bmp_8bpp(gp2x_screen8,gp2xsplash_bmp);
#else
		fast_memset(gp2x_screen8,0,GP2X_BYTE_SIZE_SCREEN);
#endif
	}
	gp2x_text_out(0,log,texto);
	log+=8;
#ifdef PSP_RES
	if(log>271) log=0;
#else
	if(log>239) log=0;
#endif
}

/* Debug Log information funcion */
void gp2x_text_log_int(int entero) {
	if (!log)
	{
#if !defined(GP2X) && !defined(NO_SPLASH)
		load_bmp_8bpp(gp2x_screen8,gp2xsplash_bmp);
#else
		fast_memset(gp2x_screen8,0,GP2X_BYTE_SIZE_SCREEN);
#endif
	}
	gp2x_text_out_int(0,log,entero);
	log+=8;
#ifdef PSP_RES
	if(log>271) log=0;
#else
	if(log>239) log=0;
#endif
}

/* Variadic functions guide found at http://www.unixpapa.com/incnote/variadic.html */
void gp2x_text_log_fmt(char* fmt, ...)
{
	char strOut[128];
	va_list marker;
	
	va_start(marker, fmt);
	vsprintf(strOut, fmt, marker);
	va_end(marker);	

	gp2x_text_log(strOut);
}

/* Create a bitmap. Also calls osd_clearbitmap() to appropriately initialize */
/* it to the background color. */
/* VERY IMPORTANT: the function must allocate also a "safety area" 8 pixels wide all */
/* around the bitmap. This is required because, for performance reasons, some graphic */
/* routines don't clip at boundaries of the bitmap. */
struct osd_bitmap *osd_new_bitmap(int width,int height,int depth)       /* ASG 980209 */
{
	struct osd_bitmap *bitmap;
	
	if (Machine->orientation & ORIENTATION_SWAP_XY)
	{
		int temp;

		temp = width;
		width = height;
		height = temp;
	}
	

	if ((bitmap = (struct osd_bitmap*)gp2x_malloc(sizeof(struct osd_bitmap))) != 0)
	{
		int i,rowlen,rdwidth;
		unsigned char *bm;
		int safety;

		if (width > 32) safety = 8;
		else safety = 0;        /* don't create the safety area for GfxElement bitmaps */

		if (depth != 8 && depth != 16) depth = 8;

		bitmap->depth = depth;
		bitmap->width = width;
		bitmap->height = height;

		rdwidth = (width + 7) & ~7;     /* round width to a quadword */
		if (depth == 16)
			rowlen = 2 * (rdwidth + 2 * safety) * sizeof(unsigned char);
		else
			rowlen =     (rdwidth + 2 * safety) * sizeof(unsigned char);

		if (!(bitmap->_private = gp2x_malloc((height + 2 * safety) * rowlen)))
		{
#ifndef DREAMCAST
			gp2x_text_log("osd_new_bitmap(): Out of Memory");
			gp2x_free(bitmap);
#endif
			return 0;
		}

		bm = (unsigned char*)bitmap->_private;
		fast_memset(bm,0,(height + 2 * safety) * rowlen);

		if (!(bitmap->line = (unsigned char**)gp2x_malloc(height * sizeof(unsigned char *))))
		{
#ifndef DREAMCAST
			gp2x_text_log("osd_new_bitmap(): Out of Memory");
			gp2x_free(bm);
			gp2x_free(bitmap);
#endif
			return 0;
		}

		for (i = 0;i < height;i++)
			bitmap->line[i] = &bm[(i + safety) * rowlen + safety];
	}
#ifndef DREAMCAST
	if(!bitmap)
		gp2x_text_log("osd_new_bitmap(): Out of Memory");
#endif
	return bitmap;
}



/* set the bitmap to black */
void osd_clearbitmap(struct osd_bitmap *bitmap)
{
	if (bitmap->_private==bitmap->line[0])
		fast_memset(bitmap->_private,0,bitmap->width*bitmap->height);
	else
		fast_memset(bitmap->_private,0,(bitmap->width+16)*(bitmap->height+16));
/*
	int i;
	for (i = 0;i < bitmap->height;i++)
		fast_memset(bitmap->line[i],0,bitmap->width);
*/
}



void osd_free_bitmap(struct osd_bitmap *bitmap)
{
	if (bitmap)
	{
		gp2x_free(bitmap->line);
		bitmap->line=NULL;
		gp2x_free(bitmap->_private);
		bitmap->_private=NULL;
		gp2x_free(bitmap);
		bitmap=NULL;
	}
}


/* Create a display screen, or window, large enough to accomodate a bitmap */
/* of the given dimensions. Attributes are the ones defined in driver.h. */
/* Return a osd_bitmap pointer or 0 in case of error. */
struct osd_bitmap *osd_create_display(int width,int height,int attributes, int visible_width, int visible_height,
					                  int visible_width_offset, int visible_height_offset )
{
	int     i;
	game_width = width;
	game_height = height;

	if (attributes & VIDEO_TYPE_VECTOR) {
		visible_game_width = width;
		visible_game_height = height;
		visible_game_width_offset = 0;
		visible_game_height_offset = 0;
	} else {
		visible_game_width = visible_width;
		visible_game_height = visible_height;
		visible_game_width_offset = visible_width_offset;
		visible_game_height_offset = visible_height_offset;
	}

	/* Offset alignment */
	{
		int alignment;
		alignment= visible_game_width_offset % 4;
		if (alignment) {
			visible_game_width_offset-=alignment;
			visible_game_width+=alignment;	
		}
		alignment= visible_game_width % 4;
		if (alignment) {
			visible_game_width+=(4-alignment);
		}
		alignment= visible_game_height_offset % 4;
		if (alignment) {
			visible_game_height_offset-=alignment;
			visible_game_height+=alignment;	
		}
		alignment= visible_game_height % 4;
		if (alignment) {
			visible_game_height+=(4-alignment);
		}
	}

	if (Machine->orientation & ORIENTATION_SWAP_XY)
	{
		int temp;

		temp = game_width;
		game_width = game_height;
		game_height = temp;
		temp = visible_game_width;
		visible_game_width = visible_game_height;
		visible_game_height = temp;
		temp = visible_game_width_offset;
		visible_game_width_offset = visible_game_height_offset;
		visible_game_height_offset = temp;
	}

	if (Machine->orientation & ORIENTATION_FLIP_X)    /* offset by other side */
		visible_game_width_offset = game_width-visible_game_width-visible_game_width_offset;

	if (Machine->orientation & ORIENTATION_FLIP_Y)
		visible_game_height_offset = game_height-visible_game_height-visible_game_height_offset;
	
	gp2x_adjust_display();

	frame_buffer=1;
	scrbitmap = osd_new_bitmap(width,height,8);
	frame_buffer=0;
	if (!scrbitmap) return 0;

	for (i = 0;i < 256;i++)
	{
		dirtycolor[i] = 1;
	}
	dirtypalette = 1;

    	return scrbitmap;
}


/* shut up the display */
void osd_close_display(void)
{
	gp2x_clear_screen();
	if (scrbitmap)
	{
		osd_free_bitmap(scrbitmap);
		scrbitmap = NULL;
	}
}


/* palette is an array of 'totalcolors' R,G,B triplets. The function returns */
/* in *pens the pen values corresponding to the requested colors. */
/* If 'totalcolors' is 32768, 'palette' is ignored and the *pens array is filled */
/* with pen values corresponding to a 5-5-5 15-bit palette */
void osd_allocate_colors(unsigned int totalcolors,const unsigned char *palette,unsigned short *pens)
{

	unsigned int i;

	/* initialize the palette */
	for (i = 0;i < 256;i++)
		current_palette[i][0] = current_palette[i][1] = current_palette[i][2] = 0;

	if (totalcolors >= 255)
	{
		int bestblack,bestwhite;
		int bestblackscore,bestwhitescore;

		for (i = 0;i < totalcolors;i++)
			pens[i] = i;

		bestblack = bestwhite = 0;
		bestblackscore = 3*255*255;
		bestwhitescore = 0;
		for (i = 0;i < totalcolors;i++)
		{
			int r,g,b,score;

			r = palette[3*i];
			g = palette[3*i+1];
			b = palette[3*i+2];
			score = r*r + g*g + b*b;

			if (score < bestblackscore)
			{
				bestblack = i;
				bestblackscore = score;
			}
			if (score > bestwhitescore)
			{
				bestwhite = i;
				bestwhitescore = score;
			}
		}
	}
	else
	{
		/* reserve color 1 for the user interface text */
		current_palette[1][0] = current_palette[1][1] = current_palette[1][2] = 0xff;

		/* fill the palette starting from the end, so we mess up badly written */
		/* drivers which don't go through Machine->pens[] */
		for (i = 0;i < totalcolors;i++)
			pens[i] = 255-i;
	}

	for (i = 0;i < totalcolors;i++)
	{
		current_palette[pens[i]][0] = palette[3*i];
		current_palette[pens[i]][1] = palette[3*i+1];
		current_palette[pens[i]][2] = palette[3*i+2];
	}
}


void osd_modify_pen(int pen,unsigned char red, unsigned char green, unsigned char blue)
{
	if (scrbitmap->depth != 8)
	{
		return;
	}


	if (current_palette[pen][0] != red ||
			current_palette[pen][1] != green ||
			current_palette[pen][2] != blue)
	{
		current_palette[pen][0] = red;
		current_palette[pen][1] = green;
		current_palette[pen][2] = blue;

		dirtycolor[pen] = 1;
		dirtypalette = 1;
	}
}


void osd_get_pen(int pen,unsigned char *red, unsigned char *green, unsigned char *blue)
{
	*red = current_palette[pen][0];
	*green = current_palette[pen][1];
	*blue = current_palette[pen][2];
}


void gp2x_clear_screen(void)
{
	/* Clean screens */
   	gp2x_video_flip();
   	fast_memset(gp2x_screen8, 0, GP2X_BYTE_SIZE_SCREEN);
   	gp2x_video_flip();
   	fast_memset(gp2x_screen8, 0, GP2X_BYTE_SIZE_SCREEN);
   	gp2x_video_flip_single();
}

#ifdef PSP
unsigned long prev;
/* Wait for correct synchronization */
INLINE void update_timing (void)
{
	unsigned long curr=gp2x_timer_read();
	unsigned long previ=prev;
	unsigned long interval=((frameskip+1) * 1000)/(Machine->drv->frames_per_second);

	/* now wait until it's time to update the screen */
	if ((curr-previ)<interval) {
		/* Auto-Frameskip -> frameskip-- */
		if (gp2x_frameskip_auto && frameskip>0)
			frameskip--;
		//if (gp2x_vsync!=-1 && ((!gp2x_frameskip_auto) || (!frameskip) || (gp2x_vsync==1)))
		{
			while ((curr - previ) < interval) {
				curr = gp2x_timer_read();
			}
		}
	} else if ((curr-previ)>(interval)) {
		/* Auto-Frameskip -> frameskip++ */
		if (gp2x_frameskip_auto && frameskip<gp2x_frameskip)
			frameskip++;
	}
	prev = curr;
}
#endif

#ifdef GP2X
static int gp2x_fps;
int osd_update_frameskip (void)
{
	static int framecount=0;
	static int framecount_pal=0;

	gp2x_fps=Machine->drv->frames_per_second;
	
	if (gp2x_pal_50hz)
	{
		/* In PAL mode, force a maximum of 50 Hz */
		framecount_pal++;
		int frames_not_skipped;
		switch (gp2x_fps)
		{
			case 50: frames_not_skipped=0; break;
			case 30: gp2x_fps=25; frames_not_skipped=5; break;
			default: gp2x_fps=50; frames_not_skipped=5; break;
		}
		if (frames_not_skipped)
		{
			if (framecount_pal>frames_not_skipped)
			{
				framecount_pal=0;
				return 1; /* Skip this frame */
			}
		}
	}

	framecount++;
	if (framecount <= frameskip)
		return 1; /* Skip this frame */
	else
	{
		framecount = 0;
		return 0; /* Draw this frame */
	}
}

/* Wait for correct synchronization */
INLINE void update_timing (void)
{
	static unsigned long prev=0;
	unsigned long curr=gp2x_timer_read_real();
	unsigned long interval=(((unsigned long)((frameskip+1)*1000)) * gp2x_timer_read_scale())/((unsigned long)(gp2x_fps));

	/* now wait until it's time to update the screen */
	if ((curr-prev)<interval) {
		/* Auto-Frameskip -> frameskip-- */
		if (gp2x_frameskip_auto && frameskip>0)
			frameskip--;
		if (gp2x_vsync!=-1) {
			while ((curr - prev) < interval) {
				curr = gp2x_timer_read_real();
			}
		}
	} else if ((curr-prev)>(interval)) {
		/* Auto-Frameskip -> frameskip++ */
		if (gp2x_frameskip_auto && frameskip<gp2x_frameskip)
			frameskip++;
	}
	prev = gp2x_timer_read_real();
	if (gp2x_vsync==1)
		gp2x_video_wait_vsync(); /* Wait VSync */
}
/* Show FPS Counter */
INLINE void frame_counter (void)
{
	static unsigned int gp2x_fc=0; /* Elapsed frame counter */
	static unsigned int gp2x_fc_v=0; /* Frame Counter to show */
	static unsigned long gp2x_fc_t0; /* Last T0 */
	unsigned long gp2x_fc_t1=gp2x_timer_read_real();
	char texto[8];
	int offsetx=0;
	int offsety=0;
	gp2x_fc++;
	if (gp2x_fc_t1-gp2x_fc_t0>=1000*gp2x_timer_read_scale()) {
		gp2x_fc_t0=gp2x_fc_t1;
		gp2x_fc_v=gp2x_fc;
		gp2x_fc=0;
	}
	sprintf(texto,"%d/%d",gp2x_fc_v,gp2x_fps);
	if (gp2x_bordertvout) {
		offsetx+=((((gp2x_bordertvout_width*gp2x_text_width)/100)+7)&~7)/2;
		offsety+=((((gp2x_bordertvout_height*(gp2x_rotate&1?visible_game_width:visible_game_height))/100)+7)&~7)/2;
	}
	fast_memset(&gp2x_screen8[(0+offsety)*gp2x_text_width]+offsetx,0,48);
	fast_memset(&gp2x_screen8[(1+offsety)*gp2x_text_width]+offsetx,0,48);
	fast_memset(&gp2x_screen8[(2+offsety)*gp2x_text_width]+offsetx,0,48);
	fast_memset(&gp2x_screen8[(3+offsety)*gp2x_text_width]+offsetx,0,48);
	fast_memset(&gp2x_screen8[(4+offsety)*gp2x_text_width]+offsetx,0,48);
	fast_memset(&gp2x_screen8[(5+offsety)*gp2x_text_width]+offsetx,0,48);
	fast_memset(&gp2x_screen8[(6+offsety)*gp2x_text_width]+offsetx,0,48);
	fast_memset(&gp2x_screen8[(7+offsety)*gp2x_text_width]+offsetx,0,48);
	gp2x_text(gp2x_screen8,1,1,texto,0);
	gp2x_text(gp2x_screen8,0,0,texto,1);
}

#else
static unsigned proximo_frameskip=0;
static unsigned frameskip_umbral=20;
static unsigned frameskip_medio=10;
unsigned mame4all_totalframes=0;
#ifdef DEBUG_FRAMERATE
unsigned mame4all_frameskipped=0;
unsigned mame4all_frameswaiting=0;
float mame4all_framerate=0.0;
extern float mame4all_real_framerate;
extern unsigned mame4all_real_totalframes;
void print_frameskip(void)
{
	float sk=(((float)mame4all_frameskipped)*100.0)/((float)mame4all_totalframes);
	float wt=(((float)mame4all_frameswaiting)*100.0)/((float)mame4all_totalframes);
	printf("%i frames, skipped %i (%.2f%%), waiting %i (%.2f%%)\n",mame4all_totalframes,mame4all_frameskipped,sk,mame4all_frameswaiting,wt);
	printf("Machine framerate %.4f\n", mame4all_framerate);
	printf("Real framerate %.4f (%i frames)\n", mame4all_real_framerate,mame4all_real_totalframes);
}
INLINE void calculate_framerate(unsigned now)
{
	static unsigned start_time=0;
	static unsigned start_numframes=0;
	if (!start_time)
	{
		start_time=now;
		start_numframes=mame4all_totalframes;
	}
	else
	{
		if (now-start_time>=1000)
		{
			if (mame4all_framerate!=0.0)
				mame4all_framerate=(mame4all_framerate+((float)(mame4all_totalframes-start_numframes)))/2.0;
			else
				mame4all_framerate=(float)(mame4all_totalframes-start_numframes);
			start_time=now;
			start_numframes=mame4all_totalframes;
		}
	}
}
#else
#define calculate_framerate(AHORA)
#endif

int osd_update_frameskip(void)
{
	static int cuantos=0;
	unsigned ahora=gp2x_timer_read();

	calculate_framerate(ahora);
	proximo_frameskip+=frameskip_umbral;
	mame4all_totalframes++;
#ifdef DEBUG_FRAMERATE_MAX
	if (mame4all_totalframes>DEBUG_FRAMERATE_MAX)
	{
		print_frameskip();
		exit(0);
	}
#endif
	if ((ahora-frameskip_medio)>proximo_frameskip)
	{
		cuantos++;
		if (cuantos>3) //5)
		{
			proximo_frameskip=ahora+2;
			cuantos=0;
			return 0;
		}
		else
		{
#ifdef DEBUG_FRAMERATE
			mame4all_frameskipped++;
#endif
			return 1;
		}
	}
	else
	{
		if ((ahora+frameskip_medio)<proximo_frameskip)
		{
			if (!cuantos)
			{
#ifdef DEBUG_FRAMERATE
				mame4all_frameswaiting++;
#endif
				gp2x_timer_delay(proximo_frameskip-ahora);
				proximo_frameskip=gp2x_timer_read();
			}
		}
		cuantos=0;
	}
	return 0;
}
void osd_reset_frameskip(void)
{
	unsigned t;
	proximo_frameskip=gp2x_timer_read();
	frameskip_umbral=1000/Machine->drv->frames_per_second;
	t=10000/Machine->drv->frames_per_second;
	t=t-(frameskip_umbral*10);
	if (t>5)
		frameskip_umbral++;
	frameskip_medio=frameskip_umbral>>1;
#ifdef DREAMCAST
	frameskip_medio+=1; //2;
	frameskip_umbral+=1; //3;
#endif
#ifdef DEBUG_FRAMERATE
	printf("Trying to framerate %i fps\n",(Machine->drv->frames_per_second));
	mame4all_frameskipped=0;
	mame4all_frameswaiting=0;
	mame4all_framerate=0.0;
	mame4all_real_framerate=0.0;
	mame4all_real_totalframes=0;
#endif
	mame4all_totalframes=0;
}
#endif

/* Update GP2X Palette */
static void update_palette (void)
{
	int i;
	unsigned char *dirty=dirtycolor;

	for (i = 0;i < 256;i++)
	{
		if (dirty[i])
		{
			dirty[i] = 0;
			gp2x_video_color8(i,(current_palette[i][0]),(current_palette[i][1]),(current_palette[i][2]));
		}
	}
	dirtypalette = 0;

	/* Update Palette */
	gp2x_video_setpalette();
}	



#ifdef GP2X
/* Update Display: Not rotated */
static void update_display (void)
{
	unsigned char *buffer_scr=gp2x_screen8+gp2x_xscreen+(320*gp2x_yscreen);
	const unsigned int buffer_mem_offset = ((unsigned int)(scrbitmap->line[1] - scrbitmap->line[0]));
	unsigned char *buffer_mem = (unsigned char *)(scrbitmap->line[gp2x_yoffset]+gp2x_xoffset);
	const unsigned int width=gp2x_width;
	unsigned int height=gp2x_height>>3;
	do {
		fast_memcpy(buffer_scr,buffer_mem,width); buffer_scr+=320; buffer_mem+=buffer_mem_offset;
		fast_memcpy(buffer_scr,buffer_mem,width); buffer_scr+=320; buffer_mem+=buffer_mem_offset;
		fast_memcpy(buffer_scr,buffer_mem,width); buffer_scr+=320; buffer_mem+=buffer_mem_offset;
		fast_memcpy(buffer_scr,buffer_mem,width); buffer_scr+=320; buffer_mem+=buffer_mem_offset;
		fast_memcpy(buffer_scr,buffer_mem,width); buffer_scr+=320; buffer_mem+=buffer_mem_offset;
		fast_memcpy(buffer_scr,buffer_mem,width); buffer_scr+=320; buffer_mem+=buffer_mem_offset;
		fast_memcpy(buffer_scr,buffer_mem,width); buffer_scr+=320; buffer_mem+=buffer_mem_offset;
		fast_memcpy(buffer_scr,buffer_mem,width); buffer_scr+=320; buffer_mem+=buffer_mem_offset;
		height--;
	} while (height);
}	

static void update_display_bordertvout (void)
{
	unsigned char *buffer_scr=gp2x_screen8+gp2x_xscreen+((((((320*gp2x_bordertvout_width)/100)+7)&~7)+320)*gp2x_yscreen)+
		(((((320*gp2x_bordertvout_width)/100)+7)&~7)/2)+
		((320+((((320*gp2x_bordertvout_width)/100)+7)&~7))*(((((240*gp2x_bordertvout_height)/100)+7)&~7)/2));
	const unsigned int buffer_mem_offset = ((unsigned int)(scrbitmap->line[1] - scrbitmap->line[0]));
	unsigned char *buffer_mem = (unsigned char *)(scrbitmap->line[gp2x_yoffset]+gp2x_xoffset);
	const unsigned int width=gp2x_width;
	const unsigned int width_scr=320+((((320*gp2x_bordertvout_width)/100)+7)&~7);
	unsigned int height=gp2x_height>>3;

	do {
		fast_memcpy(buffer_scr,buffer_mem,width); buffer_scr+=width_scr; buffer_mem+=buffer_mem_offset;		
		fast_memcpy(buffer_scr,buffer_mem,width); buffer_scr+=width_scr; buffer_mem+=buffer_mem_offset;		
		fast_memcpy(buffer_scr,buffer_mem,width); buffer_scr+=width_scr; buffer_mem+=buffer_mem_offset;		
		fast_memcpy(buffer_scr,buffer_mem,width); buffer_scr+=width_scr; buffer_mem+=buffer_mem_offset;		
		fast_memcpy(buffer_scr,buffer_mem,width); buffer_scr+=width_scr; buffer_mem+=buffer_mem_offset;		
		fast_memcpy(buffer_scr,buffer_mem,width); buffer_scr+=width_scr; buffer_mem+=buffer_mem_offset;		
		fast_memcpy(buffer_scr,buffer_mem,width); buffer_scr+=width_scr; buffer_mem+=buffer_mem_offset;		
		fast_memcpy(buffer_scr,buffer_mem,width); buffer_scr+=width_scr; buffer_mem+=buffer_mem_offset;		
		height--;
	} while (height);
}

/* Update Display: Scaled */
static void update_display_scaled (void)
{
	unsigned char *buffer_scr=gp2x_screen8;
	const unsigned int buffer_mem_offset = ((unsigned int)(scrbitmap->line[1] - scrbitmap->line[0]));
	unsigned char *buffer_mem = (unsigned char *)(scrbitmap->line[visible_game_height_offset]+visible_game_width_offset);
	const unsigned int width=visible_game_width;
	unsigned int height=visible_game_height>>3;

	do {
		fast_memcpy(buffer_scr,buffer_mem,width); buffer_scr+=width; buffer_mem+=buffer_mem_offset;		
		fast_memcpy(buffer_scr,buffer_mem,width); buffer_scr+=width; buffer_mem+=buffer_mem_offset;		
		fast_memcpy(buffer_scr,buffer_mem,width); buffer_scr+=width; buffer_mem+=buffer_mem_offset;		
		fast_memcpy(buffer_scr,buffer_mem,width); buffer_scr+=width; buffer_mem+=buffer_mem_offset;		
		fast_memcpy(buffer_scr,buffer_mem,width); buffer_scr+=width; buffer_mem+=buffer_mem_offset;		
		fast_memcpy(buffer_scr,buffer_mem,width); buffer_scr+=width; buffer_mem+=buffer_mem_offset;		
		fast_memcpy(buffer_scr,buffer_mem,width); buffer_scr+=width; buffer_mem+=buffer_mem_offset;		
		fast_memcpy(buffer_scr,buffer_mem,width); buffer_scr+=width; buffer_mem+=buffer_mem_offset;		
		height--;
	} while (height);
}	

static void update_display_scaled_bordertvout (void)
{
	unsigned char *buffer_scr=gp2x_screen8+(((((visible_game_width*gp2x_bordertvout_width)/100)+7)&~7)/2)+
		((visible_game_width+((((visible_game_width*gp2x_bordertvout_width)/100)+7)&~7))*(((((visible_game_height*gp2x_bordertvout_height)/100)+7)&~7)/2));
	const unsigned int buffer_mem_offset = ((unsigned int)(scrbitmap->line[1] - scrbitmap->line[0]));
	unsigned char *buffer_mem = (unsigned char *)(scrbitmap->line[visible_game_height_offset]+visible_game_width_offset);
	const unsigned int width=visible_game_width;
	const unsigned int width_scr=visible_game_width+((((visible_game_width*gp2x_bordertvout_width)/100)+7)&~7);
	unsigned int height=visible_game_height>>3;

	do {
		fast_memcpy(buffer_scr,buffer_mem,width); buffer_scr+=width_scr; buffer_mem+=buffer_mem_offset;		
		fast_memcpy(buffer_scr,buffer_mem,width); buffer_scr+=width_scr; buffer_mem+=buffer_mem_offset;		
		fast_memcpy(buffer_scr,buffer_mem,width); buffer_scr+=width_scr; buffer_mem+=buffer_mem_offset;		
		fast_memcpy(buffer_scr,buffer_mem,width); buffer_scr+=width_scr; buffer_mem+=buffer_mem_offset;		
		fast_memcpy(buffer_scr,buffer_mem,width); buffer_scr+=width_scr; buffer_mem+=buffer_mem_offset;		
		fast_memcpy(buffer_scr,buffer_mem,width); buffer_scr+=width_scr; buffer_mem+=buffer_mem_offset;		
		fast_memcpy(buffer_scr,buffer_mem,width); buffer_scr+=width_scr; buffer_mem+=buffer_mem_offset;		
		fast_memcpy(buffer_scr,buffer_mem,width); buffer_scr+=width_scr; buffer_mem+=buffer_mem_offset;		
		height--;
	} while (height);
}

/* Update Display: Rotated */
static void update_display_rotated (void) 
{
	char *buffer_scr_write;
	int *buffer_mem = (int *)(scrbitmap->line[gp2x_yoffset]+gp2x_xoffset);
	const char *buffer_scr=(char *)gp2x_screen8;
	const int buffer_mem_offset = (((int)(scrbitmap->line[1] - scrbitmap->line[0]))-gp2x_width)>>2;
	int x,y;
	const int xscreen=gp2x_xscreen;
	const int yscreen=gp2x_yscreen;
	const int width=gp2x_width>>3;
	const int height=gp2x_height;
	
	for (y=yscreen+height-1;y>=yscreen;y--) {
		buffer_scr_write=(char *)buffer_scr+y+((xscreen-1)*320);
		x=width;
		do {
			*(buffer_scr_write+=320)= *buffer_mem;
			*(buffer_scr_write+=320)= (*buffer_mem)>>8;
			*(buffer_scr_write+=320)= (*buffer_mem)>>16;
			*(buffer_scr_write+=320)= (*buffer_mem)>>24;
			buffer_mem++;
			*(buffer_scr_write+=320)= *buffer_mem;
			*(buffer_scr_write+=320)= (*buffer_mem)>>8;
			*(buffer_scr_write+=320)= (*buffer_mem)>>16;
			*(buffer_scr_write+=320)= (*buffer_mem)>>24;
			buffer_mem++;
			x--;
		} while (x);
		buffer_mem+=buffer_mem_offset;
	}
}

static void update_display_rotated_bordertvout (void)
{
	char *buffer_scr_write;
	int *buffer_mem = (int *)(scrbitmap->line[gp2x_yoffset]+gp2x_xoffset);
	const char *buffer_scr=(char *)gp2x_screen8;
	const int buffer_mem_offset = (((int)(scrbitmap->line[1] - scrbitmap->line[0]))-gp2x_width)>>2;
	int x,y;
	const int xscreen=gp2x_xscreen+(((((gp2x_bordertvout_width*320)/100)+7)&~7)/2);
	const int yscreen=gp2x_yscreen+(((((gp2x_bordertvout_height*240)/100)+7)&~7)/2);
	const int width=gp2x_width>>3;
	const int height=gp2x_height;
	const int width_scr=320+((((320*gp2x_bordertvout_width)/100)+7)&~7);
	
	for (y=yscreen+height-1;y>=yscreen;y--) {
		buffer_scr_write=(char *)buffer_scr+y+((xscreen-1)*width_scr);
		x=width;
		do {
			*(buffer_scr_write+=width_scr)= *buffer_mem;
			*(buffer_scr_write+=width_scr)= (*buffer_mem)>>8;
			*(buffer_scr_write+=width_scr)= (*buffer_mem)>>16;
			*(buffer_scr_write+=width_scr)= (*buffer_mem)>>24;
			buffer_mem++;
			*(buffer_scr_write+=width_scr)= *buffer_mem;
			*(buffer_scr_write+=width_scr)= (*buffer_mem)>>8;
			*(buffer_scr_write+=width_scr)= (*buffer_mem)>>16;
			*(buffer_scr_write+=width_scr)= (*buffer_mem)>>24;
			buffer_mem++;
			x--;
		} while (x);
		buffer_mem+=buffer_mem_offset;
	}
}

/* Update Display: Scaled Rotated */
static void update_display_scaled_rotated (void) 
{
	char *buffer_scr_write;
	int *buffer_mem = (int *)(scrbitmap->line[visible_game_height_offset]+visible_game_width_offset);
	const char *buffer_scr=(char *)gp2x_screen8;
	const int buffer_mem_offset = (((int)(scrbitmap->line[1] - scrbitmap->line[0]))-visible_game_width)>>2;
	int x,y;
	const int width=visible_game_width>>3;
	const int height=visible_game_height;
	
	for (y=height-1;y>=0;y--) {
		buffer_scr_write=(char *)buffer_scr+y-height;
		x=width;
		do {
			*(buffer_scr_write+=height)= *buffer_mem;
			*(buffer_scr_write+=height)= (*buffer_mem)>>8;
			*(buffer_scr_write+=height)= (*buffer_mem)>>16;
			*(buffer_scr_write+=height)= (*buffer_mem)>>24;
			buffer_mem++;
			*(buffer_scr_write+=height)= *buffer_mem;
			*(buffer_scr_write+=height)= (*buffer_mem)>>8;
			*(buffer_scr_write+=height)= (*buffer_mem)>>16;
			*(buffer_scr_write+=height)= (*buffer_mem)>>24;
			buffer_mem++;
			x--;
		} while(x);
		buffer_mem+=buffer_mem_offset;
	}
}

static void update_display_scaled_rotated_bordertvout (void)
{
	char *buffer_scr_write;
	int *buffer_mem = (int *)(scrbitmap->line[visible_game_height_offset]+visible_game_width_offset);
	const char *buffer_scr=(char *)gp2x_screen8;
	const int buffer_mem_offset = (((int)(scrbitmap->line[1] - scrbitmap->line[0]))-visible_game_width)>>2;
	int x,y;
	const int xscreen=(((((gp2x_bordertvout_width*visible_game_height)/100)+7)&~7)/2);
	const int yscreen=(((((gp2x_bordertvout_height*visible_game_width)/100)+7)&~7)/2);
	const int width=visible_game_width>>3;
	const int height=visible_game_height;
	const int height_scr=visible_game_height+((((visible_game_height*gp2x_bordertvout_width)/100)+7)&~7);
	
	for (y=height-1;y>=0;y--) {
		buffer_scr_write=(char *)buffer_scr+y-height_scr+xscreen+(yscreen*height_scr);
		x=width;
		do {
			*(buffer_scr_write+=height_scr)= *buffer_mem;
			*(buffer_scr_write+=height_scr)= (*buffer_mem)>>8;
			*(buffer_scr_write+=height_scr)= (*buffer_mem)>>16;
			*(buffer_scr_write+=height_scr)= (*buffer_mem)>>24;
			buffer_mem++;
			*(buffer_scr_write+=height_scr)= *buffer_mem;
			*(buffer_scr_write+=height_scr)= (*buffer_mem)>>8;
			*(buffer_scr_write+=height_scr)= (*buffer_mem)>>16;
			*(buffer_scr_write+=height_scr)= (*buffer_mem)>>24;
			buffer_mem++;
			x--;
		} while(x);
		buffer_mem+=buffer_mem_offset;
	}
}

static update_display_func update_display_array[8]=
{
	update_display, update_display_rotated, 
	update_display_scaled, update_display_scaled_rotated,
	update_display_bordertvout, update_display_rotated_bordertvout, 
	update_display_scaled_bordertvout, update_display_scaled_rotated_bordertvout
};
static update_display_func p_update_display=update_display;
void update_p_update_display_func(unsigned n)
{
	p_update_display=update_display_array[n];
}
#else
extern update_display_func p_update_display;
void update_p_update_display_func();
#endif

#ifdef MAME4ALL_BENCH
static void update_bench(void)
{
	static Uint32 t=0;
	static Uint32 init=0;
	if (!t)
		init=SDL_GetTicks();
	t++;
	if (t>MAME4ALL_BENCH)
	{
		Uint32 f=SDL_GetTicks()-init;
		puts("-----");
		printf("BENCH TOTAL TIME %.3f sec\n",(double)f/1000.0);
//		printf("\t\t %.3f FPS\n",(1000.0*MAME4ALL_BENCH)/((double)f));
		puts("-----");
		fflush(stdout);
		exit(0);
	}
}
#endif

/* Update the display. */
void osd_update_display(void)
{
#if defined(GP2X) || defined(PSP)
	/* Update Timing */
	update_timing();
#endif

#ifdef PSP
	/* Wait VSync */
	if (gp2x_vsync==1)
		gp2x_video_wait_vsync();
#endif
	/* Manage the palette */
	if (dirtypalette)
		update_palette();

	/* Update Display */
	p_update_display();

#ifdef GP2X
	/* Show FPS Counter */
	if (gp2x_showfps)
		frame_counter(); 
#endif
	
#ifdef GP2X
	/* Double Buffer Video Flip */
	if (gp2x_double_buffer)
		gp2x_video_flip();
#endif

#ifdef MAME4ALL_BENCH
	update_bench();
#endif
}
