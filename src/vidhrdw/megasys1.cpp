#define EXTRA_MEM_PATCH 

#ifdef DREAMCAST
#include<kos.h>
#endif

/***************************************************************************

						-= Jaleco Mega System 1 =-

				driver by	Luca Elia (eliavit@unina.it)

**********  There are 3 scrolling layers, 2 bytes per tile info:

* Note: MS1-Z has 2 layers only.

  A page is 256x256, approximately the visible screen size. Each layer is
  made up of 8 pages (8x8 tiles) or 32 pages (16x16 tiles). The number of
  horizontal  pages and the tiles size  is selectable, using the  layer's
  control register. I think that when tiles are 16x16 a layer can be made
  of 16x2, 8x4, 4x8 or 2x16 pages (see below). When tile size is 8x8 we
  have two examples to guide the choice:

  the copyright screen of p47j (0x12) should be 4x2 (unless it's been hacked :)
  the ending sequence of 64th street (0x13) should be 2x4.

  I don't see a relation.


MS1-A MS1-B MS1-C
-----------------

					Scrolling layers:

90000 50000 e0000	Scroll 1
94000 54000 e8000	Scroll 2
98000 58000 f0000	Scroll 3					* Note: missing on MS1-Z

Tile format:	fedc------------	Palette
				----ba9876543210	Tile Number



84000 44000 c2208	Layers Enable				* Note: missing on MS1-Z?

	fedc ---- ---- ---- ? (unused?)
	---- ba-- ---- ----	? (used!)
	---- --9- ---- ----	Sprites below megasys_fg (sprites over megasys_fg on MS1-C?)
	---- ---8 ---- ----	Swap megasys_txt with megasys_fg (swap megasys_bg with megasys_fg on MS1-C?)
	---- ---- 7654 ----	? (unused?)
	---- ---- ---- 3---	Enable Sprites
	---- ---- ---- -210	Enable Layer 321



84200 44200 c2000	Scroll 1 Control
84208 44208 c2008	Scroll 2 Control
84008 44008 c2100	Scroll 3 Control		* Note: missing on MS1-Z

Offset:		00					   Scroll X
			02					   Scroll Y
			04 fedc ba98 765- ---- ? (unused?)
			   ---- ---- ---4 ---- 16x16 Tiles
			   ---- ---- ---- 32-- ? (used, by p47!)
			   ---- ---- ---- --10 N: Layer H pages = 16 / (2^N)



84300 44300 c2308	Screen Control

	fedc ba9- ---- ---- ? (unused?)
	---- ---8 ---- ---- Portrait F/F (?FullFill?)
	---- ---- 765- ---- ? (unused?)
	---- ---- ---4 ---- ? (used, see p47j copyright screen!)
	---- ---- ---- 321- ? (unused?)
	---- ---- ---- ---0	Screen V Flip



**********  There are 256*4 colors (256*3 for MS1-Z):

Colors		MS1-A/C			MS1-Z

000-0ff		Scroll 1		Scroll 1
100-1ff		Scroll 2		Sprites
200-2ff		Scroll 3		Scroll 2
300-3ff		Sprites			-

88000 48000 f8000	Palette

	fedc--------3---	Red
	----ba98-----2--	Blue
	--------7654--1-	Green
	---------------0	? (used, not RGB! [not changed in fades])


**********  There are 256 sprites (128 for MS1-Z):

RAM[8000]	Sprite Data	(16 bytes/entry. 128? entries)

Offset:		0-6						? (used, but as normal RAM, I think)
			08 	fed- ---- ---- ----	?
				---c ---- ---- ----	mosaic sol.	(?)
				---- ba98 ---- ----	mosaic		(?)
				---- ---- 7--- ----	y flip
				---- ---- -6-- ----	x flip
				---- ---- --45 ----	?
				---- ---- ---- 3210	color code (* bit 3 = priority *)
			0A						H position
			0C						V position
			0E	fedc ---- ---- ----	? (used by p47j, 0-8!)
				---- ba98 7654 3210	Number



Object RAM tells the hw how to use Sprite Data (missing on MS1-Z).
This makes it possible to group multiple small sprite, up to 256,
into one big virtual sprite (up to a whole screen):

8e000 4e000 d2000	Object RAM (8 bytes/entry. 256*4 entries)

Offset:		00	Index into Sprite Data RAM
			02	H	Displacement
			04	V	Displacement
			06	Number	Displacement

Only one of these four 256 entries is used to see if the sprite is to be
displayed, according to this latter's flipx/y state:

Object RAM entries:		Useb by sprites with:

000-0ff					No Flip
100-1ff					Flip X
200-2ff					Flip Y
300-3ff					Flip X & Y




No? No? c2108	Sprite Bank

	fedc ba98 7654 321- ? (unused?)
	---- ---- ---- ---0 Sprite Bank



84100 44100 c2200 Sprite Control

			fedc ba9- ---- ---- ? (unused?)
			---- ---8 ---- ---- Enable sprite priority effect ? (see p47j sprite test 1!)
			---- ---- 765- ---- ? (unused?)
			---- ---- ---4 ----	Enable Effect (?)
			---- ---- ---- 3210	Effect Number (?)

I think bit 4 enables some sort of color cycling for sprites having priority
bit set. See code of p7j at 6488,  affecting the rotating sprites before the
Jaleco logo is shown: values 11-1f, then 0. I fear the Effect Number is an
offset to be applied over the pens used by those sprites. As for bit 8, it's
not used during game, but it is turned on when sprite/foreground priority is
tested, along with Effect Number being 1, so ...


**********  Priority (ouch!)

Sprite order is from first in Sprite Data RAM (frontmost) to last.
* Note: the opposite on MS1-Z

Foreground isn't splittable in a "back" part using, say, pens 0-7 and a
"front part" using pens 8-15 usually. Sprites aren't splittable either.
But level A of 64th street has megasys_fg that is splittable. I think bit 0 of
the color in paletteram has a role also.


Here's how I draw things (surely incomplete/inaccurate):

	MS1-Z/A/B		MS1-C
megasys_bg  Scroll RAM 1	Scroll RAM 2
megasys_fg  Scroll RAM 2	Scroll RAM 1
megasys_txt Scroll RAM 3	Scroll RAM 3

MS1-Z	-
MS1-A	bit 8 of 84000 is on  -> swap megasys_fg, megasys_txt
MS1-B	bit 8 of 44000 is on  -> swap megasys_fg, megasys_txt
MS1-C	bit 8 of c2208 is off -> swap megasys_bg, megasys_fg


bit x is sprite priority enable:			bit 8 of 84100 44100 c2200
bit 9 is a foreground priority control:		bit 9 of 84000 44000 !c2208

					megasys_bg
	if bit x
		if !bit 9	megasys_fg
					sprites (priority bit 1)
		if  bit 9	megasys_fg
					sprites (priority bit 0)
	else
		if !bit 9	megasys_fg
					sprites (priority doesn't affect order)
		if  bit 9	megasys_fg

					megasys_txt (missing on MS1-Z)

***************************************************************************/

#include "vidhrdw/generic.h"

extern unsigned char *megasys_scrollram[3],*megasys_scrollram_dirty[3];		
extern unsigned char *megasys_objectram, *megasys_ram;
extern int megasys_scrollx[3],megasys_scrolly[3],megasys_scrollflag[3],megasys_nx[3],megasys_ny[3];	
extern int megasys_active_layers, megasys_spritebank, megasys_spriteflag,megasys_hardware_type;
extern int megasys_bg, megasys_fg, megasys_txt;
extern struct GameDriver avspirit_driver;
struct osd_bitmap *megasys_scroll_bitmap[3];

void mark_dirty(int n) { fast_memset(megasys_scrollram_dirty[n],1,256*256/64*8); }
void mark_dirty_all(void)
{
	mark_dirty(0);
	mark_dirty(1);
	mark_dirty(2);
}

static unsigned char megasys_scrollram_dirty_static[4*256*256/64*8];

#if defined(DREAMCAST) && defined(EXTRA_MEM_PATCH)
static struct osd_bitmap extra_bitmap[2];
static unsigned extra_punts[2]={0x04400000,0x04400000+0x00212900};
#endif

int megasys_vh_start(void)
{
	int i;

#if defined(DREAMCAST) && defined(EXTRA_MEM_PATCH)
	extra_punts[0]=0x04400000; //((unsigned)vram_l)+0x00400000; //(unsigned)pvr_mem_malloc(0x00212900*2); //0xa5400000;
	extra_punts[1]=extra_punts[0]+0x00212900;
#endif
	
/* allocate a dirty page for each scrolling layer */

	for (i = 0; i < 3; i++) {
	 megasys_scrollram_dirty[i] = (unsigned char *)&megasys_scrollram_dirty_static[i*(256*256/64*8)];
//	 if ((megasys_scrollram_dirty[i] = (unsigned char *)gp2x_malloc(256*256/64*8))==0) return 1;
	 megasys_scroll_bitmap[i]=NULL;
	}

/*  temporary bitmaps are not created here. They are instead
   created on the fly when refreshing the screen. */

	mark_dirty_all();

	return 0;
}


void megasys_vh_stop(void)
{
int i;

#if defined(DREAMCAST) && defined(EXTRA_MEM_PATCH)
//	pvr_mem_free((void *)extra_punts[0]);
#endif
	for (i = 0; i < 3; i++)
	{
#if defined(DREAMCAST) && defined(EXTRA_MEM_PATCH)
	 if (i==1 || i==2)
		 megasys_scroll_bitmap[i]=NULL;
#endif
	 if (megasys_scroll_bitmap[i])		osd_free_bitmap(megasys_scroll_bitmap[i]);
//	 if (megasys_scrollram_dirty[i])	gp2x_free(megasys_scrollram_dirty[i]);
	}
}


/*

 Draw sprites in the given bitmap. Priority may be:

 0	Draw sprites whose priority bit is 0
 1	Draw sprites whose priority bit is 1
-1	Draw sprites regardless of their priority

 Sprite Data:

	Offset		Data

 	00-07						?
	08 		fed- ---- ---- ----	?
			---c ---- ---- ----	mosaic sol.	(?)
			---- ba98 ---- ----	mosaic		(?)
			---- ---- 7--- ----	y flip
			---- ---- -6-- ----	x flip
			---- ---- --45 ----	?
			---- ---- ---- 3210	color code (bit 3 = priority)
	0A		H position
	0C		V position
	0E		Number

*/
static void draw_sprites(struct osd_bitmap *bitmap, int priority)
{
int color,code,sx,sy,attr,sprite,offs;
unsigned char *spritedata, *objectdata;

/* objram: 0x100*4 entries		spritedata: 0x80? entries */

	/* move the bit to the relevant position (and invert it) */
	if (priority != -1)	priority = (priority ^ 1) << 3;

	/* sprite order is from first in Sprite Data RAM (frontmost) to last */

	if (megasys_hardware_type != 'Z')
	{
		for (sprite = 0x80-1; sprite >= 0 ; sprite --)
		{
			spritedata = &spriteram[sprite*0x10];

			attr = READ_WORD(&spritedata[0x08]);

			if ( (attr & 0x08) == priority ) continue;

			objectdata = &megasys_objectram[((attr & 0xc0) >> 6) * 0x800];

			for (offs = 0x000; offs < 0x800 ; offs += 8)
			{
				/* seek a reference to current sprite */
				if ( sprite != READ_WORD(&objectdata[offs+0x00]) ) continue;

				/* apply the position displacements */
				sx = ( READ_WORD(&spritedata[0x0A]) + READ_WORD(&objectdata[offs+0x02]) ) % 512;
				sy = ( READ_WORD(&spritedata[0x0C]) + READ_WORD(&objectdata[offs+0x04]) ) % 512;

				if (sx > 256-1) sx -= 512;
				if (sy > 256-1) sy -= 512;

				/* out of screen */
				if ((sx > 256-1) ||(sy > 256-1) ||(sx < 0-15) ||(sy < 0-15)) continue;

				code  = READ_WORD(&spritedata[0x0E]) + READ_WORD(&objectdata[offs+0x06]);
				color = (attr & 0x0F);

				drawgfx(bitmap,Machine->gfx[3],
					(code & 0xfff ) + (megasys_spritebank << 12),
					color,
					attr & 0x40,attr & 0x80,
					sx, sy,
					&Machine->drv->visible_area,
					TRANSPARENCY_PEN,15);
			}	/* offs */
		}	/* sprite */
	}	/* non Z hw */
	else
	{

		/* MS1-Z just draws Sprite Data, and in reverse order */

		for (sprite = 0; sprite < 0x80 ; sprite ++)
		{
			spritedata = &spriteram[sprite*0x10];

			attr = READ_WORD(&spritedata[0x08]);
			if ( (attr & 0x08) == priority ) continue;

			sx = READ_WORD(&spritedata[0x0A]) % 512;
			sy = READ_WORD(&spritedata[0x0C]) % 512;

			if (sx > 256-1) sx -= 512;
			if (sy > 256-1) sy -= 512;

			/* out of screen */
			if ((sx > 256-1) ||(sy > 256-1) ||(sx < 0-15) ||(sy < 0-15)) continue;

			code  = READ_WORD(&spritedata[0x0E]);
			color = (attr & 0x0F);

			drawgfx(bitmap,Machine->gfx[2],
				code,
				color,
				attr & 0x40,attr & 0x80,
				sx, sy,
				&Machine->drv->visible_area,
				TRANSPARENCY_PEN,15);
		}	/* sprite */
	}	/* Z hw */

}

#if defined(DREAMCAST) && defined(EXTRA_MEM_PATCH)
static struct osd_bitmap *osd_new_bitmap_using_vram(int width,int height,int nbitmap)
{
	unsigned extra_punt=extra_punts[nbitmap&1];
	osd_bitmap *bitmap=(osd_bitmap *)&extra_bitmap[nbitmap&1];

	if (Machine->orientation & ORIENTATION_SWAP_XY)
	{
		int temp;

		temp = width;
		width = height;
		height = temp;
	}
	
	int i,rowlen,rdwidth;
	unsigned char *bm;
	int safety;

	if (width > 32) safety = 8;
	else safety = 0;        /* don't create the safety area for GfxElement bitmaps */


	bitmap->depth = 8;
	bitmap->width = width;
	bitmap->height = height;

	rdwidth = (width + 7) & ~7;     /* round width to a quadword */
	rowlen =     (rdwidth + 2 * safety) * sizeof(unsigned char);

	bitmap->_private=(void *)extra_punt;
	extra_punt+= (height + 2 * safety) * rowlen;

	bm = (unsigned char*)bitmap->_private;
	fast_memset(bm,0,(height + 2 * safety) * rowlen);

	bitmap->line =(unsigned char**)extra_punt;
	extra_punt+=height * sizeof(unsigned char *);

	for (i = 0;i < height;i++)
		bitmap->line[i] = &bm[(i + safety) * rowlen + safety];

	if (!nbitmap)
		extra_punts[1]=extra_punt;

	return bitmap;
}



#endif


/***************************************************************************

  Draw the game screen in the given osd_bitmap.

***************************************************************************/
void paletteram_RRRRGGGGBBBBRGBx_word_w(int offset, int data);
void megasys_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh)
{
int offs,code,sx,sy,attr,i,j;
unsigned char *current_ram, *current_dirty;
int current_mask;
int mask[3] = {0xfff,0xfff,0xfff};

unsigned int *pen_usage;
int color_codes_start, color, colmask[16], layers_n;

int active_layers1;

	active_layers1 = megasys_active_layers;

	layers_n = 3;

	switch (megasys_hardware_type)
	{
		/* MS1-Z has scroll 1,2 & Sprites only (always active, no priority ?) */
		case 'Z':
		{
			layers_n = 2;
			megasys_bg = 0;		megasys_fg = 1;		megasys_txt = 2;
			megasys_active_layers = 0x020B;
		}	break;

		case 'A':
		{
			megasys_bg = 0;
			if (megasys_active_layers & 0x0100)	{megasys_fg = 2;	megasys_txt = 1;}
			else						{megasys_fg = 1;	megasys_txt = 2;}
		}

		case 'B':
		{
			if (Machine->gamedrv != &avspirit_driver)
			{
				megasys_bg = 0;
				if (megasys_active_layers & 0x0100)	{megasys_fg = 2;	megasys_txt = 1;}
				else						{megasys_fg = 1;	megasys_txt = 2;}
			}
			else
			{
				megasys_txt = 2;
				if (megasys_active_layers & 0x0100)	{megasys_bg = 0;	megasys_fg = 1;}
				else						{megasys_bg = 1;	megasys_fg = 0;}
			}
		}	break;

		case 'C':
		{
			megasys_active_layers ^= 0x0200;	/* sprites/megasys_fg priority swapped */
			megasys_txt = 2;
			if (megasys_active_layers & 0x0100)	{megasys_bg = 0;	megasys_fg = 1;}
			else						{megasys_bg = 1;	megasys_fg = 0;}
		}	break;

	}


/* Palette stuff */

	palette_init_used_colors();

/* We consider whole layers, redraw whole layers: worst case is slow! */

	for (j = 0 ; j < layers_n ; j++ )
	{
		/* let's skip disabled layers */

		pen_usage = Machine->gfx[j]->pen_usage;
		color_codes_start = Machine->drv->gfxdecodeinfo[j].color_codes_start;

		for (color = 0 ; color < 16 ; color++) colmask[color] = 0;

		for (offs  = 0 ; offs < 0x4000 ; offs += 2)
		{
			color = ( READ_WORD(&megasys_scrollram[j][offs]) & 0xF000 ) >> 12;
			code  =   READ_WORD(&megasys_scrollram[j][offs]) & mask[j];

			if (megasys_scrollflag[j] & 0x10)
					colmask[color] |= pen_usage[code];
			else{	colmask[color] |= pen_usage[code*4+0];
					colmask[color] |= pen_usage[code*4+1];
					colmask[color] |= pen_usage[code*4+2];
					colmask[color] |= pen_usage[code*4+3];}
		}

		for (color = 0; color < 16; color++)
		{
			if (colmask[color])
			{
				for (i = 0; i < 16; i++)
					if (colmask[color] & (1 << i))
						palette_used_colors[16 * color + i + color_codes_start] = PALETTE_COLOR_USED;
			}
		}

		if (j != megasys_bg)	/* background colors are all used */
		{
			for (color = 0; color < 16; color++)
				palette_used_colors[16 * color + 15 + color_codes_start] = PALETTE_COLOR_TRANSPARENT;
		}
	}



	/* Sprites */

	for (color = 0 ; color < 16 ; color++) colmask[color] = 0;

	/* different sprites hw */
	if (megasys_hardware_type == 'Z')
	{
	int sprite;

		pen_usage = Machine->gfx[2]->pen_usage;
		color_codes_start = Machine->drv->gfxdecodeinfo[2].color_codes_start;

		for (sprite = 0; sprite < 0x80 ; sprite++)
		{
		unsigned char* spritedata;

			spritedata = &spriteram[sprite*16];

			sx = READ_WORD(&spritedata[0x0A]) % 512;
			sy = READ_WORD(&spritedata[0x0C]) % 512;

			if (sx > 256-1) sx -= 512;
			if (sy > 256-1) sy -= 512;

			if ((sx > 256-1) ||(sy > 256-1) ||(sx < 0-15) ||(sy < 0-15)) continue;

			code  = READ_WORD(&spritedata[0x0E]);
			color = READ_WORD(&spritedata[0x08]) & 0x0F;

			colmask[color] |= pen_usage[code];
		}
	}
	else
	{
		pen_usage = Machine->gfx[3]->pen_usage;
		color_codes_start = Machine->drv->gfxdecodeinfo[3].color_codes_start;

		for (offs = 0; offs < 0x2000 ; offs += 8)
		{
		int sprite;
		unsigned char* spritedata;

			sprite = READ_WORD(&megasys_objectram[offs+0x00]);
			spritedata = &spriteram[(sprite&0x7F)*16];

			attr = READ_WORD(&spritedata[0x08]);
			if ( (attr & 0xc0) != ((offs/0x800)<<6) ) continue;

			sx = ( READ_WORD(&spritedata[0x0A]) + READ_WORD(&megasys_objectram[offs+0x02]) ) % 512;
			sy = ( READ_WORD(&spritedata[0x0C]) + READ_WORD(&megasys_objectram[offs+0x04]) ) % 512;

			if (sx > 256-1) sx -= 512;
			if (sy > 256-1) sy -= 512;

			if ((sx > 256-1) ||(sy > 256-1) ||(sx < 0-15) ||(sy < 0-15)) continue;

			code  = READ_WORD(&spritedata[0x0E]) + READ_WORD(&megasys_objectram[offs+0x06]);
			code  =	(code & 0xfff ) + (megasys_spritebank << 12);
			color = (attr & 0x0F);

			colmask[color] |= pen_usage[code];
		}
	}


	for (color = 0; color < 16; color++)
	 for (i = 0; i < 16; i++)
	  if (colmask[color] & (1 << i)) palette_used_colors[16 * color + i + color_codes_start] = PALETTE_COLOR_USED;

	if (palette_recalc()) mark_dirty_all();



/* Allocate off screen bitmaps */

	for (i = 0; i < layers_n ; i++)
	{
		megasys_nx[i] = 16 / ( 1 << (megasys_scrollflag[i] & 0x3) );
		megasys_ny[i] = 32 / megasys_nx[i];

		if (megasys_scrollflag[i]&0x10)
		{
			if (megasys_ny[i] > 4)
			{
				megasys_nx[i] /= 1;
				megasys_ny[i] /= 4;
			}
			else
			{
				megasys_nx[i] /= 2;
				megasys_ny[i] /= 2;
			}
		}

		/* let's skip disabled layers */
		if ( (megasys_active_layers & (1 << i )) && (megasys_scroll_bitmap[i] == 0) )
		{
#if defined(DREAMCAST) && defined(EXTRA_MEM_PATCH)
			if (i==1 || i==2)
				megasys_scroll_bitmap[i] = osd_new_bitmap_using_vram(256*megasys_nx[i], 256*megasys_ny[i],i-1);
			else
#endif
			if ((megasys_scroll_bitmap[i] = osd_new_bitmap(256*megasys_nx[i], 256*megasys_ny[i], 4))==0)
			{
				return;
			}

		}

	}


/* Update the scrolling layers */

	for (i = 0; i < layers_n ; i++)
	{
		/* let's skip disabled layers */
		if (!(megasys_active_layers & (1 << i )))	continue;

		current_ram   = megasys_scrollram[i];
		current_dirty = megasys_scrollram_dirty[i];
		current_mask  = mask[i];

		if (megasys_scrollflag[i] & 0x10)	/* 8x8 tiles */
		{
		 for (j=0 ; j < megasys_nx[i]*megasys_ny[i] ; j++)
		  for (sx = (j%megasys_nx[i])*256; sx < (j%megasys_nx[i])*256+256; sx += 8)
		   for (sy = (j/megasys_nx[i])*256; sy < (j/megasys_nx[i])*256+256; sy += 8)
		   {
			if (current_dirty[0])
			{
				current_dirty[0] = 0;
				code = READ_WORD(&current_ram[0]);
				drawgfx(megasys_scroll_bitmap[i],Machine->gfx[i],
					code & current_mask,
					(code & 0xF000)>>12,
					0,0,
					sx,sy,
					0,TRANSPARENCY_NONE,0);
			};
			current_dirty++;	current_ram+=2;
		   }
		}
		else	/* 16x16 tiles */
		{
		 for (j=0 ; j < megasys_nx[i]*megasys_ny[i] ; j++)
		  for (sx = (j%megasys_nx[i])*256; sx < (j%megasys_nx[i])*256+256; sx += 16)
		   for (sy = (j/megasys_nx[i])*256; sy < (j/megasys_nx[i])*256+256; sy += 16)
		   {
			if (current_dirty[0])
			{
				current_dirty[0] = 0;
				code = READ_WORD(&current_ram[0]);
				attr = (code >> 12) & 0x0F;
				code = (code & current_mask) * 4;
				drawgfx(megasys_scroll_bitmap[i],Machine->gfx[i],
					code+0,
					attr,
					0,0,sx+0,sy+0,0,TRANSPARENCY_NONE,0);
				drawgfx(megasys_scroll_bitmap[i],Machine->gfx[i],
					code+1,
					attr,
					0,0,sx+0,sy+8,0,TRANSPARENCY_NONE,0);
				drawgfx(megasys_scroll_bitmap[i],Machine->gfx[i],
					code+2,
					attr,
					0,0,sx+8,sy+0,0,TRANSPARENCY_NONE,0);
				drawgfx(megasys_scroll_bitmap[i],Machine->gfx[i],
					code+3,
					attr,
					0,0,sx+8,sy+8,0,TRANSPARENCY_NONE,0);
			};
			current_dirty++;	current_ram+=2;
		   }
		} /* 16x16 tiles */
	} /* i */

/* copy each (enabled) layer */


#define copylayer(_n_,_transparency_,_pen_)\
	if (megasys_active_layers & (1 << _n_ ))\
	{\
		copyscrollbitmap(bitmap, megasys_scroll_bitmap[_n_],\
				1,&megasys_scrollx[_n_],1,&megasys_scrolly[_n_],\
				&Machine->drv->visible_area,\
				_transparency_,_pen_);\
	}


	/* Copy the background */
	copylayer(megasys_bg,TRANSPARENCY_NONE,0)
	else	osd_clearbitmap(Machine->scrbitmap);

	/* If priority effect is active .. */
	if (megasys_spriteflag & 0x0100)
	{
		/* Copy the foreground below ... */
		if (!(megasys_active_layers &0x0200))	copylayer(megasys_fg,TRANSPARENCY_PEN,palette_transparent_pen);

		/* draw sprites with color >= 8 */
		if (megasys_active_layers & 0x08)	draw_sprites(bitmap,1);

		/* ... or over sprites with color >= 8 */
		if (megasys_active_layers &0x0200)	copylayer(megasys_fg,TRANSPARENCY_PEN,palette_transparent_pen);

		/* Draw sprites with color 0-7 */
		if (megasys_active_layers & 0x08)	draw_sprites(bitmap,0);
	}
	else
	{
		/* Copy the foreground below ... */
		if (!(megasys_active_layers &0x0200))	copylayer(megasys_fg,TRANSPARENCY_PEN,palette_transparent_pen);

		/* Draw sprites regardless of their color */
		if (megasys_active_layers & 0x08)	draw_sprites(bitmap,-1);

		/* ... or over sprites */
		if (megasys_active_layers &0x0200)	copylayer(megasys_fg,TRANSPARENCY_PEN,palette_transparent_pen);
	}

	/* Copy the text layer */
	copylayer(megasys_txt,TRANSPARENCY_PEN,palette_transparent_pen);

	megasys_active_layers = active_layers1;
}
