/***************************************************************************

"Combat School" (also known as "Boot Camp") - (Konami GX611)

Credits:

	Hardware Info:
		Jose Tejada Gomez
		Manuel Abadia
		Cesareo Gutierrez

	MAME Driver:
		Phil Stroffolino
		Manuel Abadia

Memory Maps (preliminary):

***************************
* Combat School (bootleg) *
***************************

MAIN CPU:
---------
00c0-00c3	Objects control
0500		bankswitch control
0600-06ff	palette
0800-1fff	RAM
2000-2fff	Video RAM (banked)
3000-3fff	Object RAM (banked)
4000-7fff	Banked Area + IO + Video Registers
8000-ffff	ROM

SOUND CPU:
----------
0000-8000	ROM
8000-87ef	RAM
87f0-87ff	???
9000-9001	YM2203
9008		???
9800		OKIM5205?
a000		soundlatch?
a800		OKIM5205?
fffc-ffff	???

To Do:
	fix colors of frontmost layer
	hook up sound in bootleg (the current sound is a hack, making use of the Konami ROMset)

		Notes about the sound systsem of the bootleg:
        ---------------------------------------------
        The positions 0x87f0-0x87ff are very important, it
        does work similar to a semaphore (same as a lot of
        vblank bits). For example in the init code, it writes
        zero to 0x87fa, then it waits to it 'll be different
        to zero, but it isn't written by this cpu. (shareram?)
        I have tried put here a K007232 chip, but it didn't
        work.

		Sound chips: OKI M5205 & YM2203

		We are using the other sound hardware for now.

****************************
* Combat School (Original) *
****************************

0000-005f	Video Registers (banked)
0400-0404	input ports
0410-0410	bankswitch control
0600-06ff	palette
0800-1fff	RAM
2000-2fff	Video RAM (banked)
3000-3fff	Object RAM (banked)
4000-7fff	Banked Area + IO + Video Registers
8000-ffff	ROM

SOUND CPU:
----------
0000-8000	ROM
8000-87ff	RAM
9000		uPD7759
b000		uPD7759
c000		uPD7759
d000		soundlatch_r
e000-e001	YM2203

To Do:
	fix objects

***************************************************************************/

#include "driver.h"
#include "z80/z80.h"

extern unsigned char* banked_area;

/* from machine/combatsc.c */
extern void combatsc_bankselect_w( int offset, int data );
extern void combatsc_init_machine( void );
extern int combatsc_workram_r( int offset );
extern void combatsc_workram_w( int offset, int data );

/* from vidhrdw/combatsc.c */
extern void combatsc_convert_color_prom( unsigned char *palette, unsigned short *colortable, const unsigned char *color_prom );
extern int combatsc_video_r( int offset );
extern void combatsc_video_w( int offset, int data );
extern int combatsc_vh_start( void );
extern void combatsc_vh_stop( void );
extern void cmbatscb_vh_screenrefresh( struct osd_bitmap *bitmap, int fullrefresh );
extern void combatsc_vh_screenrefresh( struct osd_bitmap *bitmap, int fullrefresh );
extern void combatsc_io_w( int offset, int data );
void combatsc_vreg_w(int offset, int data);
void combatsc_sh_irqtrigger_w(int offset, int data);

/****************************************************************************/

void combatsc_play_w(int offset,int data)
{
	if (data & 0x02)
		UPD7759_start_w(offset, data);
}

void combatsc_voice_reset_w(int offset,int data)
{
	if ((data & 0x01) == 0)
 	{
 		UPD7759_reset_w(0,data);
 	}
}

void combatsc_portA_w(int offset,int data)
{
	/* unknown. always write 0 */
}

/****************************************************************************/

static struct MemoryReadAddress readmem[] =
{
	{ 0x0000, 0x04ff, MRA_RAM },
	{ 0x0600, 0x06ff, MRA_RAM, &paletteram },	/* palette */
	{ 0x0800, 0x1fff, MRA_RAM },
	{ 0x2000, 0x3fff, combatsc_video_r },
	{ 0x4000, 0x7fff, MRA_BANK1 },				/* banked ROM/RAM area */
	{ 0x8000, 0xffff, MRA_ROM },				/* ROM */
	{ -1 }
};

static struct MemoryWriteAddress writemem[] =
{
	{ 0x0000, 0x04ff, MWA_RAM },
	{ 0x0500, 0x0500, combatsc_bankselect_w },
	{ 0x0600, 0x06ff, paletteram_xBBBBBGGGGGRRRRR_w },
	{ 0x0800, 0x1fff, MWA_RAM },
	{ 0x2000, 0x3fff, combatsc_video_w },
	{ 0x4000, 0x7fff, MWA_BANK1, &banked_area },/* banked ROM/RAM area */
	{ 0x8000, 0xffff, MWA_ROM },				/* ROM */
	{ -1 }
};

static struct MemoryReadAddress combatsc_readmem_sound[] =
{
	{ 0x0000, 0x7fff, MRA_ROM },					/* ROM */
	{ 0x8000, 0x87ff, MRA_RAM },					/* RAM */
	{ 0xb000, 0xb000, UPD7759_busy_r },				/* UPD7759 busy? */
	{ 0xd000, 0xd000, soundlatch_r },				/* soundlatch_r? */
    { 0xe000, 0xe000, YM2203_status_port_0_r },		/* YM 2203 */
	{ -1 }
};

static struct MemoryWriteAddress combatsc_writemem_sound[] =
{
	{ 0x0000, 0x7fff, MWA_ROM },				/* ROM */
	{ 0x8000, 0x87ff, MWA_RAM },				/* RAM */
	{ 0x9000, 0x9000, combatsc_play_w },		/* uPD7759 play voice */
	{ 0xa000, 0xa000, UPD7759_message_w },		/* uPD7759 voice select */
	{ 0xc000, 0xc000, combatsc_voice_reset_w },	/* uPD7759 reset? */
 	{ 0xe000, 0xe000, YM2203_control_port_0_w },/* YM 2203 */
	{ 0xe001, 0xe001, YM2203_write_port_0_w },	/* YM 2203 */
	{ -1 }
};

static struct MemoryReadAddress combatsc_readmem[] =
{
	{ 0x0000, 0x005f, combatsc_workram_r },
	{ 0x0060, 0x03ff, MRA_RAM },				/* RAM? */
	{ 0x0400, 0x0400, input_port_0_r },
	{ 0x0401, 0x0401, input_port_1_r },			/* DSW #3 */
	{ 0x0402, 0x0402, input_port_2_r },			/* DSW #1 */
	{ 0x0403, 0x0403, input_port_3_r },			/* DSW #2 */
	{ 0x0404, 0x0404, input_port_4_r },			/* 1P & 2P controls */
	{ 0x0600, 0x06ff, MRA_RAM, &paletteram },	/* palette */
	{ 0x0800, 0x1fff, MRA_RAM },
	{ 0x2000, 0x3fff, combatsc_video_r },
	{ 0x4000, 0x7fff, MRA_BANK1, &banked_area },/* banked ROM area */
	{ 0x8000, 0xffff, MRA_ROM },				/* ROM */
	{ -1 }
};

static struct MemoryWriteAddress combatsc_writemem[] =
{
	{ 0x0000, 0x005f, combatsc_workram_w },
	{ 0x0060, 0x00ff, MWA_RAM },					/* RAM */
	{ 0x0200, 0x0201, MWA_RAM },					/* ??? */
	{ 0x0206, 0x0206, MWA_RAM },					/* ??? */
	{ 0x0408, 0x0408, MWA_NOP },					/* ??? */
	{ 0x040c, 0x040c, combatsc_vreg_w },
	{ 0x0410, 0x0410, combatsc_bankselect_w },
	{ 0x0414, 0x0414, combatsc_sh_irqtrigger_w },
	{ 0x0418, 0x0418, MWA_NOP },					/* ??? */
	{ 0x041c, 0x041c, watchdog_reset_w },			/* watchdog reset? */
	{ 0x0600, 0x06ff, paletteram_xBBBBBGGGGGRRRRR_w },
	{ 0x0800, 0x1fff, MWA_RAM },					/* RAM */
	{ 0x2000, 0x3fff, combatsc_video_w },
	{ 0x4000, 0x7fff, MWA_BANK1 },					/* banked ROM area */
	{ 0x8000, 0xffff, MWA_ROM },					/* ROM */
	{ -1 }
};


#define COMBATSC_COINAGE \
	PORT_DIPNAME( 0x0f, 0x0f, "Coin_A", IP_KEY_NONE ) \
	PORT_DIPSETTING(    0x02, "4 Coins/1 Credit" ) \
	PORT_DIPSETTING(    0x05, "3 Coins/1 Credit" ) \
	PORT_DIPSETTING(    0x08, "2 Coins/1 Credit" ) \
	PORT_DIPSETTING(    0x04, "3 Coins/2 Credits" ) \
	PORT_DIPSETTING(    0x01, "4 Coins/3 Credits" ) \
	PORT_DIPSETTING(    0x0f, "1 Coin/1 Credit" ) \
	PORT_DIPSETTING(    0x03, "3 Coins/4 Credits" ) \
	PORT_DIPSETTING(    0x07, "2 Coins/3 Credits" ) \
	PORT_DIPSETTING(    0x0e, "1 Coin/2 Credits" ) \
	PORT_DIPSETTING(    0x06, "2 Coins/5 Credits" ) \
	PORT_DIPSETTING(    0x0d, "1 Coin/3 Credits" ) \
	PORT_DIPSETTING(    0x0c, "1 Coin/4 Credits" ) \
	PORT_DIPSETTING(    0x0b, "1 Coin/5 Credits" ) \
	PORT_DIPSETTING(    0x0a, "1 Coin/6 Credits" ) \
	PORT_DIPSETTING(    0x09, "1 Coin/7 Credits" ) \
	PORT_DIPSETTING(    0x00, "Free_Play" ) \
	PORT_DIPNAME( 0xf0, 0xf0, "Coin_B", IP_KEY_NONE ) \
	PORT_DIPSETTING(    0x20, "4 Coins/1 Credit" ) \
	PORT_DIPSETTING(    0x50, "3 Coins/1 Credit" ) \
	PORT_DIPSETTING(    0x80, "2 Coins/1 Credit" ) \
	PORT_DIPSETTING(    0x40, "3 Coins/2 Credits" ) \
	PORT_DIPSETTING(    0x10, "4 Coins/3 Credits" ) \
	PORT_DIPSETTING(    0xf0, "1 Coin/1 Credit" ) \
	PORT_DIPSETTING(    0x30, "3 Coins/4 Credits" ) \
	PORT_DIPSETTING(    0x70, "2 Coins/3 Credits" ) \
	PORT_DIPSETTING(    0xe0, "1 Coin/2 Credits" ) \
	PORT_DIPSETTING(    0x60, "2 Coins/5 Credits" ) \
	PORT_DIPSETTING(    0xd0, "1 Coin/3 Credits" ) \
	PORT_DIPSETTING(    0xc0, "1 Coin/4 Credits" ) \
	PORT_DIPSETTING(    0xb0, "1 Coin/5 Credits" ) \
	PORT_DIPSETTING(    0xa0, "1 Coin/6 Credits" ) \
	PORT_DIPSETTING(    0x90, "1 Coin/7 Credits" ) \
	PORT_DIPSETTING(    0x00, "Unused" )

INPUT_PORTS_START( cmbatscb_input_ports )
	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_PLAYER2 | IPF_8WAY )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_PLAYER2 | IPF_8WAY )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_PLAYER2 | IPF_8WAY )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_PLAYER2 | IPF_8WAY )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY )

	PORT_START
	COMBATSC_COINAGE

	PORT_START
	PORT_DIPNAME( 0x01, 0x00, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(	0x01, "Off" )
	PORT_DIPSETTING(	0x00, "On" )
	PORT_DIPNAME( 0x02, 0x00, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(	0x02, "Off" )
	PORT_DIPSETTING(	0x00, "On" )
	PORT_DIPNAME( 0x04, 0x00, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(	0x04, "Off" )
	PORT_DIPSETTING(	0x00, "On" )
	PORT_DIPNAME( 0x08, 0x00, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(	0x08, "Off" )
	PORT_DIPSETTING(	0x00, "On" )

	PORT_DIPNAME( 0x10, 0x10, "Allow Continue", IP_KEY_NONE )
	PORT_DIPSETTING( 0x10, "No" )
	PORT_DIPSETTING( 0x00, "Yes" )
	PORT_DIPNAME( 0x60, 0x60, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING( 0x60, "Easy" )
	PORT_DIPSETTING( 0x40, "Normal" )
	PORT_DIPSETTING( 0x20, "Difficult" )
	PORT_DIPSETTING( 0x00, "Very Difficult" )
	PORT_DIPNAME( 0x80, 0x00, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING( 0x80, "Off" )
	PORT_DIPSETTING( 0x00, "On" )
INPUT_PORTS_END

INPUT_PORTS_START( combatsc_input_ports )
	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x20, 0x00, "Service Mode", IP_KEY_NONE )
	PORT_DIPSETTING(	0x20, "Off" )
	PORT_DIPSETTING(	0x00, "On" )

	PORT_START	/* DSW #3 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x10, 0x10, "Flip Screen", IP_KEY_NONE )
	PORT_DIPSETTING(	0x10, "No" )
	PORT_DIPSETTING(	0x00, "Yes"  )
	PORT_DIPNAME( 0x20, 0x00, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(	0x20, "Off" )
	PORT_DIPSETTING(	0x00, "On" )
	PORT_DIPNAME( 0x40, 0x40, "Mode", IP_KEY_NONE )
	PORT_DIPSETTING(	0x40, "Game Mode" )
	PORT_DIPSETTING(	0x00, "Test Mode" )
	PORT_DIPNAME( 0x80, 0x00, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(	0x80, "Off" )
	PORT_DIPSETTING(	0x00, "On" )

	PORT_START	/* DSW # 1 */
	COMBATSC_COINAGE

	PORT_START	/* DSW #2 */
	PORT_DIPNAME( 0x01, 0x01, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(	0x01, "Off" )
	PORT_DIPSETTING(	0x00, "On" )
	PORT_DIPNAME( 0x02, 0x02, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(	0x02, "Off" )
	PORT_DIPSETTING(	0x00, "On" )
	PORT_DIPNAME( 0x04, 0x00, "Cabinet", IP_KEY_NONE )
	PORT_DIPSETTING(	0x00, "Upright" )
	PORT_DIPSETTING(	0x04, "Cocktail" )
	PORT_DIPNAME( 0x08, 0x08, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(	0x08, "Off" )
	PORT_DIPSETTING(	0x00, "On" )
	PORT_DIPNAME( 0x10, 0x10, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(	0x10, "Off" )
	PORT_DIPSETTING(	0x00, "On" )
	PORT_DIPNAME( 0x60, 0x60, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING( 0x60, "Easy" )
	PORT_DIPSETTING( 0x40, "Normal" )
	PORT_DIPSETTING( 0x20, "Difficult" )
	PORT_DIPSETTING( 0x00, "Very Difficult" )
	PORT_DIPNAME( 0x80, 0x00, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING( 0x80, "Off" )
	PORT_DIPSETTING( 0x00, "On" )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_PLAYER2 | IPF_8WAY )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_PLAYER2 | IPF_8WAY )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_PLAYER2 | IPF_8WAY )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_PLAYER2 | IPF_8WAY )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY )

INPUT_PORTS_END

static struct GfxLayout tile_layout = {
	8,8,
	0x2000, /* number of tiles */
	4,		/* bitplanes */
	{ 0*0x10000*8, 1*0x10000*8, 2*0x10000*8, 3*0x10000*8 }, /* plane offsets */
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	8*8
};

static struct GfxLayout sprite_layout = {
	16,16,
	0x800,	/* number of sprites */
	4,		/* bitplanes */
	{ 3*0x10000*8, 2*0x10000*8, 1*0x10000*8, 0*0x10000*8 }, /* plane offsets */
	{
		0,1,2,3,4,5,6,7,
		16*8+0,16*8+1,16*8+2,16*8+3,16*8+4,16*8+5,16*8+6,16*8+7
	},
	{
		0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,
		8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8
	},
	8*8*4
};

static struct GfxDecodeInfo gfxdecodeinfo[] =
{
	{ 2, 0x00000, &tile_layout,		0, 8*16 },
	{ 2, 0x40000, &tile_layout,		0, 8*16 },
	{ 2, 0x80000, &sprite_layout,	0, 8*16 },
	{ 2, 0xc0000, &sprite_layout,	0, 8*16 },
	{ -1 }
};

static struct YM2203interface ym2203_interface =
{
	1,							/* 1 chip */
	3500000,					/* 3.5 MHz? */
	{ YM2203_VOL(20,20) },
	{ 0 },
	{ 0 },
	{ 0 },
	{ combatsc_portA_w },
	{ 0 }
};

static struct UPD7759_interface upd7759_interface =
{
	1,							/* number of chips */
	UPD7759_STANDARD_CLOCK,
	{ 70 },						/* volume */
	4,							/* memory region */
	UPD7759_STANDALONE_MODE,	/* chip mode */
	{0}
};

static struct MachineDriver cmbatscb_machine_driver =
{
	{
		{
			CPU_M6309,
			5000000,	/* 5 MHz? */
			0,
			readmem,writemem,0,0,
			interrupt,1
		},
		{
			CPU_Z80 | CPU_AUDIO_CPU,
			1500000,
			1,
			combatsc_readmem_sound,combatsc_writemem_sound,0,0, /* FAKE */
			ignore_interrupt,0 	/* IRQs are caused by the main CPU */
		},
	},
	60, DEFAULT_REAL_60HZ_VBLANK_DURATION,
	10, /* CPU slices */
	combatsc_init_machine,

	/* video hardware */
	32*8, 32*8, { 0*8, 32*8-1, 2*8, 30*8-1 },
	gfxdecodeinfo,
	128,128*16,combatsc_convert_color_prom,
	VIDEO_TYPE_RASTER | VIDEO_MODIFIES_PALETTE,
	0,
	combatsc_vh_start,
	combatsc_vh_stop,
	cmbatscb_vh_screenrefresh,

	/* We are using the original sound subsystem */
	0,0,0,0,
	{
		{
			SOUND_YM2203,
			&ym2203_interface
		},
		{
			SOUND_UPD7759,
			&upd7759_interface
		}
	}
};

static struct MachineDriver combatsc_machine_driver =
{
	{
		{
			CPU_M6309,
			5000000,	/* 5 MHz? */
			0,
			combatsc_readmem,combatsc_writemem,0,0,
			interrupt,1
		},
		{
			CPU_Z80 | CPU_AUDIO_CPU,
			1500000,	/* 1.5 MHz? */
			1,
			combatsc_readmem_sound,combatsc_writemem_sound,0,0,
			ignore_interrupt,1 	/* IRQs are caused by the main CPU */
		}
	},
	60, DEFAULT_REAL_60HZ_VBLANK_DURATION,
	10, /* CPU slices */
	combatsc_init_machine,

	/* video hardware */
	32*8, 32*8, { 0*8, 32*8-1, 2*8, 30*8-1 },
	gfxdecodeinfo,
	128,128*16,combatsc_convert_color_prom,
	VIDEO_TYPE_RASTER | VIDEO_MODIFIES_PALETTE,
	0,
	combatsc_vh_start,
	combatsc_vh_stop,
	combatsc_vh_screenrefresh,

	/* sound hardware */
	0,0,0,0,
	{
		{
			SOUND_YM2203,
			&ym2203_interface
		},
		{
			SOUND_UPD7759,
			&upd7759_interface
		}
	}
};

ROM_START( combatscb_rom )
	ROM_REGION( 0x48000 ) /* 6809 code */
	ROM_LOAD( "combat.002",	0x10000, 0x10000, 0x0996755d )
	ROM_LOAD( "combat.003",	0x20000, 0x10000, 0x229c93b2 )
	ROM_LOAD( "combat.004",	0x30000, 0x10000, 0xa069cb84 )
	/* extra 0x8000 for banked RAM */

	ROM_REGION( 0x10000 ) /* sound CPU */
	ROM_LOAD( "combat.001", 0x00000, 0x10000, 0x61456b3b )
	ROM_LOAD( "611g03.rom", 0x00000, 0x08000, 0x2a544db5 ) /* FAKE - from Konami set! */

	ROM_REGION_DISPOSE( 0x100000 ) /* graphics */
	ROM_LOAD( "combat.006",	0x00000, 0x10000, 0x8dc29a1f ) /* tiles, bank 0 */
	ROM_LOAD( "combat.008",	0x10000, 0x10000, 0x61599f46 )
	ROM_LOAD( "combat.010",	0x20000, 0x10000, 0xd5cda7cd )
	ROM_LOAD( "combat.012",	0x30000, 0x10000, 0xca0a9f57 )

	ROM_LOAD( "combat.005",	0x40000, 0x10000, 0x0803a223 ) /* tiles, bank 1 */
	ROM_LOAD( "combat.007",	0x50000, 0x10000, 0x23caad0c )
	ROM_LOAD( "combat.009",	0x60000, 0x10000, 0x5ac80383 )
	ROM_LOAD( "combat.011",	0x70000, 0x10000, 0xcda83114 )

	ROM_LOAD( "combat.013",	0x80000, 0x10000, 0x4bed2293 ) /* sprites, bank 0 */
	ROM_LOAD( "combat.015",	0x90000, 0x10000, 0x26c41f31 )
	ROM_LOAD( "combat.017",	0xa0000, 0x10000, 0x6071e6da )
	ROM_LOAD( "combat.019",	0xb0000, 0x10000, 0x3b1cf1b8 )

	ROM_LOAD( "combat.014",	0xc0000, 0x10000, 0x82ea9555 ) /* sprites, bank 1 */
	ROM_LOAD( "combat.016",	0xd0000, 0x10000, 0x2e39bb70 )
	ROM_LOAD( "combat.018",	0xe0000, 0x10000, 0x575db729 )
	ROM_LOAD( "combat.020",	0xf0000, 0x10000, 0x8d748a1a )

	ROM_REGION( 0x400 ) /* color lookup table */
	ROM_LOAD( "611g10.h6",	0x000, 0x100, 0xf916129a ) /* player sprites */
	ROM_RELOAD(				0x100, 0x100 )
	ROM_LOAD( "611g09.h7",	0x200, 0x100, 0x207a7b07 ) /* tilemap clut */
	ROM_RELOAD(				0x300, 0x100 )

    ROM_REGION(0x20000)	/* uPD7759 data */
	ROM_LOAD( "611g04.rom", 0x00000, 0x20000, 0x2987e158 )	/* FAKE - from Konami set! */
ROM_END

ROM_START( combatsc_rom )
	ROM_REGION( 0x48000 ) /* 6309 code */
	ROM_LOAD( "611g01.rom",	0x10000, 0x10000, 0x857ffffe )
	ROM_LOAD( "611g02.rom",	0x20000, 0x20000, 0x9ba05327 )
	/* extra 0x8000 for banked RAM */

	ROM_REGION( 0x10000 ) /* sound CPU */
	ROM_LOAD( "611g03.rom", 0x00000, 0x08000, 0x2a544db5 )

	ROM_REGION_DISPOSE( 0x100000 ) /* graphics - from bootleg set! (they may or may not be the same) */
	ROM_LOAD( "combat.006",	0x00000, 0x10000, 0x8dc29a1f ) /* tiles, bank 0 */
	ROM_LOAD( "combat.008",	0x10000, 0x10000, 0x61599f46 )
	ROM_LOAD( "combat.010",	0x20000, 0x10000, 0xd5cda7cd )
	ROM_LOAD( "combat.012",	0x30000, 0x10000, 0xca0a9f57 )

	ROM_LOAD( "combat.005",	0x40000, 0x10000, 0x0803a223 ) /* tiles, bank 1 */
	ROM_LOAD( "combat.007",	0x50000, 0x10000, 0x23caad0c )
	ROM_LOAD( "combat.009",	0x60000, 0x10000, 0x5ac80383 )
	ROM_LOAD( "combat.011",	0x70000, 0x10000, 0xcda83114 )

	ROM_LOAD( "combat.013",	0x80000, 0x10000, 0x4bed2293 ) /* sprites, bank 0 */
	ROM_LOAD( "combat.015",	0x90000, 0x10000, 0x26c41f31 )
	ROM_LOAD( "combat.017",	0xa0000, 0x10000, 0x6071e6da )
	ROM_LOAD( "combat.019",	0xb0000, 0x10000, 0x3b1cf1b8 )

	ROM_LOAD( "combat.014",	0xc0000, 0x10000, 0x82ea9555 ) /* sprites, bank 1 */
	ROM_LOAD( "combat.016",	0xd0000, 0x10000, 0x2e39bb70 )
	ROM_LOAD( "combat.018",	0xe0000, 0x10000, 0x575db729 )
	ROM_LOAD( "combat.020",	0xf0000, 0x10000, 0x8d748a1a )

	ROM_REGION( 0x400 ) /* color lookup table */
	ROM_LOAD( "611g10.h6",	0x000, 0x100, 0xf916129a ) /* player sprites */
	ROM_RELOAD(				0x100, 0x100 )
	ROM_LOAD( "611g09.h7",	0x200, 0x100, 0x207a7b07 ) /* tilemap clut */
	ROM_RELOAD(				0x300, 0x100 )

    ROM_REGION(0x20000)	/* uPD7759 data */
	ROM_LOAD( "611g04.rom", 0x00000, 0x20000, 0x2987e158 )

ROM_END


static void gfx_untangle( void ){
	unsigned char *gfx = Machine->memory_region[2];
	int i;
	for( i=0; i<0x80000; i++ ){
		gfx[i] = ~gfx[i];
	}
}

	/* load the high score table */
static int combatsc_hiload( void )
{
	void *f;
	unsigned char *RAM = Machine->memory_region[0];
	unsigned char temp []={ 0x4b, 0x3d, 0x41, 0x07 };

	/* check if the hi score table has already been initialized */
	if (memcmp( &RAM[0x1362], temp, 0x04 ) == 0)
	{
		if ((f = osd_fopen( Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0 )) != 0)
		{
			osd_fread( f, &RAM[0x1320], 0x46 );
			osd_fclose( f );
		}
		return 1;
	}
	else
		return 0;	/* we can't load the hi scores yet */
}

	/* save the high score table */
static void combatsc_hisave( void )
{
	void *f;
	unsigned char *RAM = Machine->memory_region[0];

	if ((f = osd_fopen( Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1 )) != 0)
	{
		osd_fwrite( f, &RAM[0x1320], 0x46 );
		osd_fclose( f );
	}
}

struct GameDriver combasc_driver =
{
	__FILE__,
	0,
	"combasc",
	"Combat School",
	"1988",
	"Konami",
	"Manuel Abadia\nJose Tejada\nCesareo Gutierrez\nPhil Stroffolino",
	GAME_NOT_WORKING,
	&combatsc_machine_driver,
	0,

	combatsc_rom,
	gfx_untangle,
	0,
	0,
	0, /* sound_prom */

	combatsc_input_ports,

	PROM_MEMORY_REGION(3), 0, 0, /* color lookup table */
	ORIENTATION_DEFAULT,
	combatsc_hiload, combatsc_hisave	/* hiload,hisave */
};

struct GameDriver combascb_driver =  {
	__FILE__,
	&combasc_driver,
	"combascb",
	"Combat School (bootleg)",
	"1988",
	"bootleg",
	"Manuel Abadia\nJose Tejada\nCesareo Gutierrez\nPhil Stroffolino",
	GAME_IMPERFECT_COLORS,
	&cmbatscb_machine_driver,
	0,

	combatscb_rom,
	gfx_untangle,
	0,
	0,
	0,	/* sound_prom */

	cmbatscb_input_ports,

	PROM_MEMORY_REGION(3), 0, 0, /* color lookup table */
	ORIENTATION_DEFAULT,
	combatsc_hiload, combatsc_hisave	/* hiload,hisave */
};
