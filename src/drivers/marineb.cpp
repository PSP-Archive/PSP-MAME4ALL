/***************************************************************************

Marine Boy hardware memory map (preliminary)

driver by Zsolt Vasvari

MAIN CPU:

0000-7fff ROM (not all games use the entire region)
8000-87ff RAM
8c18-8c3f sprite RAM (Hoccer only)
8800-8bff video RAM	 \ (contains sprite RAM in invisible regions)
9000-93ff color RAM	 /

read:
a000	  IN0
a800	  IN1
b000	  DSW
b800      IN2/watchdog reset

write:
9800      column scroll
9a00      char palette bank bit 0 (not used by Hoccer)
9c00      char palette bank bit 1 (not used by Hoccer)
a000      NMI interrupt acknowledge/enable
a001      flipy
a002      flipx
b800	  ???


I/0 ports:
write
08        8910  control
09        8910  write

***************************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"



extern unsigned char *marineb_column_scroll;
extern int marineb_active_low_flipscreen;

void espial_vh_convert_color_prom(unsigned char *palette, unsigned short *colortable,const unsigned char *color_prom);

void marineb_palbank0_w(int offset, int data );
void marineb_palbank1_w(int offset, int data );

void marineb_flipscreen_x_w(int offset, int data );
void marineb_flipscreen_y_w(int offset, int data );

void marineb_vh_screenrefresh (struct osd_bitmap *bitmap,int full_refresh);
void wanted_vh_screenrefresh (struct osd_bitmap *bitmap,int full_refresh);


static void marineb_init_machine(void)
{
	marineb_active_low_flipscreen = 0;
	/*espial_init_machine();*/
	interrupt_enable_w(0, 0);
}


static struct MemoryReadAddress readmem[] =
{
	{ 0x0000, 0x7fff, MRA_ROM },
	{ 0x8000, 0x8bff, MRA_RAM },
	{ 0x9000, 0x93ff, MRA_RAM },
	{ 0xa000, 0xa000, input_port_0_r },
	{ 0xa800, 0xa800, input_port_1_r },
	{ 0xb000, 0xb000, input_port_2_r },
	{ 0xb800, 0xb800, input_port_3_r },  /* also watchdog */
	{ -1 }	/* end of table */
};

static struct MemoryWriteAddress writemem[] =
{
	{ 0x0000, 0x7fff, MWA_ROM },
	{ 0x8000, 0x87ff, MWA_RAM },
	{ 0x8800, 0x8bff, videoram_w, &videoram, &videoram_size },
	{ 0x8c00, 0x8c3f, MWA_RAM, &spriteram },  /* Hoccer only */
	{ 0x9000, 0x93ff, colorram_w, &colorram },
	{ 0x9800, 0x9800, MWA_RAM, &marineb_column_scroll },
	{ 0x9a00, 0x9a00, marineb_palbank0_w },
	{ 0x9c00, 0x9c00, marineb_palbank1_w },
	{ 0xa000, 0xa000, interrupt_enable_w },
	{ 0xa001, 0xa001, marineb_flipscreen_y_w },
	{ 0xa002, 0xa002, marineb_flipscreen_x_w },
	{ 0xb800, 0xb800, MWA_NOP },
	{ -1 }	/* end of table */
};


static struct IOWritePort marineb_writeport[] =
{
	{ 0x08, 0x08, AY8910_control_port_0_w },
	{ 0x09, 0x09, AY8910_write_port_0_w },
	{ -1 }	/* end of table */
};

static struct IOWritePort wanted_writeport[] =
{
	{ 0x00, 0x00, AY8910_control_port_0_w },
	{ 0x01, 0x01, AY8910_write_port_0_w },
	{ 0x02, 0x02, AY8910_control_port_1_w },
	{ 0x03, 0x03, AY8910_write_port_1_w },
	{ -1 }  /* end of table */
};


INPUT_PORTS_START( marineb_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x3c, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )

	PORT_START      /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    | IPF_8WAY )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  | IPF_8WAY )
	PORT_BIT( 0x3c, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  | IPF_8WAY )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT | IPF_8WAY )

	PORT_START      /* DSW1 */
	PORT_DIPNAME( 0x03, 0x00, "Lives", IP_KEY_NONE)
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x1c, 0x00, "Coinage", IP_KEY_NONE)
	PORT_DIPSETTING(    0x00, "1C_1C")
	PORT_DIPSETTING(    0x1c, "Free_Play")
	PORT_DIPNAME( 0x20, 0x00, "Bonus_Life", IP_KEY_NONE)
	PORT_DIPSETTING(    0x00, "20000 50000" )
	PORT_DIPSETTING(    0x20, "40000 70000" )
	PORT_DIPNAME( 0x40, 0x40, "Cabinet", IP_KEY_NONE)
	PORT_DIPSETTING(    0x40, "Upright")
	PORT_DIPSETTING(    0x00, "Cocktail")
	PORT_DIPNAME( 0x80, 0x00, "Unknown", IP_KEY_NONE)
	PORT_DIPSETTING(    0x00, "Off")
	PORT_DIPSETTING(    0x80, "On")

	PORT_START	/* IN3 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 | IPF_PLAYER2)
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

INPUT_PORTS_START( wanted_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )

	PORT_START      /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  | IPF_8WAY )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  | IPF_8WAY )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    | IPF_8WAY )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_COIN1 )

	PORT_START      /* DSW1 */
	PORT_DIPNAME( 0x03, 0x00, "Lives", IP_KEY_NONE)
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x04, 0x00, "Unknown", IP_KEY_NONE)
	PORT_DIPSETTING(    0x00, "Off")
	PORT_DIPSETTING(    0x04, "On")
	PORT_DIPNAME( 0x08, 0x00, "Unknown", IP_KEY_NONE)
	PORT_DIPSETTING(    0x00, "Off")
	PORT_DIPSETTING(    0x08, "On")
	PORT_DIPNAME( 0x10, 0x10, "Flip_Screen", IP_KEY_NONE)
	PORT_DIPSETTING(    0x10, "Off")
	PORT_DIPSETTING(    0x00, "On")
	PORT_DIPNAME( 0x20, 0x20, "Cabinet", IP_KEY_NONE)
	PORT_DIPSETTING(    0x20, "Upright")
	PORT_DIPSETTING(    0x00, "Cocktail")
	PORT_DIPNAME( 0x40, 0x00, "Unknown", IP_KEY_NONE)
	PORT_DIPSETTING(    0x00, "Off")
	PORT_DIPSETTING(    0x40, "On")
	PORT_DIPNAME( 0x80, 0x00, "Unknown", IP_KEY_NONE)
	PORT_DIPSETTING(    0x00, "Off")
	PORT_DIPSETTING(    0x80, "On")

	PORT_START      /* DSW2 */
	PORT_DIPNAME( 0x01, 0x00, "Unknown", IP_KEY_NONE)
	PORT_DIPSETTING(    0x00, "Off")
	PORT_DIPSETTING(    0x01, "On")
	PORT_DIPNAME( 0x02, 0x00, "Unknown", IP_KEY_NONE)
	PORT_DIPSETTING(    0x00, "Off")
	PORT_DIPSETTING(    0x02, "On")
	PORT_DIPNAME( 0x04, 0x00, "Unknown", IP_KEY_NONE)
	PORT_DIPSETTING(    0x00, "Off")
	PORT_DIPSETTING(    0x04, "On")
	PORT_DIPNAME( 0x08, 0x00, "Unknown", IP_KEY_NONE)
	PORT_DIPSETTING(    0x00, "Off")
	PORT_DIPSETTING(    0x08, "On")
	PORT_DIPNAME( 0xf0, 0x10, "Coinage", IP_KEY_NONE)
	PORT_DIPSETTING(    0x40, " A 3C/1C  B 3C/1C" )
	PORT_DIPSETTING(    0xe0, " A 3C/1C  B 1C/2C" )
	PORT_DIPSETTING(    0xf0, " A 3C/1C  B 1C/4C" )
	PORT_DIPSETTING(    0x20, " A 2C/1C  B 2C/1C" )
	PORT_DIPSETTING(    0xd0, " A 2C/1C  B 1C/1C" )
	PORT_DIPSETTING(    0x70, " A 2C/1C  B 1C/3C" )
	PORT_DIPSETTING(    0xb0, " A 2C/1C  B 1C/5C" )
	PORT_DIPSETTING(    0xc0, " A 2C/1C  B 1C/6C" )
	PORT_DIPSETTING(    0x60, " A 1C/1C  B 4C/1C" )
	PORT_DIPSETTING(    0x50, " A 1C/1C  B 2C/1C" )
	PORT_DIPSETTING(    0x10, " A 1C/1C  B 1C/1C" )
	PORT_DIPSETTING(    0x30, " A 1C/2C  B 1C/2C" )
	PORT_DIPSETTING(    0xa0, " A 1C/1C  B 1C/3C" )
	PORT_DIPSETTING(    0x80, " A 1C/1C  B 1C/5C" )
	PORT_DIPSETTING(    0x90, " A 1C/1C  B 1C/6C" )
	PORT_DIPSETTING(    0x00, "Free_Play")
INPUT_PORTS_END


static struct GfxLayout marineb_charlayout =
{
	8,8,	/* 8*8 characters */
	512,	/* 512 characters */
	2,	    /* 2 bits per pixel */
	{ 0, 4 },	/* the two bitplanes for 4 pixels are packed into one byte */
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },	/* bits are packed in groups of four */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8	/* every char takes 16 bytes */
};

static struct GfxLayout wanted_charlayout =
{
	8,8,	/* 8*8 characters */
	1024,	/* 1024 characters */
	2,	    /* 2 bits per pixel */
	{ 4, 0 },	/* the two bitplanes for 4 pixels are packed into one byte */
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },	/* bits are packed in groups of four */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8	/* every char takes 16 bytes */
};

static struct GfxLayout marineb_small_spritelayout =
{
	16,16,	/* 16*16 sprites */
	64,	    /* 64 sprites */
	2,	    /* 2 bits per pixel */
	{ 0, 256*32*8 },	/* the two bitplanes are separated */
	{     0,     1,     2,     3,     4,     5,     6,     7,
	  8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{  0*8,  1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8,
	  16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8	/* every sprite takes 32 consecutive bytes */
};

static struct GfxLayout marineb_big_spritelayout =
{
	32,32,	/* 32*32 sprites */
	64,	    /* 64 sprites */
	2,	    /* 2 bits per pixel */
	{ 0, 256*32*8 },	/* the two bitplanes are separated */
	{      0,      1,      2,      3,      4,      5,      6,      7,
	   8*8+0,  8*8+1,  8*8+2,  8*8+3,  8*8+4,  8*8+5,  8*8+6,  8*8+7,
	  32*8+0, 32*8+1, 32*8+2, 32*8+3, 32*8+4, 32*8+5, 32*8+6, 32*8+7,
	  40*8+0, 40*8+1, 40*8+2, 40*8+3, 40*8+4, 40*8+5, 40*8+6, 40*8+7 },
	{  0*8,  1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8,
	  16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8,
	  64*8, 65*8, 66*8, 67*8, 68*8, 69*8, 70*8, 71*8,
	  80*8, 81*8, 82*8, 83*8, 84*8, 85*8, 86*8, 87*8 },
	4*32*8	/* every sprite takes 128 consecutive bytes */
};

static struct GfxDecodeInfo marineb_gfxdecodeinfo[] =
{
	{ 1, 0x0000, &marineb_charlayout,          0, 64 },
	{ 2, 0x0000, &marineb_small_spritelayout,  0, 64 },
	{ 2, 0x0000, &marineb_big_spritelayout,    0, 64 },
	{ -1 } /* end of array */
};

static struct GfxDecodeInfo wanted_gfxdecodeinfo[] =
{
	{ 1, 0x0000, &wanted_charlayout,           0, 64 },
	{ 2, 0x0000, &marineb_small_spritelayout,  0, 64 },
	{ 2, 0x0000, &marineb_big_spritelayout,    0, 64 },
	{ -1 } /* end of array */
};


static struct AY8910interface marineb_ay8910_interface =
{
	1,	/* 1 chip */
	1500000,	/* 1.5 MHz ? */
	{ 50 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 }
};

static struct AY8910interface wanted_ay8910_interface =
{
	2,	/* 2 chips */
	1500000,	/* 1.5 MHz ? */
	{ 25, 25 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 }
};

#define DRIVER(NAME, INITMACHINE, SNDHRDW, INTERRUPT, CONVERT)		\
static struct MachineDriver NAME##_machine_driver =					\
{																	\
	/* basic machine hardware */									\
	{																\
		{															\
			CPU_Z80,												\
			3072000,	/* 3.072 Mhz */								\
			0, \
			readmem,writemem,0,SNDHRDW##_writeport,					\
			INTERRUPT,1	 	                                        \
		}															\
	},																\
	60, 5000,	/* frames per second, vblank duration */			\
	1,	/* single CPU game */										\
	INITMACHINE##_init_machine,										\
																	\
	/* video hardware */											\
	32*8, 32*8, { 0*8, 32*8-1, 2*8, 30*8-1 },						\
	NAME##_gfxdecodeinfo,											\
	256,256,														\
	CONVERT,									\
	VIDEO_TYPE_RASTER,												\
	0,																\
	generic_vh_start,												\
	generic_vh_stop,												\
	NAME##_vh_screenrefresh,										\
																	\
	/* sound hardware */											\
	0,0,0,0,														\
	{																\
		{															\
			SOUND_AY8910,											\
			&SNDHRDW##_ay8910_interface								\
		}															\
	}																\
}


/*     NAME      INITMACH  SNDHRDW	INTERRUPT      CONVERT_COLOR_PROM */
DRIVER(marineb,  marineb,  marineb, nmi_interrupt, espial_vh_convert_color_prom);
DRIVER(wanted,   marineb,  wanted,  interrupt,     espial_vh_convert_color_prom);


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( marineb_rom )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "marineb.1",     0x0000, 0x1000, 0x661d6540 )
	ROM_LOAD( "marineb.2",     0x1000, 0x1000, 0x922da17f )
	ROM_LOAD( "marineb.3",     0x2000, 0x1000, 0x820a235b )
	ROM_LOAD( "marineb.4",     0x3000, 0x1000, 0xa157a283 )
	ROM_LOAD( "marineb.5",     0x4000, 0x1000, 0x9ffff9c0 )

	ROM_REGION_DISPOSE(0x2000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "marineb.6",     0x0000, 0x2000, 0xee53ec2e )

	ROM_REGION_DISPOSE(0x4000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "marineb.8",     0x0000, 0x2000, 0xdc8bc46c )
	ROM_LOAD( "marineb.7",     0x2000, 0x2000, 0x9d2e19ab )

	ROM_REGION(0x0200)	/* color proms */
	ROM_LOAD( "marineb.1b",    0x0000, 0x0100, 0xf32d9472 ) /* palette low 4 bits */
	ROM_LOAD( "marineb.1c",    0x0100, 0x0100, 0x93c69d3e ) /* palette high 4 bits */
ROM_END

ROM_START( wanted_rom )
	ROM_REGION(0x10000)       /* 64k for code */
	ROM_LOAD( "prg-1",		   0x0000, 0x2000, 0x2dd90aed )
	ROM_LOAD( "prg-2",		   0x2000, 0x2000, 0x67ac0210 )
	ROM_LOAD( "prg-3",		   0x4000, 0x2000, 0x373c7d82 )

	ROM_REGION_DISPOSE(0x4000)  /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "vram-1",		   0x0000, 0x2000, 0xc4226e54 )
	ROM_LOAD( "vram-2",		   0x2000, 0x2000, 0x2a9b1e36 )

	ROM_REGION_DISPOSE(0x4000)  /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "obj-a",		   0x0000, 0x2000, 0x90b60771 )
	ROM_LOAD( "obj-b",		   0x2000, 0x2000, 0xe14ee689 )

	ROM_REGION(0x0200)  /* color proms */
        ROM_LOAD( "wanted.k7",     0x0000, 0x0100, 0x2ba90a00 ) /* palette low 4 bits */
        ROM_LOAD( "wanted.k6",     0x0100, 0x0100, 0xa93d87cc ) /* palette high 4 bits */
ROM_END


static int wanted_hiload(void)
{
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];

	/* check if the hi score table has already been initialized */
	if (memcmp(&RAM[0x81b4],"\x00\x03\x00",3) == 0)
	{
		void *f;

		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
			osd_fread(f,&RAM[0x81b4], 16*5);        /* HS table */

			RAM[0x81b4] = RAM[0x81b4];      /* update high score */
			RAM[0x81b5] = RAM[0x81b5];      /* on top of screen */
			RAM[0x81b6] = RAM[0x81b6];
			RAM[0x81b7] = RAM[0x81b7];
			RAM[0x81b8] = RAM[0x81b8];
			RAM[0x81b9] = RAM[0x81b9];
			osd_fclose(f);
		}

		return 1;
	}
	else return 0;	/* we can't load the hi scores yet */
}

static void wanted_hisave(void)
{
	void *f;
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];

	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
		osd_fwrite(f,&RAM[0x81b4], 16*5);       /* HS table */
		osd_fclose(f);
	}
}


struct GameDriver marineb_driver =
{
	__FILE__,
	0,
	"marineb",
	"Marine Boy",
	"1982",
	"Orca",
	"Zsolt Vasvari",
	0,
	&marineb_machine_driver,
	0,

	marineb_rom,
	0, 0,
	0,
	0,	/* sound_prom */

	marineb_input_ports,

	PROM_MEMORY_REGION(3), 0, 0,
	ORIENTATION_DEFAULT,

	0, 0
};

struct GameDriver wanted_driver =
{
	__FILE__,
	0,
	"wanted",
	"Wanted",
	"1984",
	"Sigma Ent. Inc.",
	"Zsolt Vasvari",
	0,
	&wanted_machine_driver,
	0,

	wanted_rom,
	0,
	0,
	0,
	0,      /* sound_prom */

	wanted_input_ports,

	PROM_MEMORY_REGION(3), 0, 0,
	ORIENTATION_ROTATE_90,

	wanted_hiload, wanted_hisave
};

