/***************************************************************************

	Prehistoric Isle in 1930 (World)		(c) 1989 SNK
	Prehistoric Isle in 1930 (USA)			(c) 1989 SNK
	Genshi-Tou 1930's (Japan)				(c) 1989 SNK


	Ikari 3 should run on hardware close to this.

 	Emulation by Bryan McPhail, mish@tendril.force9.net

***************************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"
#include "z80/z80.h"

void prehisle_vh_screenrefresh(struct osd_bitmap *bitmap, int full_refresh);
void prehisle_video_w(int offset,int data);
int prehisle_control_r(int offset);
void prehisle_control_w(int offset,int data);
int prehisle_video_r(int offset);
void prehisle_vh_stop (void);
int prehisle_vh_start (void);

static unsigned char *prehisle_ram;
extern unsigned char *prehisle_video;

/******************************************************************************/

static void prehisle_sound_w(int offset, int data)
{
	soundlatch_w(0,data&0xff);
	cpu_cause_interrupt(1,Z80_NMI_INT);
}

/*******************************************************************************/

static struct MemoryReadAddress prehisle_readmem[] =
{
	{ 0x000000, 0x03ffff, MRA_ROM },
	{ 0x070000, 0x073fff, MRA_BANK1 },
	{ 0x090000, 0x0907ff, MRA_BANK4 },
	{ 0x0a0000, 0x0a07ff, MRA_BANK5 },
	{ 0x0b0000, 0x0b3fff, prehisle_video_r },
	{ 0x0d0000, 0x0d07ff, paletteram_word_r },
	{ 0x0e0000, 0x0e00ff, prehisle_control_r },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress prehisle_writemem[] =
{
	{ 0x000000, 0x03ffff, MWA_ROM },
	{ 0x070000, 0x073fff, MWA_BANK1, &prehisle_ram },
	{ 0x090000, 0x0907ff, MWA_BANK4, &videoram },
	{ 0x0a0000, 0x0a07ff, MWA_BANK5, &spriteram },
	{ 0x0b0000, 0x0b3fff, prehisle_video_w, &prehisle_video },
	{ 0x0d0000, 0x0d07ff, paletteram_RRRRGGGGBBBBxxxx_word_w, &paletteram },
	{ 0x0f0070, 0x0ff071, prehisle_sound_w },
	{ 0x0f0000, 0x0ff0ff, prehisle_control_w },
	{ -1 }  /* end of table */
};

/******************************************************************************/

static void D7759_write_port_0_w(int offset, int data)
{
	UPD7759_reset_w (0,0);
	UPD7759_message_w(offset,data);
	UPD7759_start_w (0,1);
}

static struct MemoryReadAddress prehisle_sound_readmem[] =
{
	{ 0x0000, 0xefff, MRA_ROM },
	{ 0xf000, 0xf7ff, MRA_RAM },
	{ 0xf800, 0xf800, soundlatch_r },
	{ -1 }	/* end of table */
};

static struct MemoryWriteAddress prehisle_sound_writemem[] =
{
	{ 0x0000, 0xefff, MWA_ROM },
	{ 0xf000, 0xf7ff, MWA_RAM },
	{ -1 }	/* end of table */
};

static struct IOReadPort prehisle_sound_readport[] =
{
	{ 0x00, 0x00, YM3812_status_port_0_r },
	{ -1 }
};

static struct IOWritePort prehisle_sound_writeport[] =
{
	{ 0x00, 0x00, YM3812_control_port_0_w },
	{ 0x20, 0x20, YM3812_write_port_0_w },
	{ 0x40, 0x40, D7759_write_port_0_w},
	{ 0x80, 0x80, MWA_NOP }, /* IRQ ack? */
	{ -1 }
};

/******************************************************************************/

INPUT_PORTS_START( prehisle_input_ports )
	PORT_START	/* Player 1 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START	/* Player 2 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2  )

	PORT_START	/* coin */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BITX(0x08, IP_ACTIVE_LOW, IPT_SERVICE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* Dip switches */
	PORT_DIPNAME( 0x01, 0x01, "Flip Screen", IP_KEY_NONE )
	PORT_DIPSETTING(    0x01, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x02, 0x02, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x02, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x04, 0x04, "Bonus Life", IP_KEY_NONE )
	PORT_DIPSETTING(    0x04, "Only Twice" )
	PORT_DIPSETTING(    0x00, "Always" )
	PORT_DIPNAME( 0x08, 0x08, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x30, 0x30, "Coin A", IP_KEY_NONE )
	PORT_DIPSETTING(    0x30, "C 1 C 1" )
	PORT_DIPSETTING(    0x20, "A 2/1 B 1/2" )
	PORT_DIPSETTING(    0x10, "A 3/1 B 1/3" )
	PORT_DIPSETTING(    0x00, "A 4/1 B 1/4" )
	PORT_DIPNAME( 0xc0, 0xc0, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x00, "5" )

	PORT_START
	PORT_DIPNAME( 0x03, 0x03, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x02, "Easy" )
	PORT_DIPSETTING(    0x03, "Normal" )
	PORT_DIPSETTING(    0x01, "Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0x0c, 0x0c, "Demo", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Sound Off" )
	PORT_DIPSETTING(    0x0c, "Sound On" )
	PORT_DIPSETTING(    0x00, "Freeze" )
	PORT_BITX( 0,       0x04, IPT_DIPSWITCH_SETTING | IPF_CHEAT, "Infinite Lives", IP_KEY_NONE, IP_JOY_NONE, 0 )
	PORT_DIPNAME( 0x30, 0x30, "Bonus Life", IP_KEY_NONE )
	PORT_DIPSETTING(    0x30, "100000 200000" )
	PORT_DIPSETTING(    0x20, "150000 300000" )
	PORT_DIPSETTING(    0x10, "300000 500000" )
	PORT_DIPSETTING(    0x00, "None" )
	PORT_DIPNAME( 0x40, 0x40, "Allow Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "No" )
	PORT_DIPSETTING(    0x40, "Yes" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK )
INPUT_PORTS_END

/******************************************************************************/

static struct GfxLayout charlayout =
{
	8,8,    /* 8*8 characters */
	1024,
	4,      /* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 0, 4, 8, 12, 16, 20, 24, 28},
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8    /* every char takes 32 consecutive bytes */
};

static struct GfxLayout tilelayout =
{
	16,16,	/* 16*16 sprites */
	0x800,
	4,	/* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 0,4,8,12,16,20,24,28,
		0+64*8,4+64*8,8+64*8,12+64*8,16+64*8,20+64*8,24+64*8,28+64*8 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
		8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	128*8	/* every sprite takes 64 consecutive bytes */
};

static struct GfxLayout spritelayout =
{
	16,16,	/* 16*16 sprites */
	5120,
	4,	/* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 0,4,8,12,16,20,24,28,
		0+64*8,4+64*8,8+64*8,12+64*8,16+64*8,20+64*8,24+64*8,28+64*8 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
		8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	128*8	/* every sprite takes 64 consecutive bytes */
};

static struct GfxDecodeInfo gfxdecodeinfo[] =
{
	{ 1, 0x000000, &charlayout,   0, 16 },
	{ 1, 0x008000, &tilelayout, 768, 16 },
	{ 1, 0x048000, &tilelayout, 512, 16 },
	{ 1, 0x088000, &spritelayout, 256, 16 },
	{ -1 } /* end of array */
};

/******************************************************************************/

static struct YM3812interface ym3812_interface =
{
	1,			/* 1 chip (no more supported) */
	4000000,	/* 4 Mhz */
	{ 15 }		/* 20 (not supported) */
};

static struct UPD7759_interface upd7759_interface =
{
	1,		/* number of chips */
	UPD7759_STANDARD_CLOCK,
	{ 50 }, /* volume 75 */
	3,		/* memory region */
	UPD7759_STANDALONE_MODE,		/* chip mode */
	{0}
};

/******************************************************************************/

static struct MachineDriver prehisle_machine_driver =
{
	/* basic machine hardware */
	{
 		{
			CPU_M68000,
			12000000,
			0,
			prehisle_readmem,prehisle_writemem,0,0,
			m68_level4_irq,1
		},
		{
			CPU_Z80 | CPU_AUDIO_CPU,
			4000000,
			2,
			prehisle_sound_readmem,prehisle_sound_writemem,
			prehisle_sound_readport,prehisle_sound_writeport,
			interrupt,1
		}
	},
	60, DEFAULT_REAL_60HZ_VBLANK_DURATION,
	1,
	0,

	/* video hardware */
	32*8, 32*8, { 0*8, 32*8-1, 2*8, 30*8-1 },

	gfxdecodeinfo,
	1024, 1024,
	0,

	VIDEO_TYPE_RASTER | VIDEO_MODIFIES_PALETTE,
	0,
	prehisle_vh_start,
	prehisle_vh_stop,
	prehisle_vh_screenrefresh,

	/* sound hardware */
	0,0,0,0,
	{
		{
			SOUND_UPD7759,
			&upd7759_interface
		},
		{
			SOUND_YM3812,
			&ym3812_interface
		}
	}
};

/******************************************************************************/

ROM_START( prehisle_rom )
	ROM_REGION(0x40000)
	ROM_LOAD_EVEN( "gt.2", 0x00000, 0x20000, 0x7083245a )
	ROM_LOAD_ODD ( "gt.3", 0x00000, 0x20000, 0x6d8cdf58 )

	ROM_REGION_DISPOSE(0x128000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "gt15.b15",   0x000000, 0x08000, 0xac652412 )
	ROM_LOAD( "pi8914.b14", 0x008000, 0x40000, 0x207d6187 )
	ROM_LOAD( "pi8916.h16", 0x048000, 0x40000, 0x7cffe0f6 )
	ROM_LOAD( "pi8910.k14", 0x088000, 0x80000, 0x5a101b0b )
	ROM_LOAD( "gt.5",       0x108000, 0x20000, 0x3d3ab273 )

	ROM_REGION(0x10000)	/* Sound CPU */
	ROM_LOAD( "gt.1",  0x000000, 0x10000, 0x80a4c093 )

	ROM_REGION(0x20000)	/* ADPCM samples */
	ROM_LOAD( "gt.4",  0x000000, 0x20000, 0x85dfb9ec )

	ROM_REGION(0x10000) /* Tile map */
	ROM_LOAD( "gt.11",  0x000000, 0x10000, 0xb4f0fcf0 )
ROM_END

ROM_START( prehislu_rom )
	ROM_REGION(0x40000)
	ROM_LOAD_EVEN( "gt-u2.2h", 0x00000, 0x20000, 0xa14f49bb )
	ROM_LOAD_ODD ( "gt-u3.3h", 0x00000, 0x20000, 0xf165757e )

	ROM_REGION_DISPOSE(0x128000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "gt15.b15",   0x000000, 0x08000, 0xac652412 )
	ROM_LOAD( "pi8914.b14", 0x008000, 0x40000, 0x207d6187 )
	ROM_LOAD( "pi8916.h16", 0x048000, 0x40000, 0x7cffe0f6 )
	ROM_LOAD( "pi8910.k14", 0x088000, 0x80000, 0x5a101b0b )
	ROM_LOAD( "gt.5",       0x108000, 0x20000, 0x3d3ab273 )

	ROM_REGION(0x10000)	/* Sound CPU */
	ROM_LOAD( "gt.1",  0x000000, 0x10000, 0x80a4c093 )

	ROM_REGION(0x20000)	/* ADPCM samples */
	ROM_LOAD( "gt.4",  0x000000, 0x20000, 0x85dfb9ec )

	ROM_REGION(0x10000) /* Tile map */
	ROM_LOAD( "gt.11",  0x000000, 0x10000, 0xb4f0fcf0 )
ROM_END

ROM_START( prehislj_rom )
	ROM_REGION(0x40000)
	ROM_LOAD_EVEN( "gt2j.bin", 0x00000, 0x20000, 0xa2da0b6b )
	ROM_LOAD_ODD ( "gt3j.bin", 0x00000, 0x20000, 0xc1a0ae8e )

	ROM_REGION_DISPOSE(0x128000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "gt15.b15",   0x000000, 0x08000, 0xac652412 )
	ROM_LOAD( "pi8914.b14", 0x008000, 0x40000, 0x207d6187 )
	ROM_LOAD( "pi8916.h16", 0x048000, 0x40000, 0x7cffe0f6 )
	ROM_LOAD( "pi8910.k14", 0x088000, 0x80000, 0x5a101b0b )
	ROM_LOAD( "gt.5",       0x108000, 0x20000, 0x3d3ab273 )

	ROM_REGION(0x10000)	/* Sound CPU */
	ROM_LOAD( "gt.1",  0x000000, 0x10000, 0x80a4c093 )

	ROM_REGION(0x20000)	/* ADPCM samples */
	ROM_LOAD( "gt.4",  0x000000, 0x20000, 0x85dfb9ec )

	ROM_REGION(0x10000) /* Tile map */
	ROM_LOAD( "gt.11",  0x000000, 0x10000, 0xb4f0fcf0 )
ROM_END

/******************************************************************************/

static int world_cycle_r(int offset)
{
	int pc=cpu_getpc();
	int ret=READ_WORD(&prehisle_ram[0x24]);

	if ((ret&0x8000) && (pc==0x260c || pc==0x268a || pc==0x2b0a || pc==0x34a8 || pc==0x6ae4 || pc==0x83ac || pc==0x25ce || pc==0x29c4)) {
		cpu_spinuntil_int();
		return ret&0x7fff;
	}
	return ret;
}

static void world_memory(void)
{
	install_mem_read_handler(0, 0x70024, 0x70025, world_cycle_r);
}

static int usa_cycle_r(int offset)
{
	int pc=cpu_getpc();
	int ret=READ_WORD(&prehisle_ram[0x24]);

	if ((ret&0x8000) && (pc==0x281e || pc==0x28a6 || pc==0x295a || pc==0x2868 || pc==0x8f98 || pc==0x3b1e)) {
		cpu_spinuntil_int();
		return ret&0x7fff;
	}
	return ret;
}

static void usa_memory(void)
{
	install_mem_read_handler(0, 0x70024, 0x70025, usa_cycle_r);
}

static int jap_cycle_r(int offset)
{
	int pc=cpu_getpc();
	int ret=READ_WORD(&prehisle_ram[0x24]);

	if ((ret&0x8000) && (pc==0x34b6 /* Todo! */ )) {
		cpu_spinuntil_int();
		return ret&0x7fff;
	}
	return ret;
}

static void jap_memory(void)
{
	install_mem_read_handler(0, 0x70024, 0x70025, jap_cycle_r);
}

/******************************************************************************/

struct GameDriver prehisle_driver =
{
	__FILE__,
	0,
	"prehisle",
	"Prehistoric Isle in 1930 (World)",
	"1989",
	"SNK Corp.",
	"Bryan McPhail\nCarlos Alberto Lozano Baides",
	0,
	&prehisle_machine_driver,
	world_memory,

	prehisle_rom,
	0, 0,
	0,
	0,

	prehisle_input_ports,

	0, 0, 0,   /* colors, palette, colortable */
	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver prehislu_driver =
{
	__FILE__,
	&prehisle_driver,
	"prehislu",
	"Prehistoric Isle in 1930 (US)",
	"1989",
	"SNK Corp. of America",
	"Bryan McPhail\nCarlos Alberto Lozano Baides",
	0,
	&prehisle_machine_driver,
	usa_memory,

	prehislu_rom,
	0, 0,
	0,
	0,

	prehisle_input_ports,

	0, 0, 0,   /* colors, palette, colortable */
	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver prehislj_driver =
{
	__FILE__,
	&prehisle_driver,
	"gensitou",
	"Genshi-Tou 1930's",
	"1989",
	"SNK Corp.",
	"Bryan McPhail\nCarlos Alberto Lozano Baides",
	0,
	&prehisle_machine_driver,
	jap_memory,

	prehislj_rom,
	0, 0,
	0,
	0,

	prehisle_input_ports,

	0, 0, 0,   /* colors, palette, colortable */
	ORIENTATION_DEFAULT,
	0, 0
};

