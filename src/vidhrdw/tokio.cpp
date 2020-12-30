#include "driver.h"
#include "vidhrdw/generic.h"
#include "ctype.h"

extern unsigned char *tokio_sharedram;

unsigned char *tokio_objectram;
int tokio_objectram_size;



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/
int tokio_vh_start(void)
{
	return 0;
}

/***************************************************************************

  Stop the video hardware emulation.

***************************************************************************/
void tokio_vh_stop(void)
{
}

int tokio_videoram_r(int offset)
{
	return videoram[offset];
}

void tokio_videoram_w(int offset,int data)
{
	if (videoram[offset] != data)
		videoram[offset] = data;
}

int tokio_objectram_r(int offset)
{
	return tokio_objectram[offset];
}

void tokio_objectram_w(int offset,int data)
{
	if (tokio_objectram[offset] != data)
		tokio_objectram[offset] = data;
}



/***************************************************************************

  Draw the game screen in the given osd_bitmap.
  Do NOT call osd_update_display() from this function, it will be called by
  the main emulation engine.

***************************************************************************/
void tokio_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh)
{
	int offs;
	int sx,sy,xc,yc;
	int gfx_num,gfx_attr,gfx_offs;
	


	/* recalc the palette if necessary */
	palette_recalc();

	/* Tokio doesn't have a real video RAM! */
	/* All graphics (characters and sprites) are stored in the same */
	/* memory region, and information on the background character */
	/* columns is stored in the area dd00-dd3f. */

	/* This clears & redraws the entire screen each pass */
	fillbitmap(bitmap,Machine->gfx[0]->colortable[0],&Machine->drv->visible_area);

	sx = 0;
	for (offs = 0;offs < tokio_objectram_size;offs += 4)
	{
		int height;
		
		/* skip empty sprites */
		/* this is dword aligned so the UINT32 * cast shouldn't give problems */
		/* on any architecture */
		if (*(UINT32 *)(&tokio_objectram[offs]) == 0)
			continue;
		
		gfx_num = tokio_objectram[offs + 1];
		gfx_attr = tokio_objectram[offs + 3];
		
		if ((gfx_num & 0x80) == 0)	/* 16x16 sprites */
		{
			gfx_offs = ((gfx_num & 0x1f) * 0x80) + ((gfx_num & 0x60) >> 1) + 12;
			height = 2;
		}
		else	/* tilemaps (each sprite is a 16x256 column) */
		{
			gfx_offs = ((gfx_num & 0x3f) * 0x80);
			height = 32;
		}

		if ((gfx_num & 0xc0) == 0xc0)
			sx += 16;
		else
		{
			sx = tokio_objectram[offs + 2];
			if (gfx_attr & 0x40) sx -= 256;
		}
		sy = 256 - height*8 - (tokio_objectram[offs + 0]);

		for (xc = 0;xc < 2;xc++)
		{
			for (yc = 0;yc < height;yc++)
			{
				int goffs,code,color,flipx,flipy,x,y;

				goffs = gfx_offs + xc * 0x40 + yc * 0x02;
				code = videoram[goffs] + 256 * (videoram[goffs + 1] & 0x03) + 1024 * (gfx_attr & 0x0f);
				color = (videoram[goffs + 1] & 0x3c) >> 2;
				flipx = videoram[goffs + 1] & 0x40;
				flipy = videoram[goffs + 1] & 0x80;
				x = sx + xc * 8;
				y = (sy + yc * 8) & 0xff;

				drawgfx(bitmap,Machine->gfx[0],
						code,
						color,
						flipx,flipy,
						x,y,
						&Machine->drv->visible_area,TRANSPARENCY_PEN,0);
			}
		}
	}
}
		
