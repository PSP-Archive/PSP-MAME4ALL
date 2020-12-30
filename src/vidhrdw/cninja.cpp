/***************************************************************************

   Caveman Ninja Video emulation - Bryan McPhail, mish@tendril.force9.net

****************************************************************************

Data East custom chip 55:  Generates two playfields, playfield 1 is underneath
playfield 2.  Caveman Ninja uses two of these chips.

	16 bytes of control registers per chip.

	Word 0:
		Mask 0x0080: Flip screen
		Mask 0x007f: ?
	Word 2:
		Mask 0xffff: Playfield 2 X scroll (top playfield)
	Word 4:
		Mask 0xffff: Playfield 2 Y scroll (top playfield)
	Word 6:
		Mask 0xffff: Playfield 1 X scroll (bottom playfield)
	Word 8:
		Mask 0xffff: Playfield 1 Y scroll (bottom playfield)
	Word 0xa:
		Mask 0xc000: Playfield 1 shape??
		Mask 0x3000: Playfield 1 rowscroll style (maybe mask 0x3800??)
		Mask 0x0300: Playfield 1 colscroll style (maybe mask 0x0700??)?

		Mask 0x00c0: Playfield 2 shape??
		Mask 0x0030: Playfield 2 rowscroll style (maybe mask 0x0038??)
		Mask 0x0003: Playfield 2 colscroll style (maybe mask 0x0007??)?
	Word 0xc:
		Mask 0x8000: Playfield 1 is 8*8 tiles else 16*16
		Mask 0x4000: Playfield 1 rowscroll enabled
		Mask 0x2000: Playfield 1 colscroll enabled
		Mask 0x1f00: ?

		Mask 0x0080: Playfield 2 is 8*8 tiles else 16*16
		Mask 0x0040: Playfield 2 rowscroll enabled
		Mask 0x0020: Playfield 2 colscroll enabled
		Mask 0x001f: ?
	Word 0xe:
		??

Colscroll style:
	0	64 columns across bitmap
	1	32 columns across bitmap

Rowscroll style:
	0	512 rows across bitmap


Locations 0 & 0xe are mostly unknown:

							 0		14
Caveman Ninja (bottom):		0053	1100 (see below)
Caveman Ninja (top):		0010	0081
Two Crude (bottom):			0053	0000
Two Crude (top):			0010	0041
Dark Seal (bottom):			0010	0000
Dark Seal (top):			0053	4101
Tumblepop:					0010	0000
Super Burger Time:			0010	0000

Location 14 in Cninja (bottom):
 1100 = pf2 uses graphics rom 1, pf3 uses graphics rom 2
 0000 = pf2 uses graphics rom 2, pf3 uses graphics rom 2
 1111 = pf2 uses graphics rom 1, pf3 uses graphics rom 1

**************************************************************************

Sprites - Data East custom chip 52

	8 bytes per sprite, unknowns bits seem unused.

	Word 0:
		Mask 0x8000 - ?
		Mask 0x4000 - Y flip
		Mask 0x2000 - X flip
		Mask 0x1000 - Sprite flash
		Mask 0x0800 - ?
		Mask 0x0600 - Sprite height (1x, 2x, 4x, 8x)
		Mask 0x01ff - Y coordinate

	Word 2:
		Mask 0xffff - Sprite number

	Word 4:
		Mask 0x8000 - ?
		Mask 0x4000 - Sprite is drawn beneath top 8 pens of playfield 4
		Mask 0x3e00 - Colour (32 palettes, most games only use 16)
		Mask 0x01ff - X coordinate

	Word 6:
		Always unused.

***************************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"

unsigned char *cninja_pf1_data,*cninja_pf2_data;
unsigned char *cninja_pf3_data,*cninja_pf4_data;
unsigned char *cninja_pf1_rowscroll,*cninja_pf2_rowscroll;
unsigned char *cninja_pf3_rowscroll,*cninja_pf4_rowscroll;

static struct tilemap *cninja_pf1_tilemap,*cninja_pf2_tilemap,*cninja_pf3_tilemap,*cninja_pf4_tilemap;
static unsigned char *gfx_base;
static int gfx_bank;

static unsigned char cninja_control_0[16];
static unsigned char cninja_control_1[16];

static int cninja_pf2_bank,cninja_pf3_bank;
static int bootleg;

/******************************************************************************/

static void update_24bitcol(int offset)
{
	int r,g,b;

	if (offset%4) offset-=2;

	b = (READ_WORD(&paletteram[offset]) >> 0) & 0xff;
	g = (READ_WORD(&paletteram[offset+2]) >> 8) & 0xff;
	r = (READ_WORD(&paletteram[offset+2]) >> 0) & 0xff;

	palette_change_color(offset / 4,r,g,b);
}

void cninja_palette_24bit_w(int offset,int data)
{
	COMBINE_WORD_MEM(&paletteram[offset],data);
	update_24bitcol(offset);
}

/******************************************************************************/

static void mark_sprites_colors(void)
{
	int offs,color,i,pal_base;
	int colmask[16];
    unsigned int *pen_usage; /* Save some struct derefs */

	/* Sprites */
	pal_base = Machine->drv->gfxdecodeinfo[4].color_codes_start;
	pen_usage=Machine->gfx[4]->pen_usage;
	for (color = 0;color < 16;color++) colmask[color] = 0;

	for (offs = 0;offs < 0x800;offs += 8)
	{
		int x,y,sprite,multi;

		sprite = READ_WORD (&spriteram[offs+2]) & 0x3fff;
		if (!sprite) continue;

		x = READ_WORD(&spriteram[offs+4]);
		y = READ_WORD(&spriteram[offs]);

		color = (x >> 9) &0xf;
		multi = (1 << ((y & 0x0600) >> 9)) - 1;
		sprite &= ~multi;

		/* Save palette by missing offscreen sprites */
		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 256) x -= 512;
		x = 240 - x;
		if (x>256) continue;

		while (multi >= 0)
		{
			colmask[color] |= pen_usage[sprite + multi];
			multi--;
		}
	}

	for (color = 0;color < 16;color++)
	{
		for (i = 1;i < 16;i++)
		{
			if (colmask[color] & (1 << i))
				palette_used_colors[pal_base + 16 * color + i] = PALETTE_COLOR_USED;
		}
	}
}

static void cninja_drawsprites(struct osd_bitmap *bitmap, int pri)
{
	int offs;

	for (offs = 0;offs < 0x800;offs += 8)
	{
		int x,y,sprite,colour,multi,fx,fy,inc,flash;
		sprite = READ_WORD (&spriteram[offs+2]) & 0x3fff;
		if (!sprite) continue;

		x = READ_WORD(&spriteram[offs+4]);

		/* Sprite/playfield priority */
		if ((x&0x4000) && pri==1) continue;
		if (!(x&0x4000) && pri==0) continue;

		y = READ_WORD(&spriteram[offs]);
		flash=y&0x1000;
		if (flash && (cpu_getcurrentframe() & 1)) continue;
		colour = (x >> 9) &0xf;

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x0600) >> 9)) - 1;	/* 1x, 2x, 4x, 8x height */

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 256) x -= 512;
		if (y >= 256) y -= 512;
		x = 240 - x;
		y = 240 - y;

		if (x>256) continue; /* Speedup */

		sprite &= ~multi;
		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		while (multi >= 0)
		{
			drawgfx(bitmap,Machine->gfx[4],
					sprite - multi * inc,
					colour,
					fx,fy,
					x,y - 16 * multi,
					&Machine->drv->visible_area,TRANSPARENCY_PEN,0);

			multi--;
		}
	}
}

/* Function for all 16x16 1024x512 layers */
static void get_back_tile_info( int col, int row )
{
	int offs,tile,color;

	if (col>31 && row>15) offs=0xc00 + (col-32)*2 + (row-16) *64; /* Bottom right */
	else if (col>31) offs=0x800 + (col-32)*2 + row *64; /* Top right */
	else if (row>15) offs=0x400 + col*2 + (row-16) *64; /* Bottom left */
	else offs=col*2 + row *64; /* Top left */

	tile=READ_WORD(&gfx_base[offs]);
	color=tile >> 12;
	tile=tile&0xfff;

	SET_TILE_INFO(gfx_bank,tile,color)
}

/* 8x8 top layer */
static void get_fore_tile_info( int col, int row )
{
	int offs=(col*2) + (row*128);
	int tile=READ_WORD(&cninja_pf1_data[offs]);
	int color=tile >> 12;

	tile=tile&0xfff;

	SET_TILE_INFO(0,tile,color)
}

/******************************************************************************/

void cninja_vh_screenrefresh(struct osd_bitmap *bitmap, int full_refresh)
{
	int offs;
	int pf23_control,pf1_control;

	/* Handle gfx rom switching */
	pf23_control=READ_WORD (&cninja_control_0[0xe]);
	if ((pf23_control&0xff)==0x00)
		cninja_pf3_bank=2;
	else
		cninja_pf3_bank=1;

	if ((pf23_control&0xff00)==0x00)
		cninja_pf2_bank=2;
	else
		cninja_pf2_bank=1;

	/* Setup scrolling */
	pf23_control=READ_WORD (&cninja_control_0[0xc]);
	pf1_control=READ_WORD (&cninja_control_1[0xc]);

	/* Background - Rowscroll enable */
	if (pf23_control&0x4000) {
		int scrollx=READ_WORD(&cninja_control_0[6]);
		tilemap_set_scroll_cols(cninja_pf3_tilemap,1);
		tilemap_set_scrolly( cninja_pf3_tilemap,0, READ_WORD(&cninja_control_0[8]) );

		/* Several different rowscroll styles! */
		switch ((READ_WORD (&cninja_control_0[0xa])>>12)&7) {
			case 2: /* 16 rows (of 16 pixels) */
				tilemap_set_scroll_rows(cninja_pf2_tilemap,16);
				for (offs = 0;offs < 16;offs++)
					tilemap_set_scrollx( cninja_pf2_tilemap,offs, scrollx + READ_WORD(&cninja_pf2_rowscroll[2*offs]) );
				break;
			case 0: /* 256 rows (doubled because the bitmap is 512 */
				tilemap_set_scroll_rows(cninja_pf2_tilemap,512);
				for (offs = 0;offs < 512; offs++)
					tilemap_set_scrollx( cninja_pf2_tilemap,offs, scrollx + READ_WORD(&cninja_pf2_rowscroll[2*offs]) );
				break;
		}
	}
	else {
		tilemap_set_scroll_rows(cninja_pf2_tilemap,1);
		tilemap_set_scroll_cols(cninja_pf2_tilemap,1);
		tilemap_set_scrollx( cninja_pf2_tilemap,0, READ_WORD(&cninja_control_0[6]) );
		tilemap_set_scrolly( cninja_pf2_tilemap,0, READ_WORD(&cninja_control_0[8]) );
	}

	/* Playfield 3 */
	if (pf23_control&0x40) { /* Rowscroll */
		int scrollx=READ_WORD(&cninja_control_0[2]);
		tilemap_set_scroll_rows(cninja_pf3_tilemap,32);
		tilemap_set_scroll_cols(cninja_pf3_tilemap,1);
		tilemap_set_scrolly( cninja_pf3_tilemap,0, READ_WORD(&cninja_control_0[4]) );

		/* We have to map 16 screen tiles to the size of the bitmap... */
		for (offs = 0;offs < 32;offs++)
			tilemap_set_scrollx( cninja_pf3_tilemap,offs, scrollx + READ_WORD(&cninja_pf3_rowscroll[2*offs]) );
	}
	else if (pf23_control&0x20) { /* Colscroll */
		int scrolly=READ_WORD(&cninja_control_0[4]);
		tilemap_set_scroll_rows(cninja_pf3_tilemap,1);
		tilemap_set_scroll_cols(cninja_pf3_tilemap,64);
		tilemap_set_scrollx( cninja_pf3_tilemap,0, READ_WORD(&cninja_control_0[2]) );

		/* Used in lava level & Level 1 */
		for (offs=0 ; offs < 32;offs++)
			tilemap_set_scrolly( cninja_pf3_tilemap,offs+32, scrolly + READ_WORD(&cninja_pf3_rowscroll[(2*offs)+0x400]) );
	}
	else {
		tilemap_set_scroll_rows(cninja_pf3_tilemap,1);
		tilemap_set_scroll_cols(cninja_pf3_tilemap,1);
		tilemap_set_scrollx( cninja_pf3_tilemap,0, READ_WORD(&cninja_control_0[2]) );
		tilemap_set_scrolly( cninja_pf3_tilemap,0, READ_WORD(&cninja_control_0[4]) );
	}

	/* Top foreground */
	if (pf1_control&0x2000) { /* Colscroll */
		int scrolly=READ_WORD(&cninja_control_1[8]);
		tilemap_set_scroll_rows(cninja_pf4_tilemap,1);
		tilemap_set_scroll_cols(cninja_pf4_tilemap,64);
		tilemap_set_scrollx( cninja_pf4_tilemap,0, READ_WORD(&cninja_control_0[2]) );

		/* Used in first lava level */
		for (offs=0 ; offs < 64;offs++)
			tilemap_set_scrolly( cninja_pf4_tilemap,offs, scrolly + READ_WORD(&cninja_pf4_rowscroll[(2*offs)+0x400]) );
	}
	else {
		tilemap_set_scroll_rows(cninja_pf4_tilemap,1);
		tilemap_set_scroll_cols(cninja_pf4_tilemap,1);
		tilemap_set_scrollx( cninja_pf4_tilemap,0, READ_WORD(&cninja_control_1[6]) );
		tilemap_set_scrolly( cninja_pf4_tilemap,0, READ_WORD(&cninja_control_1[8]) );
	}

	/* Playfield 1 - 8 * 8 Text */
	if (pf1_control&0x40) { /* Rowscroll */
		int scrollx=READ_WORD(&cninja_control_1[2]);
		tilemap_set_scroll_rows(cninja_pf1_tilemap,256);
		tilemap_set_scroll_cols(cninja_pf1_tilemap,1);
		tilemap_set_scrolly( cninja_pf1_tilemap,0, READ_WORD(&cninja_control_1[4]) );

		/* We have to map 16 screen tiles to the size of the bitmap... */
		for (offs = 0;offs < 256;offs++)
			tilemap_set_scrollx( cninja_pf1_tilemap,offs, scrollx + READ_WORD(&cninja_pf1_rowscroll[2*offs]) );
	}
	else {
		tilemap_set_scrollx( cninja_pf1_tilemap,0, READ_WORD(&cninja_control_1[2]) );
		tilemap_set_scrolly( cninja_pf1_tilemap,0, READ_WORD(&cninja_control_1[4]) );
	}

	/* Update playfields */
	gfx_bank=cninja_pf2_bank;
	gfx_base=cninja_pf2_data;
	tilemap_update(cninja_pf2_tilemap);

	gfx_bank=cninja_pf3_bank;
	gfx_base=cninja_pf3_data;
	tilemap_update(cninja_pf3_tilemap);

	gfx_bank=3;
	gfx_base=cninja_pf4_data;
	tilemap_update(cninja_pf4_tilemap);
	tilemap_update(cninja_pf1_tilemap);

	palette_init_used_colors();
	mark_sprites_colors();
	if (palette_recalc())
		tilemap_mark_all_pixels_dirty(ALL_TILEMAPS);

	/* Draw playfields */
	tilemap_render(ALL_TILEMAPS);
	tilemap_draw(bitmap,cninja_pf2_tilemap,0);
	tilemap_draw(bitmap,cninja_pf3_tilemap,0);
	tilemap_draw(bitmap,cninja_pf4_tilemap,TILEMAP_BACK);
	cninja_drawsprites(bitmap,0);
	tilemap_draw(bitmap,cninja_pf4_tilemap,TILEMAP_FRONT);
	cninja_drawsprites(bitmap,1);
	tilemap_draw(bitmap,cninja_pf1_tilemap,0);
}

/******************************************************************************/

void cninja_pf1_data_w(int offset,int data)
{
	int oldword = READ_WORD(&cninja_pf1_data[offset]);
	int newword = COMBINE_WORD(oldword,data);

	if (oldword != newword)
	{
		WRITE_WORD(&cninja_pf1_data[offset],newword);
		tilemap_mark_tile_dirty(cninja_pf1_tilemap,(offset%128)/2,offset/128);
	}
}

void cninja_pf2_data_w(int offset,int data)
{
	int oldword = READ_WORD(&cninja_pf2_data[offset]);
	int newword = COMBINE_WORD(oldword,data);
	int dx=0,dy=0;

	if (oldword != newword)
	{
		WRITE_WORD(&cninja_pf2_data[offset],newword);

		if (offset>0xbff) {offset-=0xc00;dx=32; dy=16;}
		else if (offset>0x7ff) {offset-=0x800;dx=32; dy=0;}
		else if (offset>0x3ff) {offset-=0x400;dx=0; dy=16;}
		dx+=(offset%64)/2; dy+=(offset/64);
		tilemap_mark_tile_dirty(cninja_pf2_tilemap,dx,dy);
	}
}

void cninja_pf3_data_w(int offset,int data)
{
	int oldword = READ_WORD(&cninja_pf3_data[offset]);
	int newword = COMBINE_WORD(oldword,data);
	int dx=0,dy=0;

	if (oldword != newword)
	{
		WRITE_WORD(&cninja_pf3_data[offset],newword);

		if (offset>0xbff) {offset-=0xc00;dx=32; dy=16;}
		else if (offset>0x7ff) {offset-=0x800;dx=32; dy=0;}
		else if (offset>0x3ff) {offset-=0x400;dx=0; dy=16;}
		dx+=(offset%64)/2; dy+=(offset/64);
		tilemap_mark_tile_dirty(cninja_pf3_tilemap,dx,dy);
	}
}

void cninja_pf4_data_w(int offset,int data)
{
	int oldword = READ_WORD(&cninja_pf4_data[offset]);
	int newword = COMBINE_WORD(oldword,data);
	int dx=0,dy=0;

	if (oldword != newword)
	{
		WRITE_WORD(&cninja_pf4_data[offset],newword);

		if (offset>0xbff) {offset-=0xc00;dx=32; dy=16;}
		else if (offset>0x7ff) {offset-=0x800;dx=32; dy=0;}
		else if (offset>0x3ff) {offset-=0x400;dx=0; dy=16;}
		dx+=(offset%64)/2; dy+=(offset/64);
		tilemap_mark_tile_dirty(cninja_pf4_tilemap,dx,dy);
	}
}

void cninja_control_0_w(int offset,int data)
{
	if (bootleg && offset==6) {
		COMBINE_WORD_MEM(&cninja_control_0[offset],data+0xa);
		return;
	}
	COMBINE_WORD_MEM(&cninja_control_0[offset],data);
}

void cninja_control_1_w(int offset,int data)
{
	if (bootleg) {
		switch (offset) {
			case 2:
				COMBINE_WORD_MEM(&cninja_control_1[offset],data-2);
				return;
			case 6:
				COMBINE_WORD_MEM(&cninja_control_1[offset],data+0xa);
				return;
		}
	}
	COMBINE_WORD_MEM(&cninja_control_1[offset],data);
}

int cninja_pf1_data_r(int offset)
{
	return READ_WORD(&cninja_pf1_data[offset]);
}

void cninja_pf1_rowscroll_w(int offset,int data)
{
	COMBINE_WORD_MEM(&cninja_pf1_rowscroll[offset],data);
}

void cninja_pf2_rowscroll_w(int offset,int data)
{
	COMBINE_WORD_MEM(&cninja_pf2_rowscroll[offset],data);
}

void cninja_pf3_rowscroll_w(int offset,int data)
{
	COMBINE_WORD_MEM(&cninja_pf3_rowscroll[offset],data);
}

int cninja_pf3_rowscroll_r(int offset)
{
	return READ_WORD(&cninja_pf3_rowscroll[offset]);
}

void cninja_pf4_rowscroll_w(int offset,int data)
{
	COMBINE_WORD_MEM(&cninja_pf4_rowscroll[offset],data);
}

/******************************************************************************/

void cninja_vh_stop (void)
{
	/* Nothing, remove later */
}

int cninja_vh_start(void)
{
	/* The bootleg has some broken scroll registers... */
	if (!strcmp(Machine->gamedrv->name,"stoneage"))
		bootleg=1;
	else
		bootleg=0;

	cninja_pf2_bank=1;
	cninja_pf3_bank=2;

	cninja_pf2_tilemap = tilemap_create(
		get_back_tile_info,
		0,
		16,16,
		64,32 /* 1024 by 512 */
	);

	cninja_pf3_tilemap = tilemap_create(
		get_back_tile_info,
		TILEMAP_TRANSPARENT,
		16,16,
		64,32
	);

	cninja_pf4_tilemap = tilemap_create(
		get_back_tile_info,
		TILEMAP_TRANSPARENT | TILEMAP_SPLIT,
		16,16,
		64,32
	);

	cninja_pf1_tilemap = tilemap_create(
		get_fore_tile_info,
		TILEMAP_TRANSPARENT,
		8,8,
		64,32
	);

	cninja_pf1_tilemap->transparent_pen = 0;
	cninja_pf3_tilemap->transparent_pen = 0;
	cninja_pf4_tilemap->transparent_pen = 0;
	cninja_pf4_tilemap->transmask[0] = 0x00ff;
	cninja_pf4_tilemap->transmask[1] = 0xff00;

	return 0;
}

/******************************************************************************/
