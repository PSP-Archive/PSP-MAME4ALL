/**************************************************************************

Based on drivers from Juno First emulator by Chris Hardy (chris@junofirst.freeserve.co.uk)

***************************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"
#include "m6809/m6809.h"
#include "i8039/i8039.h"
#include "z80/z80.h"

extern unsigned char *megazone_scrollx;
extern unsigned char *megazone_scrolly;

static unsigned char *megazone_sharedram;

static int i8039_irqenable;
static int i8039_status;

void megazone_flipscreen_w(int offset,int data);
void megazone_vh_convert_color_prom(unsigned char *palette, unsigned short *colortable,const unsigned char *color_prom);
void megazone_sprite_bank_select_w(int offset, int data);
void megazone_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh);

unsigned char KonamiDecode( unsigned char opcode, unsigned short address );

void megazone_init_machine(void)
{
	/* Set optimization flags for M6809 */
	m6809_Flags = M6809_FAST_S | M6809_FAST_U;
}

int megazone_sh_timer_r(int offset)
{
	int timer;

	timer = (cpu_gettotalcycles() / 1024) & 0x0f;

	/* low three bits come from the 8039 */
	return (timer & 0xF)<<4 | i8039_status;
}

static void megazone_portB_w(int offset,int data)
{
	int i;


	for (i = 0;i < 3;i++)
	{
		int C;


		C = 0;
		if (data & 1) C += 47000;	/* 47000pF = 0.047uF */
		if (data & 2) C += 220000;	/* 220000pF = 0.22uF */
		data >>= 2;
		set_RC_filter(i,1000,2200,200,C);
	}
}


int megazone_sharedram_r(int offset)
{
	return(megazone_sharedram[offset]);
}

void megazone_sharedram_w(int offset,int data)
{
	megazone_sharedram[offset] = data;
}

static void megazone_i8039_irq_w(int offset,int data)
{
	if (i8039_irqenable)
		cpu_cause_interrupt(2,I8039_EXT_INT);
}

void i8039_irqen_and_status_w(int offset,int data)
{
	i8039_irqenable = data & 0x80;
	i8039_status = (data & 0x70) >> 4;
}

static struct MemoryReadAddress readmem[] =
{
	{ 0x2000, 0x33ff, MRA_RAM },
	{ 0x6000, 0xffff, MRA_ROM },
	{ 0x3800, 0x3fff, megazone_sharedram_r },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress writemem[] =
{
	{ 0x0007, 0x0007, interrupt_enable_w },
	{ 0x0800, 0x0800, watchdog_reset_w },

	{ 0x3800, 0x3fff, megazone_sharedram_w },

	{ 0x1800, 0x1800, MWA_RAM, &megazone_scrollx },
	{ 0x1000, 0x1000, MWA_RAM, &megazone_scrolly },
	{ 0x2000, 0x27ff, videoram_w, &videoram, &videoram_size },
	{ 0x2800, 0x2fff, colorram_w, &colorram },
	{ 0x3000, 0x37ff, MWA_RAM, &spriteram, &spriteram_size },
	{ 0x6000, 0xffff, MWA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryReadAddress sound_readmem[] =
{
	{ 0x0000, 0x1fff, MRA_ROM },
	{ 0x4000, 0x43ff, MRA_RAM },
	{ 0xe000, 0xe7ff, megazone_sharedram_r, &megazone_sharedram },  /* Shared with $3800->3fff of main CPU */

	{ 0x6000, 0x6000, input_port_0_r }, /* IO Coin */
	{ 0x6001, 0x6001, input_port_1_r }, /* P1 IO */
	{ 0x6002, 0x6002, input_port_2_r }, /* P2 IO */
	{ 0x8001, 0x8001, input_port_3_r }, /* DIP 1 */
	{ 0x8000, 0x8000, input_port_4_r }, /* DIP 2 */

	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress sound_writemem[] =
{
	{ 0x0000, 0x1fff, MWA_ROM },
	{ 0xe000, 0xe7ff, megazone_sharedram_w },	/* Shared with $3800->3fff of main CPU */

	{ 0x2000, 0x2000, megazone_i8039_irq_w },	/* START line. Interrupts 8039 */
	{ 0x4000, 0x4000, soundlatch_w },			/* CODE  line. Command Interrupts 8039 */
	{ 0xA000, 0xA000, MWA_RAM },				/* INTMAIN - Interrupts main CPU (unused) */
	{ 0xC000, 0xC000, MWA_RAM },				/* INT (Actually is NMI) enable/disable (unused)*/
	{ 0xC001, 0xC001, watchdog_reset_w },

	{ -1 }  /* end of table */
};

static struct IOReadPort sound_readport[] =
{
	{ 0x00, 0x02, AY8910_read_port_0_r },
	{ -1 }
};

static struct IOWritePort sound_writeport[] =
{
	{ 0x00, 0x00, AY8910_control_port_0_w },
	{ 0x02, 0x02, AY8910_write_port_0_w },
	{ -1 }	/* end of table */
};

static struct MemoryReadAddress i8039_readmem[] =
{
	{ 0x0000, 0x0fff, MRA_ROM },
	{ -1 }	/* end of table */
};

static struct MemoryWriteAddress i8039_writemem[] =
{
	{ 0x0000, 0x0fff, MWA_ROM },
	{ -1 }	/* end of table */
};

static struct IOReadPort i8039_readport[] =
{
	{ 0x00, 0xff, soundlatch_r },
	{ 0x111,0x111, IORP_NOP },
	{ -1 }
};

static struct IOWritePort i8039_writeport[] =
{
	{ I8039_p1, I8039_p1, DAC_data_w },
	{ I8039_p2, I8039_p2, i8039_irqen_and_status_w },
	{ -1 }	/* end of table */
};

INPUT_PORTS_START( input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_COCKTAIL)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_COCKTAIL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_START      /* DSW0 */

	PORT_DIPNAME( 0x0f, 0x0f, "Coin_A", IP_KEY_NONE )
	PORT_DIPSETTING(    0x02, "4C_1C" )
	PORT_DIPSETTING(    0x05, "3C_1C" )
	PORT_DIPSETTING(    0x08, "2C_1C" )
	PORT_DIPSETTING(    0x04, "3C_2C" )
	PORT_DIPSETTING(    0x01, "4C_3C" )
	PORT_DIPSETTING(    0x0f, "1C_1C" )
	PORT_DIPSETTING(    0x03, "3C_4C" )
	PORT_DIPSETTING(    0x07, "2C_3C" )
	PORT_DIPSETTING(    0x0e, "1C_2C" )
	PORT_DIPSETTING(    0x06, "2C_5C" )
	PORT_DIPSETTING(    0x0d, "1C_3C" )
	PORT_DIPSETTING(    0x0c, "1C_4C" )
	PORT_DIPSETTING(    0x0b, "1C_5C" )
	PORT_DIPSETTING(    0x0a, "1C_6C" )
	PORT_DIPSETTING(    0x09, "1C_7C" )
	PORT_DIPSETTING(    0x00, "Free_Play" )
	PORT_DIPNAME( 0xf0, 0xf0, "Coin_B", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "4C_1C" )
	PORT_DIPSETTING(    0x50, "3C_1C" )
	PORT_DIPSETTING(    0x80, "2C_1C" )
	PORT_DIPSETTING(    0x40, "3C_2C" )
	PORT_DIPSETTING(    0x10, "4C_3C" )
	PORT_DIPSETTING(    0xf0, "1C_1C" )
	PORT_DIPSETTING(    0x30, "3C_4C" )
	PORT_DIPSETTING(    0x70, "2C_3C")
	PORT_DIPSETTING(    0xe0, "1C_2C" )
	PORT_DIPSETTING(    0x60, "2C_5C" )
	PORT_DIPSETTING(    0xd0, "1C_3C" )
	PORT_DIPSETTING(    0xc0, "1C_4C" )
	PORT_DIPSETTING(    0xb0, "1C_5C" )
	PORT_DIPSETTING(    0xa0, "1C_6C" )
	PORT_DIPSETTING(    0x90, "1C_7C" )
	PORT_DIPSETTING(    0x00, "Free_Play" )

	PORT_START      /* DSW1 */
	PORT_DIPNAME( 0x03, 0x03, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPNAME( 0x04, 0x00, "Cabinet", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Upright" )
	PORT_DIPSETTING(    0x04, "Cocktail" )
	PORT_DIPNAME( 0x18, 0x08, "Bonus_Life", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "30000 100000" )
	PORT_DIPSETTING(    0x08, "30000 90000" )
	PORT_DIPSETTING(    0x18, "20000 80000" )
	PORT_DIPSETTING(    0x18, "20000 70000" )


	PORT_DIPNAME( 0x60, 0x40, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Very Difficult" )
	PORT_DIPSETTING(    0x20, "Difficult" )
	PORT_DIPSETTING(    0x40, "Normal" )
	PORT_DIPSETTING(    0x60, "Easy" )

	PORT_DIPNAME( 0x80, 0x00, "Demo_Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
INPUT_PORTS_END



static struct GfxLayout charlayout =
{
	8,8,    /* 8*8 characters */
	512,    /* 512 characters */
	4,      /* 4 bits per pixel */
	{ 0, 1, 2, 3 }, /* the four bitplanes are packed in one nibble */
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8    /* every char takes 8 consecutive bytes */
};

static struct GfxLayout spritelayout =
{
	16,16,	/* 16*16 sprites */
	256,	/* 256 sprites */
	4,	/* 4 bits per pixel */
	{ 0x4000*8+4, 0x4000*8+0, 4, 0 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 ,
		32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8, },
	64*8	/* every sprite takes 64 consecutive bytes */
};

static struct GfxDecodeInfo gfxdecodeinfo[] =
{
	{ 1, 0x0000, &charlayout,       0, 16 },
	{ 1, 0x4000, &spritelayout, 16*16, 16 },
	{ -1 } /* end of array */
};

static struct AY8910interface ay8910_interface =
{
	1,	/* 1 chip */
	14318000/8,	/* 1.78975 MHz */
	{ 30 },
	{ megazone_sh_timer_r },
	{ 0 },
	{ 0 },
	{ megazone_portB_w }
};

static struct DACinterface dac_interface =
{
	1,
	{ 50 }
};

static struct MachineDriver machine_driver =
{
	/* basic machine hardware */
	{
		{
			CPU_M6809,
			2048000,        /* 2 Mhz */
			0,
			readmem,writemem,0,0,
			interrupt,1
		},
		{
			CPU_Z80,
			18420000/6,     /* Z80 Clock is derived from the H1 signal */
			3,      /* memory region #3 */
			sound_readmem,sound_writemem,sound_readport,sound_writeport,
			interrupt,1
		},
		{
			CPU_I8039 | CPU_AUDIO_CPU,
			14318000/2/15,	/* 1/2 14MHz crystal */
			4,	/* memory region #4 */
			i8039_readmem,i8039_writemem,i8039_readport,i8039_writeport,
			ignore_interrupt,1
		}
	},
	60, DEFAULT_60HZ_VBLANK_DURATION,       /* frames per second, vblank duration */
	15,      /* 1 CPU slice per frame - interleaving is forced when a sound command is written */
	megazone_init_machine,

	/* video hardware */
	32*8, 32*8, { 0*8, 32*8-1, 2*8, 30*8-1 },
	gfxdecodeinfo,
	32,16*16+16*16,
	megazone_vh_convert_color_prom,

	VIDEO_TYPE_RASTER,
	0,
	generic_vh_start,
	generic_vh_stop,
	megazone_vh_screenrefresh,

	/* sound hardware */
	0,0,0,0,
	{
		{
			SOUND_AY8910,
			&ay8910_interface
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

ROM_START( megazone_rom )
	ROM_REGION(0x10000)     /* 64k for code */
	ROM_LOAD( "ic59_cpu.bin",  0x6000, 0x2000, 0xf41922a0 )
	ROM_LOAD( "ic58_cpu.bin",  0x8000, 0x2000, 0x7fd7277b )
	ROM_LOAD( "ic57_cpu.bin",  0xa000, 0x2000, 0xa4b33b51 )
	ROM_LOAD( "ic56_cpu.bin",  0xc000, 0x2000, 0x2aabcfbf )
	ROM_LOAD( "ic55_cpu.bin",  0xe000, 0x2000, 0xb33a3c37 )

	ROM_REGION_DISPOSE(0x10000)    /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "ic40_vid.bin",  0x0000, 0x2000, 0x07b8b24b )
	ROM_LOAD( "ic58_vid.bin",  0x2000, 0x2000, 0x3d8f3743 )
	ROM_LOAD( "ic15_vid.bin",  0x4000, 0x2000, 0x965a7ff6 )
	ROM_LOAD( "ic05_vid.bin",  0x6000, 0x2000, 0x5eaa7f3e )
	ROM_LOAD( "ic14_vid.bin",  0x8000, 0x2000, 0x7bb1aeee )
	ROM_LOAD( "ic04_vid.bin",  0xa000, 0x2000, 0x6add71b1 )

	ROM_REGION(0x220)      /* color proms */
	ROM_LOAD( "319b18.a16",  0x0000, 0x020, 0x23cb02af ) /* palette */
	ROM_LOAD( "319b16.c6",   0x0020, 0x100, 0x5748e933 ) /* sprite lookup table */
	ROM_LOAD( "319b17.a11",  0x0120, 0x100, 0x1fbfce73 ) /* character lookup table */

	ROM_REGION(0x10000)     /* 64k for the audio CPU */
	ROM_LOAD( "ic25_cpu.bin", 0x0000, 0x2000, 0xd5d45edb )

	ROM_REGION(0x1000)     /* 4k for the 8039 DAC CPU */
	ROM_LOAD( "ic02_cpu.bin", 0x0000, 0x1000, 0xed5725a0 )
ROM_END


static void megazone_decode(void)
{
	int A;
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];


	for (A = 0x6000;A < 0x10000;A++)
	{
		ROM[A] = KonamiDecode(RAM[A],A);
	}
}

struct GameDriver megazone_driver =
{
	__FILE__,
	0,
	"megazone",
	"Mega Zone",
	"1983",
	"Konami / Interlogic + Kosuka",
	"Chris Hardy",
	0,
	&machine_driver,
	0,

	megazone_rom,
	0, megazone_decode,
	0,
	0,      /* sound_prom */

	input_ports,

	PROM_MEMORY_REGION(2), 0, 0,
	ORIENTATION_ROTATE_90,

	0,0 /* hiload, hisave */
};
