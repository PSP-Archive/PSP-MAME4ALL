/***************************************************************************

  vidhrdw.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"

static int char_palette,sprite_palette;
static int char_bank;

static UINT8 *background_latches;
static UINT16 *background_gfx[4];
static UINT8 current_latches[16];
static int last_scanline_update;

static UINT8 *background_mixer;

#define BACKGROUND_X_START      32
#define BACKGROUND_X_START_FLIP 72
        
#define VISIBLE_X_MIN                   (12*8)
#define VISIBLE_X_MAX                   (52*8)
#define VISIBLE_Y_MIN                   (2*8)
#define VISIBLE_Y_MAX                   (30*8)



/***************************************************************************

  Convert the color PROMs into a more useable format.

  The palette PROM is connected to the RGB output this way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/
void exerion_vh_convert_color_prom(unsigned char *palette, unsigned short *colortable,const unsigned char *color_prom)
{
	int i;
	#define TOTAL_COLORS(gfxn) (Machine->gfx[gfxn]->total_colors * Machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn,offs) (colortable[Machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])


	for (i = 0;i < Machine->drv->total_colors;i++)
	{
		int bit0,bit1,bit2;


		/* red component */
		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;
		*(palette++) = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;
		*(palette++) = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;
		*(palette++) = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		color_prom++;
	}

        /* color_prom now points to the beginning of the char lookup table */

        /* fg chars */
        for (i = 0; i < 256; i++)
                colortable[i + 0x000] = 16 + (color_prom[(i & 0xc0) | ((i & 3) << 4) | ((i >> 2) & 15)] & 15);
        color_prom += 256;

        /* color_prom now points to the beginning of the sprite lookup table */

        /* sprites */
        for (i = 0; i < 256; i++)
                colortable[i + 0x100] = 16 + (color_prom[(i & 0xc0) | ((i & 3) << 4) | ((i >> 2) & 15)] & 15);
        color_prom += 256;

        /* bg chars (this is not the full story... there are four layers mixed */
        /* using another PROM */
        for (i = 0; i < 256; i++)
                colortable[i + 0x200] = *color_prom++ & 15;
}



int exerion_vh_start (void)
{
        UINT16 *dst;
        UINT8 *src;
        int i, x, y;

        /* get pointers to the mixing and lookup PROMs */
        background_mixer = Machine->memory_region[4]+0x320;

        /* allocate memory to track the background latches */
        background_latches = (UINT8 *)gp2x_malloc(Machine->drv->screen_height * 16);
        if (!background_latches)
                return 1;

        /* allocate memory for the decoded background graphics */
        background_gfx[0] = (UINT16 *)gp2x_malloc(2 * 256 * 256 * 4);
        background_gfx[1] = background_gfx[0] + 256 * 256;
        background_gfx[2] = background_gfx[1] + 256 * 256;
        background_gfx[3] = background_gfx[2] + 256 * 256;
        if (!background_gfx[0])
        {
                gp2x_free(background_latches);
                background_latches = NULL;
                return 1;
        }

        /*---------------------------------
         * Decode the background graphics
         *
         * We decode the 4 background layers separately, but shuffle the bits so that
         * we can OR all four layers together. Each layer has 2 bits per pixel. Each
         * layer is decoded into the following bit patterns:
         *
         *      000a 0000 00AA
         *  00b0 0000 BB00
         *  0c00 00CC 0000
         *  d000 DD00 0000
         *
         * Where AA,BB,CC,DD are the 2bpp data for the pixel,and a,b,c,d are the OR
         * of these two bits together.
         */
        for (i = 0; i < 4; i++)
        {
                src = Machine->memory_region[3] + (i * 0x2000);
                dst = background_gfx[i];

                for (y = 0; y < 256; y++)
                {
                        for (x = 0; x < 128; x += 4)
                        {
                                UINT8 data = *src++;
                                UINT16 val;

                                val = ((data >> 3) & 2) | ((data >> 0) & 1);
                                if (val) val |= 0x100 >> i;
                                *dst++ = val << (2 * i);

                                val = ((data >> 4) & 2) | ((data >> 1) & 1);
                                if (val) val |= 0x100 >> i;
                                *dst++ = val << (2 * i);

                                val = ((data >> 5) & 2) | ((data >> 2) & 1);
                                if (val) val |= 0x100 >> i;
                                *dst++ = val << (2 * i);

                                val = ((data >> 6) & 2) | ((data >> 3) & 1);
                                if (val) val |= 0x100 >> i;
                                *dst++ = val << (2 * i);
                        }
                        for (x = 0; x < 128; x++)
                                *dst++ = 0;
                }
        }
 
	return generic_vh_start();
}

void exerion_vh_stop (void)
{

        /* free the background graphics data */
        if (background_gfx[0])
                gp2x_free(background_gfx[0]);
        background_gfx[0] = NULL;
        background_gfx[1] = NULL;
        background_gfx[2] = NULL;
        background_gfx[3] = NULL;

        /* free the background latches data */
        if (background_latches)
                gp2x_free(background_latches);
        background_latches = NULL;

	generic_vh_stop();
}


void exerion_videoreg_w (int offset,int data)
{
	/* bit 0 = flip screen and joystick input multiplexor */

	/* bits 1-2 char lookup table bank */
	char_palette = (data & 0x06) >> 1;

	/* bits 3 char bank */
	char_bank = (data & 0x08) >> 3;

	/* bits 4-5 unused */

	/* bits 6-7 sprite lookup table bank */
	sprite_palette = (data & 0xc0) >> 6;
}

void exerion_video_latch_w (int offset,int data)
{
        int ybeam = cpu_getscanline();

        if (ybeam >= Machine->drv->screen_height)
                ybeam = Machine->drv->screen_height - 1;

        /* copy data up to and including the current scanline */
        while (ybeam != last_scanline_update)
        {
                last_scanline_update = (last_scanline_update + 1) % Machine->drv->screen_height;
                fast_memcpy(&background_latches[last_scanline_update * 16], current_latches, 16);
        }

        /* modify data on the current scanline */
        if (offset != -1)
                current_latches[offset] = data;
}

int exerion_video_timing_r(int offset)
{
        /* bit 1 is VBLANK */
        /* bit 0 is the SNMI signal, which is low for H >= 0x1c0 and /VBLANK */

        int xbeam = cpu_gethorzbeampos();
        int ybeam = cpu_getscanline();
        UINT8 result = 0;

        if (ybeam >= VISIBLE_Y_MAX)
                result |= 2;
        if (xbeam < 0x1c0 && ybeam < VISIBLE_Y_MAX)
                result |= 1;

        return result;
}

#define ADJUST_FOR_ORIENTATION(orientation, bitmap, dst, x, y, xadv) \
        if (orientation) \
        { \
                int dy = bitmap->line[1] - bitmap->line[0]; \
                int tx = x, ty = y, temp; \
                if (orientation & ORIENTATION_SWAP_XY) \
                { \
                        temp = tx; tx = ty; ty = temp; \
                        xadv = dy / (bitmap->depth / 8); \
                } \
                if (orientation & ORIENTATION_FLIP_X) \
                { \
                        tx = bitmap->width - 1 - tx; \
                        if (!(orientation & ORIENTATION_SWAP_XY)) xadv = -xadv; \
                } \
                if (orientation & ORIENTATION_FLIP_Y) \
                { \
                        ty = bitmap->height - 1 - ty; \
                        if ((orientation & ORIENTATION_SWAP_XY)) xadv = -xadv; \
                } \
                /* can't lookup line because it may be negative! */ \
                dst = (UINT8 *)(bitmap->line[0] + dy * ty) + tx; \
        }


void draw_background(struct osd_bitmap *bitmap)
{
        UINT8 *latches = &background_latches[VISIBLE_Y_MIN * 16];
        int orientation = Machine->orientation;
        int x, y;

        /* loop over all visible scanlines */
        for (y = VISIBLE_Y_MIN; y < VISIBLE_Y_MAX; y++, latches += 16)
        {
                UINT16 *src0 = &background_gfx[0][latches[1] << 8];
                UINT16 *src1 = &background_gfx[1][latches[3] << 8];
                UINT16 *src2 = &background_gfx[2][latches[5] << 8];
                UINT16 *src3 = &background_gfx[3][latches[7] << 8];
                int xoffs0 = latches[0];
                int xoffs1 = latches[2];
                int xoffs2 = latches[4];
                int xoffs3 = latches[6];
                int start0 = latches[8] & 0x0f;
                int start1 = latches[9] & 0x0f;
                int start2 = latches[10] & 0x0f;
                int start3 = latches[11] & 0x0f;
                int stop0 = latches[8] >> 4;
                int stop1 = latches[9] >> 4;
                int stop2 = latches[10] >> 4;
                int stop3 = latches[11] >> 4;
                UINT16 *pens = &Machine->colortable[0x200 + (latches[12] >> 4) * 16];
                UINT8 *mixer = &background_mixer[(latches[12] << 4) & 0xf0];
                UINT8 *dst = &((UINT8 *)bitmap->line[y])[VISIBLE_X_MIN];
                int xadv = 1;

                /* adjust in case we're oddly oriented */
                ADJUST_FOR_ORIENTATION(orientation, bitmap, dst, VISIBLE_X_MIN, y, xadv);

		/* skip processing anything that's not visible */
		for (x = BACKGROUND_X_START; x < VISIBLE_X_MIN; x++)
		{
			if (!(++xoffs0 & 0x1f)) start0++, stop0++;
			if (!(++xoffs1 & 0x1f)) start1++, stop1++;
			if (!(++xoffs2 & 0x1f)) start2++, stop2++;
			if (!(++xoffs3 & 0x1f)) start3++, stop3++;
		}

		/* draw the rest of the scanline fully */
		for (x = VISIBLE_X_MIN; x < VISIBLE_X_MAX; x++, dst += xadv)
		{
			UINT16 combined = 0;
			UINT8 lookupval, colorindex;

			/* the output enable is controlled by the carries on the start/stop counters */
			/* they are only active when the start has carried but the stop hasn't */
			if ((start0 ^ stop0) & 0x10) combined |= src0[xoffs0 & 0xff];
			if ((start1 ^ stop1) & 0x10) combined |= src1[xoffs1 & 0xff];
			if ((start2 ^ stop2) & 0x10) combined |= src2[xoffs2 & 0xff];
			if ((start3 ^ stop3) & 0x10) combined |= src3[xoffs3 & 0xff];

			/* bits 8-11 of the combined value contains the lookup for the mixer PROM */
			lookupval = mixer[combined >> 8] & 3;

			/* the color index comes from the looked up value combined with the pixel data */
			colorindex = (lookupval << 2) | ((combined >> (2 * lookupval)) & 3);
			*dst = pens[colorindex];

			/* the start/stop counters are clocked when the low 5 bits of the X counter overflow */
			if (!(++xoffs0 & 0x1f)) start0++, stop0++;
			if (!(++xoffs1 & 0x1f)) start1++, stop1++;
			if (!(++xoffs2 & 0x1f)) start2++, stop2++;
			if (!(++xoffs3 & 0x1f)) start3++, stop3++;
		}
        }
}

/***************************************************************************

  Draw the game screen in the given osd_bitmap.
  Do NOT call osd_update_display() from this function, it will be called by
  the main emulation engine.

***************************************************************************/
void exerion_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh)
{
	int sx,sy,offs,i;

        /* finish updating the scanlines */
        exerion_video_latch_w(-1, 0);
                
        /* draw background */
	draw_background(bitmap);

        /* draw sprites */
        for (i = 0; i < spriteram_size; i += 4)
        {
                int flags = spriteram[i + 0];
                int y = spriteram[i + 1] ^ 255;
                int code = spriteram[i + 2];
                int x = spriteram[i + 3] * 2 + 72;

                int xflip = flags & 0x80;
                int yflip = flags & 0x40;
                int doubled = flags & 0x10;
                int wide = flags & 0x08;
                int code2 = code;

                int color = ((flags >> 1) & 0x03) | ((code >> 5) & 0x04) | (code & 0x08) | (sprite_palette * 16);
                const struct GfxElement *gfx = doubled ? Machine->gfx[2] : Machine->gfx[1];

                if (wide)
                {
                        if (yflip)
                                code |= 0x10, code2 &= ~0x10;
                        else
                                code &= ~0x10, code2 |= 0x10;

                        drawgfx(bitmap, gfx, code2, color, xflip, yflip, x, y + gfx->height,
                                0, TRANSPARENCY_COLOR, 16);
                }
                drawgfx(bitmap, gfx, code, color, xflip, yflip, x, y,
                        0, TRANSPARENCY_COLOR, 16);

                if (doubled) i += 4;
        }

        /* draw the visible text layer */
        for (sy = VISIBLE_Y_MIN>>3; sy < VISIBLE_Y_MAX>>3; sy++)
                for (sx = VISIBLE_X_MIN>>3; sx < VISIBLE_X_MAX>>3; sx++)
                {
                        offs = sx + (sy << 6);
			if (videoram[offs])
			{
				drawgfx(bitmap, Machine->gfx[0],
					videoram[offs] + (char_bank<<8),
					((videoram[offs] & 0xf0) >> 4) + (char_palette<<4),
					0, 0, sx<<3, sy<<3,
					0, TRANSPARENCY_PEN, 0);
			}
                }

}
