#include "driver.h"
#include "vidhrdw/generic.h"

/* in vidhrdw/sichuan2.c */
void sichuan2_bankswitch_w(int offset,int data);
void sichuan2_paletteram_w(int offset,int data);
void sichuan2_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh);

/* in sndhrdw/sichuan2.c */
void sichuan2_init_machine(void);
void sichuan2_ym2151_irq_handler(int irq);
void sichuan2_sound_command_w(int offset,int data);
void sichuan2_sound_irq_ack_w(int offset,int data);
void sichuan2_sample_addr_w(int offset,int data);
int sichuan2_sample_r(int offset);
void sichuan2_sample_w(int offset,int data);



static int sichuan2_dsw1_r(int offset)
{
	int ret = input_port_3_r(0);

	/* Based on the coin mode fill in the upper bits */
	if (input_port_4_r(0) & 0x04)
	{
		/* Mode 1 */
		ret	|= (input_port_5_r(0) << 4);
	}
	else
	{
		/* Mode 2 */
		ret	|= (input_port_5_r(0) & 0xf0);
	}

	return ret;
}

static void sichuan2_coin_w(int offset,int data)
{
	coin_counter_w(0,data & 2);
	coin_counter_w(1,data & 4);
}



static struct MemoryReadAddress readmem[] =
{
	{ 0x0000, 0x7fff, MRA_ROM },
	{ 0x8000, 0xbfff, MRA_BANK1 },
	{ 0xc800, 0xcaff, MRA_RAM },
	{ 0xd000, 0xffff, MRA_RAM },
	{ -1 }	/* end of table */
};

static struct MemoryWriteAddress writemem[] =
{
	{ 0x0000, 0xbfff, MWA_ROM },
	{ 0xc800, 0xcaff, sichuan2_paletteram_w, &paletteram },
	{ 0xd000, 0xdfff, videoram_w, &videoram, &videoram_size },
	{ 0xe000, 0xffff, MWA_RAM },
	{ -1 }	/* end of table */
};

static struct IOReadPort readport[] =
{
	{ 0x00, 0x00, sichuan2_dsw1_r },
	{ 0x01, 0x01, input_port_4_r },
	{ 0x02, 0x02, input_port_0_r },
	{ 0x03, 0x03, input_port_1_r },
	{ 0x04, 0x04, input_port_2_r },
	{ -1 }	/* end of table */
};

static struct IOWritePort writeport[] =
{
	{ 0x00, 0x00, sichuan2_coin_w },
	{ 0x01, 0x01, sichuan2_sound_command_w },
	{ 0x02, 0x02, sichuan2_bankswitch_w },
	{ -1 }	/* end of table */
};

static struct MemoryReadAddress sound_readmem[] =
{
	{ 0x0000, 0x3fff, MRA_ROM },
	{ 0xfd00, 0xffff, MRA_RAM },
	{ -1 }	/* end of table */
};

static struct MemoryWriteAddress sound_writemem[] =
{
	{ 0x0000, 0x3fff, MWA_ROM },
	{ 0xfd00, 0xffff, MWA_RAM },
	{ -1 }	/* end of table */
};

static struct IOReadPort sound_readport[] =
{
	{ 0x01, 0x01, YM2151_status_port_0_r },
	{ 0x80, 0x80, soundlatch_r },
	{ 0x84, 0x84, sichuan2_sample_r },
	{ -1 }	/* end of table */
};

static struct IOWritePort sound_writeport[] =
{
	{ 0x00, 0x00, YM2151_register_port_0_w },
	{ 0x01, 0x01, YM2151_data_port_0_w },
	{ 0x80, 0x81, sichuan2_sample_addr_w },
	{ 0x82, 0x82, sichuan2_sample_w },
	{ 0x83, 0x83, sichuan2_sound_irq_ack_w },
	{ -1 }	/* end of table */
};



INPUT_PORTS_START( input_ports )
	PORT_START	/* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* COIN */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* DSW1 */
	PORT_DIPNAME( 0x03, 0x03, "Timer", IP_KEY_NONE )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x0c, 0x0c, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )  /* Gets filled in based on the coin mode */

	PORT_START	/* DSW2 */
	PORT_DIPNAME( 0x01, 0x00, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x01, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x02, 0x00, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x02, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x04, 0x04, "Coin Mode", IP_KEY_NONE )
	PORT_DIPSETTING(    0x04, "Mode 1" )
	PORT_DIPSETTING(    0x00, "Mode 2" )
	PORT_DIPNAME( 0x08, 0x08, "Naughty Pics", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x08, "On" )
	PORT_DIPNAME( 0x10, 0x10, "Gal Select", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "No" )
	PORT_DIPSETTING(    0x10, "Yes" )
	/* In stop mode, press 2 to stop and 1 to restart */
	PORT_BITX   ( 0x20, 0x20, IPT_DIPSWITCH_NAME | IPF_CHEAT, "Stop Mode", IP_KEY_NONE, IP_JOY_NONE,0 )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x40, "Max Players", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_BITX(    0x80, 0x80, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE,0 )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	/* Fake port to support the two different coin modes */
	PORT_START
	PORT_DIPNAME( 0x0f, 0x0f, "Coinage Mode 1", IP_KEY_NONE )   /* mapped on coin mode 1 */
	PORT_DIPSETTING(    0x0a, "6C_1C" )
	PORT_DIPSETTING(    0x0b, "5C_1C" )
	PORT_DIPSETTING(    0x0c, "4C_1C" )
	PORT_DIPSETTING(    0x0d, "3C_1C" )
	PORT_DIPSETTING(    0x01, "8 Coins/3 Credits" )
	PORT_DIPSETTING(    0x02, "5 Coins/3 Credits" )
	PORT_DIPSETTING(    0x0e, "2C_1C" )
	PORT_DIPSETTING(    0x03, "3C_2C" )
	PORT_DIPSETTING(    0x0f, "1C_1C" )
	PORT_DIPSETTING(    0x04, "2C_3C" )
	PORT_DIPSETTING(    0x09, "1C_2C" )
	PORT_DIPSETTING(    0x08, "1C_3C" )
	PORT_DIPSETTING(    0x07, "1C_4C" )
	PORT_DIPSETTING(    0x06, "1C_5C" )
	PORT_DIPSETTING(    0x05, "1C_6C" )
	PORT_DIPSETTING(    0x00, "Free_Play" )
 	PORT_DIPNAME( 0x30, 0x30, "Coin A  Mode 2", IP_KEY_NONE )   /* mapped on coin mode 2 */
	PORT_DIPSETTING(    0x00, "5C_1C" )
	PORT_DIPSETTING(    0x10, "3C_1C" )
	PORT_DIPSETTING(    0x20, "2C_1C" )
	PORT_DIPSETTING(    0x30, "1C_1C" )
	PORT_DIPNAME( 0xc0, 0xc0, "Coin B  Mode 2", IP_KEY_NONE )
	PORT_DIPSETTING(    0xc0, "1C_2C" )
	PORT_DIPSETTING(    0x80, "1C_3C" )
	PORT_DIPSETTING(    0x40, "1C_5C" )
	PORT_DIPSETTING(    0x00, "1C_6C" )
INPUT_PORTS_END



static struct GfxLayout charlayout =
{
	8,8,
	4096,
	4,
	{ 0, 4, 0x80000*8+0, 0x80000*8+4 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8
};


static struct GfxDecodeInfo gfxdecodeinfo[] =
{
	{ 1, 0x00000, &charlayout,  0, 16 },
	{ 1, 0x10000, &charlayout,  0, 16 },
	{ 1, 0x20000, &charlayout,  0, 16 },
	{ 1, 0x30000, &charlayout,  0, 16 },
	{ 1, 0x40000, &charlayout,  0, 16 },
	{ 1, 0x50000, &charlayout,  0, 16 },
	{ 1, 0x60000, &charlayout,  0, 16 },
	{ 1, 0x70000, &charlayout,  0, 16 },
	{ -1 } /* end of array */
};



static struct YM2151interface ym2151_interface =
{
	1,			/* 1 chip */
	3579000,	/* 3.579 MHz */
	{ 50 },
	{ (void (*)())sichuan2_ym2151_irq_handler },
	{ 0 }
};

static struct DACinterface dac_interface =
{
	1,	/* 1 channel */
	{ 25 }
};



static struct MachineDriver machine_driver =
{
	/* basic machine hardware */
	{
		{
			CPU_Z80,
			6000000,	/* 6 Mhz ? */
			0,
			readmem,writemem,readport,writeport,
			interrupt,1
		},
		{
			CPU_Z80,
			6000000,	/* 6 Mhz ? */
			2,
			sound_readmem,sound_writemem,sound_readport,sound_writeport,
			ignore_interrupt,0,	/* IRQs are generated by main Z80 and YM2151 */
			nmi_interrupt,8000	/* NMIs are generated by the sample player */
		}
	},
	60, DEFAULT_60HZ_VBLANK_DURATION,	/* frames per second, vblank duration */
	1 /* 100 FRANXIS 28-02-2006 */,
	sichuan2_init_machine,

	/* video hardware */
	64*8, 32*8, { 0*8, 64*8-1, 0*8, 32*8-1 },
	gfxdecodeinfo,
	256, 256,
	0,

	VIDEO_TYPE_RASTER|VIDEO_MODIFIES_PALETTE|VIDEO_SUPPORTS_DIRTY|VIDEO_PIXEL_ASPECT_RATIO_1_2,
	0,
	generic_vh_start,
	generic_vh_stop,
	sichuan2_vh_screenrefresh,

	/* sound hardware */
	/*SOUND_SUPPORTS_STEREO*/0,0,0,0,
	{
		{
			SOUND_YM2151,
			&ym2151_interface
		},
		{
			SOUND_DAC,
			&dac_interface
		}
	}
};



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( sichuan2_rom )
	ROM_REGION(0x30000)	/* 64k+128k for main CPU */
	ROM_LOAD( "ic06.06",      0x00000, 0x10000, 0x98a2459b )
	ROM_RELOAD(               0x10000, 0x10000 )
	ROM_LOAD( "ic07.03",      0x20000, 0x10000, 0x0350f6e2 )

	ROM_REGION_DISPOSE(0x100000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "ic08.04",      0x00000, 0x10000, 0x1c0e221c )
	ROM_LOAD( "ic09.05",      0x10000, 0x10000, 0x8a7d8284 )
	ROM_LOAD( "ic12.08",      0x20000, 0x10000, 0x48e1d043 )
	ROM_LOAD( "ic13.09",      0x30000, 0x10000, 0x3feff3f2 )
	ROM_LOAD( "ic14.10",      0x40000, 0x10000, 0xb76a517d )
	ROM_LOAD( "ic15.11",      0x50000, 0x10000, 0x8ff5ee7a )
	ROM_LOAD( "ic16.12",      0x60000, 0x10000, 0x64e5d837 )
	ROM_LOAD( "ic17.13",      0x70000, 0x10000, 0x02c1b2c4 )
	ROM_LOAD( "ic18.14",      0x80000, 0x10000, 0xf5a8370e )
	ROM_LOAD( "ic19.15",      0x90000, 0x10000, 0x7a9b7671 )
	ROM_LOAD( "ic20.16",      0xa0000, 0x10000, 0x7fb396ad )
	ROM_LOAD( "ic21.17",      0xb0000, 0x10000, 0xfb83c652 )
	ROM_LOAD( "ic22.18",      0xc0000, 0x10000, 0xd8b689e9 )
	ROM_LOAD( "ic23.19",      0xd0000, 0x10000, 0xe6611947 )
	ROM_LOAD( "ic10.06",      0xe0000, 0x10000, 0x473b349a )
	ROM_LOAD( "ic11.07",      0xf0000, 0x10000, 0xd9a60285 )

	ROM_REGION(0x10000)	/* 64k for the audio CPU */
	ROM_LOAD( "ic01.01",      0x00000, 0x10000, 0x51b0a26c )

	ROM_REGION(0x40000)	/* samples */
	ROM_LOAD( "ic02.02",      0x00000, 0x10000, 0x92f0093d )
	ROM_LOAD( "ic03.03",      0x10000, 0x10000, 0x116a049c )
	ROM_LOAD( "ic04.04",      0x20000, 0x10000, 0x6840692b )
	ROM_LOAD( "ic05.05",      0x30000, 0x10000, 0x92ffe22a )
ROM_END

ROM_START( shisen_rom )
	ROM_REGION(0x30000)	/* 64k+128k for main CPU */
	ROM_LOAD( "a-27-a.rom",   0x00000, 0x20000, 0xde2ecf05 )
	ROM_RELOAD(               0x10000, 0x20000 )

	ROM_REGION_DISPOSE(0x100000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "ic08.04",      0x00000, 0x10000, 0x1c0e221c )
	ROM_LOAD( "ic09.05",      0x10000, 0x10000, 0x8a7d8284 )
	ROM_LOAD( "ic12.08",      0x20000, 0x10000, 0x48e1d043 )
	ROM_LOAD( "ic13.09",      0x30000, 0x10000, 0x3feff3f2 )
	ROM_LOAD( "ic14.10",      0x40000, 0x10000, 0xb76a517d )
	ROM_LOAD( "ic15.11",      0x50000, 0x10000, 0x8ff5ee7a )
	ROM_LOAD( "ic16.12",      0x60000, 0x10000, 0x64e5d837 )
	ROM_LOAD( "ic17.13",      0x70000, 0x10000, 0x02c1b2c4 )
	ROM_LOAD( "ic18.14",      0x80000, 0x10000, 0xf5a8370e )
	ROM_LOAD( "ic19.15",      0x90000, 0x10000, 0x7a9b7671 )
	ROM_LOAD( "ic20.16",      0xa0000, 0x10000, 0x7fb396ad )
	ROM_LOAD( "ic21.17",      0xb0000, 0x10000, 0xfb83c652 )
	ROM_LOAD( "ic22.18",      0xc0000, 0x10000, 0xd8b689e9 )
	ROM_LOAD( "ic23.19",      0xd0000, 0x10000, 0xe6611947 )
	ROM_LOAD( "ic10.06",      0xe0000, 0x10000, 0x473b349a )
	ROM_LOAD( "ic11.07",      0xf0000, 0x10000, 0xd9a60285 )

	ROM_REGION(0x10000)	/* 64k for the audio CPU */
	ROM_LOAD( "ic01.01",      0x00000, 0x10000, 0x51b0a26c )

	ROM_REGION(0x40000)	/* samples */
	ROM_LOAD( "ic02.02",      0x00000, 0x10000, 0x92f0093d )
	ROM_LOAD( "ic03.03",      0x10000, 0x10000, 0x116a049c )
	ROM_LOAD( "ic04.04",      0x20000, 0x10000, 0x6840692b )
	ROM_LOAD( "ic05.05",      0x30000, 0x10000, 0x92ffe22a )
ROM_END



static int sichuan2_hiload(void)
{
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];

	/* check if the hi score table has already been initialized */
        if (memcmp(&RAM[0xfcb0],"\x2b\x03\x0f",3) == 0)
	{
		void *f;

		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
                        osd_fread(f,&RAM[0xfcb0], 5*16);        /* HS table */

                        RAM[0xfcae] = RAM[0xfcbd];      /* update high score */
                        RAM[0xfcaf] = RAM[0xfcbe];      /* on top of screen */
			osd_fclose(f);
		}

		return 1;
	}
	else return 0;	/* we can't load the hi scores yet */
}

static void sichuan2_hisave(void)
{
	void *f;
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];

	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
                osd_fwrite(f,&RAM[0xfcb0], 5*16);       /* HS table */
		osd_fclose(f);
	}
}



struct GameDriver sichuan2_driver =
{
	__FILE__,
	0,
	"sichuan2",
	"Sichuan II (hack?)",
	"1989",
	"Tamtex",
	"Nicola Salmoria",
	0,
	&machine_driver,
	0,

	sichuan2_rom,
	0, 0,
	0,
	0,	/* sound_prom */

	input_ports,

	0, 0, 0,
	ORIENTATION_DEFAULT,

	sichuan2_hiload, sichuan2_hisave
};

struct GameDriver shisen_driver =
{
	__FILE__,
	&sichuan2_driver,
	"shisen",
	"Shisensho - Joshiryo-Hen (Japan)",
	"1989",
	"Tamtex",
	"Nicola Salmoria",
	0,
	&machine_driver,
	0,

	shisen_rom,
	0, 0,
	0,
	0,	/* sound_prom */

	input_ports,

	0, 0, 0,
	ORIENTATION_DEFAULT,

	sichuan2_hiload, sichuan2_hisave
};
