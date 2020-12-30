/***************************************************************************

	Arkanoid driver (Preliminary)


	The location BFFFh can be changed to indicate the country:

	0x33 - Probably the world version (Copyright Taito Corp. Japan)
	0x76 - Japan (Copyright Taito Corporation with Japanese only warning)
	0x92 - USA (Copyright Taito America Corp. - Licensed to ROMSTAR for U.S.A)

	Japanese version support cocktail mode (DSW #7), the others don't.

	Here are the versions we have:

	arkanoid	World version, probably an earlier revision
	arknoidu	USA version, probably a later revision; There has been code
			    inserted, NOT patched, so I don't think it's a bootleg
	arkbl2		Bootleg of the early Japanese version we don't have.
				The only difference is that the warning text has been replaced
				by "WAIT"
				ROM	E2.6F should be identical to the real Japanese one.
				(It only differs in the country byte from A75_11.ROM)
				This version works fine with the real MCU ROM
	arkatayt	Another bootleg of the early Japanese one, more heavily modified
	arkblock	Another bootleg of the early Japanese one, more heavily modified
	arkbl3   	Another bootleg of the early Japanese one, more heavily modified

***************************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"



extern unsigned char *arkanoid_stat;

void arkanoid_d008_w(int offset,int data);
void arkanoid_vh_convert_color_prom(unsigned char *palette, unsigned short *colortable,const unsigned char *color_prom);
void arkanoid_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh);

int arkanoid_Z80_mcu_r (int value);
void arkanoid_Z80_mcu_w (int offset, int value);

int arkanoid_68705_mcu_r (int offset);
void arkanoid_68705_mcu_w (int offset, int value);

int arkanoid_68705_stat_r (int offset);
void arkanoid_68705_stat_w (int offset, int data);

int arkanoid_68705_input_0_r (int offset);
int arkanoid_input_2_r (int offset);


static struct MemoryReadAddress readmem[] =
{
	{ 0x0000, 0xbfff, MRA_ROM },
	{ 0xc000, 0xcfff, MRA_RAM },
	{ 0xd001, 0xd001, AY8910_read_port_0_r },
	{ 0xd00c, 0xd00c, arkanoid_68705_input_0_r },  /* mainly an input port, with 2 bits from the 68705 */
	{ 0xd010, 0xd010, input_port_1_r },
	{ 0xd018, 0xd018, arkanoid_Z80_mcu_r },  /* input from the 68705 */
	{ 0xe000, 0xefff, MRA_RAM },
	{ 0xf000, 0xffff, MRA_ROM },
	{ -1 }	/* end of table */
};

static struct MemoryWriteAddress writemem[] =
{
	{ 0x0000, 0xbfff, MWA_ROM },
	{ 0xc000, 0xcfff, MWA_RAM },
	{ 0xd000, 0xd000, AY8910_control_port_0_w },
	{ 0xd001, 0xd001, AY8910_write_port_0_w },
	{ 0xd008, 0xd008, arkanoid_d008_w },	/* gfx bank, flip screen etc. */
	{ 0xd010, 0xd010, watchdog_reset_w },
	{ 0xd018, 0xd018, arkanoid_Z80_mcu_w }, /* output to the 68705 */
	{ 0xe000, 0xe7ff, videoram_w, &videoram, &videoram_size },
	{ 0xe800, 0xe83f, MWA_RAM, &spriteram, &spriteram_size },
	{ 0xe840, 0xefff, MWA_RAM },
	{ 0xf000, 0xffff, MWA_ROM },
	{ -1 }	/* end of table */
};

static struct MemoryReadAddress boot_readmem[] =
{
	{ 0x0000, 0xbfff, MRA_ROM },
	{ 0xc000, 0xcfff, MRA_RAM },
	{ 0xd001, 0xd001, AY8910_read_port_0_r },
	{ 0xd00c, 0xd00c, input_port_0_r },
	{ 0xd010, 0xd010, input_port_1_r },
	{ 0xd018, 0xd018, arkanoid_input_2_r },
	{ 0xe000, 0xefff, MRA_RAM },
	{ 0xf000, 0xffff, MRA_ROM },
	{ -1 }	/* end of table */
};

static struct MemoryWriteAddress boot_writemem[] =
{
	{ 0x0000, 0xbfff, MWA_ROM },
	{ 0xc000, 0xcfff, MWA_RAM },
	{ 0xd000, 0xd000, AY8910_control_port_0_w },
	{ 0xd001, 0xd001, AY8910_write_port_0_w },
	{ 0xd008, 0xd008, arkanoid_d008_w },	/* gfx bank, flip screen etc. */
	{ 0xd010, 0xd010, watchdog_reset_w },
	{ 0xd018, 0xd018, MWA_NOP },
	{ 0xe000, 0xe7ff, videoram_w, &videoram, &videoram_size },
	{ 0xe800, 0xe83f, MWA_RAM, &spriteram, &spriteram_size },
	{ 0xe840, 0xefff, MWA_RAM },
	{ 0xf000, 0xffff, MWA_ROM },
	{ -1 }	/* end of table */
};


static struct MemoryReadAddress mcu_readmem[] =
{
	{ 0x0000, 0x0000, arkanoid_68705_mcu_r },
	{ 0x0001, 0x0001, arkanoid_input_2_r },
	{ 0x0002, 0x0002, arkanoid_68705_stat_r, &arkanoid_stat },
	{ 0x0003, 0x000f, MRA_RAM },
	{ 0x0010, 0x007f, MRA_RAM },
	{ 0x0080, 0x07ff, MRA_ROM },
	{ -1 }	/* end of table */
};

static struct MemoryWriteAddress mcu_writemem[] =
{
	{ 0x0000, 0x0000, arkanoid_68705_mcu_w },
	{ 0x0001, 0x0001, MWA_RAM },
	{ 0x0002, 0x0002, arkanoid_68705_stat_w },
	{ 0x0003, 0x000f, MWA_RAM },
	{ 0x0010, 0x007f, MWA_RAM },
	{ 0x0080, 0x07ff, MWA_ROM },
	{ -1 }	/* end of table */
};


INPUT_PORTS_START( input_ports )
	PORT_START	/* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )	/* input from the 68705 */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* input from the 68705 */

	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* IN2 - spinner Player 1 */
	PORT_ANALOG( 0xff, 0x00, IPT_DIAL, 20, 0, 0, 0)

	PORT_START      /* IN3 - spinner Player 2  */
	PORT_ANALOG( 0xff, 0x00, IPT_DIAL | IPF_COCKTAIL, 20, 0, 0, 0)

	PORT_START	/* DSW1 */
	PORT_DIPNAME( 0x01, 0x00, "Allow Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x01, "No" )
	PORT_DIPSETTING(    0x00, "Yes" )
	PORT_DIPNAME( 0x02, 0x02, "Flip Screen", IP_KEY_NONE )
	PORT_DIPSETTING(    0x02, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_BITX ( 0x04, 0x04, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
	PORT_DIPSETTING ( 0x04, "Off" )
	PORT_DIPSETTING ( 0x00, "On" )
	PORT_DIPNAME( 0x08, 0x08, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Easy" )
	PORT_DIPSETTING(    0x00, "Hard" )
	PORT_DIPNAME( 0x10, 0x10, "Bonus Life", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "20000 60000" )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPNAME( 0x20, 0x20, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0xc0, 0xc0, "Coinage", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0xc0, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x80, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin/6 Credits" )
INPUT_PORTS_END

/* These are the input ports of the real Japanese ROM set (we don't have it yet) */
/* 'Block' uses the these ones as well.	The Tayto bootleg is different			 */
/*  in coinage and # of lives.                    								 */

INPUT_PORTS_START( japan_input_ports )
	PORT_START	/* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* IN2 - spinner (multiplexed for player 1 and 2) */
	PORT_ANALOG( 0xff, 0x00, IPT_DIAL, 20, 0, 0, 0)

	PORT_START      /* IN3 - spinner Player 2  */
	PORT_ANALOG( 0xff, 0x00, IPT_DIAL | IPF_COCKTAIL, 20, 0, 0, 0)

	PORT_START	/* DSW1 */
	PORT_DIPNAME( 0x01, 0x00, "Allow Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x01, "No" )
	PORT_DIPSETTING(    0x00, "Yes" )
	PORT_DIPNAME( 0x02, 0x02, "Flip Screen", IP_KEY_NONE )
	PORT_DIPSETTING(    0x02, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_BITX ( 0x04, 0x04, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
	PORT_DIPSETTING ( 0x04, "Off" )
	PORT_DIPSETTING ( 0x00, "On" )
	PORT_DIPNAME( 0x08, 0x08, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Easy" )
	PORT_DIPSETTING(    0x00, "Hard" )
	PORT_DIPNAME( 0x10, 0x10, "Bonus Life", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "20000 60000" )
	PORT_DIPSETTING(    0x00, "20000?" )
	PORT_DIPNAME( 0x20, 0x00, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x40, 0x40, "Coinage", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "1 Coins/2 Credit" )
	PORT_DIPSETTING(    0x40, "1 Coin/1 Credit" )
	PORT_DIPNAME( 0x80, 0x00, "Cabinet", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Upright" )
	PORT_DIPSETTING(    0x80, "Cocktail" )
INPUT_PORTS_END



static struct GfxLayout charlayout =
{
	8,8,	/* 8*8 characters */
	4096,	/* 4096 characters */
	3,	/* 3 bits per pixel */
	{ 2*4096*8*8, 4096*8*8, 0 },	/* the two bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8	/* every char takes 8 consecutive bytes */
};



static struct GfxDecodeInfo gfxdecodeinfo[] =
{
	{ 1, 0x00000, &charlayout,  0, 64 },
	/* sprites use the same characters above, but are 16x8 */
	{ -1 } /* end of array */
};



static struct AY8910interface ay8910_interface =
{
	1,	/* 1 chips */
	1500000,	/* 1.5 MHz ???? */
	{ 255 },
	{ 0 },
	{ input_port_4_r },
	{ 0 },
	{ 0 }
};



static struct MachineDriver machine_driver =
{
	/* basic machine hardware */
	{
		{
			CPU_Z80,
			6000000,	/* 6 Mhz ?? */
			0,
			readmem,writemem,0,0,
			interrupt,1
		},
		{
			CPU_M68705,
			500000,	/* .5 Mhz (don't know really how fast, but it doesn't need to even be this fast) */
			3,
			mcu_readmem,mcu_writemem,0,0,
			ignore_interrupt,1
		},
	},
	60, DEFAULT_60HZ_VBLANK_DURATION,	/* frames per second, vblank duration */
	100, /* 100 CPU slices per second to synchronize between the MCU and the main CPU */
	0,

	/* video hardware */
	32*8, 32*8, { 0*8, 32*8-1, 2*8, 30*8-1 },
	gfxdecodeinfo,
	512, 512,
	arkanoid_vh_convert_color_prom,

	VIDEO_TYPE_RASTER | VIDEO_SUPPORTS_DIRTY,
	0,
	generic_vh_start,
	generic_vh_stop,
	arkanoid_vh_screenrefresh,

	/* sound hardware */
	0,0,0,0,
	{
		{
			SOUND_AY8910,
			&ay8910_interface
		}
	}
};

static struct MachineDriver bootleg_machine_driver =
{
	/* basic machine hardware */
	{
		{
			CPU_Z80,
			6000000,	/* 6 Mhz ?? */
			0,
			boot_readmem,boot_writemem,0,0,
			interrupt,1
		},
	},
	60, DEFAULT_60HZ_VBLANK_DURATION,	/* frames per second, vblank duration */
	1,
	0,

	/* video hardware */
	32*8, 32*8, { 0*8, 32*8-1, 2*8, 30*8-1 },
	gfxdecodeinfo,
	512, 512,
	arkanoid_vh_convert_color_prom,

	VIDEO_TYPE_RASTER | VIDEO_SUPPORTS_DIRTY,
	0,
	generic_vh_start,
	generic_vh_stop,
	arkanoid_vh_screenrefresh,

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

ROM_START( arkanoid_rom )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "a75_01-1.rom", 0x0000, 0x8000, 0x5bcda3b0 )
	ROM_LOAD( "a75_11.rom",   0x8000, 0x8000, 0xeafd7191 )

	ROM_REGION_DISPOSE(0x18000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "a75_03.rom",   0x00000, 0x8000, 0x038b74ba )
	ROM_LOAD( "a75_04.rom",   0x08000, 0x8000, 0x71fae199 )
	ROM_LOAD( "a75_05.rom",   0x10000, 0x8000, 0xc76374e2 )

	ROM_REGION(0x0600)	/* color PROMs */
	ROM_LOAD( "07.bpr",       0x0000, 0x0200, 0x0af8b289 )	/* red component */
	ROM_LOAD( "08.bpr",       0x0200, 0x0200, 0xabb002fb )	/* green component */
	ROM_LOAD( "09.bpr",       0x0400, 0x0200, 0xa7c6c277 )	/* blue component */

	ROM_REGION(0x0800)	/* 8k for the microcontroller */
	ROM_LOAD( "arkanoid.uc",  0x0080, 0x0780, 0x8a02f903 )
ROM_END

ROM_START( arknoidu_rom )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "a75-19.bin",   0x0000, 0x8000, 0xd3ad37d7 )
	ROM_LOAD( "a75-18.bin",   0x8000, 0x8000, 0xcdc08301 )

	ROM_REGION_DISPOSE(0x18000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "a75_03.rom",   0x00000, 0x8000, 0x038b74ba )
	ROM_LOAD( "a75_04.rom",   0x08000, 0x8000, 0x71fae199 )
	ROM_LOAD( "a75_05.rom",   0x10000, 0x8000, 0xc76374e2 )

	ROM_REGION(0x0600)	/* color PROMs */
	ROM_LOAD( "07.bpr",       0x0000, 0x0200, 0x0af8b289 )	/* red component */
	ROM_LOAD( "08.bpr",       0x0200, 0x0200, 0xabb002fb )	/* green component */
	ROM_LOAD( "09.bpr",       0x0400, 0x0200, 0xa7c6c277 )	/* blue component */

	ROM_REGION(0x0800)	/* 8k for the microcontroller */
	ROM_LOAD( "arkanoid.uc",  0x0080, 0x0780, 0x8a02f903 )
ROM_END

ROM_START( arkbl2_rom )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "e1.6d",        0x0000, 0x8000, 0xdd4f2b72 )
	ROM_LOAD( "e2.6f",        0x8000, 0x8000, 0xbbc33ceb )

	ROM_REGION_DISPOSE(0x18000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "a75_03.rom",   0x00000, 0x8000, 0x038b74ba )
	ROM_LOAD( "a75_04.rom",   0x08000, 0x8000, 0x71fae199 )
	ROM_LOAD( "a75_05.rom",   0x10000, 0x8000, 0xc76374e2 )

	ROM_REGION(0x0600)	/* color PROMs */
	ROM_LOAD( "07.bpr",       0x0000, 0x0200, 0x0af8b289 )	/* red component */
	ROM_LOAD( "08.bpr",       0x0200, 0x0200, 0xabb002fb )	/* green component */
	ROM_LOAD( "09.bpr",       0x0400, 0x0200, 0xa7c6c277 )	/* blue component */

	ROM_REGION(0x0800)	/* 8k for the microcontroller */
	ROM_LOAD( "68705p3.6i",   0x0080, 0x0780, 0xe3c5024e )
ROM_END

ROM_START( arkbl3_rom )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "arkanunk.1",   0x0000, 0x8000, 0xb0f73900 )
	ROM_LOAD( "arkanunk.2",   0x8000, 0x8000, 0x9827f297 )

	ROM_REGION_DISPOSE(0x18000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "a75_03.rom",   0x00000, 0x8000, 0x038b74ba )
	ROM_LOAD( "a75_04.rom",   0x08000, 0x8000, 0x71fae199 )
	ROM_LOAD( "a75_05.rom",   0x10000, 0x8000, 0xc76374e2 )

	ROM_REGION(0x0600)	/* color PROMs */
	ROM_LOAD( "07.bpr",       0x0000, 0x0200, 0x0af8b289 )	/* red component */
	ROM_LOAD( "08.bpr",       0x0200, 0x0200, 0xabb002fb )	/* green component */
	ROM_LOAD( "09.bpr",       0x0400, 0x0200, 0xa7c6c277 )	/* blue component */
ROM_END

ROM_START( arkatayt_rom )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "arkanoid.1",   0x0000, 0x8000, 0x6e0a2b6f )
	ROM_LOAD( "arkanoid.2",   0x8000, 0x8000, 0x5a97dd56 )

	ROM_REGION_DISPOSE(0x18000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "a75_03.rom",   0x00000, 0x8000, 0x038b74ba )
	ROM_LOAD( "a75_04.rom",   0x08000, 0x8000, 0x71fae199 )
	ROM_LOAD( "a75_05.rom",   0x10000, 0x8000, 0xc76374e2 )

	ROM_REGION(0x0600)	/* color PROMs */
	ROM_LOAD( "07.bpr",       0x0000, 0x0200, 0x0af8b289 )	/* red component */
	ROM_LOAD( "08.bpr",       0x0200, 0x0200, 0xabb002fb )	/* green component */
	ROM_LOAD( "09.bpr",       0x0400, 0x0200, 0xa7c6c277 )	/* blue component */
ROM_END

ROM_START( arkblock_rom )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "block01.bin",  0x0000, 0x8000, 0x5be667e1 )
	ROM_LOAD( "block02.bin",  0x8000, 0x8000, 0x4f883ef1 )

	ROM_REGION_DISPOSE(0x18000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "a75_03.rom",   0x00000, 0x8000, 0x038b74ba )
	ROM_LOAD( "a75_04.rom",   0x08000, 0x8000, 0x71fae199 )
	ROM_LOAD( "a75_05.rom",   0x10000, 0x8000, 0xc76374e2 )

	ROM_REGION(0x0600)	/* color PROMs */
	ROM_LOAD( "07.bpr",       0x0000, 0x0200, 0x0af8b289 )	/* red component */
	ROM_LOAD( "08.bpr",       0x0200, 0x0200, 0xabb002fb )	/* green component */
	ROM_LOAD( "09.bpr",       0x0400, 0x0200, 0xa7c6c277 )	/* blue component */
ROM_END


static int hiload(void)
{
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];


	/* check if the hi score table has already been initialized */
	if (memcmp(&RAM[0xef79],"\x00\x50\x00",3) == 0 && /* position 1 */
		memcmp(&RAM[0xef95],"\x00\x30\x00",3) == 0 && /* position 5 */
		memcmp(&RAM[0xc4df],"\x00\x50\x00",3) == 0)	  /* separate hi score */
	{
		void *f;


		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
			osd_fread(f,&RAM[0xef79],7*5);
			RAM[0xc4df] = RAM[0xef79];
			RAM[0xc4e0] = RAM[0xef7a];
			RAM[0xc4e1] = RAM[0xef7b];
			osd_fclose(f);
		}

		return 1;
	}
	else return 0;	/* we can't load the hi scores yet */
}

static void hisave(void)
{
	void *f;
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];


	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
		osd_fwrite(f,&RAM[0xef79],7*5);
		osd_fclose(f);
	}
}



struct GameDriver arkanoid_driver =
{
	__FILE__,
	0,
	"arkanoid",
	"Arkanoid (World)",
	"1986",
	"Taito Corp Japan",
	"Brad Oliver (MAME driver)\nNicola Salmoria (MAME driver)\nAaron Giles (68705 emulation)",
	0,
	&machine_driver,
	0,

	arkanoid_rom,
	0, 0,
	0,
	0,	/* sound_prom */

	input_ports,

	PROM_MEMORY_REGION(2), 0, 0,
	ORIENTATION_ROTATE_90,

	hiload, hisave
};

struct GameDriver arknoidu_driver =
{
	__FILE__,
	&arkanoid_driver,
	"arknoidu",
	"Arkanoid (USA)",
	"1986",
	"Taito America Corp (Romstar license)",
	"Brad Oliver (MAME driver)\nNicola Salmoria (MAME driver)\nAaron Giles (68705 emulation)",
	0,
	&machine_driver,
	0,

	arknoidu_rom,
	0, 0,
	0,
	0,	/* sound_prom */

	input_ports,

	PROM_MEMORY_REGION(2), 0, 0,
	ORIENTATION_ROTATE_90,

	hiload, hisave
};

struct GameDriver arkbl2_driver =
{
	__FILE__,
	&arkanoid_driver,
	"arkbl2",
	"Arkanoid (Japanese bootleg Set 2)",
	"1986",
	"bootleg",
	"Brad Oliver (MAME driver)\nNicola Salmoria (MAME driver)\nAaron Giles (68705 emulation)",
	GAME_NOT_WORKING,
	&machine_driver,
	0,

	arkbl2_rom,
	0, 0,
	0,
	0,	/* sound_prom */

	japan_input_ports,

	PROM_MEMORY_REGION(2), 0, 0,
	ORIENTATION_ROTATE_90,

	hiload, hisave
};

struct GameDriver arkbl3_driver =
{
	__FILE__,
	&arkanoid_driver,
	"arkbl3",
	"Arkanoid (Japanese bootleg Set 3)",
	"1986",
	"bootleg",
	"Brad Oliver (MAME driver)\nNicola Salmoria (MAME driver)\nAaron Giles (68705 emulation)",
	GAME_NOT_WORKING,
	&bootleg_machine_driver,
	0,

	arkbl3_rom,
	0, 0,
	0,
	0,	/* sound_prom */

	japan_input_ports,

	PROM_MEMORY_REGION(2), 0, 0,
	ORIENTATION_ROTATE_90,

	hiload, hisave
};

struct GameDriver arkatayt_driver =
{
	__FILE__,
	&arkanoid_driver,
	"arkatayt",
	"Arkanoid (Tayto bootleg, Japanese)",
	"1986",
	"bootleg",
	"Brad Oliver (MAME driver)\nNicola Salmoria (MAME driver)",
	0,
	&bootleg_machine_driver,
	0,

	arkatayt_rom,
	0, 0,
	0,
	0,	/* sound_prom */

	japan_input_ports,

	PROM_MEMORY_REGION(2), 0, 0,
	ORIENTATION_ROTATE_90,

	hiload, hisave
};

struct GameDriver arkblock_driver =
{
	__FILE__,
	&arkanoid_driver,
	"arkblock",
	"Block (bootleg, Japanese)",
	"1986",
	"bootleg",
	"Brad Oliver (MAME driver)\nNicola Salmoria (MAME driver)",
	GAME_NOT_WORKING,
	&bootleg_machine_driver,
	0,

	arkblock_rom,
	0, 0,
	0,
	0,	/* sound_prom */

	japan_input_ports,

	PROM_MEMORY_REGION(2), 0, 0,
	ORIENTATION_ROTATE_90,

	hiload, hisave
};
