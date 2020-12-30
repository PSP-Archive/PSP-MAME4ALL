/***************************************************************************

	Sexy Billiards(?)				(c) 1987 Data East Corporation
	Sexy Billiards(?) (Bootleg)		(c) 1989(?) Yada East Corporation(!)

	Emulation by Bryan McPhail, mish@tendril.force9.net

	I'm not sure if this game is called 'Sexy Billiards' as I cannot read
the Japanese lettering.  Can someone check it out please?

	Most dipswitches are unknown.

	There is a weird problem - sometimes the sprites don't appear and the
game freezes on the attract mode.  BUT, sometimes it works fine!  I can't
work out why sometimes it works and sometimes it doesn't.  Random resets
also.

***************************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"
#include "m6502/m6502.h"

/* From dec8.c - rename it to dec8_color prom later.. used by many games.. */
void ghostb_vh_convert_color_prom(unsigned char *palette, unsigned short *colortable,const unsigned char *color_prom);

void pcktgal_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh);

/***************************************************************************/

static void pcktgal_bank_w(int offset,int data)
{
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];


	if (data & 1) { cpu_setbank(1,&RAM[0x4000]); }
	else { cpu_setbank(1,&RAM[0x10000]); }

	if (data & 2) { cpu_setbank(2,&RAM[0x6000]); }
	else { cpu_setbank(2,&RAM[0x12000]); }
}

static void pcktgal_sound_w(int offset,int data)
{
	soundlatch_w(0,data);
	cpu_cause_interrupt(1,M6502_INT_NMI);
}

void pcktgal_adpcm_int(int data)
{
	static int toggle;

	toggle = 1 - toggle;
	if (/*firetrap_irq_enable &&*/ toggle)
		cpu_cause_interrupt(1,M6502_INT_IRQ);
}

void pcktgal_adpcm_data_w(int offset,int data)
{
	MSM5205_data_w(offset,data >> 4);
	MSM5205_data_w(offset,data);
}

int pcktgal_adpcm_reset_r(int offset)
{
	MSM5205_reset_w(0,0);
	return 0;
}

/***************************************************************************/

static struct MemoryReadAddress readmem[] =
{
	{ 0x0000, 0x07ff, MRA_RAM },
	{ 0x1800, 0x1800, input_port_0_r },
	{ 0x1a00, 0x1a00, input_port_1_r },
	{ 0x1c00, 0x1c00, input_port_2_r },
	{ 0x4000, 0x5fff, MRA_BANK1 },
	{ 0x6000, 0x7fff, MRA_BANK2 },
	{ 0x8000, 0xffff, MRA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress writemem[] =
{
	{ 0x0000, 0x07ff, MWA_RAM },
	{ 0x0800, 0x0fff, videoram_w, &videoram, &videoram_size },
	{ 0x1000, 0x11ff, MWA_RAM, &spriteram, &spriteram_size },
	{ 0x1801, 0x1801, MWA_NOP },	/* bit 7 = flip screen, other bits unknown */
	{ 0x1a00, 0x1a00, pcktgal_sound_w },
	{ 0x1c00, 0x1c00, pcktgal_bank_w },
	{ 0x4000, 0xffff, MWA_ROM },
	{ -1 }  /* end of table */
};

/***************************************************************************/

static struct MemoryReadAddress sound_readmem[] =
{
	{ 0x0000, 0x07ff, MRA_RAM },
	{ 0x3000, 0x3000, soundlatch_r },
	{ 0x3400, 0x3400, pcktgal_adpcm_reset_r },	/* ? not sure */
	{ 0x4000, 0x7fff, MRA_ROM },	/* probably banked, controlled by 2000, but I don't know how */
	{ 0x8000, 0xffff, MRA_ROM },
	{ -1 }	/* end of table */
};

static struct MemoryWriteAddress sound_writemem[] =
{
	{ 0x0000, 0x07ff, MWA_RAM },
    { 0x0800, 0x0800, YM2203_control_port_0_w },
    { 0x0801, 0x0801, YM2203_write_port_0_w },
	{ 0x1000, 0x1000, YM3812_control_port_0_w },
	{ 0x1001, 0x1001, YM3812_write_port_0_w },
	{ 0x1800, 0x1800, pcktgal_adpcm_data_w },	/* ADPCM data for the MSM5205 chip */
	{ 0x4000, 0xffff, MWA_ROM },
	{ -1 }	/* end of table */
};

/***************************************************************************/

INPUT_PORTS_START( input_ports )
	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )

	PORT_START	/* Dip switch */
	PORT_DIPNAME( 0x03, 0x03, "Coinage", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x03, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x02, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x01, "1 Coin/3 Credits" )
 	PORT_DIPNAME( 0x04, 0x04, "Flip Screen", IP_KEY_NONE )
	PORT_DIPSETTING(    0x04, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
 	PORT_DIPNAME( 0x08, 0x08, "Allow 2 Players Game", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "No" )
	PORT_DIPSETTING(    0x08, "Yes" )
 	PORT_DIPNAME( 0x10, 0x10, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x10, "On" )
 	PORT_DIPNAME( 0x20, 0x20, "Time", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPSETTING(    0x20, "120" )
	PORT_DIPNAME( 0x40, 0x00, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
INPUT_PORTS_END

/***************************************************************************/

static struct GfxLayout charlayout =
{
    8,8,    /* 8*8 characters */
    4096,
    4,
    { 0x10000*8, 0, 0x18000*8, 0x8000*8 },
    { 0, 1, 2, 3, 4, 5, 6, 7 },
    { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
    8*8     /* every char takes 8 consecutive bytes */
};

static struct GfxLayout bootleg_charlayout =
{
    8,8,    /* 8*8 characters */
    4096,
    4,
    { 0x18000*8, 0x8000*8, 0x10000*8, 0 },
    { 0, 1, 2, 3, 4, 5, 6, 7 },
    { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
    8*8     /* every char takes 8 consecutive bytes */
};

static struct GfxLayout spritelayout =
{
    16,16,  /* 16*16 sprites */
    1024,   /* 1024 sprites */
    2,      /* 2 bits per pixel */
    { 0x8000*8, 0 },
    { 128+0, 128+1, 128+2, 128+3, 128+4, 128+5, 128+6, 128+7, 0, 1, 2, 3, 4, 5, 6, 7 },
    { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
    32*8    /* every char takes 8 consecutive bytes */
};

static struct GfxLayout bootleg_spritelayout =
{
    16,16,  /* 16*16 sprites */
    1024,   /* 1024 sprites */
    2,      /* 2 bits per pixel */
    { 0x8000*8, 0 },
    { 128+7, 128+6, 128+5, 128+4, 128+3, 128+2, 128+1, 128+0, 7, 6, 5, 4, 3, 2, 1, 0,  },
    { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
    32*8    /* every char takes 8 consecutive bytes */
};

static struct GfxDecodeInfo gfxdecodeinfo[] =
{
    { 1, 0x00000, &charlayout,   256, 16 }, /* chars */
    { 1, 0x20000, &spritelayout,   0,  8 }, /* sprites */
    { -1 } /* end of array */
};

static struct GfxDecodeInfo bootleg_gfxdecodeinfo[] =
{
    { 1, 0x00000, &bootleg_charlayout,   256, 16 }, /* chars */
    { 1, 0x20000, &bootleg_spritelayout,   0,  8 }, /* sprites */
    { -1 } /* end of array */
};

/***************************************************************************/

static struct YM2203interface ym2203_interface =
{
	1,      /* 1 chip */
	1500000,        /* 1.5 MHz ??? */
	{ YM2203_VOL(255,255) },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 }
};

static struct YM3812interface ym3812_interface =
{
	1,		/* 1 chip (no more supported) */
	3600000,	/* 3.600000 MHz ??? */
	{ 255 }         /* (not supported) */
};

static struct MSM5205interface msm5205_interface =
{
	1,		/* 1 chip */
	8000,	/* 8000Hz playback ? */
	pcktgal_adpcm_int,		/* interrupt function */
	{ 255 }
};

/***************************************************************************/

static struct MachineDriver machine_driver =
{
	/* basic machine hardware */
	{
		{
			CPU_M6502,
			2000000,
			0,
			readmem,writemem,0,0,
			nmi_interrupt,1
		},
		{
			CPU_M6502 | CPU_AUDIO_CPU,
			2000000,
			2,
			sound_readmem,sound_writemem,0,0,
			ignore_interrupt,0
							/* IRQs are caused by the ADPCM chip */
							/* NMIs are caused by the main CPU */
		}
	},
	60, DEFAULT_60HZ_VBLANK_DURATION,  /* frames per second, vblank duration */
	1, /* 1 CPU slice per frame - interleaving is forced when a sound command is written */
	0,

	/* video hardware */
	32*8, 32*8, { 0*8, 32*8-1, 2*8, 30*8-1 },
	gfxdecodeinfo,
	512, 512,
	ghostb_vh_convert_color_prom,

	VIDEO_TYPE_RASTER | VIDEO_SUPPORTS_DIRTY,
	0,
	generic_vh_start,
	generic_vh_stop,
	pcktgal_vh_screenrefresh,

	/* sound hardware */
	0,0,0,0,
	{
	    {
	        SOUND_YM2203,
	        &ym2203_interface
	    },
	    {
	        SOUND_YM3812,
	        &ym3812_interface
		},
		{
			SOUND_MSM5205,
			&msm5205_interface
	    }
	}
};

static struct MachineDriver bootleg_machine_driver =
{
	/* basic machine hardware */
	{
		{
			CPU_M6502,
			2000000,
			0,
			readmem,writemem,0,0,
			nmi_interrupt,1
		},
		{
			CPU_M6502 | CPU_AUDIO_CPU,
			2000000,
			2,
			sound_readmem,sound_writemem,0,0,
			ignore_interrupt,0
							/* IRQs are caused by the ADPCM chip */
							/* NMIs are caused by the main CPU */
		}
	},
	60, DEFAULT_60HZ_VBLANK_DURATION,  /* frames per second, vblank duration */
	1, /* 1 CPU slice per frame - interleaving is forced when a sound command is written */
	0,

	/* video hardware */
	32*8, 32*8, { 0*8, 32*8-1, 2*8, 30*8-1 },
	bootleg_gfxdecodeinfo,
	512, 512,
	ghostb_vh_convert_color_prom,

	VIDEO_TYPE_RASTER | VIDEO_SUPPORTS_DIRTY,
	0,
	generic_vh_start,
	generic_vh_stop,
	pcktgal_vh_screenrefresh,

	/* sound hardware */
	0,0,0,0,
	{
	    {
	        SOUND_YM2203,
	        &ym2203_interface
	    },
	    {
	        SOUND_YM3812,
	        &ym3812_interface
		},
		{
			SOUND_MSM5205,
			&msm5205_interface
	    }
	}
};

/***************************************************************************/

ROM_START( pcktgal_rom )
    ROM_REGION(0x14000)     /* 64k for code + 16k for banks */
    ROM_LOAD( "eb04.bin",     0x10000, 0x4000, 0x8215d60d )
	ROM_CONTINUE(             0x04000, 0xc000)
	/* 4000-7fff is banked but code falls through from 7fff to 8000, so */
	/* I have to load the bank directly at 4000. */

    ROM_REGION_DISPOSE(0x30000)
    ROM_LOAD( "eb01.bin",     0x00000, 0x10000, 0x63542c3d )
    ROM_LOAD( "eb02.bin",     0x10000, 0x10000, 0xa9dcd339 )
    ROM_LOAD( "ebb0.bin",     0x20000, 0x10000, 0x6c1a14a8 )

    ROM_REGION(0x10000)     /* 64k for the audio cpu */
	ROM_LOAD( "eb03.bin",     0x0000, 0x10000, 0xcb029b02 )

	ROM_REGION_DISPOSE(0x0400)	/* color PROMs */
	ROM_LOAD( "82s147.084",   0x0000, 0x0200, 0x3b6198cb )
	ROM_LOAD( "82s131.101",   0x0200, 0x0200, 0x1fbd4b59 )
ROM_END

ROM_START( sexybilb_rom )
    ROM_REGION(0x14000)     /* 64k for code + 16k for banks */
    ROM_LOAD( "pcktgal.001", 0x10000, 0x4000, 0x4acb3e84 )
	ROM_CONTINUE(             0x04000, 0xc000)
	/* 4000-7fff is banked but code falls through from 7fff to 8000, so */
	/* I have to load the bank directly at 4000. */

    ROM_REGION_DISPOSE(0x30000)
    ROM_LOAD( "pcktgal.005", 0x00000, 0x10000, 0x3128dc7b )
    ROM_LOAD( "pcktgal.006", 0x10000, 0x10000, 0x0fc91eeb )
    ROM_LOAD( "pcktgal.003", 0x20000, 0x08000, 0x58182daa )
    ROM_LOAD( "pcktgal.004", 0x28000, 0x08000, 0x33a67af6 )

    ROM_REGION(0x10000)     /* 64k for the audio cpu */
	ROM_LOAD( "eb03.bin",     0x0000, 0x10000, 0xcb029b02 )

	ROM_REGION_DISPOSE(0x0400)	/* color PROMs */
	ROM_LOAD( "82s147.084",   0x0000, 0x0200, 0x3b6198cb )
	ROM_LOAD( "82s131.101",   0x0200, 0x0200, 0x1fbd4b59 )
ROM_END

/***************************************************************************/

static void deco222_decode(void)
{
	int A;
	unsigned char *RAM;
	extern int encrypted_cpu;


	/* bits 5 and 6 of the opcodes are swapped */
	RAM = Machine->memory_region[Machine->drv->cpu[1].memory_region];
	encrypted_cpu = 1;
	for (A = 0;A < 0x10000;A++)
		ROM[A] = (RAM[A] & 0x9f) | ((RAM[A] & 0x20) << 1) | ((RAM[A] & 0x40) >> 1);
}

static void pcktgal_decode(void)
{
	unsigned char *RAM = Machine->memory_region[1];
	int i,j;
	int temp[16];


	deco222_decode();

	/* Tile graphics roms have some swapped lines, original version only */
	for (i=0x00000; i < 0x20000; i+=32)
	{
		for (j=0; j<16; j++)
		{
			temp[j]=RAM[i+j+16];
			RAM[i+j+16]=RAM[i+j];
			RAM[i+j]=temp[j];
		}
	}
}

/***************************************************************************/

struct GameDriver pcktgal_driver =
{
	__FILE__,
	0,
	"pcktgal",
	"Pocket Gal",
	"1987",
	"Data East Corporation",
	"Bryan McPhail\nNicola Salmoria",
	0,
	&machine_driver,
	0,

	pcktgal_rom,
	0, pcktgal_decode,
	0,
	0,      /* sound_prom */

	input_ports,

	PROM_MEMORY_REGION(3), 0, 0,
	ORIENTATION_DEFAULT,

	0, 0
};

struct GameDriver pcktgalb_driver =
{
	__FILE__,
	&pcktgal_driver,
	"pcktgalb",
	"Pocket Gal (bootleg)",
	"1989",
	"bootleg",
	"Bryan McPhail\nNicola Salmoria",
	0,
	&bootleg_machine_driver,
	0,

	sexybilb_rom,
	0, deco222_decode,
	0,
	0,

	input_ports,

	PROM_MEMORY_REGION(3), 0, 0,
	ORIENTATION_DEFAULT,

	0, 0
};
