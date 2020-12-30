#define CPS1_MAX_TILE_PATCH
#define CPS1_MEM_PATCH
#define CPS1_DRAWGFX_PATCH
#define CPS1_BITMAP_PATCH
#define CPS1_VISIBLE_PATCH
#define CPS1_FILLBITMAP_PATCH
#define CPS1_MEMSETS_PATCH
#define CPS1_UNROLL_PATCH
// #define CPS1_DREAMCAST_SQ_PATCH

#ifndef GP2X
#ifdef DREAMCAST
#include<kos.h>
#endif
#ifdef INLINE
#undef INLINE
#endif
#define INLINE static __inline__
#endif


/***************************************************************************

  vidhrdw.c

  Functions to emulate the video hardware of the machine.

  Todo:
  7 unknown attribute bits on scroll 2/3.
  Speed, speed, speed...

  OUTPUT PORTS (preliminary)
  0x00-0x01     OBJ RAM base (/256)
  0x02-0x03     Scroll1 RAM base (/256)
  0x04-0x05     Scroll2 RAM base (/256)
  0x06-0x07     Scroll3 RAM base (/256)
  0x08-0x09     "Other" RAM - Scroll distortion (/256)
  0x0a-0x0b     Palette base (/256)
  0x0c-0x0d     Scroll 1 X
  0x0e-0x0f     Scroll 1 Y
  0x10-0x11     Scroll 2 X
  0x12-0x13     Scroll 2 Y
  0x14-0x15     Scroll 3 X
  0x16-0x17     Scroll 3 Y

  0x18-0x19     ????
  0x20-0x21     ????
  0x22-0x23     unknown but widely used - usually 0x0e
  0x24-0x25     ????


  0x4e-0x4f     ????
  0x5e-0x5f     ????

  Registers move from game to game.. following example strider
  0x66-0x67     Video control register (location varies according to game)
  0x6a-0x6b     Priority mask
  0x6c-0x6d     Priority mask
  0x6e-0x6f     Priority mask
  0x70-0x71     Control register (usually 0x003f)

  Fixed registers
  0x80-0x81     ????	    Sound output.
  0x88-0x89     ????	    Port thingy (sound fade???)

  Known Bug List
  ==============
  All games
  * Palette fades overflow palette.
  * Large sprites don't exit smoothly on the left side of the screen (e.g. Car
    in Mega Man attract mode)

  Magic Sword.
  * Distortion effect is mapped wrong.
  * Rogue scroll 2 character at end of level 1
  * during attract mode, characters are shown with a black background. There is
    a background, but the layers are disabled. I think this IS the correct
	behaviour.

  King of Dragons.
  * Distortion effect missing on player selection screen.

  1941
  * Brief flicker of scroll 2 when coining up / starting game.

  Captain Commando
  * Continue screen completely wrong

  SF2
  * Sprite lag on screens.
  * Japanese level still doesn't look 100%

  ffight
  * end of level 1 stairs: can see the player feet through them, and
  if you don't destroy the bin it gets covered by the ones behind it.
  * continue screen has garbage character tiled in the background

  mtwins
  * in the cave (with fire) and underwater, rowscroll is used. This
    doesn't seem to have any visible effect, but I think it is supposed
	to wave, to suggest hot air and water.
  * attract mode screens have garbage character tiled in the background

  cawing
  * priority: at end of level, the plane is drawn above the text
  * in level 2, you have to blow out tunnels in skyscrapers to
    fly thru. However, you can see the tunnel before you actually make it.
	Doesn't seem to be a priority problem, the gfx are just missing.
  * garbage characters inbetween text in attract mode

  knights
  * garbage characters left on screen during boot and under INSERT COIN.

  qad
  * in attract mode, "Dragons" is missing from the title screen, only
    "Quiz &" is visible.

  mercs
  * a few wrong scroll 2 tiles (check part 2 of the first attract mode round)


  Todo
  ====

  Implement CPS2 QSound system


  Unknown issues
  ==============

  There are often some redundant high bits in the scroll layer's attributes.


***************************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"
#include "drivers/cps1.h"
#include "types.h"
#include "osdepend.h"

#ifdef CPS1_MEM_PATCH
#undef READ_WORD
#define READ_WORD(a)          (*(unsigned short *)(a))
#undef WRITE_WORD
#define WRITE_WORD(a,d)       (*(unsigned short *)(a) = (d))
#endif

#ifdef CPS1_VISIBLE_PATCH
	static struct rectangle gp2x_visible_area;
	#ifdef GP2X
		#define MAME4ALL_VISIBLE_AREA (gp2x_rotate?Machine->drv->visible_area:gp2x_visible_area)
	#else
		#define MAME4ALL_VISIBLE_AREA gp2x_visible_area
	#endif
#else
	#define MAME4ALL_VISIBLE_AREA Machine->drv->visible_area
#endif

static int orientation_swap_xy=0;
static struct osd_bitmap *osd_bitmap_bitmap;
static int bitmap_width, bitmap_width_min, bitmap_width_max;
static int bitmap_height, bitmap_height_min, bitmap_height_max;
static char **bitmap_line;
#ifdef CPS1_BITMAP_PATCH
extern int gp2x_xscreen;
extern int gp2x_yscreen;
extern int gp2x_xoffset;
extern int gp2x_yoffset;
extern int gp2x_width;
extern int gp2x_height;
#ifdef GP2X
extern int gp2x_rotate;
#endif
#endif


struct CPS1VIDCFG
{
	int layer_control;
	int priority0;	/* usually 0x00 */
	int priority1;
	int priority2;
	int priority3;
	int control_reg;  /* Control register? Usually contains 0x3f */
	int scrl1_enable_mask;
	int scrl2_enable_mask;
	int scrl3_enable_mask;
	int distort_alt;
};

/* Configuration tables */
/* ideally, the layer enable masks should consist of only one bit, */
/* but in many cases it is unknown which bit is which. */
static struct CPS1VIDCFG cps1_vid_cfg[]=
{
	{0x66,0x68,0x6a,0x6c,0x6e,0x70, 0x38,0x38,0x38,0}, /* 00 = Un Squadron */
	{0x70,0x6e,0x6c,0x6a,0x68,0x66, 0x20,0x10,0x08,0}, /* 01 = Willow */
	{0x6e,0x66,0x70,0x68,0x72,0x6a, 0x02,0x0c,0x0c,1}, /* 02 = Final Fight */
	{0x66,0x68,0x6a,0x6c,0x6e,0x70, 0x02,0x0c,0x0c,1}, /* 03 = ffightu */
	{0x66,0x68,0x6a,0x6c,0x6e,0x70, 0x02,0x0c,0x0c,0}, /* 04 = Strider */
	{0x66,0x68,0x6a,0x6c,0x6e,0x70, 0x02,0x04,0x08,0}, /* 05 = Ghouls */
	{0x68,0x66,0x64,0x62,0x60,0x70, 0x20,0x10,0x02,0}, /* 06 = Knights */
	{0x62,0x64,0x66,0x68,0x6a,0x6c, 0x20,0x06,0x06,2}, /* 07 = Magic Sword */
	{0x42,0x44,0x46,0x48,0x4a,0x4c, 0x04,0x22,0x22,0}, /* 08 = Nemo */
	{0x6c,0x6a,0x68,0x66,0x64,0x62, 0x02,0x04,0x08,0}, /* 09 = DWJ */
	{0x60,0x6e,0x6c,0x6a,0x68,0x70, 0x30,0x08,0x30,0}, /* 10 = KOD */
	{0x4c,0x4a,0x48,0x46,0x44,0x42, 0x10,0x0a,0x0a,0}, /* 11 = Carrier Air Wing */
	{0x54,0x52,0x50,0x4e,0x4c,0x4a, 0x08,0x12,0x12,3}, /* 12 = Street Fighter 2 */
	{0x62,0x64,0x66,0x68,0x6a,0x6c, 0x20,0x06,0x06,3}, /* 13 = sf2j */
	{0x66,0x68,0x6a,0x6c,0x6e,0x70, 0x02,0x0c,0x0c,3}, /* 14 = Street Fighter 2 (Turbo) */
	{0xdc,0xda,0xd8,0xd6,0xd4,0xd2, 0x10,0x0a,0x0a,3}, /* 15 = SF2 (Rev E) */
	{0x68,0x66,0x64,0x62,0x60,0x70, 0x20,0x04,0x08,2}, /* 16 = 3 Wonders */
	{0x6e,0x66,0x70,0x68,0x72,0x6a, 0x02,0x0c,0x0c,1}, /* 17 = Varth */
	{0x60,0x6e,0x6c,0x6a,0x68,0x70, 0x20,0x06,0x06,1}, /* 18 = varthj */
	{0x68,0x6a,0x6c,0x6e,0x70,0x72, 0x02,0x08,0x20,0}, /* 19 = 1941 */
	{0x66,0x68,0x6a,0x6c,0x6e,0x70, 0x02,0x0c,0x0c,0}, /* 20 = Megaman */
	{0x52,0x54,0x56,0x58,0x5a,0x5c, 0x38,0x38,0x38,1}, /* 21 = Mega Twins */
	{0x6c,0x00,0x00,0x00,0x00,0x62, 0x02,0x04,0x08,0}, /* 22 = Mercs (uses port 74) */
	{0x6c,0x00,0x00,0x00,0x00,0x52, 0x16,0x16,0x16,0}, /* 23 = Quiz and Dragons */
	{0x60,0x00,0x00,0x00,0x00,0x70, 0x20,0x14,0x14,0}, /* 24 = cworld2j */
	{0x66,0x00,0x00,0x00,0x00,0x70, 0x0e,0x0e,0x0e,0}, /* 25 = Pnickies */
	{0x66,0x68,0x6a,0x6c,0x6e,0x70, 0x0e,0x0e,0x0e,0}, /* 26 = qadj */
	{0x4a,0x4c,0x4e,0x40,0x42,0x44, 0x16,0x16,0x16,0}, /* 27 = Cadillacs & Dinosaurs */
	{0x52,0x54,0x56,0x48,0x4a,0x4c, 0x04,0x02,0x20,0}, /* 28 = Punisher */
	{0x60,0x6e,0x6c,0x6a,0x68,0x70, 0x20,0x12,0x12,0}, /* 29 = Captain Commando */
	{0x66,0x68,0x6a,0x6c,0x6e,0x70, 0x02,0x04,0x08,0}, /* 30 = wof */
	{0x56,0x40,0x42,0x68,0x6a,0x6c, 0x1c,0x1c,0x1c,0}, /* 31 = Slam Masters */
	{0x6a,0x6c,0x6e,0x70,0x72,0x5c, 0x1c,0x1c,0x1c,0}, /* 32 = Muscle Bomber Duo */
	{0x62,0x64,0x66,0x68,0x6a,0x6c, 0x10,0x08,0x04,0}, /* 33 = wofj */
	{0x66,0x68,0x6a,0x6c,0x6e,0x70, 0x02,0x0c,0x0c,0}, /* 34 = pang3 */
};

#define cps1_port(offset) (READ_WORD(&cps1_output[offset]))
#define cps1_base(offset) (&cps1_gfxram[(((READ_WORD(&cps1_output[offset]))<<8)&0x3ffff)])

int cps1_output_r(int offset)
{
	/* Pang 3 EEPROM interface */
	if (offset == 0x7a && cps1_game_config->gng_sprite_kludge == 5)
		return cps1_eeprom_port_r(0);
	else
		return READ_WORD(&cps1_output[offset]);
}

void cps1_output_w(int offset, int data)
{
	int value;

	COMBINE_WORD_MEM(&cps1_output[offset],data);

	/* protections */
	switch (offset)
	{
		case 0x40:
		case 0x42:
			if (cps1_game_config->alternative==14 )
			{
				/* Only apply protection to sf2t (other games use 0x40-0x47)*/
				value=cps1_port(0x40);
				value*=cps1_port(0x42);
				/* Slight variation ... only one word result */
				WRITE_WORD(&cps1_output[0x44], value&0xffff);
			}
			else if (cps1_game_config->alternative==11)
			{
				/* Carrier Air Wing... unknown protection */
				WRITE_WORD(&cps1_output[0x40], 0x0406);
			}
			break;

		case 0x44:
		case 0x46:
		if (cps1_game_config->alternative==6)
			{
				/* Only apply protection to knights (other games use 0x40-0x47)*/
				value=cps1_port(0x44);
				value*=cps1_port(0x46);
				WRITE_WORD(&cps1_output[0x40], value>>16);
				WRITE_WORD(&cps1_output[0x42], value&0xffff);
			}
			break;

		case 0x4c:
		case 0x4e:
			if (cps1_game_config->alternative==16   /* 3wonders */
					|| cps1_game_config->alternative==18)   /* varthj */
			{
				/* Only apply protection to 3wonders (other games use 0x4c-0x4f)*/
				value=cps1_port(0x4c);
				value*=cps1_port(0x4e);
				WRITE_WORD(&cps1_output[0x48], value>>16);
				WRITE_WORD(&cps1_output[0x4a], value&0xffff);
			}
			break;

		case 0x5c:
		case 0x5e:
		if (cps1_game_config->alternative==10)
			{
				/* Only apply protection to KOD (other games use 0x58-0x5b)*/
				value=cps1_port(0x5c);
				value*=cps1_port(0x5e);
				WRITE_WORD(&cps1_output[0x58], value>>16);
				WRITE_WORD(&cps1_output[0x5a], value&0xffff);
			}
			break;
		case 0x7a:
		/* Pang 3 EEPROM interface */
		if (offset == 0x7a && cps1_game_config->gng_sprite_kludge == 5)
		{
			cps1_eeprom_port_w(0,data);
		}
		break;
	}
}



/* Public variables */
unsigned char *cps1_gfxram;
unsigned char *cps1_output;
int cps1_gfxram_size;
int cps1_output_size;

/* Private */

/* Offset of each palette entry */
#define cps1_obj_palette 0
#define cps1_scroll1_palette 32
#define cps1_scroll2_palette (32+32)
#define cps1_scroll3_palette (32+32+32)
#define cps1_palette_entries (32*4)
#define cps1_scroll1_size 0x4000
#define cps1_scroll2_size 0x4000
#define cps1_scroll3_size 0x4000
#define cps1_obj_size 0x4000
#define cps1_other_size 0x4000
#define cps1_palette_size (cps1_palette_entries*32)
static int cps1_flip_screen=0x8;    /* Flip screen on / off */

static unsigned char *cps1_scroll1;
static unsigned char *cps1_scroll2;
static unsigned char *cps1_scroll3;
static unsigned char *cps1_obj;
static unsigned char *cps1_palette;
static unsigned char *cps1_other;
static unsigned char *cps1_old_palette;

/* Working variables */
static int cps1_last_sprite_offset;     /* Offset of the last sprite */
static int cps1_layer_control;	  /* Layer control register */
static int cps1_layer_enabled[4];       /* Layer enabled [Y/N] */

int scroll1x, scroll1y, scroll2x, scroll2y, scroll3x, scroll3y;
struct CPS1config *cps1_game_config;
static unsigned cps1_scroll2_old_p;
static unsigned char *cps1_scroll2_old;
static struct osd_bitmap *cps1_scroll2_bitmap;


/* Output ports */
#define CPS1_OBJ_BASE	   0x00    /* Base address of objects */
#define CPS1_SCROLL1_BASE       0x02    /* Base address of scroll 1 */
#define CPS1_SCROLL2_BASE       0x04    /* Base address of scroll 2 */
#define CPS1_SCROLL3_BASE       0x06    /* Base address of scroll 3 */
#define CPS1_OTHER_BASE	 0x08    /* Base address of other video */
#define CPS1_PALETTE_BASE       0x0a    /* Base address of palette */
#define CPS1_SCROLL1_SCROLLX    0x0c    /* Scroll 1 X */
#define CPS1_SCROLL1_SCROLLY    0x0e    /* Scroll 1 Y */
#define CPS1_SCROLL2_SCROLLX    0x10    /* Scroll 2 X */
#define CPS1_SCROLL2_SCROLLY    0x12    /* Scroll 2 Y */
#define CPS1_SCROLL3_SCROLLX    0x14    /* Scroll 3 X */
#define CPS1_SCROLL3_SCROLLY    0x16    /* Scroll 3 Y */

#define CPS1_SCROLL2_WIDTH      0x40
#define CPS1_SCROLL2_HEIGHT     0x40
#define CPS1_SCROLL2_SIZE	0x400

/*
CPS1 VIDEO RENDERER

*/
#define cps1_gfx_region 1
static dword *cps1_gfx;		 /* Converted GFX memory */
static int *cps1_char_pen_usage;	/* pen usage array */
static int *cps1_tile16_pen_usage;      /* pen usage array */
static int *cps1_tile32_pen_usage;      /* pen usage array */
static int cps1_max_char;	       /* Maximum number of 8x8 chars */
static int cps1_max_tile16;	     /* Maximum number of 16x16 tiles */
static int cps1_max_tile32;	     /* Maximum number of 32x32 tiles */

#ifdef CPS1_MAX_TILE_PATCH
static int cps1_max_tile16_mask;
static int cps1_max_tile32_mask;
#endif

int cps1_gfx_start(void)
{
	dword dwval;
	int size=Machine->memory_region_length[cps1_gfx_region];
	unsigned char *data = Machine->memory_region[cps1_gfx_region];
	int i,j,nchar,penusage,gfxsize;

	gfxsize=size/4;

	/* Set up maximum values */
	cps1_max_char  =(gfxsize/2)/8;
	cps1_max_tile16=(gfxsize/4)/8;
	cps1_max_tile32=(gfxsize/16)/8;

#ifdef CPS1_MAX_TILE_PATCH
	cps1_max_tile16_mask=cps1_max_tile16-1;
	cps1_max_tile32_mask=cps1_max_tile32-1;
#endif


#ifdef DREAMCAST
	{
		void *punt=(void *)0x04096000; //0x04100000;
		void *punt_mem=Machine->memory_region[cps1_gfx_region];
		fast_memcpy(punt,punt_mem, Machine->memory_region_length[cps1_gfx_region]);
		Machine->memory_region[cps1_gfx_region]=(unsigned char*)punt;
		data=(unsigned char*)punt;
		cps1_gfx=(dword *)punt_mem;
	}
#else
	cps1_gfx=(dword*)gp2x_malloc(gfxsize*sizeof(dword));
	if (!cps1_gfx)
	{
		return -1;
	}
#endif

	cps1_char_pen_usage=(int*)gp2x_malloc(cps1_max_char*sizeof(int));
	if (!cps1_char_pen_usage)
	{
		return -1;
	}
	fast_memset(cps1_char_pen_usage, 0, cps1_max_char*sizeof(int));

	cps1_tile16_pen_usage=(int*)gp2x_malloc(cps1_max_tile16*sizeof(int));
	if (!cps1_tile16_pen_usage)
		return -1;
	fast_memset(cps1_tile16_pen_usage, 0, cps1_max_tile16*sizeof(int));

	cps1_tile32_pen_usage=(int*)gp2x_malloc(cps1_max_tile32*sizeof(int));
	if (!cps1_tile32_pen_usage)
	{
		return -1;
	}
	fast_memset(cps1_tile32_pen_usage, 0, cps1_max_tile32*sizeof(int));

	{
		for (i=0; i<gfxsize>>1; i++)
		{
		       nchar=i>>3;  /* 8x8 char number */
		       dwval=0;
		       for (j=0; j<8; j++)
		       {
				int n,mask;
				n=0;
				mask=0x80>>j;
				if (*(data+(size>>2))&mask)	   n|=1;
				if (*(data+(size>>2)+1)&mask)	 n|=2;
				if (*(data+(size>>1)+(size>>2))&mask)    n|=4;
				if (*(data+(size>>1)+(size>>2)+1)&mask)  n|=8;
				dwval|=n<<(28-(j<<2));
				penusage=1<<n;
				cps1_char_pen_usage[nchar]|=penusage;
				cps1_tile16_pen_usage[nchar>>1]|=penusage;
				cps1_tile32_pen_usage[nchar>>3]|=penusage;
		       }
		       cps1_gfx[i<<1]=dwval;
		       dwval=0;
		       for (j=0; j<8; j++)
		       {
				int n,mask;
				n=0;
				mask=0x80>>j;
				if (*(data)&mask)	  n|=1;
				if (*(data+1)&mask)	n|=2;
				if (*(data+(size>>1))&mask)   n|=4;
				if (*(data+(size>>1)+1)&mask) n|=8;
				dwval|=n<<(28-(j<<2));
				penusage=1<<n;
				cps1_char_pen_usage[nchar]|=penusage;
				cps1_tile16_pen_usage[nchar>>1]|=penusage;
				cps1_tile32_pen_usage[nchar>>3]|=penusage;
		       }
		       cps1_gfx[(i<<1)+1]=dwval;
		       data+=2;
		}
	}
	return 0;
}

void cps1_gfx_stop(void)
{
	if (cps1_gfx)
	{
		gp2x_free(cps1_gfx);
		cps1_gfx=NULL;
	}
	if (cps1_char_pen_usage)
	{
		gp2x_free(cps1_char_pen_usage);
		cps1_char_pen_usage=NULL;
	}
	if (cps1_tile16_pen_usage)
	{
		gp2x_free(cps1_tile16_pen_usage);
		cps1_tile16_pen_usage=NULL;
	}
	if (cps1_tile32_pen_usage)
	{
		gp2x_free(cps1_tile32_pen_usage);
		cps1_tile32_pen_usage=NULL;
	}
}

#ifdef CPS1_UNROLL_PATCH

#include "cps1_unroll.h"

#else

static void cps1_draw_gfx(
	const struct GfxElement *gfx,
	unsigned int code,
	int color,
	int flipx,
	int flipy,
	int sx,
	int sy,
	int tpens,
	int *pusage,
	int size,
	int max,
	int delta,
	int srcdelta)
{
	int i, j, ex, ey;
	dword *src;
	const unsigned short *paldata;

	tpens=(~tpens) & 0xffff;

	if ( code > max || !(tpens & pusage[code]))
	{
		/* No pens to draw (object is empty) */
		return;
	}

	if (orientation_swap_xy)
	{
		int temp;
		temp=sx;
		sx=sy;
		sy=bitmap_height-temp-size;
		temp=flipx;
		flipx=flipy>>1;
		flipy=(temp^1)<<1;
	}

	if (cps1_flip_screen)
	{
		/* Handle flipped screen */
		flipx^=1;
		flipy^=2;
		sx=bitmap_width-sx-size;
		sy=bitmap_height-sy-size;
	}

#ifndef CPS1_BITMAP_PATCH
	if (sx<0 || sx > bitmap_width-size || sy<0 || sy>bitmap_height-size )
#else
	#ifdef GP2X
	if (gp2x_rotate) {
		if (sx<0 || sx > bitmap_width-size || sy<0 || sy>bitmap_height-size ) return;
	} else 
	#endif
	if (sx<bitmap_width_min-size || sx > bitmap_width_max+size || sy<bitmap_height_min-size || sy>bitmap_height_max+size ) return;
#endif

	src = cps1_gfx+code*delta;
	paldata = &gfx->colortable[gfx->color_granularity * color];

	if (orientation_swap_xy)
	{
		if (flipy)
		{
		      sy+=size;
		      if (flipx)
		      {
				sx+=size;
				for (i=0; i<size; i++)
				{
					int ny=sy;
					for (j=0; j<(size>>3); j++)
					{
					      register unsigned char *bm;
					      register unsigned dwval=*src;
					      register unsigned n=(dwval>>28)&0x0f;
					      bm=(unsigned char *)bitmap_line[--ny]+sx;
					      if (tpens & (0x01 << n)) bm[-1]=paldata[n];
					      n=(dwval>>24)&0x0f;
					      bm=(unsigned char *)bitmap_line[--ny]+sx;
					      if (tpens & (0x01 << n)) bm[-1]=paldata[n];
					      n=(dwval>>20)&0x0f;
					      bm=(unsigned char *)bitmap_line[--ny]+sx;
					      if (tpens & (0x01 << n)) bm[-1]=paldata[n];
					      n=(dwval>>16)&0x0f;
					      bm=(unsigned char *)bitmap_line[--ny]+sx;
					      if (tpens & (0x01 << n)) bm[-1]=paldata[n];
					      n=(dwval>>12)&0x0f;
					      bm=(unsigned char *)bitmap_line[--ny]+sx;
					      if (tpens & (0x01 << n)) bm[-1]=paldata[n];
					      n=(dwval>>8)&0x0f;
					      bm=(unsigned char *)bitmap_line[--ny]+sx;
					      if (tpens & (0x01 << n)) bm[-1]=paldata[n];
					      n=(dwval>>4)&0x0f;
					      bm=(unsigned char *)bitmap_line[--ny]+sx;
					      if (tpens & (0x01 << n)) bm[-1]=paldata[n];
					      n=dwval&0x0f;
					      bm=(unsigned char *)bitmap_line[--ny]+sx;
					      if (tpens & (0x01 << n)) bm[-1]=paldata[n];
					      src++;
					}
					sx--;
					src+=srcdelta;
				}
		      }
		      else
		      {
				for (i=0; i<size; i++)
				{
					int ny=sy;
					for (j=0; j<(size>>3); j++)
					{
					      register unsigned char *bm;
					      register unsigned dwval=*src;
					      register unsigned n=(dwval>>28)&0x0f;
					      bm=(unsigned char *)bitmap_line[--ny]+sx;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      n=(dwval>>24)&0x0f;
					      bm=(unsigned char *)bitmap_line[--ny]+sx;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      n=(dwval>>20)&0x0f;
					      bm=(unsigned char *)bitmap_line[--ny]+sx;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      n=(dwval>>16)&0x0f;
					      bm=(unsigned char *)bitmap_line[--ny]+sx;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      n=(dwval>>12)&0x0f;
					      bm=(unsigned char *)bitmap_line[--ny]+sx;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      n=(dwval>>8)&0x0f;
					      bm=(unsigned char *)bitmap_line[--ny]+sx;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      n=(dwval>>4)&0x0f;
					      bm=(unsigned char *)bitmap_line[--ny]+sx;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      n=dwval&0x0f;
					      bm=(unsigned char *)bitmap_line[--ny]+sx;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      src++;
					}
					sx++;
					src+=srcdelta;
				}
		       }
		}
		else
		{
		       if (flipx)
		       {
				sx+=size;
				for (i=0; i<size; i++)
				{
					int ny=sy;
					for (j=0; j<(size>>3); j++)
					{
					      register unsigned char *bm;
					      register unsigned dwval=*src;
					      register unsigned n=(dwval>>28)&0x0f;
					      bm=(unsigned char *)bitmap_line[ny++]+sx;
					      if (tpens & (0x01 << n)) bm[-1]=paldata[n];
					      n=(dwval>>24)&0x0f;
					      bm=(unsigned char *)bitmap_line[ny++]+sx;
					      if (tpens & (0x01 << n)) bm[-1]=paldata[n];
					      n=(dwval>>20)&0x0f;
					      bm=(unsigned char *)bitmap_line[ny++]+sx;
					      if (tpens & (0x01 << n)) bm[-1]=paldata[n];
					      n=(dwval>>16)&0x0f;
					      bm=(unsigned char *)bitmap_line[ny++]+sx;
					      if (tpens & (0x01 << n)) bm[-1]=paldata[n];
					      n=(dwval>>12)&0x0f;
					      bm=(unsigned char *)bitmap_line[ny++]+sx;
					      if (tpens & (0x01 << n)) bm[-1]=paldata[n];
					      n=(dwval>>8)&0x0f;
					      bm=(unsigned char *)bitmap_line[ny++]+sx;
					      if (tpens & (0x01 << n)) bm[-1]=paldata[n];
					      n=(dwval>>4)&0x0f;
					      bm=(unsigned char *)bitmap_line[ny++]+sx;
					      if (tpens & (0x01 << n)) bm[-1]=paldata[n];
					      n=dwval&0x0f;
					      bm=(unsigned char *)bitmap_line[ny++]+sx;
					      if (tpens & (0x01 << n)) bm[-1]=paldata[n];
					      src++;
					}
					sx--;
					src+=srcdelta;
				}
			}
			else
			{
				for (i=0; i<size; i++)
				{
					int ny=sy;
					for (j=0; j<(size>>3); j++)
					{
					      register unsigned char *bm;
					      register unsigned dwval=*src;
					      register unsigned n=(dwval>>28)&0x0f;
					      bm=(unsigned char *)bitmap_line[ny++]+sx;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      n=(dwval>>24)&0x0f;
					      bm=(unsigned char *)bitmap_line[ny++]+sx;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      n=(dwval>>20)&0x0f;
					      bm=(unsigned char *)bitmap_line[ny++]+sx;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      n=(dwval>>16)&0x0f;
					      bm=(unsigned char *)bitmap_line[ny++]+sx;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      n=(dwval>>12)&0x0f;
					      bm=(unsigned char *)bitmap_line[ny++]+sx;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      n=(dwval>>8)&0x0f;
					      bm=(unsigned char *)bitmap_line[ny++]+sx;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      n=(dwval>>4)&0x0f;
					      bm=(unsigned char *)bitmap_line[ny++]+sx;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      n=dwval&0x0f;
					      bm=(unsigned char *)bitmap_line[ny++]+sx;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      src++;
					}
					sx++;
					src+=srcdelta;
				 }
			}
		}

	}
	else
	{
		if (flipy)
		{
		      sy+=size-1;
		      if (flipx)
		      {
				sx+=size;
				for (i=0; i<size; i++)
				{
					register unsigned char *bm=(unsigned char *)bitmap_line[sy-i]+sx;
					for (j=0; j<(size>>3); j++)
					{
					      register unsigned dwval=*src;
					      register unsigned n=(dwval>>28)&0x0f;
					      bm--;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      n=(dwval>>24)&0x0f;
					      bm--;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      n=(dwval>>20)&0x0f;
					      bm--;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      n=(dwval>>16)&0x0f;
					      bm--;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      n=(dwval>>12)&0x0f;
					      bm--;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      n=(dwval>>8)&0x0f;
					      bm--;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      n=(dwval>>4)&0x0f;
					      bm--;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      n=dwval&0x0f;
					      bm--;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      src++;
					}
					src+=srcdelta;
				}
		      }
		      else
		      {
				for (i=0; i<size; i++)
				{
					register unsigned char *bm=(unsigned char *)bitmap_line[sy-i]+sx;
					for (j=0; j<(size>>3); j++)
					{
					      register unsigned dwval=*src;
					      register unsigned n=(dwval>>28)&0x0f;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      bm++;
					      n=(dwval>>24)&0x0f;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      bm++;

					      n=(dwval>>20)&0x0f;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      bm++;
					      n=(dwval>>16)&0x0f;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      bm++;
					      n=(dwval>>12)&0x0f;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      bm++;
					      n=(dwval>>8)&0x0f;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      bm++;
					      n=(dwval>>4)&0x0f;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      bm++;
					      n=dwval&0x0f;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      bm++;
					      src++;
					}
					src+=srcdelta;
				}
		       }
		}
		else
		{
		       if (flipx)
		       {
				sx+=size;
				for (i=0; i<size; i++)
				{
					register unsigned char *bm=(unsigned char *)bitmap_line[sy+i]+sx;
					for (j=0; j<(size>>3); j++)
					{
					      register unsigned dwval=*src;
					      register unsigned n=(dwval>>28)&0x0f;
					      bm--;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      n=(dwval>>24)&0x0f;
					      bm--;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      n=(dwval>>20)&0x0f;
					      bm--;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      n=(dwval>>16)&0x0f;
					      bm--;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      n=(dwval>>12)&0x0f;
					      bm--;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      n=(dwval>>8)&0x0f;
					      bm--;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      n=(dwval>>4)&0x0f;
					      bm--;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      n=dwval&0x0f;
					      bm--;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      src++;
					}
					src+=srcdelta;
				}
			}
			else
			{
				for (i=0; i<size; i++)
				{
					register unsigned char *bm=(unsigned char *)bitmap_line[sy+i]+sx;
					for (j=0; j<(size>>3); j++)
					{
					      register unsigned dwval=*src;
					      register unsigned n=(dwval>>28)&0x0f;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      bm++;
					      n=(dwval>>24)&0x0f;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      bm++;
					      n=(dwval>>20)&0x0f;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      bm++;
					      n=(dwval>>16)&0x0f;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      bm++;
					      n=(dwval>>12)&0x0f;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      bm++;
					      n=(dwval>>8)&0x0f;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      bm++;
					      n=(dwval>>4)&0x0f;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      bm++;
					      n=dwval&0x0f;
					      if (tpens & (0x01 << n)) *bm=paldata[n];
					      bm++;
					      src++;
					}
					src+=srcdelta;
				 }
			}
		}
       }
}

#define cps1_draw_scroll1(code,color,flipx,flipy,sx,sy) \
 cps1_draw_gfx(Machine->gfx[0],code,color,flipx,flipy,sx,sy,0x8000,cps1_char_pen_usage,8,cps1_max_char,16,1)
#define cps1_draw_tile16(gfx,code,color,flipx,flipy,sx,sy,tpens) \
 cps1_draw_gfx(gfx,code,color,flipx,flipy,sx,sy,tpens,cps1_tile16_pen_usage,16,cps1_max_tile16,16*2,0)

#define cps1_draw_tile32(gfx,code,color,flipx,flipy,sx,sy,tpens) \
 cps1_draw_gfx(gfx,code,color,flipx,flipy,sx,sy,tpens,cps1_tile32_pen_usage,32,cps1_max_tile32,16*2*4,0)
#define adjust_draw_gfx_pointers()

#endif

static int cps1_transparency_scroll[4];

INLINE void cps1_get_video_base(void )
{
	struct CPS1VIDCFG *pCFG=&cps1_vid_cfg[cps1_game_config->alternative];
	int layercontrol;


	/* Re-calculate the VIDEO RAM base */
	cps1_scroll1=cps1_base(CPS1_SCROLL1_BASE);
	cps1_scroll2=cps1_base(CPS1_SCROLL2_BASE);
	cps1_scroll3=cps1_base(CPS1_SCROLL3_BASE);
	cps1_obj=cps1_base(CPS1_OBJ_BASE);
	cps1_palette=cps1_base(CPS1_PALETTE_BASE);
	cps1_other=cps1_base(CPS1_OTHER_BASE);

	/* Get scroll values */
	scroll1x=cps1_port(CPS1_SCROLL1_SCROLLX);
	scroll1y=cps1_port(CPS1_SCROLL1_SCROLLY);
	scroll2x=cps1_port(CPS1_SCROLL2_SCROLLX);
	scroll2y=cps1_port(CPS1_SCROLL2_SCROLLY);
	scroll3x=cps1_port(CPS1_SCROLL3_SCROLLX);
	scroll3y=cps1_port(CPS1_SCROLL3_SCROLLY);

	/* Get transparency registers */
	if (pCFG->priority1)
	{
		cps1_transparency_scroll[0]=~cps1_port(pCFG->priority0);
		cps1_transparency_scroll[1]=~cps1_port(pCFG->priority1);
		cps1_transparency_scroll[2]=~cps1_port(pCFG->priority2);
		cps1_transparency_scroll[3]=~cps1_port(pCFG->priority3);
	 }

	/* Get layer enable bits */
	layercontrol=READ_WORD(&cps1_output[cps1_layer_control]);
	cps1_layer_enabled[0]=1;
	cps1_layer_enabled[1]=layercontrol & pCFG->scrl1_enable_mask;
	cps1_layer_enabled[2]=layercontrol & pCFG->scrl2_enable_mask;
	cps1_layer_enabled[3]=layercontrol & pCFG->scrl3_enable_mask;
}


int cps1_gfxram_r(int offset)
{
	return READ_WORD(&cps1_gfxram[offset]);
}

void cps1_gfxram_w(int offset, int data)
{
	COMBINE_WORD_MEM(&cps1_gfxram[offset],data);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

int cps1_vh_start(void)
{
	int i;

	if (cps1_gfx_start())
	{
		return -1;
	}

	if (Machine->orientation & ORIENTATION_SWAP_XY)
	{
		orientation_swap_xy=0x8;
		cps1_scroll2_bitmap=osd_new_bitmap(CPS1_SCROLL2_HEIGHT*16,
			CPS1_SCROLL2_WIDTH*16, Machine->scrbitmap->depth );
#ifdef CPS1_VISIBLE_PATCH
		gp2x_visible_area.min_x=gp2x_yoffset;
		if (gp2x_visible_area.min_x<Machine->drv->visible_area.min_x)
			gp2x_visible_area.min_x=Machine->drv->visible_area.min_x;

		gp2x_visible_area.min_y=gp2x_xoffset;
		if (gp2x_visible_area.min_y<Machine->drv->visible_area.min_y)
			gp2x_visible_area.min_y=Machine->drv->visible_area.min_y;

		gp2x_visible_area.max_x=gp2x_yoffset+gp2x_height;
		if (gp2x_visible_area.max_x>Machine->drv->visible_area.max_x)
			gp2x_visible_area.max_x=Machine->drv->visible_area.max_x;

		gp2x_visible_area.max_y=gp2x_xoffset+gp2x_width;
		if (gp2x_visible_area.max_y>Machine->drv->visible_area.max_y)
			gp2x_visible_area.max_y=Machine->drv->visible_area.max_y;
#endif
	}
	else
	{
		orientation_swap_xy=0;
		cps1_scroll2_bitmap=osd_new_bitmap(CPS1_SCROLL2_WIDTH*16,
			CPS1_SCROLL2_HEIGHT*16, Machine->scrbitmap->depth );
#ifdef CPS1_VISIBLE_PATCH
		gp2x_visible_area.min_x=gp2x_xoffset;
		if (gp2x_visible_area.min_x<Machine->drv->visible_area.min_x)
			gp2x_visible_area.min_x=Machine->drv->visible_area.min_x;

		gp2x_visible_area.min_y=gp2x_yoffset;
		if (gp2x_visible_area.min_y<Machine->drv->visible_area.min_y)
			gp2x_visible_area.min_y=Machine->drv->visible_area.min_y;

		gp2x_visible_area.max_x=gp2x_xoffset+gp2x_width;
		if (gp2x_visible_area.max_x>Machine->drv->visible_area.max_x)
			gp2x_visible_area.max_x=Machine->drv->visible_area.max_x;

		gp2x_visible_area.max_y=gp2x_yoffset+gp2x_height;
		if (gp2x_visible_area.max_y>Machine->drv->visible_area.max_y)
			gp2x_visible_area.max_y=Machine->drv->visible_area.max_y;
#endif
	}
	adjust_draw_gfx_pointers();

	if (!cps1_scroll2_bitmap)
	{
		return -1;
	}
	cps1_scroll2_old_p=(unsigned)gp2x_malloc(cps1_scroll2_size+32);
	if (!cps1_scroll2_old_p)
	{
		return -1;
	}
	fast_memset((void *)cps1_scroll2_old_p, 0xff, cps1_scroll2_size+32);
	cps1_scroll2_old=(unsigned char *) ((cps1_scroll2_old_p+32)-(cps1_scroll2_old_p&31));

	cps1_old_palette=(unsigned char *)gp2x_malloc(cps1_palette_size);
	if (!cps1_old_palette)
	{
		return -1;
	}
	fast_memset(cps1_old_palette, 0xff, cps1_palette_size);

	fast_memset(cps1_gfxram, 0, cps1_gfxram_size);   /* Clear GFX RAM */
	fast_memset(cps1_output, 0, cps1_output_size);   /* Clear output ports */

	if (!cps1_game_config)
	{
		return -1;
	}

	if (cps1_game_config->alternative >
		sizeof(cps1_vid_cfg) / sizeof(cps1_vid_cfg[0]))
	{
		return -1;
	}


	cps1_layer_control=cps1_vid_cfg[cps1_game_config->alternative].layer_control;

	/* Put in some defaults */
	WRITE_WORD(&cps1_output[0x00], 0x9200);
	WRITE_WORD(&cps1_output[0x02], 0x9000);
	WRITE_WORD(&cps1_output[0x04], 0x9040);
	WRITE_WORD(&cps1_output[0x06], 0x9080);
	WRITE_WORD(&cps1_output[0x08], 0x9100);
	WRITE_WORD(&cps1_output[0x0a], 0x90c0);



	/* Set up old base */
	cps1_get_video_base();   /* Calculate base pointers */
	cps1_get_video_base();   /* Calculate old base pointers */


	/*
		Some games interrogate a couple of registers on bootup.
		These are CPS1 board B self test checks. They wander from game to
		game.
	*/
	if (cps1_game_config->cpsb_addr)
	{
		WRITE_WORD(&cps1_output[cps1_game_config->cpsb_addr],
			   cps1_game_config->cpsb_value);
	}

	for (i=0; i<4; i++)
	{
		cps1_transparency_scroll[i]=0xffff;
	}
	return 0;
}

/***************************************************************************

  Stop the video hardware emulation.

***************************************************************************/
void cps1_vh_stop(void)
{
	if (cps1_old_palette)
		gp2x_free(cps1_old_palette);
	if (cps1_scroll2_bitmap)
		osd_free_bitmap(cps1_scroll2_bitmap);
	if (cps1_scroll2_old_p)
		gp2x_free((void *)cps1_scroll2_old_p);
	cps1_gfx_stop();
}

/***************************************************************************

  Build palette from palette RAM

  12 bit RGB with a 4 bit brightness value.

***************************************************************************/

INLINE void cps1_build_palette(void)
{
	int offset;

	for (offset = 0; offset < cps1_palette_entries*16; offset++)
	{
		int palette = READ_WORD (&cps1_palette[offset << 1]);

		if (palette != READ_WORD (&cps1_old_palette[offset << 1]) )
		{
		   int red, green, blue, bright;

		   bright= (palette>>12);
		   if (bright) bright += 2;

		   red   = ((palette>>8)&0x0f) * bright;
		   green = ((palette>>4)&0x0f) * bright;
		   blue  = (palette&0x0f) * bright;

		   palette_change_color (offset, red, green, blue);
		   WRITE_WORD(&cps1_old_palette[offset << 1], palette);
		}
	}
}

/***************************************************************************

  Scroll 1 (8x8)

  Attribute word layout:
  0x0001	colour
  0x0002	colour
  0x0004	colour
  0x0008	colour
  0x0010	colour
  0x0020	X Flip
  0x0040	Y Flip
  0x0080
  0x0100
  0x0200
  0x0400
  0x0800
  0x1000
  0x2000
  0x4000
  0x8000


***************************************************************************/

static void cps1_palette_scroll1(unsigned short *base)
{
	int x,y, offs, offsx;

	int scrlxrough=(scroll1x>>3)+8;
	int scrlyrough=(scroll1y>>3);

	for (x=0; x<0x36; x++)
	{
		 offsx=(scrlxrough+x)<<7;
		 offsx&=0x1fff;

		 for (y=0; y<0x20; y++)
		 {
			int code, colour, offsy;
			int n=scrlyrough+y;
			offsy=( ((n&0x1f)<<2) | ((n&0x20)<<8)) & 0x3fff;
			offs=offsy+offsx;
			offs &= 0x3fff;
			code=READ_WORD(&cps1_scroll1[offs]);
			colour=READ_WORD(&cps1_scroll1[offs+2]);
			code+=cps1_game_config->bank_scroll1<<15;
			base[colour&0x1f] |=
			      cps1_char_pen_usage[code % cps1_max_char]&0x7fff;
		}
	}
}

static void cps1_render_scroll1(int priority)
{
	int x,y, offs, offsx, sx, sy, ytop;

	int scrlxrough=(scroll1x>>3)+4;
	int scrlyrough=(scroll1y>>3);

	sx=-(scroll1x&0x07);
	ytop=-(scroll1y&0x07)+32;

	for (x=0; x<0x35; x++)
	{
		 sy=ytop;
		 offsx=(scrlxrough+x)<<7;
		 offsx&=0x1fff;

		 for (y=0; y<0x20; y++)
		 {
			int code, offsy, colour;
			int n=scrlyrough+y;
			offsy=( ((n&0x1f)<<2) | ((n&0x20)<<8)) & 0x3fff;
			offs=offsy+offsx;
			offs &= 0x3fff;

			code=READ_WORD(&cps1_scroll1[offs]);
			colour=READ_WORD(&cps1_scroll1[offs+2]);

			if (code != 0x20)
			{
				/* 0x0020 appears to never be drawn */
				code+=cps1_game_config->bank_scroll1<<15;
				if (!priority || colour & 0x0180)
				{
					cps1_draw_scroll1(
					     code,
					     colour&0x1f,
					     colour&0x20,
					     colour&0x40,
					     sx,sy);
				}
			 }
			 sy+=8;
		 }
		 sx+=8;
	}
}




/***************************************************************************

								Sprites
								=======

  Sprites are represented by a number of 8 byte values

  xx xx yy yy nn nn aa aa

  where xxxx = x position
		yyyy = y position
		nnnn = tile number
		aaaa = attribute word
					0x0001	colour
					0x0002	colour
					0x0004	colour
					0x0008	colour
					0x0010	colour
					0x0020	X Flip
					0x0040	Y Flip
					0x0080	unknown
					0x0100	X block size (in sprites)
					0x0200	X block size
					0x0400	X block size
					0x0800	X block size
					0x1000	Y block size (in sprites)
					0x2000	Y block size
					0x4000	Y block size
					0x8000	Y block size

  The end of the table (may) be marked by an attribute value of 0xff00.

***************************************************************************/

INLINE void cps1_find_last_sprite(void)    /* Find the offset of last sprite */
{
	int offset=6;
	/* Locate the end of table marker */
	while (offset < cps1_obj_size)
	{
		int colour=READ_WORD(&cps1_obj[offset]);
		if (colour == 0xff00)
		{
			/* Marker found. This is the last sprite. */
			cps1_last_sprite_offset=offset-6-8;
			return;
		}
		offset+=8;
	}
	/* Sprites must use full sprite RAM */
	cps1_last_sprite_offset=cps1_obj_size-8;
}

/* Find used colours */
static void cps1_palette_sprites(unsigned short *base)
{
	int i;
	int gng_obj_kludge=cps1_game_config->gng_sprite_kludge;

	for (i=cps1_last_sprite_offset; i>=0; i-=8)
	{
		int x=READ_WORD(&cps1_obj[i]);
		int y=READ_WORD(&cps1_obj[i+2]);
		if (x && y)
		{
	    int colour=READ_WORD(&cps1_obj[i+6]);
	    int col=colour&0x1f;
			unsigned int code=READ_WORD(&cps1_obj[i+4]);
			code+=cps1_game_config->bank_obj<<14;
			if (gng_obj_kludge == 1 && code >= 0x01000)
			{
			       code += 0x4000;
			}
			if (gng_obj_kludge == 2 && code >= 0x02a00)
			{
			       code += 0x4000;
			}

	    if ( colour & 0xff00 )
			{
		int nys, nxs;
		int nx=(colour & 0x0f00) >> 8;
		int ny=(colour & 0xf000) >> 12;
		nx++;
		ny++;

		if (colour & 0x40)   /* Y Flip */					      /* Y flip */
		{
		    if (colour &0x20)
		    {
			for (nys=0; nys<ny; nys++)
			{
			    for (nxs=0; nxs<nx; nxs++)
			    {
				int cod=code+(nx-1)-nxs+((ny-1-nys)<<4);
				base[col] |=
#ifdef CPS1_MAX_TILE_PATCH
				    cps1_tile16_pen_usage[cod & cps1_max_tile16_mask];
#else
				    cps1_tile16_pen_usage[cod % cps1_max_tile16];
#endif
			    }
			}
		    }
		    else
		    {
			for (nys=0; nys<ny; nys++)
			{
			    for (nxs=0; nxs<nx; nxs++)
			    {
				int cod=code+nxs+((ny-1-nys)<<4);
				base[col] |=
#ifdef CPS1_MAX_TILE_PATCH
					cps1_tile16_pen_usage[cod & cps1_max_tile16_mask];
#else
					cps1_tile16_pen_usage[cod % cps1_max_tile16];
#endif
			    }
			}
					}
		}
		else
		{
		    if (colour &0x20)
		    {
			 for (nys=0; nys<ny; nys++)
			 {
			      for (nxs=0; nxs<nx; nxs++)
			      {
				    int cod=code+(nx-1)-nxs+(nys<<4);
				    base[col] |=
#ifdef CPS1_MAX_TILE_PATCH
					cps1_tile16_pen_usage[cod & cps1_max_tile16_mask];
#else
					cps1_tile16_pen_usage[cod % cps1_max_tile16];
#endif

				}
			  }
		     }
		     else
		     {
				for (nys=0; nys<ny; nys++)
				{
					for (nxs=0; nxs<nx; nxs++)
					{
				    		int cod=code+nxs+(nys<<4);
				    		base[col] |=
#ifdef CPS1_MAX_TILE_PATCH
							cps1_tile16_pen_usage[cod & cps1_max_tile16_mask];
#else
							cps1_tile16_pen_usage[cod % cps1_max_tile16];
#endif
					}
				}
		      }
		  }
		  base[col]&=0x7fff;
	      }
	      else
	      {
			base[col] |=
#ifdef CPS1_MAX_TILE_PATCH
			    cps1_tile16_pen_usage[code & cps1_max_tile16_mask]&0x7fff;
#else
			    cps1_tile16_pen_usage[code % cps1_max_tile16]&0x7fff;
#endif
	      }
	 }
	}
}



static void cps1_render_sprites(int priority)
{
	int i;
	int gng_obj_kludge=cps1_game_config->gng_sprite_kludge;

	/* Draw the sprites */
	for (i=cps1_last_sprite_offset; i>=0; i-=8)
	{
	int x=READ_WORD(&cps1_obj[i]);
	int y=READ_WORD(&cps1_obj[i+2]);
		if (x && y )
		{
			unsigned int code=READ_WORD(&cps1_obj[i+4]);
			int colour=READ_WORD(&cps1_obj[i+6]);
			int col=colour&0x1f;
			code+=cps1_game_config->bank_obj<<14;

			if (y & 0x0300)
			{
				if (colour & 0xff00)
				{
					y=-0x200+y;
				}
				else
				{
					y &= 0x1ff;
				}
			}
			x-=0x20;
			y+=0x20;

			if (gng_obj_kludge == 1 && code >= 0x01000)
			{
			       code += 0x4000;
			}
			if (gng_obj_kludge == 2 && code >= 0x02a00)
			{
			       code += 0x4000;
			}

			if (colour & 0xff00 )
			{
					/* handle blocked sprites */
					int nx=(colour & 0x0f00) >> 8;
					int ny=(colour & 0xf000) >> 12;
					int nxs, nys;
					nx++;
					ny++;

					if (colour & 0x40)
					{
						/* Y flip */
						if (colour &0x20)
						{
							for (nys=0; nys<ny; nys++)
							{
								for (nxs=0; nxs<nx; nxs++)
								{
									cps1_draw_tile16(Machine->gfx[1],
										code+(nx-1)-nxs+((ny-1-nys)<<4),
										col&0x1f,
										0x20,0x40,
										x+(nxs<<4),y+(nys<<4),0x8000);
								}
							}
						}
						else
						{
							for (nys=0; nys<ny; nys++)
							{
								for (nxs=0; nxs<nx; nxs++)
								{
									cps1_draw_tile16(Machine->gfx[1],
										code+nxs+((ny-1-nys)<<4),
										col&0x1f,
										0,0x40,
										x+(nxs<<4),y+(nys<<4),0x8000 );
								}
							}
						}
					}
					else
					{
						if (colour &0x20)
						{
							for (nys=0; nys<ny; nys++)
							{
								for (nxs=0; nxs<nx; nxs++)
								{
									cps1_draw_tile16(Machine->gfx[1],
										code+(nx-1)-nxs+(nys<<4),
										col&0x1f,
										0x20,0,
										x+(nxs<<4),y+(nys<<4),0x8000
										);
								}
							}
						}
						else
						{
							for (nys=0; nys<ny; nys++)
							{
								for (nxs=0; nxs<nx; nxs++)
								{
									cps1_draw_tile16(Machine->gfx[1],
										code+nxs+(nys<<4),
										col&0x1f,
										0,0,
										x+(nxs<<4),y+(nys<<4), 0x8000);
								}
							}
						}
					}
				}
				else
				{
					/* Simple case... 1 sprite */
					cps1_draw_tile16(Machine->gfx[1],
						   code,
						   col&0x1f,
						   colour&0x20,
						   colour&0x40,
						   x,y,0x8000);
				}
		}
	}
}




/***************************************************************************

  Scroll 2 (16x16 layer)

  Attribute word layout:
  0x0001	colour
  0x0002	colour
  0x0004	colour
  0x0008	colour
  0x0010	colour
  0x0020	X Flip
  0x0040	Y Flip
  0x0080	??? Priority
  0x0100	??? Priority
  0x0200
  0x0400
  0x0800
  0x1000
  0x2000
  0x4000
  0x8000


***************************************************************************/

static void cps1_palette_scroll2(unsigned short *base)
{
	int offs, code, colour;
	for (offs=cps1_scroll2_size-4; offs>=0; offs-=4)
	{
	     code=READ_WORD(&cps1_scroll2[offs]);
	     colour=READ_WORD(&cps1_scroll2[offs+2])&0x1f;
	     code+=cps1_game_config->bank_scroll2<<14;
#ifdef CPS1_MAX_TILE_PATCH
	     base[colour] |= cps1_tile16_pen_usage[code & cps1_max_tile16_mask];
#else
	     base[colour] |= cps1_tile16_pen_usage[code % cps1_max_tile16];
#endif
	}
}

static void cps1_render_scroll2_bitmap(int priority)
{
	int sx, sy;
	int ny=(scroll2y>>4);	  /* Rough Y */
	struct osd_bitmap *bitmap_tmp=osd_bitmap_bitmap;

	osd_bitmap_bitmap=cps1_scroll2_bitmap;
	bitmap_width=CPS1_SCROLL2_SIZE;
	bitmap_height=CPS1_SCROLL2_SIZE;
	bitmap_line=(char **)cps1_scroll2_bitmap->line;
#ifdef CPS1_BITMAP_PATCH
	#ifdef GP2X
	if (!gp2x_rotate) {
	#endif
	bitmap_width_min=0;
	bitmap_height_min=0;
	bitmap_width_max=CPS1_SCROLL2_SIZE;
	bitmap_height_max=CPS1_SCROLL2_SIZE;
	#ifdef GP2X
	}
	#endif
#endif
	for (sx=0; sx<CPS1_SCROLL2_WIDTH; sx++)
	{
		int n=ny;
		for (sy=0; sy<0x09*2; sy++)
		{
			long newvalue;
			int offsy, offsx, offs, colour, code;

			n&=0x3f;
			offsy  = (((n&0x0f)<<2) | ((n&0x30)<<8))&0x3fff;
			offsx=(sx<<6)&0xfff;
			offs=offsy+offsx;

			colour=READ_WORD(&cps1_scroll2[offs+2]);

			newvalue=*(long*)(&cps1_scroll2[offs]);
			if ( newvalue != *(long*)(&cps1_scroll2_old[offs]) )
			{
				*(long*)(&cps1_scroll2_old[offs])=newvalue;
				code=READ_WORD(&cps1_scroll2[offs]);
				code+=cps1_game_config->bank_scroll2<<14;
				/* Draw entire tile */
				cps1_draw_tile16(
						Machine->gfx[2],
						code,
						colour&0x1f,
						colour&0x20,
						colour&0x40,
						sx<<4,n<<4,
						0x0000);
			}
			n++;
		}
	}
	osd_bitmap_bitmap=bitmap_tmp;
	bitmap_width=osd_bitmap_bitmap->width;
	bitmap_height=osd_bitmap_bitmap->height;
	bitmap_line=(char **)osd_bitmap_bitmap->line;
#ifdef CPS1_BITMAP_PATCH
	#ifdef GP2X
	if (!gp2x_rotate) {
	#endif
	bitmap_width_min=gp2x_xoffset;
	bitmap_height_min=gp2x_yoffset;
	bitmap_width_max=gp2x_xoffset+gp2x_width;
	bitmap_height_max=gp2x_yoffset+gp2x_height;
	#ifdef GP2X
	}
	#endif
#endif
}


static void cps1_render_scroll2_high(int priority)
{
	int sx, sy;
	int nxoffset=(scroll2x&0x0f)+32;    /* Smooth X */
	int nyoffset=(scroll2y&0x0f)+32;    /* Smooth Y */
	int nx=(scroll2x>>4);	  /* Rough X */
	int ny=(scroll2y>>4)-4;	/* Rough Y */

	for (sx=0; sx<0x32/2+4; sx++)
	{
		for (sy=0; sy<0x09*2; sy++)
		{
			int offsy, offsx, offs, colour, code, mask;
			int n;
			n=ny+sy;
			offsy  = (((n&0x0f)<<2) | ((n&0x30)<<8))&0x3fff;
			offsx=((nx+sx)<<6)&0xfff;
			offs=offsy+offsx;
			offs &= 0x3fff;

			code=READ_WORD(&cps1_scroll2[offs]);
			colour=READ_WORD(&cps1_scroll2[offs+2]);
			mask=colour & 0x0180;
			if (priority && mask)
			{
				int transp;
				transp=cps1_transparency_scroll[mask>>7];
				code+=cps1_game_config->bank_scroll2<<14;
				cps1_draw_tile16(
							Machine->gfx[2],
							code,
							colour&0x1f,
							colour&0x20,
							colour&0x40,
							(sx<<4)-nxoffset,
							(sy<<4)-nyoffset,
							transp);
			}
		}
	}
}

#ifdef CPS1_DRAWGFX_PATCH

#define read_dword(addr) (((int)addr & 3)? ( ((((unsigned char *)(addr))[0])) | ((((unsigned char *)(addr))[1]) <<  8) | ((((unsigned char *)(addr))[2]) << 16) | ((((unsigned char *)(addr))[3]) << 24) ) : (*(int *)addr) )
#define write_dword(addr,value) (((int)addr & 3)? ( ((unsigned char *)(addr))[0] = (unsigned char)((int)(value)), ((unsigned char *)(addr))[1] = (unsigned char)((int)(value) >>  8), ((unsigned char *)(addr))[2] = (unsigned char)((int)(value) >> 16), ((unsigned char *)(addr))[3] = (unsigned char)((int)(value) >> 24) ) : (*(int *)addr = value) )

static void cps1_render_scroll2_low_drawgfx(int sx, int sy, const struct rectangle *clip)
{
	struct osd_bitmap *dest=osd_bitmap_bitmap;
	int ox,oy,ex,ey,start,dy;
	struct rectangle myclip;

	if (orientation_swap_xy)
	{
		int temp=sx;
		sx = sy;
		sy = temp;
		temp = clip->min_x;
		myclip.min_x = clip->min_y;
		myclip.min_y = temp;
		temp = clip->max_x;
		myclip.max_x = clip->max_y;
		myclip.max_y = temp;
		clip = &myclip;
		sy = dest->height - CPS1_SCROLL2_SIZE - sy;
//		myclip.min_x = clip->min_x;
//		myclip.max_x = clip->max_x;
		temp = clip->min_y;
		myclip.min_y = dest->height-1 - clip->max_y;
		myclip.max_y = dest->height-1 - temp;
	}
	ox = sx;
	oy = sy;
	ex = sx + CPS1_SCROLL2_SIZE-1;
	if (sx < 0) sx = 0;
	if (sx < clip->min_x) sx = clip->min_x;
	if (ex >= dest->width) ex = dest->width-1;
	if (ex > clip->max_x) ex = clip->max_x;
	if (sx > ex) return;
	ey = sy + CPS1_SCROLL2_SIZE-1;
	if (sy < 0) sy = 0;
	if (sy < clip->min_y) sy = clip->min_y;
	if (ey >= dest->height) ey = dest->height-1;
	if (ey > clip->max_y) ey = clip->max_y;
	if (sy > ey) return;

	start = sy-oy;
	dy = 1;
	
	unsigned char **gfx_line=cps1_scroll2_bitmap->line;
	int trans4 = palette_transparent_pen * 0x01010101;
	int y;
	for (y = sy;y <= ey;y++)
	{
		unsigned char *bm = dest->line[y];
		unsigned char *bme = bm + ex;
		int *sd4 = (int *)(gfx_line[start] + (sx-ox));
		bm += sx;
		while( bm <= bme-3 )
		{
//			while( bm <= bme-3 && (*sd4) == trans4 )
			while( bm <= bme-3 && read_dword(sd4) == trans4 )
			{
				sd4++;
				bm+=4;
			}
			int col4;
//			while( bm <= bme-3 && (col4=*sd4))
			while( bm <= bme-3 && (col4=read_dword(sd4)))
			{
				register int xod4 = col4^trans4;
				if( (xod4&0x000000ff) && (xod4&0x0000ff00) && (xod4&0x00ff0000) && (xod4&0xff000000) )
				{
//					*((int *)bm) = col4;
					write_dword((int *)bm,col4);
					bm+=4;
				}
				else
				{
					if(xod4&0x000000ff) *bm = col4;
					bm++;
					if(xod4&0x0000ff00) *bm = col4>>8;
					bm++;
					if(xod4&0x00ff0000) *bm = col4>>16;
					bm++;
					if(xod4&0xff000000) *bm = col4>>24;
					bm++;
				}
				sd4++;
			}
			{
				register unsigned char *sd = (unsigned char *)sd4;
				for( ; bm <= bme ; bm++ )
				{
					register int col = *(sd++);
					if (col != palette_transparent_pen)
						*bm = col;
				}
			}
			start+=dy;
		}
	}
}
#endif


static void cps1_render_scroll2_low(int priority)
{
      int scrly=-(scroll2y-0x20);
      int scrlx=-(scroll2x+0x40-0x20);

      if (cps1_flip_screen)
      {
	    scrly=(CPS1_SCROLL2_HEIGHT*16)-scrly;
      }

      cps1_render_scroll2_bitmap(priority);

#ifndef CPS1_DRAWGFX_PATCH
      copyscrollbitmap(osd_bitmap_bitmap,cps1_scroll2_bitmap,
	1,&scrlx,1,&scrly,&MAME4ALL_VISIBLE_AREA,TRANSPARENCY_PEN,palette_transparent_pen);
#else
      {
	      const int *rowscroll=&scrlx;
              const int *colscroll=&scrly;

	      int scrollx,scrolly;
	      if (rowscroll[0] < 0) scrollx = CPS1_SCROLL2_SIZE - (-rowscroll[0]) & (CPS1_SCROLL2_SIZE-1);
	      else scrollx = rowscroll[0] & (CPS1_SCROLL2_SIZE-1);
	      if (colscroll[0] < 0) scrolly = CPS1_SCROLL2_SIZE - (-colscroll[0]) & (CPS1_SCROLL2_SIZE-1);
	      else scrolly = colscroll[0] & (CPS1_SCROLL2_SIZE-1);

	      cps1_render_scroll2_low_drawgfx(scrollx,scrolly,&MAME4ALL_VISIBLE_AREA);
	      cps1_render_scroll2_low_drawgfx(scrollx,scrolly - CPS1_SCROLL2_SIZE,&MAME4ALL_VISIBLE_AREA);
	      cps1_render_scroll2_low_drawgfx(scrollx - CPS1_SCROLL2_SIZE,scrolly,&MAME4ALL_VISIBLE_AREA);
	      cps1_render_scroll2_low_drawgfx(scrollx - CPS1_SCROLL2_SIZE,scrolly - CPS1_SCROLL2_SIZE,&MAME4ALL_VISIBLE_AREA);
      }
#endif
}


static void cps1_render_scroll2_distort(int priority)
{
      #define lines (0x40*0x10)
      int other=0;
      int scrly=-(scroll2y-0x20);
      int i,scroll[lines];

      if (cps1_flip_screen)
      {
	    scrly=(CPS1_SCROLL2_HEIGHT<<4)-scrly;
      }


      cps1_render_scroll2_bitmap(priority);

      switch (cps1_vid_cfg[cps1_game_config->alternative].distort_alt)
      {
		case 1:
		      other+=0x100;
		      for (i = 0;i < lines;i++)
		      {
			     int xdistort;
			     other&=0x1ff;
			     xdistort=READ_WORD(&cps1_other[other]);
			     other+=2;
			     scroll[i] = -(scroll2x+xdistort+0x40-0x20);
		      }
		      break;
		case 2:
		      other+=0x200;
		      for (i = 0;i < lines;i++)
		      {
			     int xdistort;
			     int n;
			     other&=0x7ff;
			     n=other;
			     /*if (other & 0x100)
			     {
				n=0x400-(other&0xff);
			     } */
			     xdistort=READ_WORD(&cps1_other[n]);
			     other+=2;
			     scroll[i]   = -(scroll2x+xdistort+0x40-0x20);
		      }
		      break;
		case 3:
		      for (i = 0;i < lines;i++)
		      {
			     int xdistort;
			     other&=0x7ff;
			     xdistort=READ_WORD(&cps1_other[other]);
			     other+=2;
			     scroll[i] = -(scroll2x+xdistort+0x40-0x20);
		      }
		      break;

		default:
		      for (i = 0;i < lines;i++)
		      {
			     scroll[i] = 0;
		      }
		      break;
      }

#ifndef CPS1_DRAWGFX_PATCH
      copyscrollbitmap(osd_bitmap_bitmap,cps1_scroll2_bitmap,
	lines,scroll,1,&scrly,&MAME4ALL_VISIBLE_AREA,TRANSPARENCY_PEN,palette_transparent_pen);
#else
      {
	      const int *rowscroll=scroll;
              const int *colscroll=&scrly;
	      struct rectangle myclip;
	      int row,/*rows,*/rowheight;

	      int scrolly;

	      if (colscroll[0] < 0) scrolly = CPS1_SCROLL2_SIZE - (-colscroll[0]) & (CPS1_SCROLL2_SIZE-1);
	      else scrolly = colscroll[0] & (CPS1_SCROLL2_SIZE-1);

	      rowheight = CPS1_SCROLL2_SIZE >> 10;

	      myclip.min_x = MAME4ALL_VISIBLE_AREA.min_x;
	      myclip.max_x = MAME4ALL_VISIBLE_AREA.max_x;

	      row=0;
	      while (row < 1024)
	      {
		      int cons=1;
		      int scroll = rowscroll[row];

		      while (row + cons < lines /*rows*/ && rowscroll[row + cons] == scroll)
			      cons++;

		      if (scroll < 0) scroll = CPS1_SCROLL2_SIZE - (-scroll) & (CPS1_SCROLL2_SIZE-1);
		      else scroll &= (CPS1_SCROLL2_SIZE-1);

		      myclip.min_y = row * rowheight + scrolly;
		      if (myclip.min_y < MAME4ALL_VISIBLE_AREA.min_y) myclip.min_y = MAME4ALL_VISIBLE_AREA.min_y;
		      myclip.max_y = (row + cons) * rowheight - 1 + scrolly;
		      if (myclip.max_y > MAME4ALL_VISIBLE_AREA.max_y) myclip.max_y = MAME4ALL_VISIBLE_AREA.max_y;

		      cps1_render_scroll2_low_drawgfx(scroll,scrolly,&myclip);
		      cps1_render_scroll2_low_drawgfx(scroll - CPS1_SCROLL2_SIZE,scrolly,&myclip);

		      myclip.min_y = row * rowheight + scrolly - CPS1_SCROLL2_SIZE;
		      if (myclip.min_y < MAME4ALL_VISIBLE_AREA.min_y) myclip.min_y = MAME4ALL_VISIBLE_AREA.min_y;
		      myclip.max_y = (row + cons) * rowheight - 1 + scrolly - CPS1_SCROLL2_SIZE;
		      if (myclip.max_y > MAME4ALL_VISIBLE_AREA.max_y) myclip.max_y = MAME4ALL_VISIBLE_AREA.max_y;

		      cps1_render_scroll2_low_drawgfx(scroll,scrolly - CPS1_SCROLL2_SIZE,&myclip);
		      cps1_render_scroll2_low_drawgfx(scroll - CPS1_SCROLL2_SIZE,scrolly - CPS1_SCROLL2_SIZE,&myclip);

		      row += cons;
	      }
      }
#endif

      
}


/***************************************************************************

  Scroll 3 (32x32 layer)

  Attribute word layout:
  0x0001	colour
  0x0002	colour
  0x0004	colour
  0x0008	colour
  0x0010	colour
  0x0020	X Flip
  0x0040	Y Flip
  0x0080
  0x0100
  0x0200
  0x0400
  0x0800
  0x1000
  0x2000
  0x4000
  0x8000

***************************************************************************/

static void cps1_palette_scroll3(unsigned short *base)
{
	int sx,sy;
	int nx=(scroll3x>>5)+1;
	int ny=(scroll3y>>5)-1;
	int gng_obj_kludge=cps1_game_config->gng_sprite_kludge;

	for (sx=0; sx<0x32/4+2; sx++)
	{
		for (sy=0; sy<0x20/4+1; sy++)
		{
			int offsy, offsx, offs, colour, code;
			int n;
			n=ny+sy;
			offsy  = (((n&0x07)<<2) | ((n&0xf8)<<8))&0x3fff;
			offsx=((nx+sx)<<5)&0x7ff;
			offs=offsy+offsx;
			offs &= 0x3fff;
			code=READ_WORD(&cps1_scroll3[offs]);
			code+=cps1_game_config->bank_scroll3<<12;
			if (gng_obj_kludge == 2 && code >= 0x01500)
			{
			       code -= 0x1000;
			}
			colour=READ_WORD(&cps1_scroll3[offs+2]);
			base[colour&0x1f] |=
#ifdef CPS1_MAX_TILE_PATCH
				cps1_tile32_pen_usage[code & cps1_max_tile32_mask];
#else
				cps1_tile32_pen_usage[code % cps1_max_tile32];
#endif
		}
	}
}


void cps1_render_scroll3(int priority)
{
	int sx,sy;
	int nxoffset=scroll3x&0x1f;
	int nyoffset=(scroll3y&0x1f); 
	int nx=(scroll3x>>5)+1;
	int ny=(scroll3y>>5)-1;  /* -1 */
	int gng_obj_kludge=cps1_game_config->gng_sprite_kludge;

	for (sx=1; sx<0x32/4+2; sx++)
	{
		for (sy=1; sy<0x20/4+2; sy++)
		{
			int offsy, offsx, offs, colour, code, transp;
			int n;
			n=ny+sy;
			offsy  = (((n&0x07)<<2) | ((n&0xf8)<<8))&0x3fff;
			offsx=((nx+sx)<<5)&0x7ff;
			offs=offsy+offsx;
			offs &= 0x3fff;
			code=READ_WORD(&cps1_scroll3[offs]);
			code+=cps1_game_config->bank_scroll3<<12;
			if (gng_obj_kludge == 2 && code >= 0x01500)
			{
			       code -= 0x1000;
			}
			colour=READ_WORD(&cps1_scroll3[offs+2]);
			if (priority)
			{
					int mask=colour & 0x0180;
					if (mask && !(colour & 0xf000))
					{
						transp=cps1_transparency_scroll[mask>>7];
						cps1_draw_tile32(
								Machine->gfx[3],
								code,
								colour&0x1f,
								colour&0x20,
								colour&0x40,
								(sx<<5)-nxoffset,
								(sy<<5)-nyoffset,
								transp);
					}
			 }
			 else
			 {
				cps1_draw_tile32(Machine->gfx[3],
						code,
						colour&0x1f,
						colour&0x20,
						colour&0x40,
						(sx<<5)-nxoffset,(sy<<5)-nyoffset,
						0x8000);

			 }
		}
	}
}


static void cps1_render_layer(int layer, int distort)
{
	if (cps1_layer_enabled[layer])
	{
		switch (layer)
		{
			case 0:
				cps1_render_sprites(0);
				break;
			case 1:
				cps1_render_scroll1(0);
				break;
			case 2:
				if (distort)
					cps1_render_scroll2_distort(0);
				else
					cps1_render_scroll2_low(0);
				break;
			case 3:
				cps1_render_scroll3(0);
				break;
		}
	}
}

static void cps1_render_high_layer(int layer)
{
	if (cps1_layer_enabled[layer])
	{
		switch (layer)
		{
			case 0:
				break;
			case 1:
				break;
			case 2:
				cps1_render_scroll2_high(1);
				break;
			case 3:
				cps1_render_scroll3(1);
				break;
		}
	}
}

INLINE void cps1_fillbitmap(void)
{
	const struct rectangle *clip=&MAME4ALL_VISIBLE_AREA;
	int sx,sy,ex,ey,y,nn;
	struct rectangle myclip;

	if (orientation_swap_xy)
	{
		int temp;
		myclip.min_x = clip->min_y;
		myclip.max_x = clip->max_y;
		myclip.min_y = clip->min_x;
		myclip.max_y = clip->max_x;
		clip = &myclip;
//		myclip.min_x = clip->min_x;
//		myclip.max_x = clip->max_x;
		temp = clip->min_y;
		myclip.min_y = osd_bitmap_bitmap->height-1 - clip->max_y;
		myclip.max_y = osd_bitmap_bitmap->height-1 - temp;
	}
	sx = 0;
	ex = osd_bitmap_bitmap->width - 1;
	sy = 0;
	ey = osd_bitmap_bitmap->height - 1;
	if (clip && sx < clip->min_x) sx = clip->min_x;
	if (clip && ex > clip->max_x) ex = clip->max_x;
	if (sx > ex) return;
	if (clip && sy < clip->min_y) sy = clip->min_y;
	if (clip && ey > clip->max_y) ey = clip->max_y;
	if (sy > ey) return;
	nn=(ex-sx+1);
#if defined(DREAMCAST) && defined(CPS1_DREAMCAST_SQ_PATCH)
	if (nn&31)
		nn+=32;
#endif
	{
		register unsigned dat=palette_transparent_pen&0xff;
		dat=(dat << 24) | (dat << 16) | (dat << 8) | dat;
		for (y = sy;y <= ey;y++)
		{
#if !defined(DREAMCAST) || !defined(CPS1_DREAMCAST_SQ_PATCH)
			fast_memset((void *)&osd_bitmap_bitmap->line[y][sx],dat,nn);
#else
			sq_set((void *)&osd_bitmap_bitmap->line[y][sx],dat,nn);
#endif
		}
	}
}


static unsigned short palette_usage[cps1_palette_entries] __attribute__ ((__aligned__ (32)));

#ifdef CPS1_MEMSETS_PATCH
#if !defined(DREAMCAST) || !defined(CPS1_DREAMCAST_SQ_PATCH)
#define clear_cps1_scroll2_old() fast_memset(cps1_scroll2_old,0xFFFFFFFF,cps1_scroll2_size)
#define clear_palette_usage() fast_memset((void *)&palette_usage[0],0,cps1_palette_entries<<1)
#else
#define clear_cps1_scroll2_old() sq_set32(cps1_scroll2_old,0xFFFFFFFF,cps1_scroll2_size)
#define clear_palette_usage() sq_set32((void *)&palette_usage[0],0,cps1_palette_entries<<1)
#endif

INLINE void clear_palette_used_colors(unsigned short *src)
{
	register unsigned short *s=src;
	*s++=PALETTE_COLOR_UNUSED;
	*s++=PALETTE_COLOR_UNUSED;
	*s++=PALETTE_COLOR_UNUSED;
	*s++=PALETTE_COLOR_UNUSED;
	*s++=PALETTE_COLOR_UNUSED;
	*s++=PALETTE_COLOR_UNUSED;
	*s++=PALETTE_COLOR_UNUSED;
	*s++=PALETTE_COLOR_UNUSED;
}
#else
#define clear_cps1_scroll2_old() fast_memset(cps1_scroll2_old, 0xff, cps1_scroll2_size)
#define clear_palette_usage() fast_memset(&palette_usage, 0, sizeof(palette_usage))
#define clear_palette_used_colors(PTR) fast_memset ((PTR), PALETTE_COLOR_UNUSED, 16)
#endif


/***************************************************************************

	Refresh screen

***************************************************************************/

void cps1_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh)
{
	osd_bitmap_bitmap=bitmap;
	bitmap_width=bitmap->width;
	bitmap_height=bitmap->height;
	bitmap_line=(char **)bitmap->line;
#ifdef CPS1_BITMAP_PATCH
	#ifdef GP2X
	if (!gp2x_rotate) {
	#endif
	bitmap_width_min=gp2x_xoffset;
	bitmap_height_min=gp2x_yoffset;
	bitmap_width_max=gp2x_xoffset+gp2x_width;
	bitmap_height_max=gp2x_yoffset+gp2x_height;
	#ifdef GP2X
	}
	#endif
#endif

	int layercontrol;
	int i,offset;
	int distort_scroll2=0;
	int layer;
	int videocontrol=cps1_port(0x22);
	int old_flip;

	old_flip=cps1_flip_screen;
	cps1_flip_screen=(videocontrol&0x8000)>>13;
	if (old_flip != cps1_flip_screen)
	{
		clear_cps1_scroll2_old();
		adjust_draw_gfx_pointers();
	}

	layercontrol=READ_WORD(&cps1_output[cps1_layer_control]);

	distort_scroll2=layercontrol & 0x01;

	/* Special cases for scroll on / off */
	switch (cps1_game_config->alternative)
	{
		case  7: /* Magic Sword */
			distort_scroll2=layercontrol & 0x01;
			break;

		case 12: /* Street Fighter 2 */
		case 13: /* Street Fighter 2 (Jap )*/
		case 14: /* Street Fighter 2 (turbo) */
		case 15: /* Street Fighter 2 (Rev E ) */
			distort_scroll2=1;
			break;

		default:
			break;
	}

	/* Get video memory base registers */
	cps1_get_video_base();

	/* Find the offset of the last sprite in the sprite table */
	cps1_find_last_sprite();

	/* Build palette */
	cps1_build_palette();

	/* Compute the used portion of the palette */
	clear_palette_usage();
	cps1_palette_sprites (&palette_usage[cps1_obj_palette]);
	if (cps1_layer_enabled[1])
		cps1_palette_scroll1 (&palette_usage[cps1_scroll1_palette]);
	if (cps1_layer_enabled[2])
		cps1_palette_scroll2 (&palette_usage[cps1_scroll2_palette]);
	else
		clear_cps1_scroll2_old();
	if (cps1_layer_enabled[3])
		cps1_palette_scroll3 (&palette_usage[cps1_scroll3_palette]);

	for (i = offset = 0; i < cps1_palette_entries; i++)
	{
		int usage = palette_usage[i];
		if (usage)
		{
			int j;
			for (j = 0; j < 15; j++)
			{
				if (usage & (1 << j))
					palette_used_colors[offset++] = PALETTE_COLOR_USED;
				else
					palette_used_colors[offset++] = PALETTE_COLOR_UNUSED;
			}
			palette_used_colors[offset++] = PALETTE_COLOR_TRANSPARENT;
		}
		else
		{
			clear_palette_used_colors((unsigned short*)&palette_used_colors[offset]);
			offset += 16;
		}
	}

	if (palette_recalc ())
	{
		clear_cps1_scroll2_old();
	}

	/* Blank screen */
#ifdef CPS1_FILLBITMAP_PATCH
	cps1_fillbitmap();
#else
	fillbitmap(bitmap,palette_transparent_pen,&MAME4ALL_VISIBLE_AREA);
#endif


	/* Draw layers */
	cps1_render_layer((layercontrol>>0x06)&03, distort_scroll2);
	cps1_render_layer((layercontrol>>0x08)&03, distort_scroll2);
	cps1_render_layer((layercontrol>>0x0a)&03, distort_scroll2);
	layer=(layercontrol>>0x0c)&03;
	if (layer != 1)
	{
		/*
		Don't draw layer 1 if it is the highest priority layer
		We will draw it later.
		*/
		cps1_render_layer((layercontrol>>0x0c)&03, distort_scroll2);
	}

	/* Draw high priority layers */
	cps1_render_high_layer((layercontrol>>0x06)&03);
	cps1_render_high_layer((layercontrol>>0x08)&03);
	cps1_render_high_layer((layercontrol>>0x0a)&03);
	if (layer==1)
	{
		/* High priority sprites */
		/*
		Scroll 1 is highest priority. Must draw it over high
		priority scroll parts of 2 and 3. This is correct
		behaviour since Magic Sword interludes do not
		look correct unless we do this.
		*/
		if (cps1_layer_enabled[1])
			cps1_render_scroll1(0);
	}
	else
	{
		cps1_render_high_layer(layer);
		/* High priority sprites */
	}
	/* Render really high priority chars */
	if (cps1_layer_enabled[1])
		cps1_render_scroll1(1);
}

