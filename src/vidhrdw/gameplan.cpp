/***************************************************************************

  vidhrdw.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/


#define VERBOSE_VIDEO
#define SHOW_CHARS

#include "driver.h"
#include "vidhrdw/generic.h"

static int gameplan_this_is_kaos;
static int gameplan_this_is_megatack;
static int clear_to_colour = 0;
static int fix_clear_to_colour = -1;
static unsigned char *gameplan_videoram;

void gameplan_clear_screen(void);


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/


int gameplan_vh_start(void)
{
	if (strcmp(Machine->gamedrv->name, "kaos") == 0)
		gameplan_this_is_kaos = 1;
	else
		gameplan_this_is_kaos = 0;

	if (strcmp(Machine->gamedrv->name, "megatack") == 0)
		gameplan_this_is_megatack = 1;
	else
		gameplan_this_is_megatack = 0;

	if ((gameplan_videoram = (unsigned char *)gp2x_malloc(256*256)) == 0)
		return 1;

	if ((tmpbitmap = osd_create_bitmap(Machine->drv->screen_width, Machine->drv->screen_height)) == 0)
	{
		gp2x_free(gameplan_videoram);
		return 1;
	}

	return 0;
}


/***************************************************************************

  Stop the video hardware emulation.

***************************************************************************/


void gameplan_vh_stop(void)
{
	osd_free_bitmap(tmpbitmap);
	gp2x_free(gameplan_videoram);
}

static int port_b;
static int new_request = 0;
static int finished_sound = 0;
static int cb2 = -1;

int gameplan_sound_r(int offset)
{
	if (offset == 0)
	{
		return finished_sound;
	}
	else
		return 0;
}

void gameplan_sound_w(int offset,int data)
{
	if (offset == 1)
	{
		if (cb2 == 0)
		{
			return;
		}

		port_b = data;
		finished_sound = 0;
		new_request = 1;

		/* shortly after requesting a sound, the game board checks
		   whether the sound board has ackknowledged receipt of the
		   command - yield now to allow it to send the ACK */
	}
	else if (offset == 0x0c)	/* PCR */
	{
		if (data & 0x80)
		{
			if ((data & 0x60) == 0x60)
				cb2 = 1;
			else if ((data & 0x60) == 0x40)
				cb2 = 0;
			else cb2 = -1;
		}
	}
}

int gameplan_via5_r(int offset)
{
	if (offset == 0)
	{
		new_request = 0;
		return port_b;
	}

	if (offset == 5)
	{
		if (new_request == 1)
		{
			return 0x40;
		}
		else
		{
			return 0;
		}
	}

	return 1;
}

void gameplan_via5_w(int offset, int data)
{
	if (offset == 2)
	{
		finished_sound = data;
	}
}

int gameplan_video_r(int offset)
{
	static int x;
	x++;
	return x;
}

void gameplan_video_w(int offset,int data)
{
	int cpu_getpc(void);
	static int r0 = -1;
	static unsigned char xpos, ypos, colour = 7;

	if (offset == 0)			/* write to 2000 */
	{
		r0 = data;
	}
	else if (offset == 1)		/* write to 2001 */
	{
		if (r0 == 0)
		{
			if (gameplan_this_is_kaos)
				colour = ~data & 0x07;

			if (data & 0x20)
			{
				if (data & 0x80) xpos--;
				else xpos++;
			}
			if (data & 0x10)
			{
				if (data & 0x40) ypos--;
				else ypos++;
			}

			if (gameplan_videoram[xpos+256*ypos] != colour)
			{
				gameplan_videoram[xpos+256*ypos] = colour;

				if (gameplan_this_is_kaos)
				{
					tmpbitmap->line[256 - ypos][xpos] = Machine->pens[colour];
				}
				else
				{
					tmpbitmap->line[xpos][ypos] = Machine->pens[colour];
				}
			}
		}
		else if (r0 == 1)
		{
			ypos = data;
		}
		else if (r0 == 2)
		{
			xpos = data;
		}
		else if (r0 == 3)
		{
			if (offset == 1 && data == 0)
			{
				gameplan_clear_screen();
			}
		}
	}
	else if (offset == 2)
	{
		if (data == 7)
		{
			/* This whole 'fix_clear_to_colour' and special casing for
			 * megatack thing is ugly, and doesn't even work properly.
			 */

			if (!gameplan_this_is_megatack || fix_clear_to_colour == -1)
				clear_to_colour = colour;
		}
	}
	else if (offset == 3)
	{
		if (r0 == 0)
		{

			colour = data & 7;
		}
		else if ((data & 0xf8) == 0xf8 && data != 0xff)
		{
			clear_to_colour = fix_clear_to_colour = data & 0x07;
		}
	}
}


/***************************************************************************

  Draw the game screen in the given osd_bitmap.
  Do NOT call osd_update_display() from this function, it will be called by
  the main emulation engine.

***************************************************************************/


void gameplan_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh)
{
	/* copy the character mapped graphics */
	copybitmap(bitmap, tmpbitmap, 0, 0, 0, 0,
			   &Machine->drv->visible_area,
			   TRANSPARENCY_NONE, 0);
}


void gameplan_clear_screen(void)
{
	int x, y;

	fillbitmap(tmpbitmap, Machine->pens[clear_to_colour], 0);

	for (x = 0; x < 256; x++)
		for (y = 0; y < 256; y++)
			gameplan_videoram[x+256*y] = clear_to_colour;

	fix_clear_to_colour = -1;
}
