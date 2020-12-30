/***************************************************************************

  vidhrdw.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"

unsigned char *berzerk_magicram;
extern int berzerk_irq_end_of_screen;

static int magicram_control = 0xff;
static int magicram_latch = 0xff;
static int collision = 0;


INLINE void copy_byte(int x, int y, int data, int col)
{
	char fore, back;
	unsigned char *line1, *line2;


	if (y < 32)  return;

	fore  = Machine->pens[(col >> 4) & 0x0f];
	back  = Machine->pens[0];
	line1 = &(Machine->scrbitmap->line[y][x]);
	line2 = &(tmpbitmap->line[y][x]);

	*line1 = *line2 = (data & 0x80) ? fore : back;  line1++;  line2++;
	*line1 = *line2 = (data & 0x40) ? fore : back;  line1++;  line2++;
	*line1 = *line2 = (data & 0x20) ? fore : back;  line1++;  line2++;
	*line1 = *line2 = (data & 0x10) ? fore : back;  line1++;  line2++;

	fore  = Machine->pens[col & 0x0f];

	*line1 = *line2 = (data & 0x08) ? fore : back;  line1++;  line2++;
	*line1 = *line2 = (data & 0x04) ? fore : back;  line1++;  line2++;
	*line1 = *line2 = (data & 0x02) ? fore : back;  line1++;  line2++;
	*line1 = *line2 = (data & 0x01) ? fore : back;
}


void berzerk_videoram_w(int offset,int data)
{
	int coloroffset, x, y;

	videoram[offset] = data;

	/* Get location of color RAM for this offset */
	coloroffset = ((offset & 0xff80) >> 2) | (offset & 0x1f);

	y = (offset >> 5);
	x = (offset & 0x1f) << 3;

	/* osd_mark_dirty(x,y,x+8,y,0); */

    /* optimize the most common case--all blank */
	if (data == 0)
	{
		fast_memset(&tmpbitmap->line[y][x],Machine->pens[0],8);
		fast_memset(&Machine->scrbitmap->line[y][x],Machine->pens[0],8);
		return;
	}

    copy_byte(x, y, data, colorram[coloroffset]);
}


void berzerk_colorram_w(int offset,int data)
{
	int x, y, i;

	colorram[offset] = data;

	/* Need to change the affected pixels' colors */

	y = ((offset >> 3) & 0xfc);
	x = (offset & 0x1f) << 3;

	/* osd_mark_dirty(x,y,x+7,y+3,0); */

	for (i = 0; i < 4; i++, y++)
	{
		int dat = videoram[(y << 5) | (x >> 3)];

		if (dat)
		{
			copy_byte(x, y, dat, data);
		}
	}
}

INLINE unsigned char reverse_byte(unsigned char data)
{
	return ((data & 0x01) << 7) |
		   ((data & 0x02) << 5) |
		   ((data & 0x04) << 3) |
		   ((data & 0x08) << 1) |
		   ((data & 0x10) >> 1) |
		   ((data & 0x20) >> 3) |
		   ((data & 0x40) >> 5) |
		   ((data & 0x80) >> 7);
}

INLINE int shifter_flopper(unsigned char data)
{
	int shift_amount, outval;

	/* Bits 0-2 are the shift amount */

	shift_amount = magicram_control & 0x06;

	outval = ((data >> shift_amount) | (magicram_latch << (8 - shift_amount))) & 0x1ff;
	outval >>= (magicram_control & 0x01);


	/* Bit 3 is the flip bit */
	if (magicram_control & 0x08)
	{
		outval = reverse_byte(outval);
	}

	return outval;
}


void berzerk_magicram_w(int offset,int data)
{
	int data2;

	data2 = shifter_flopper(data);

	magicram_latch = data;

	/* Check for collision */
	if (!collision)
	{
		collision = (data2 & videoram[offset]);
	}

	switch (magicram_control & 0xf0)
	{
	case 0x00: berzerk_magicram[offset] = data2; 						break;
	case 0x10: berzerk_magicram[offset] = data2 |  videoram[offset]; 	break;
	case 0x20: berzerk_magicram[offset] = data2 | ~videoram[offset]; 	break;
	case 0x30: berzerk_magicram[offset] = 0xff;  						break;
	case 0x40: berzerk_magicram[offset] = data2 & videoram[offset]; 	break;
	case 0x50: berzerk_magicram[offset] = videoram[offset]; 			break;
	case 0x60: berzerk_magicram[offset] = ~(data2 ^ videoram[offset]); 	break;
	case 0x70: berzerk_magicram[offset] = ~data2 | videoram[offset]; 	break;
	case 0x80: berzerk_magicram[offset] = data2 & ~videoram[offset];	break;
	case 0x90: berzerk_magicram[offset] = data2 ^ videoram[offset];		break;
	case 0xa0: berzerk_magicram[offset] = ~videoram[offset];			break;
	case 0xb0: berzerk_magicram[offset] = ~(data2 & videoram[offset]); 	break;
	case 0xc0: berzerk_magicram[offset] = 0x00; 						break;
	case 0xd0: berzerk_magicram[offset] = ~data2 & videoram[offset]; 	break;
	case 0xe0: berzerk_magicram[offset] = ~(data2 | videoram[offset]); 	break;
	case 0xf0: berzerk_magicram[offset] = ~data2; 						break;
	}

	berzerk_videoram_w(offset, berzerk_magicram[offset]);
}


void berzerk_magicram_control_w(int offset,int data)
{
	magicram_control = data;
	magicram_latch = 0;
	collision = 0;
}


int berzerk_collision_r(int offset)
{
	int ret = (collision ? 0x80 : 0x00);

	return ret | berzerk_irq_end_of_screen;
}

int berzerk_bitmapped_vh_start(void)
{
        if ((tmpbitmap = osd_new_bitmap(Machine->drv->screen_width,Machine->drv->screen_height,Machine->scrbitmap->depth)) == 0)
        {
                return 1;
        }

        return 0;
}

void berzerk_bitmapped_vh_stop(void)
{
        osd_free_bitmap(tmpbitmap);

        tmpbitmap = 0;
}

void berzerk_bitmapped_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh)
{
        if (full_refresh)
                copybitmap(bitmap,tmpbitmap,0,0,0,0,&Machine->drv->visible_area,TRANSPARENCY_NONE,0);
}

