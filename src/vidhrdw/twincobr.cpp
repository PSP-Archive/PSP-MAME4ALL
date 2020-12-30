/***************************************************************************

  vidhrdw.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"
#include "vidhrdw/crtc6845.h"


static unsigned char *twincobr_bgvideoram;
static unsigned char *twincobr_fgvideoram;
static int twincobr_bgvideoram_size,twincobr_fgvideoram_size;
int wardner_sprite_hack = 0;    /* Required for weird sprite priority in wardner*/
static unsigned char txscroll[4],bgscroll[4],fgscroll[4];
int twincobr_fg_rom_bank = 0;
int twincobr_bg_ram_bank = 0;
int twincobr_display_on = 0;
int twincobr_flip_screen = 0;     /* not implemented properly yet */
static int charoffs;
static int bgoffs;
static int fgoffs;

/* Wardener variables */
static int tx_offset_lsb = 0;
static int tx_offset_msb = 0;
static int bg_offset_lsb = 0;
static int bg_offset_msb = 0;
static int fg_offset_lsb = 0;
static int fg_offset_msb = 0;
static int tx_scrollx_lsb = 0;
static int tx_scrollx_msb = 0;
static int tx_scrolly_lsb = 0;
static int tx_scrolly_msb = 0;
static int bg_scrollx_lsb = 0;
static int bg_scrollx_msb = 0;
static int bg_scrolly_lsb = 0;
static int bg_scrolly_msb = 0;
static int fg_scrollx_lsb = 0;
static int fg_scrollx_msb = 0;
static int fg_scrolly_lsb = 0;
static int fg_scrolly_msb = 0;


int twincobr_vh_start(void)
{
	/* the video RAM is accessed via ports, it's not memory mapped */
	videoram_size = 0x1000;
	twincobr_bgvideoram_size = 0x4000;
	twincobr_fgvideoram_size = 0x2000;

	if ((videoram = (unsigned char*)gp2x_malloc(videoram_size)) == 0)
		return 1;
	fast_memset(videoram,0,videoram_size);

	if ((twincobr_fgvideoram = (unsigned char*)gp2x_malloc(twincobr_fgvideoram_size)) == 0)
	{
		gp2x_free(videoram);
		return 1;
	}
	fast_memset(twincobr_fgvideoram,0,twincobr_fgvideoram_size);

	if ((twincobr_bgvideoram = (unsigned char*)gp2x_malloc(twincobr_bgvideoram_size)) == 0)
	{
		gp2x_free(twincobr_fgvideoram);
		gp2x_free(videoram);
		return 1;
	}
	fast_memset(twincobr_bgvideoram,0,twincobr_bgvideoram_size);

	if ((dirtybuffer = (unsigned char*)gp2x_malloc(twincobr_bgvideoram_size)) == 0)
	{
		gp2x_free(twincobr_bgvideoram);
		gp2x_free(twincobr_fgvideoram);
		gp2x_free(videoram);
		return 1;
	}
	fast_memset(dirtybuffer,1,twincobr_bgvideoram_size);

	if ((tmpbitmap = osd_create_bitmap(Machine->drv->screen_width,2*Machine->drv->screen_height)) == 0)
	{
		gp2x_free(dirtybuffer);
		gp2x_free(twincobr_bgvideoram);
		gp2x_free(twincobr_fgvideoram);
		gp2x_free(videoram);
		return 1;
	}

	return 0;
}

void twincobr_vh_stop(void)
{
	osd_free_bitmap(tmpbitmap);
	gp2x_free(dirtybuffer);
	gp2x_free(twincobr_bgvideoram);
	gp2x_free(twincobr_fgvideoram);
	gp2x_free(videoram);
}

int twincobr_txoffs_r(void)
{
        return charoffs / 2;
}

int twincobr_60000_r(int offset)
{
	int temp6000x;
	temp6000x = crtc6845_register_r(offset);
	return temp6000x;
}

void twincobr_60000_w(int offset,int data)
{
	if (offset == 0) crtc6845_address_w(offset, data);
	if (offset == 2) crtc6845_register_w(offset, data);
}

void twincobr_70004_w(int offset,int data)
{
   charoffs = (2 * data) % videoram_size;
}
int twincobr_7e000_r(int offset)
{
	return READ_WORD(&videoram[charoffs]);
}
void twincobr_7e000_w(int offset,int data)
{
	WRITE_WORD(&videoram[charoffs],data);
}

void twincobr_72004_w(int offset,int data)
{
	bgoffs = (2 * data) % (twincobr_bgvideoram_size >> 1);
}
int twincobr_7e002_r(int offset)
{
	return READ_WORD(&twincobr_bgvideoram[bgoffs+twincobr_bg_ram_bank]);
}
void twincobr_7e002_w(int offset,int data)
{
	WRITE_WORD(&twincobr_bgvideoram[bgoffs+twincobr_bg_ram_bank],data);
	dirtybuffer[bgoffs / 2] = 1;
}

void twincobr_74004_w(int offset,int data)
{
	fgoffs = (2 * data) % twincobr_fgvideoram_size;
}
int twincobr_7e004_r(int offset)
{
	return READ_WORD(&twincobr_fgvideoram[fgoffs]);
}
void twincobr_7e004_w(int offset,int data)
{
	WRITE_WORD(&twincobr_fgvideoram[fgoffs],data);
}

void twincobr_76004_w(int offset,int data)
{
}


void twincobr_txscroll_w(int offset,int data)
{
	WRITE_WORD(&txscroll[offset],data);
}

void twincobr_bgscroll_w(int offset,int data)
{
	WRITE_WORD(&bgscroll[offset],data);
}

void twincobr_fgscroll_w(int offset,int data)
{
	WRITE_WORD(&fgscroll[offset],data);
}



void twincobr_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh)
{
  int offs;
  if (twincobr_display_on) {
  fast_memset(palette_used_colors,PALETTE_COLOR_UNUSED,Machine->drv->total_colors * sizeof(unsigned char));

	{
	int color,code,i;
	int colmask[64];
	int pal_base;


	pal_base = Machine->drv->gfxdecodeinfo[2].color_codes_start;

	for (color = 0;color < 16;color++) colmask[color] = 0;

	for (offs = (twincobr_bgvideoram_size >> 1) - 2;offs >= 0;offs -= 2)
	{
		code = READ_WORD(&twincobr_bgvideoram[offs+twincobr_bg_ram_bank]) & 0x0fff;
		color = (READ_WORD(&twincobr_bgvideoram[offs+twincobr_bg_ram_bank]) & 0xf000) >> 12;
		colmask[color] |= Machine->gfx[2]->pen_usage[code];
	}

	for (color = 0;color < 16;color++)
	{
		for (i = 0;i < 16;i++)
		{
			if (colmask[color] & (1 << i))
				palette_used_colors[pal_base + 16 * color + i] = PALETTE_COLOR_USED;
		}
	}


	pal_base = Machine->drv->gfxdecodeinfo[1].color_codes_start;

	for (color = 0;color < 16;color++) colmask[color] = 0;

	for (offs = twincobr_fgvideoram_size - 2;offs >= 0;offs -= 2)
	{
		code = (READ_WORD(&twincobr_fgvideoram[offs]) & 0x0fff) | twincobr_fg_rom_bank;
		color = (READ_WORD(&twincobr_fgvideoram[offs]) & 0xf000) >> 12;
		colmask[color] |= Machine->gfx[1]->pen_usage[code];
	}

	for (color = 0;color < 16;color++)
	{
		if (colmask[color] & (1 << 0))
			palette_used_colors[pal_base + 16 * color] = PALETTE_COLOR_TRANSPARENT;
		for (i = 1;i < 16;i++)
		{
			if (colmask[color] & (1 << i))
				palette_used_colors[pal_base + 16 * color + i] = PALETTE_COLOR_USED;
		}
	}


	pal_base = Machine->drv->gfxdecodeinfo[3].color_codes_start;

	for (color = 0;color < 64;color++) colmask[color] = 0;

	for (offs = 0;offs < spriteram_size;offs += 8)
	{
		code = READ_WORD(&spriteram[offs]) & 0x7ff;
		color = READ_WORD(&spriteram[offs + 2]) & 0x3f;
		colmask[color] |= Machine->gfx[3]->pen_usage[code];
	}

	for (color = 0;color < 64;color++)
	{
		if (colmask[color] & (1 << 0))
			palette_used_colors[pal_base + 16 * color] = PALETTE_COLOR_TRANSPARENT;
		for (i = 1;i < 16;i++)
		{
			if (colmask[color] & (1 << i))
				palette_used_colors[pal_base + 16 * color + i] = PALETTE_COLOR_USED;
		}
	}


	pal_base = Machine->drv->gfxdecodeinfo[0].color_codes_start;

	for (color = 0;color < 32;color++) colmask[color] = 0;

	for (offs = videoram_size - 2;offs >= 0;offs -= 2)
	{
		code = READ_WORD(&videoram[offs]) & 0x07ff;
		color = (READ_WORD(&videoram[offs]) & 0xf800) >> 11;
		colmask[color] |= Machine->gfx[0]->pen_usage[code];
	}

	for (color = 0;color < 32;color++)
	{
		if (colmask[color] & (1 << 0))
			palette_used_colors[pal_base + 8 * color] = PALETTE_COLOR_TRANSPARENT;
		for (i = 1;i < 8;i++)
		{
			if (colmask[color] & (1 << i))
				palette_used_colors[pal_base + 8 * color + i] = PALETTE_COLOR_USED;
		}
	}


	if (palette_recalc())
	{
		fast_memset(dirtybuffer,1,twincobr_bgvideoram_size >> 1);
	}
    }



	/* draw the background */
	for (offs = (twincobr_bgvideoram_size >> 1) - 2;offs >= 0;offs -= 2)
	{
		if (dirtybuffer[offs / 2])
		{
			int sx,sy,code,color;


			dirtybuffer[offs / 2] = 0;

			sx = (offs/2) % 64;
			sy = (offs/2) / 64;

			code = READ_WORD(&twincobr_bgvideoram[offs+twincobr_bg_ram_bank]) & 0x0fff;
			color = (READ_WORD(&twincobr_bgvideoram[offs+twincobr_bg_ram_bank]) & 0xf000) >> 12;

			drawgfx(tmpbitmap,Machine->gfx[2],
				code,
				color,
				twincobr_flip_screen,twincobr_flip_screen,
				8*sx,8*sy,
				0,TRANSPARENCY_NONE,0);
		}
	}

	/* copy the background graphics */
	{
		int scrollx,scrolly;


		scrollx = 0x1c9 - READ_WORD(&bgscroll[0]);
		scrolly = - 0x1e - READ_WORD(&bgscroll[2]);

		copyscrollbitmap(bitmap,tmpbitmap,1,&scrollx,1,&scrolly,&Machine->drv->visible_area,TRANSPARENCY_NONE,0);
	}

	/* draw the sprites in low priority (tanks under roofs) */
	for (offs = 0;offs < spriteram_size;offs += 8)
	{
		int code,color,attribute,sx,sy,flipx,flipy;

		attribute = READ_WORD(&spriteram[offs + 2]);
		if ((attribute & 0x0c00) == 0x0400) {       /* low priority */
		    code = READ_WORD(&spriteram[offs]) & 0x7ff;
		    color = attribute & 0x3f;
		    sx = READ_WORD(&spriteram[offs + 4]) >> 7;
		    sy = READ_WORD(&spriteram[offs + 6]) >> 7;
		    flipx = attribute & 0x100;
		    if (flipx) sx -= 15;
		    flipy = attribute & 0x200;

		    drawgfx(bitmap,Machine->gfx[3],
				code,
				color,
				flipx,flipy,
				sx-32,sy-16,
				&Machine->drv->visible_area,TRANSPARENCY_PEN,0);
		}
	}


	/* draw the foreground */
	for (offs = twincobr_fgvideoram_size - 2;offs >= 0;offs -= 2)
	{
		int sx,sy,code,color;
		int scrollx,scrolly;


		sx = (offs/2) % 64;
		sy = (offs/2) / 64;

		scrollx = 0x1c9 - READ_WORD(&fgscroll[0]);
		scrolly = - 0x1e - READ_WORD(&fgscroll[2]);

		code = (READ_WORD(&twincobr_fgvideoram[offs]) & 0x0fff) | twincobr_fg_rom_bank;
		color = (READ_WORD(&twincobr_fgvideoram[offs]) & 0xf000) >> 12;

		drawgfx(bitmap,Machine->gfx[1],
				code,
				color,
				twincobr_flip_screen,twincobr_flip_screen,
				((8*sx + scrollx + 8) & 0x1ff) - 8,((8*sy + scrolly + 8) & 0x1ff) - 8,
				&Machine->drv->visible_area,TRANSPARENCY_PEN,0);
	}

#if 0
/*********  Begin ugly sprite hack for Wardner when hero is in shop *********/
        if ((wardner_sprite_hack) && (fgscrollx != bgscrollx)) {        /* Wardner ? */
                if ((fgscrollx==0x1c9) || (twincobr_flip_screen && (fgscrollx==0x17a))) {       /* in the shop ? */
                        int wardner_hack = READ_WORD_Z80(&buffered_spriteram[0x0b04]);
                /* sprite position 0x6300 to 0x8700 -- hero on shop keeper (normal) */
                /* sprite position 0x3900 to 0x5e00 -- hero on shop keeper (flip) */
                        if ((wardner_hack > 0x3900) && (wardner_hack < 0x8700))
{       /* hero at shop keeper ? */
                                        wardner_hack = READ_WORD_Z80(&buffered_spriteram[0x0b02]);
                                        wardner_hack |= 0x0400; /* make hero top priority */
                                        WRITE_WORD_Z80(&buffered_spriteram[0x0b02],wardner_hack);
                                        wardner_hack = READ_WORD_Z80(&buffered_spriteram[0x0b0a]);
                                        wardner_hack |= 0x0400;
                                        WRITE_WORD_Z80(&buffered_spriteram[0x0b0a],wardner_hack);
                                        wardner_hack = READ_WORD_Z80(&buffered_spriteram[0x0b12]);
                                        wardner_hack |= 0x0400;
                                        WRITE_WORD_Z80(&buffered_spriteram[0x0b12],wardner_hack);
                                        wardner_hack = READ_WORD_Z80(&buffered_spriteram[0x0b1a]);
                                        wardner_hack |= 0x0400;
                                        WRITE_WORD_Z80(&buffered_spriteram[0x0b1a],wardner_hack);
                        }
                }
        }
/**********  End ugly sprite hack for Wardner when hero is in shop **********/
#endif



	/* draw the sprites in normal priority, underneath front layer */
	for (offs = 0;offs < spriteram_size;offs += 8)
	{
		int code,color,attribute,sx,sy,flipx,flipy;

		attribute = READ_WORD(&spriteram[offs + 2]);
		if ((attribute & 0x0c00) == 0x0800) {    /* normal priority */
		    code = READ_WORD(&spriteram[offs]) & 0x7ff;
		    color = attribute & 0x3f;
		    sx = READ_WORD(&spriteram[offs + 4]) >> 7;
		    sy = READ_WORD(&spriteram[offs + 6]) >> 7;
		    flipx = attribute & 0x100;
		    if (flipx) sx -= 15;
		    flipy = attribute & 0x200;

		    drawgfx(bitmap,Machine->gfx[3],
				code,
				color,
				flipx,flipy,
				sx-32,sy-16,
				&Machine->drv->visible_area,TRANSPARENCY_PEN,0);
		}
	}


	/* draw the front layer */
	for (offs = videoram_size - 2;offs >= 0;offs -= 2)
	{
		int sx,sy,code,color;
		int scrollx,scrolly;


		sx = (offs/2) % 64;
		sy = (offs/2) / 64;

		scrollx = 0x01c9 - READ_WORD(&txscroll[0]);
		scrolly = -0x1e - READ_WORD(&txscroll[2]);

		code = READ_WORD(&videoram[offs]) & 0x07ff;
		color = (READ_WORD(&videoram[offs]) & 0xf800) >> 11;

		drawgfx(bitmap,Machine->gfx[0],
				code,
				color,
				twincobr_flip_screen,twincobr_flip_screen,
				((8*sx + scrollx + 8) & 0x1ff) - 8,((8*sy + scrolly + 8) & 0xff) - 8,
				&Machine->drv->visible_area,TRANSPARENCY_PEN,0);
	}

	/* draw the sprites on the very top layer (title screen only) */
	for (offs = 0;offs < spriteram_size;offs += 8)
	{
		int code,color,attribute,sx,sy,flipx,flipy;

		attribute = READ_WORD(&spriteram[offs + 2]);
		if ((attribute & 0x0c00) == 0x0c00) {   /* highest priority */
		    code = READ_WORD(&spriteram[offs]) & 0x7ff;
		    color = attribute & 0x3f;
		    sx = READ_WORD(&spriteram[offs + 4]) >> 7;
		    sy = READ_WORD(&spriteram[offs + 6]) >> 7;
		    flipx = attribute & 0x100;
		    if (flipx) sx -= 15;
		    flipy = attribute & 0x200;

		    drawgfx(bitmap,Machine->gfx[3],
				code,
				color,
				flipx,flipy,
				sx-32,sy-16,
				&Machine->drv->visible_area,TRANSPARENCY_PEN,0);
		}
	}
  }
}

/*****************************************************************************/
/* Wardner Interface                                                         */
/*****************************************************************************/
void wardner_txlayer_w(int offset, int data)
{
        if (offset == 0) tx_offset_lsb = data;
        if (offset == 1) tx_offset_msb = (data << 8);
        twincobr_70004_w(0,tx_offset_msb | tx_offset_lsb);
}
void wardner_bglayer_w(int offset, int data)
{
        if (offset == 0) bg_offset_lsb = data;
        if (offset == 1) bg_offset_msb = (data<<8);
        twincobr_72004_w(0,bg_offset_msb | bg_offset_lsb);
}
void wardner_fglayer_w(int offset, int data)
{
        if (offset == 0) fg_offset_lsb = data;
        if (offset == 1) fg_offset_msb = (data<<8);
        twincobr_74004_w(0,fg_offset_msb | fg_offset_lsb);
}

void wardner_txscroll_w(int offset, int data)
{
        if (offset & 2) {
                if (offset == 2) tx_scrollx_lsb = data;
                if (offset == 3) tx_scrollx_msb = (data<<8);
                twincobr_txscroll_w(2,tx_scrollx_msb | tx_scrollx_lsb);
        }
        else
        {
                if (offset == 0) tx_scrolly_lsb = data;
                if (offset == 1) tx_scrolly_msb = (data<<8);
                twincobr_txscroll_w(0,tx_scrolly_msb | tx_scrolly_lsb);
        }
}
void wardner_bgscroll_w(int offset, int data)
{
        if (offset & 2) {
                if (offset == 2) bg_scrollx_lsb = data;
                if (offset == 3) bg_scrollx_msb = (data<<8);
                twincobr_bgscroll_w(2,bg_scrollx_msb | bg_scrollx_lsb);
        }
        else
        {
                if (offset == 0) bg_scrolly_lsb = data;
                if (offset == 1) bg_scrolly_msb = (data<<8);
                twincobr_bgscroll_w(0,bg_scrolly_msb | bg_scrolly_lsb);
        }
}

void wardner_fgscroll_w(int offset, int data)
{
        if (offset & 2) {
                if (offset == 2) fg_scrollx_lsb = data;
                if (offset == 3) fg_scrollx_msb = (data<<8);
                twincobr_fgscroll_w(2,fg_scrollx_msb | fg_scrollx_lsb);
        }
        else
        {
                if (offset == 0) fg_scrolly_lsb = data;
                if (offset == 1) fg_scrolly_msb = (data<<8);
                twincobr_fgscroll_w(0,fg_scrolly_msb | fg_scrolly_lsb);
        }
}

int wardner_videoram_r(int offset)
{
        int memdata = 0;
        switch (offset) {
                case 0: memdata =  twincobr_7e000_r(0) & 0x00ff; break;
                case 1: memdata = (twincobr_7e000_r(0) & 0xff00) >> 8; break;
                case 2: memdata =  twincobr_7e002_r(0) & 0x00ff; break;
                case 3: memdata = (twincobr_7e002_r(0) & 0xff00) >> 8; break;
                case 4: memdata =  twincobr_7e004_r(0) & 0x00ff; break;
                case 5: memdata = (twincobr_7e004_r(0) & 0xff00) >> 8; break;
        }
        return memdata;
}

void wardner_videoram_w(int offset, int data)
{
        int memdata = 0;
        switch (offset) {
                case 0: memdata = twincobr_7e000_r(0) & 0xff00;
                                memdata |= data;
                                twincobr_7e000_w(0,memdata); break;
                case 1: memdata = twincobr_7e000_r(0) & 0x00ff;
                                memdata |= (data << 8);
                                twincobr_7e000_w(0,memdata); break;
                case 2: memdata = twincobr_7e002_r(0) & 0xff00;
                                memdata |= data;
                                twincobr_7e002_w(0,memdata); break;
                case 3: memdata = twincobr_7e002_r(0) & 0x00ff;
                                memdata |= (data << 8);
                                twincobr_7e002_w(0,memdata); break;
                case 4: memdata = twincobr_7e004_r(0) & 0xff00;
                                memdata |= data;
                                twincobr_7e004_w(0,memdata); break;
                case 5: memdata = twincobr_7e004_r(0) & 0x00ff;
                                memdata |= (data << 8);
                                twincobr_7e004_w(0,memdata); break;
        }
}



void twincobr_exscroll_w(int offset,int data) /* Extra unused video layer */
{
}
