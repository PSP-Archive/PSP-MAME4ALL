/***************************************************************************

Pandora's Palace(GX328) (c) 1984 Konami/Interlogic

Driver by Manuel Abadia <manu@teleline.es>

Notes:
	Press 1P and 2P together to enter test mode.

	There is an empty space ($4000-$5fff) on the CPU A. That space probably
	is used for debugging purposes on the real thing. The code checks that
	memory location, and if a ROM is present, it starts to execute code from
	that ROM.

	The AY-8910 has a timer or something like that on port B. I've managed
	to make it sound, but it's not 100% accurate.

***************************************************************************/

#include "driver.h"
#include "m6809/m6809.h"
#include "z80/z80.h"
#include "i8039/i8039.h"
#include "vidhrdw/generic.h"

static int irq_enable_a, irq_enable_b;
static int firq_old_data_a, firq_old_data_b;
static int 	i8039_irqenable;

unsigned char *pandoras_sharedram;
static unsigned char *pandoras_sharedram2;

/* from vidhrdw */
void pandoras_vh_convert_color_prom(unsigned char *palette, unsigned short *colortable,const unsigned char *color_prom);
int pandoras_vram_r (int offset);
int pandoras_cram_r (int offset);
void pandoras_vram_w (int offset, int data);
void pandoras_cram_w (int offset, int data);
void pandoras_flipscreen_w (int offset, int data);
void pandoras_scrolly_w (int offset, int data);
int pandoras_vh_start(void);
void pandoras_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh);

static int pandoras_interrupt_a( void ){
	if (irq_enable_a)
		return M6809_INT_IRQ;
	return ignore_interrupt();
}

static int pandoras_interrupt_b( void ){
	if (irq_enable_b)
		return M6809_INT_IRQ;
	return ignore_interrupt();
}

static int pandoras_sharedram_r (int offset){
	return pandoras_sharedram[offset];
}

static void pandoras_sharedram_w (int offset, int data){
	pandoras_sharedram[offset] = data;
}

static int pandoras_sharedram2_r (int offset){
	return pandoras_sharedram2[offset];
}

static void pandoras_sharedram2_w (int offset, int data){
	pandoras_sharedram2[offset] = data;
}

static void pandoras_int_control_w (int offset, int data){
	/*	byte 0:	irq enable (CPU A)
		byte 2:	coin counter 1
		byte 3: coin counter 2
		byte 5: flip screen
		byte 6:	irq enable (CPU B)
		byte 7:	NMI to CPU B

		other bytes unknown */

	switch (offset){
		case 0x00:	if (!data) //cpu_set_irq_line(0, M6809_IRQ_LINE, CLEAR_LINE);
					cpu_clear_pending_interrupts(0);
					irq_enable_a = data;
					break;
		case 0x02:	coin_counter_w(0,data & 0x01);
					break;
		case 0x03:	coin_counter_w(1,data & 0x01);
					break;
		case 0x05:	pandoras_flipscreen_w(0, data);
					break;
		case 0x06:	if (!data) //cpu_set_irq_line(1, M6809_IRQ_LINE, CLEAR_LINE);
					cpu_clear_pending_interrupts(0);
					irq_enable_b = data;
					break;
		case 0x07:	cpu_cause_interrupt(1,M6809_INT_NMI);
					break;

	}
}

void pandoras_cpua_irqtrigger_w (int offset, int data){
	if (!firq_old_data_a && data){
		cpu_cause_interrupt(0,M6809_INT_FIRQ);
	}

	firq_old_data_a = data;
}

void pandoras_cpub_irqtrigger_w (int offset, int data){
	if (!firq_old_data_b && data){
		cpu_cause_interrupt(1,M6809_INT_FIRQ);
	}

	firq_old_data_b = data;
}

static void i8039_irqen_w (int offset, int data)
{
	/* ??? */
	i8039_irqenable = data & 0x80;
}

void pandoras_z80_irqtrigger_w (int offset, int data)
{
	cpu_cause_interrupt(2,0xff);
}

void pandoras_i8039_irqtrigger_w (int offset, int data)
{
	if (i8039_irqenable)
		cpu_cause_interrupt(3,I8039_EXT_INT);
}

static struct MemoryReadAddress pandoras_readmem_a[] =
{
	{ 0x0000, 0x0fff, pandoras_sharedram_r },	/* Work RAM (Shared with CPU B) */
	{ 0x1000, 0x13ff, pandoras_cram_r },		/* Color RAM (shared with CPU B) */
	{ 0x1400, 0x17ff, pandoras_vram_r },		/* Video RAM (shared with CPU B) */
	{ 0x4000, 0x5fff, MRA_ROM },				/* see notes */
	{ 0x6000, 0x67ff, pandoras_sharedram2_r },	/* Shared RAM with CPU B */
	{ 0x8000, 0xffff, MRA_ROM },				/* ROM */
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress pandoras_writemem_a[] =
{
	{ 0x0000, 0x0fff, pandoras_sharedram_w, &pandoras_sharedram },	/* Work RAM (Shared with CPU B) */
	{ 0x1000, 0x13ff, pandoras_cram_w, &colorram },	/* Color RAM (shared with CPU B) */
	{ 0x1400, 0x17ff, pandoras_vram_w, &videoram },	/* Video RAM (shared with CPU B) */
	{ 0x1800, 0x1807, pandoras_int_control_w },	/* INT control */
	{ 0x1a00, 0x1a00, pandoras_scrolly_w },		/* bg scroll */
	{ 0x1c00, 0x1c00, pandoras_z80_irqtrigger_w },	/* cause INT on the Z80 */
	{ 0x1e00, 0x1e00, soundlatch_w },		/* sound command to the Z80 */
	{ 0x2000, 0x2000, pandoras_cpub_irqtrigger_w },	/* cause FIRQ on CPU B */
	{ 0x2001, 0x2001, watchdog_reset_w },		/* watchdog reset */
	{ 0x4000, 0x5fff, MWA_ROM },	/* see notes */
	{ 0x6000, 0x67ff, pandoras_sharedram2_w, &pandoras_sharedram2 },/* Shared RAM with CPU B */
	{ 0x8000, 0xffff, MWA_ROM },	/* ROM */
	{ -1 }  /* end of table */
};

static struct MemoryReadAddress pandoras_readmem_b[] =
{
	{ 0x0000, 0x0fff, pandoras_sharedram_r },	/* Work RAM (Shared with CPU A) */
	{ 0x1000, 0x13ff, pandoras_cram_r },		/* Color RAM (shared with CPU A) */
	{ 0x1400, 0x17ff, pandoras_vram_r },		/* Video RAM (shared with CPU A) */
	{ 0x1800, 0x1800, input_port_0_r },			/* DIPSW #1 */
	{ 0x1c00, 0x1c00, input_port_1_r },			/* DISPW #2 */
	{ 0x1a00, 0x1a00, input_port_3_r },			/* COINSW */
	{ 0x1a01, 0x1a01, input_port_4_r },			/* 1P inputs */
	{ 0x1a02, 0x1a02, input_port_5_r },			/* 2P inputs */
	{ 0x1a03, 0x1a03, input_port_2_r },			/* DIPSW #3 */
//	{ 0x1e00, 0x1e00, MWA_NOP },				/* ??? */
	{ 0xc000, 0xc7ff, pandoras_sharedram2_r },	/* Shared RAM with the CPU A */
	{ 0xe000, 0xffff, MRA_ROM },				/* ROM */
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress pandoras_writemem_b[] =
{
	{ 0x0000, 0x0fff, pandoras_sharedram_w },	/* Work RAM (Shared with CPU A) */
	{ 0x1000, 0x13ff, pandoras_cram_w },		/* Color RAM (shared with CPU A) */
	{ 0x1400, 0x17ff, pandoras_vram_w },		/* Video RAM (shared with CPU A) */
	{ 0x1800, 0x1807, pandoras_int_control_w },	/* INT control */
	{ 0x8000, 0x8000, watchdog_reset_w },		/* watchdog reset */
	{ 0xa000, 0xa000, pandoras_cpua_irqtrigger_w },/* cause FIRQ on CPU A */
	{ 0xc000, 0xc7ff, pandoras_sharedram2_w },	/* Shared RAM with the CPU A */
	{ 0xe000, 0xffff, MWA_ROM },				/* ROM */
	{ -1 }  /* end of table */
};

static struct MemoryReadAddress pandoras_readmem_snd[] =
{
	{ 0x0000, 0x1fff, MRA_ROM },				/* ROM */
	{ 0x2000, 0x23ff, MRA_RAM },				/* RAM */
	{ 0x4000, 0x4000, soundlatch_r },			/* soundlatch_r */
	{ 0x6001, 0x6001, AY8910_read_port_0_r },	/* AY-8910 */
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress pandoras_writemem_snd[] =
{
	{ 0x0000, 0x1fff, MWA_ROM },				/* ROM */
	{ 0x2000, 0x23ff, MWA_RAM },				/* RAM */
	{ 0x6000, 0x6000, AY8910_control_port_0_w },/* AY-8910 */
	{ 0x6002, 0x6002, AY8910_write_port_0_w },	/* AY-8910 */
	{ 0x8000, 0x8000, pandoras_i8039_irqtrigger_w },/* cause INT on the 8039 */
	{ 0xa000, 0xa000, soundlatch2_w },			/* sound command to the 8039 */
	{ -1 }  /* end of table */
};

static struct MemoryReadAddress i8039_readmem[] =
{
	{ 0x0000, 0x0fff, MRA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress i8039_writemem[] =
{
	{ 0x0000, 0x0fff, MWA_ROM },
	{ -1 }  /* end of table */
};

static struct IOReadPort i8039_readport[] =
{
	{ 0x00, 0xff, soundlatch2_r },
	{ -1 }  /* end of table */
};

static struct IOWritePort i8039_writeport[] =
{
	{ 0x101, 0x101, DAC_0_data_w },
	{ 0x102, 0x102, i8039_irqen_w },
	{ -1 }  /* end of table */
};

/***************************************************************************

	Input Ports

***************************************************************************/

INPUT_PORTS_START( pandoras_input_ports )
	PORT_START	/* DSW #1 */
	PORT_DIPNAME( 0x0f, 0x0f, "Coin A" ,IP_KEY_NONE )
	PORT_DIPSETTING(    0x02, "4 Cn/1 Crd" )
	PORT_DIPSETTING(    0x05, "3 Cn/1 Crd" )
	PORT_DIPSETTING(    0x08, "2 Cn/1 Crd" )
	PORT_DIPSETTING(    0x04, "3 Cn/2 Crd" )
	PORT_DIPSETTING(    0x01, "4 Cn/3 Crd" )
	PORT_DIPSETTING(    0x0f, "1 Cn/1 Crd" )
	PORT_DIPSETTING(    0x03, "3 Cn/4 Crd" )
	PORT_DIPSETTING(    0x07, "2 Cn/3 Crd" )
	PORT_DIPSETTING(    0x0e, "1 Cn/2 Crd" )
	PORT_DIPSETTING(    0x06, "2 Cn/5 Crd" )
	PORT_DIPSETTING(    0x0d, "1 Cn/3 Crd" )
	PORT_DIPSETTING(    0x0c, "1 Cn/4 Crd" )
	PORT_DIPSETTING(    0x0b, "1 Cn/5 Crd" )
	PORT_DIPSETTING(    0x0a, "1 Cn/6 Crd" )
	PORT_DIPSETTING(    0x09, "1 Cn/7 Crd" )
	PORT_DIPSETTING(    0x00, "Free Play" )
	PORT_DIPNAME( 0xf0, 0xf0, "Coin B" ,IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "4 Cn/1 Crd" )
	PORT_DIPSETTING(    0x50, "3 Cn/1 Crd" )
	PORT_DIPSETTING(    0x80, "2 Cn/1 Crd" )
	PORT_DIPSETTING(    0x40, "3 Cn/2 Crd" )
	PORT_DIPSETTING(    0x10, "4 Cn/3 Crd" )
	PORT_DIPSETTING(    0xf0, "1 Cn/1 Crd" )
	PORT_DIPSETTING(    0x30, "3 Cn/4 Crd" )
	PORT_DIPSETTING(    0x70, "2 Cn/3 Crd" )
	PORT_DIPSETTING(    0xe0, "1 Cn/2 Crd" )
	PORT_DIPSETTING(    0x60, "2 Cn/5 Crd" )
	PORT_DIPSETTING(    0xd0, "1 Cn/3 Crd" )
	PORT_DIPSETTING(    0xc0, "1 Cn/4 Crd" )
	PORT_DIPSETTING(    0xb0, "1 Cn/5 Crd" )
	PORT_DIPSETTING(    0xa0, "1 Cn/6 Crd" )
	PORT_DIPSETTING(    0x90, "1 Cn/7 Crd" )
//	PORT_DIPSETTING(    0x00, "Invalid" )

	PORT_START	/* DSW #2 */
	PORT_DIPNAME( 0x03, 0x03, "Lives" ,IP_KEY_NONE )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, "Cabinet" ,IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Upright" )
	PORT_DIPSETTING(    0x04, "Cocktail" )
	PORT_DIPNAME( 0x18, 0x18, "Bonus Life" ,IP_KEY_NONE )
	PORT_DIPSETTING(    0x18, "20k and every 60k" )
	PORT_DIPSETTING(    0x10, "30k and every 70k" )
	PORT_DIPSETTING(    0x08, "20k" )
	PORT_DIPSETTING(    0x00, "30k" )
	PORT_DIPNAME( 0x60, 0x60, "Difficulty" ,IP_KEY_NONE )
	PORT_DIPSETTING(    0x60, "Easy" )
	PORT_DIPSETTING(    0x40, "Normal" )
	PORT_DIPSETTING(    0x20, "Difficult" )
	PORT_DIPSETTING(    0x00, "Very Difficult" )
	PORT_DIPNAME( 0x80, 0x00, "Demo Sounds" ,IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START	/* DSW #3 */
	PORT_DIPNAME( 0x01, 0x01, "Freeze" ,IP_KEY_NONE )
	PORT_DIPSETTING(    0x01, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x02, 0x02, "Unknown" ,IP_KEY_NONE )
	PORT_DIPSETTING(    0x02, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x04, 0x04, "Unknown" ,IP_KEY_NONE )
	PORT_DIPSETTING(    0x04, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x08, 0x08, "Unknown" ,IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x10, 0x10, "Unknown" ,IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown" ,IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown" ,IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown" ,IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START	/* COINSW */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* PLAYER 1 INPUTS */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* PLAYER 2 INPUTS */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

INPUT_PORTS_END

static struct GfxLayout charlayout =
{
	8,8,			/* 8*8 characters */
	0x4000/32,		/* 512 characters */
	4,				/* 4bpp */
	{ 0, 1, 2, 3 },	/* the four bitplanes are packed in one nibble */
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8			/* every character takes 32 consecutive bytes */
};

static struct GfxLayout spritelayout =
{
	16,16,			/* 16*16 sprites */
	0x6000/128,		/* 192 sprites */
	4,				/* 4 bpp */
	{ 0, 1, 2, 3 }, /* the four bitplanes are packed in one nibble */
	{ 15*4, 14*4, 13*4, 12*4, 11*4, 10*4, 9*4, 8*4,
			7*4, 6*4, 5*4, 4*4, 3*4, 2*4, 1*4, 0*4 },
	{ 15*4*16, 14*4*16, 13*4*16, 12*4*16, 11*4*16, 10*4*16, 9*4*16, 8*4*16,
			7*4*16, 6*4*16, 5*4*16, 4*4*16, 3*4*16, 2*4*16, 1*4*16, 0*4*16 },
	32*4*8			/* every sprite takes 128 consecutive bytes */
};

static struct GfxDecodeInfo gfxdecodeinfo[] =
{
	{ 1, 0, &charlayout,       0, 16 },
	{ 2, 0, &spritelayout, 16*16, 16 },
	{ -1 } /* end of array */
};

/***************************************************************************

	Machine Driver

***************************************************************************/

static void pandoras_init_machine( void )
{
	firq_old_data_a = firq_old_data_b = 0;
	irq_enable_a = irq_enable_b = 0;
}

static int pandoras_portB_r (int offset)
{
	/* ??? */
	return (cpu_gettotalcycles() / 1024) & 0x0f;
}

static void pandoras_portB_w (int offset, int data)
{
	/* ??? */
}

static struct AY8910interface ay8910_interface =
{
	1,			/* 1 chip */
	1789750,	/* ??????? */
	{ 25 },
	{ 0 },
	{ pandoras_portB_r },
	{ 0 },
	{ pandoras_portB_w }
};

static struct DACinterface dac_interface =
{
	1,
	{ 50 }
};

static const struct MachineDriver machine_driver_pandoras =
{
	/* basic machine hardware */
	{
		{
			CPU_M6809,		/* CPU A */
			3072000,		/* ? */
			0,
			pandoras_readmem_a,pandoras_writemem_a,0,0,
            pandoras_interrupt_a,1,
		},
		{
			CPU_M6809,		/* CPU B */
			3072000,		/* ? */
			3,
			pandoras_readmem_b,pandoras_writemem_b,0,0,
            pandoras_interrupt_b,1,
		},
		{
			CPU_Z80 | CPU_AUDIO_CPU,
			1789750,		/* ? */
			4,
			pandoras_readmem_snd,pandoras_writemem_snd,0,0,
			ignore_interrupt,1
		},
		{
			CPU_I8039 | CPU_AUDIO_CPU,
			477266,		/* ? */
			5,
			i8039_readmem,i8039_writemem,i8039_readport,i8039_writeport,
			ignore_interrupt,1
		},
	},
	60, DEFAULT_60HZ_VBLANK_DURATION,	/* frames per second, vblank duration */
	50,	/* slices per frame */
	pandoras_init_machine,

	/* video hardware */
	32*8, 32*8, { 0*8, 32*8-1, 2*8, 30*8-1 },
	gfxdecodeinfo,
	32, 16*16+16*16,
	pandoras_vh_convert_color_prom,

	VIDEO_TYPE_RASTER,
	0,
	pandoras_vh_start,
	0,
	pandoras_vh_screenrefresh,

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


static int hiload(void)
{
        void *f;
        unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];


        /* check if the hi score table has already been initialized */
        if (memcmp(&RAM[0x6100],"\x18",1) == 0 &&
            memcmp(&RAM[0x612c],"\x20",1) == 0 &&
            memcmp(&RAM[0x60d0],"\x00",1) == 0 &&
            memcmp(&RAM[0x60d3],"\x50",1) == 0)     /* high score */
        {
                if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
                {
                        osd_fread(f,&RAM[0x6100],45);
                        osd_fread(f,&RAM[0x60d0],3);
                        osd_fclose(f);
                }

                return 1;
        }
        else return 0; /* we can't load the hi scores yet */
}

static void hisave(void)
{
        void *f;
        unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];


        if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
        {
                osd_fwrite(f,&RAM[0x6100],45);
                osd_fwrite(f,&RAM[0x60d0],3);
                osd_fclose(f);
        }
}


/***************************************************************************

  Game ROMs

***************************************************************************/

ROM_START( pandoras_rom )
	ROM_REGION( 0x10000 ) /* 64K for the CPU A */
	ROM_LOAD( "pand_j13.cpu",	0x08000, 0x02000, 0x7a0fe9c5 )
	ROM_LOAD( "pand_j12.cpu",	0x0a000, 0x02000, 0x7dc4bfe1 )
	ROM_LOAD( "pand_j10.cpu",	0x0c000, 0x02000, 0xbe3af3b7 )
	ROM_LOAD( "pand_j9.cpu",	0x0e000, 0x02000, 0xe674a17a )

	ROM_REGION_DISPOSE( 0x4000 )
	ROM_LOAD( "pand_a18.cpu",	0x00000, 0x02000, 0x23706d4a )	/* tiles */
	ROM_LOAD( "pand_a19.cpu",	0x02000, 0x02000, 0xa463b3f9 )

	ROM_REGION_DISPOSE( 0x6000 )
	ROM_LOAD( "pand_j18.cpu",	0x00000, 0x02000, 0x99a696c5 )	/* sprites */
	ROM_LOAD( "pand_j17.cpu",	0x02000, 0x02000, 0x38a03c21 )
	ROM_LOAD( "pand_j16.cpu",	0x04000, 0x02000, 0xe0708a78 )

	ROM_REGION( 0x10000 ) /* 64K for the CPU B */
	ROM_LOAD( "pand_j5.cpu",	0x0e000, 0x02000, 0x4aab190b )

	ROM_REGION( 0x10000 ) /* 64K for the Sound CPU */
	ROM_LOAD( "pand_6c.snd",	0x00000, 0x02000, 0x0c1f109d )

	ROM_REGION( 0x1000 ) /* 4K for the Sound CPU 2 */
	ROM_LOAD( "pand_7e.snd",	0x00000, 0x01000, 0x18b0f9d0 )

	ROM_REGION( 0x0220 )
	ROM_LOAD( "pandora.2a",		0x0000, 0x020, 0x4d56f939 ) /* palette */
	ROM_LOAD( "pandora.17g",	0x0020, 0x100, 0xc1a90cfc ) /* sprite lookup table */
	ROM_LOAD( "pandora.16b",	0x0120, 0x100, 0xc89af0c3 ) /* character lookup table */
ROM_END


struct GameDriver pandoras_driver =
{
        __FILE__,
        0,
        "pandoras",
        "Pandora's Palace",
        "1984",
        "Konami/Interlogic",
        "Slaanesh",
        0,
        &machine_driver_pandoras,
        0,

        pandoras_rom,
        0, 0,   /* ROM decode and opcode decode functions */
        0,	/* samples */
        0,      /* sound_prom */

        pandoras_input_ports,

        PROM_MEMORY_REGION(6), 0, 0,
        ORIENTATION_ROTATE_90,

        hiload, hisave 
};
