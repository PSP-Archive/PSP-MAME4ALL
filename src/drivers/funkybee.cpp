/***************************************************************************

Funky Bee/Sky Lancer memory map (preliminary)

MAIN CPU:

0000-4fff ROM
8000-87ff RAM
a000-bfff video RAM (only 0x20 bytes of each 0x100 byte block is used)
                    (also contains sprite RAM)
c000-dfff color RAM (only 0x20 bytes of each 0x100 byte block is used)
                    (also contains sprite RAM)

read:
f000	  interrupt ACK
f800	  IN0/watchdog
f801      IN1
f802	  IN2

write:
e000      row scroll
e800	  ???
e802-e803 coin counter
e804	  ???
e805	  gfx bank select
e806	  ???
f800	  watchdog


I/0 ports:
write
00        8910  control
01        8910  write

AY8910 Port A = DSW

***************************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"



extern unsigned char *funkyb_row_scroll;

void funkybee_vh_convert_color_prom(unsigned char *palette, unsigned short *colortable,const unsigned char *color_prom);
void funkybee_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh);

void funkybee_gfx_bank_w(int offset, int data);


static int funkybee_input_port_0_r(int offset)
{
	watchdog_reset_r(0);
	return input_port_0_r(offset);
}


static struct MemoryReadAddress readmem[] =
{
	{ 0x0000, 0x4fff, MRA_ROM },
	{ 0x8000, 0x87ff, MRA_RAM },
	{ 0xa000, 0xdfff, MRA_RAM },
	{ 0xf000, 0xf000, MRA_NOP },	/* IRQ Ack */
	{ 0xf800, 0xf800, funkybee_input_port_0_r },
	{ 0xf801, 0xf801, input_port_1_r },
	{ 0xf802, 0xf802, input_port_2_r },
	{ -1 }	/* end of table */
};

static struct MemoryWriteAddress writemem[] =
{
	{ 0x0000, 0x4fff, MWA_ROM },
	{ 0x8000, 0x87ff, MWA_RAM },
	{ 0xa000, 0xbfff, videoram_w, &videoram, &videoram_size },
	{ 0xc000, 0xdfff, colorram_w, &colorram },
	{ 0xe000, 0xe000, MWA_RAM, &funkyb_row_scroll },
	{ 0xe802, 0xe803, coin_counter_w },
	{ 0xe805, 0xe805, funkybee_gfx_bank_w },
	{ 0xf800, 0xf800, watchdog_reset_w },
	{ -1 }	/* end of table */
};


static struct IOReadPort readport[] =
{
	{ 0x02, 0x02, AY8910_read_port_0_r },
	{ -1 }	/* end of table */
};

static struct IOWritePort writeport[] =
{
	{ 0x00, 0x00, AY8910_control_port_0_w },
	{ 0x01, 0x01, AY8910_write_port_0_w },
	{ -1 }	/* end of table */
};


INPUT_PORTS_START( funkybee_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_DIPNAME( 0x20, 0x20, "Freeze" ,IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  | IPF_8WAY )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    | IPF_8WAY )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  | IPF_8WAY )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START		/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START      /* DSW */
	PORT_DIPNAME( 0x03, 0x03, "Coin A" ,IP_KEY_NONE )
	PORT_DIPSETTING(    0x03, "1 Cn/1 Crd" )
	PORT_DIPSETTING(    0x02, "1 Cn/2 Crd" )
	PORT_DIPSETTING(    0x01, "1 Cn/3 Crd" )
	PORT_DIPSETTING(    0x00, "1 Cn/4 Crd" )
	PORT_DIPNAME( 0x0c, 0x0c, "Coin B" ,IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "2 Cn/1 Crd" )
	PORT_DIPSETTING(    0x0c, "1 Cn/1 Crd" )
	PORT_DIPSETTING(    0x04, "2 Cn/3 Crd" )
	PORT_DIPSETTING(    0x00, "1 Cn/6 Crd" )
	PORT_DIPNAME( 0x30, 0x30, "Lives" ,IP_KEY_NONE )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x40, 0x00, "Unknown" ,IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x40, "On" )
	PORT_DIPNAME( 0x80, 0x00, "Cabinet" ,IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Upright" )
	PORT_DIPSETTING(    0x80, "Cocktail" )
INPUT_PORTS_END

INPUT_PORTS_START( skylancr_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_DIPNAME( 0x20, 0x20, "Freeze" ,IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  | IPF_8WAY )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    | IPF_8WAY )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  | IPF_8WAY )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START		/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START      /* DSW */
	PORT_DIPNAME( 0x03, 0x03, "Coin A" ,IP_KEY_NONE )
	PORT_DIPSETTING(    0x03, "1 Cn/1 Crd" )
	PORT_DIPSETTING(    0x02, "1 Cn/2 Crd" )
	PORT_DIPSETTING(    0x01, "1 Cn/3 Crd" )
	PORT_DIPSETTING(    0x00, "1 Cn/6 Crd" )
	PORT_DIPNAME( 0x0c, 0x0c, "Coin B" ,IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "2 Cn/1 Crd" )
	PORT_DIPSETTING(    0x0c, "1 Cn/1 Crd" )
	PORT_DIPSETTING(    0x04, "2 Cn/3 Crd" )
	PORT_DIPSETTING(    0x00, "1 Cn/6 Crd" )
	PORT_DIPNAME( 0x30, 0x30, "Lives" ,IP_KEY_NONE )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x40, 0x40, "Bonus Life" ,IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "20000 50000" )
	PORT_DIPSETTING(    0x00, "40000 70000" )
	PORT_DIPNAME( 0x80, 0x00, "Cabinet" ,IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Upright" )
	PORT_DIPSETTING(    0x80, "Cocktail" )
INPUT_PORTS_END


static struct GfxLayout charlayout =
{
	8,8,	/* 8*8 characters */
	256,	/* 256 characters */
	2,	    /* 2 bits per pixel */
	{ 0, 4 },	/* the two bitplanes for 4 pixels are packed into one byte */
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8	/* every char takes 16 bytes */
};

static struct GfxLayout spritelayout =
{
	8,32,	/* 8*32 sprites */
	128,		/* 128 sprites */
	2,	    /* 2 bits per pixel */
	{ 0, 4 },	/* the two bitplanes for 4 pixels are packed into one byte */
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{  0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
	  16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8,
	  32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8,
	  48*8, 49*8, 50*8, 51*8, 52*8, 53*8, 54*8, 55*8 },
	4*16*8	/* every sprite takes 64 bytes */
};

static struct GfxDecodeInfo gfxdecodeinfo[] =
{
	{ 1, 0x0000, &charlayout,	0,  8 },
	{ 1, 0x2000, &charlayout,	0,  8 },
	{ 1, 0x0000, &spritelayout,	16, 4 },
	{ 1, 0x2000, &spritelayout,	16, 4 },
	{ -1 } /* end of array */
};


static struct AY8910interface ay8910_interface =
{
	1,	/* 1 chip */
	1500000,	/* 1.5 MHZ?????? */
	{ 100 },
	{ input_port_3_r },
	{ 0 },
	{ 0 },
	{ 0 }
};


static struct MachineDriver machine_driver =
{
	/* basic machine hardware */
	{
		{
			CPU_Z80,
			3072000,	/* 3.072 Mhz */
			0,
			readmem,writemem,readport,writeport,
			interrupt,1
		}
	},
	60, DEFAULT_REAL_60HZ_VBLANK_DURATION,	/* frames per second, vblank duration */
	1,	/* single CPU game */
	0,

	/* video hardware */
	32*8, 32*8, { 12, 32*8-1-12, 0*8, 28*8-1 },
	gfxdecodeinfo,
	32,32,
	funkybee_vh_convert_color_prom,

	VIDEO_TYPE_RASTER,
	0,
	generic_vh_start,
	generic_vh_stop,
	funkybee_vh_screenrefresh,

	/* sound hardware */
	0,0,0,0,
	{
		{
			SOUND_AY8910,
			&ay8910_interface
		}
	}
};


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( funkybee_rom )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "funkybee.1",    0x0000, 0x1000, 0x3372cb33 )
	ROM_LOAD( "funkybee.3",    0x1000, 0x1000, 0x7bf7c62f )
	ROM_LOAD( "funkybee.2",    0x2000, 0x1000, 0x8cc0fe8e )
	ROM_LOAD( "funkybee.4",    0x3000, 0x1000, 0x1e1aac26 )

	ROM_REGION_DISPOSE(0x6000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "funkybee.5",    0x0000, 0x2000, 0x86126655 )
	ROM_LOAD( "funkybee.6",    0x2000, 0x2000, 0x5fffd323 )

	ROM_REGION(0x0020)  /* color prom */
	ROM_LOAD( "funkybee.clr",  0x0000, 0x0020, 0xe2cf5fe2 )
ROM_END

ROM_START( skylancr_rom )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "1.5a",          0x0000, 0x2000, 0x82d55824 )
	ROM_LOAD( "2.5c",          0x2000, 0x2000, 0xdff3a682 )
	ROM_LOAD( "3.5d",          0x4000, 0x1000, 0x7c006ee6 )

	ROM_REGION_DISPOSE(0x6000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "4.6a",          0x0000, 0x2000, 0x0f8ede07 )
	ROM_LOAD( "5.6b",          0x2000, 0x2000, 0x24cec070 )

	ROM_REGION(0x0020)  /* color prom */
	ROM_LOAD( "18s030.1a",     0x0000, 0x0020, 0xe645bacb )
ROM_END


struct GameDriver funkybee_driver =
{
	__FILE__,
	0,
	"funkybee",
	"Funky Bee",
	"1982",
	"Orca Corporation",
	"Zsolt Vasvari",
	0,
	&machine_driver,
	0,

	funkybee_rom,
	0, 0,
	0,
	0,	/* sound_prom */

	funkybee_input_ports,

	PROM_MEMORY_REGION(2), 0, 0,
	ORIENTATION_ROTATE_90,

	0, 0
};

struct GameDriver skylancr_driver =
{
	__FILE__,
	0,
	"skylancr",
	"Sky Lancer",
	"1983",
	"Orca (Esco Trading Co license)",
	"Zsolt Vasvari",
	0,
	&machine_driver,
	0,

	skylancr_rom,
	0, 0,
	0,
	0,	/* sound_prom */

	skylancr_input_ports,

	PROM_MEMORY_REGION(2), 0, 0,
	ORIENTATION_ROTATE_90,

	0, 0
};


