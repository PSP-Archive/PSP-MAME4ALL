/***************************************************************************

 COSMIC.C

 emulation of video hardware of cosmic machines of 1979-1980(ish)

***************************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"


static int refresh_tmpbitmap;
static int flipscreen;
static int (*map_color)(int x, int y);

static int color_registers[3];
static int color_base = 0;


void panic_color_register_w(int offset,int data)
{
	/* 7c0c & 7c0e = Rom Address Offset
 	   7c0d        = high / low nibble */

	if (color_registers[offset] != (data & 0x80))
    {
    	color_registers[offset] = data & 0x80;
    	color_base = (color_registers[0] << 2) + (color_registers[2] << 3);
    	refresh_tmpbitmap = 1;
    }
}

void cosmicg_color_register_w(int offset,int data)
{
	if (color_registers[offset] != data)
    {
    	color_registers[offset] = data;

        color_base = 0;

        if (color_registers[0] == 1) color_base += 0x100;
        if (color_registers[1] == 1) color_base += 0x200;

	refresh_tmpbitmap = 1;
    }
}


static int panic_map_color(int x, int y)
{
	/* 8 x 16 coloring */
	unsigned char byte = Machine->memory_region[3][color_base + (x>>4) * 32 + (y>>3)];

	if (color_registers[1])
		return byte >> 4;
	else
		return byte & 0x0f;
}

//static int cosmicg_map_color(int x, int y)
//{
//	unsigned char byte;

	/* 16 x 16 coloring */
	//if ((x >= 224) && (x < 232))
	//	byte = color_rom[color_base + (y / 16) * 16 + 9];
	//else
//		byte = Machine->memory_region[3][color_base + (y>>4) * 16 + (x>>4)];

	/* the upper 4 bits are for cocktail mode support */

//	return byte & 0x0f;
//}


static const unsigned char panic_remap_sprite_code[64][2] =
{
{0x00,0},{0x00,0},{0x00,0},{0x00,0},{0x00,0},{0x00,0},{0x00,0},{0x00,0}, /* 00 */
{0x00,0},{0x26,0},{0x25,0},{0x24,0},{0x23,0},{0x22,0},{0x21,0},{0x20,0}, /* 08 */
{0x00,0},{0x00,0},{0x00,0},{0x00,0},{0x00,0},{0x00,0},{0x00,0},{0x00,0}, /* 10 */
{0x00,0},{0x16,0},{0x15,0},{0x14,0},{0x13,0},{0x12,0},{0x11,0},{0x10,0}, /* 18 */
{0x00,0},{0x00,0},{0x00,0},{0x00,0},{0x00,0},{0x00,0},{0x00,0},{0x00,0}, /* 20 */
{0x00,0},{0x06,0},{0x05,0},{0x04,0},{0x03,0},{0x02,0},{0x01,0},{0x00,0}, /* 28 */
{0x00,0},{0x00,0},{0x00,0},{0x00,0},{0x00,0},{0x00,0},{0x00,0},{0x00,0}, /* 30 */
{0x07,2},{0x06,2},{0x05,2},{0x04,2},{0x03,2},{0x02,2},{0x01,2},{0x00,2}, /* 38 */
};

/*
 * Panic Color table setup
 *
 * Bit 0 = RED, Bit 1 = GREEN, Bit 2 = BLUE
 *
 * First 8 colors are normal intensities
 *
 * But, bit 3 can be used to pull Blue via a 2k resistor to 5v
 * (1k to ground) so second version of table has blue set to 2/3
 *
 */

void panic_vh_convert_color_prom(unsigned char *palette, unsigned short *colortable,const unsigned char *color_prom)
{
	int i;
	#define TOTAL_COLORS(gfxn) (Machine->gfx[gfxn]->total_colors * Machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn,offs) (colortable[Machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])

	for (i = 0;i < Machine->drv->total_colors;i++)
	{
		*(palette++) = 0xff * ((i >> 0) & 1);
		*(palette++) = 0xff * ((i >> 1) & 1);
		if ((i & 0x0c) == 0x08)
			*(palette++) = 0xaa;
		else
			*(palette++) = 0xff * ((i >> 2) & 1);
	}


	for (i = 0;i < TOTAL_COLORS(0);i++)
		COLOR(0,i) = *(color_prom++) & 0x0f;


    map_color = panic_map_color;
}


void cosmica_videoram_w(int offset,int data)
{
    int i,x,y,col;

    videoram[offset] = data;

	y = offset >> 5;
	x = 8 * (offset & 31);

    col = Machine->pens[map_color(x, y)];

    for (i = 0; i < 8; i++)
    {
		if (flipscreen)
			//plot_pixel(tmpbitmap, 255-x, 255-y, (data & 0x80) ? col : Machine->pens[0]);
			tmpbitmap->line[x][y] = (data & 0x80) ? col: Machine->pens[0];
		else
			tmpbitmap->line[255-x][y] = (data & 0x80) ? col: Machine->pens[0];
			//tmpbitmap->line[255-x][255-y] = (data & 0x80) ? col: Machine->pens[0];

	    x++;
	    data <<= 1;
    }
}

void cosmica_flipscreen_w(int offset, int data)
{
        if (data != flipscreen)
        {
                flipscreen = data;

                refresh_tmpbitmap = 1;
        }
}

void cosmicg_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh)
{
	if (refresh_tmpbitmap)
	{
		int offs;

		for (offs = 0; offs < videoram_size; offs++)
		{
			cosmica_videoram_w(offs, videoram[offs]);
		}

		refresh_tmpbitmap = 0;
	}

	copybitmap(bitmap,tmpbitmap,0,0,0,0,&Machine->drv->visible_area,TRANSPARENCY_NONE,0);
}


void panic_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh)
{
	int offs;


	cosmicg_vh_screenrefresh(bitmap, full_refresh);


    /* draw the sprites */

	for (offs = spriteram_size - 4;offs >= 0;offs -= 4)
	{
		if (spriteram[offs] != 0)
		{
			int code,bank,flipy;

			/* panic_remap_sprite_code sprite number to my layout */

			code = panic_remap_sprite_code[(spriteram[offs] & 0x3F)][0];
			bank = panic_remap_sprite_code[(spriteram[offs] & 0x3F)][1];
			flipy = spriteram[offs] & 0x40;

			/* Switch Bank */

			if(spriteram[offs+3] & 0x08) bank=1;

			if (flipscreen)
			{
				flipy = !flipy;
			}

			drawgfx(bitmap,Machine->gfx[bank],
					code,
					7 - (spriteram[offs+3] & 0x07),
					flipscreen,flipy,
					256-spriteram[offs+2],spriteram[offs+1],
					&Machine->drv->visible_area,TRANSPARENCY_PEN,0);
		}
	}
}


int panic_vh_start(void)
{
        if ((tmpbitmap = osd_new_bitmap(Machine->drv->screen_width,Machine->drv->screen_height, 8)) == 0)
        {
                return 1;
        }

        return 0;
}


void panic_vh_stop(void)
{
        osd_free_bitmap(tmpbitmap);

        tmpbitmap = 0;
}
