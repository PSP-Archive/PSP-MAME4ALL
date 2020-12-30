/***************************************************************************

Burger Time hardware description:

Actually Lock'n'Chase is (C)1981 while Burger Time is (C)1982, so it might
be more accurate to say 'Lock'n'Chase hardware'.

The bootleg called Cook Race runs on hardware similar but different. The fact
that it addresses the program ROMs in the range 0500-3fff instead of the usual
c000-ffff makes me suspect that it is a bootleg of the *tape system* version.
Little is known about that system, but it is quite likely that it would have
RAM in the range 0000-3fff and load the program there from tape.


This hardware is pretty straightforward, but has a couple of interesting
twists. There are two ports to the video and color RAMs, one normal access,
and one with X and Y coordinates swapped. The sprite RAM occupies the
first row of the swapped area, so it appears in the regular video RAM as
the first column of on the left side.

These games don't have VBLANK interrupts, but instead an IRQ or NMI
(depending on the particular board) is generated when a coin is inserted.

Some of the games also have a background playfield which, in the
case of Bump 'n' Jump and Zoar, can be scrolled vertically.

These boards use two 8910's for sound, controlled by a dedicated 6502 (except
in Eggs, where the main processor handles the sound). The main processor
triggers an IRQ request when writing a command to the sound CPU.

Main clock: XTAL = 12 MHz
Horizontal video frequency: HSYNC = XTAL/768?? = 15.625 kHz ??
Video frequency: VSYNC = HSYNC/272 = 57.44 Hz ?
VBlank duration: 1/VSYNC * (24/272) = 1536 us ?


Note on Lock'n'Chase:

The watchdog test prints "WATCHDOG TEST ER". Just by looking at the code,
I can't see how it could print anything else, there is only one path it
can take. Should the game reset????

***************************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"
#include "m6502/m6502.h"

extern unsigned char *lnc_charbank;
extern unsigned char *bnj_backgroundram;
extern int bnj_backgroundram_size;
extern unsigned char *zoar_scrollram;

void btime_vh_convert_color_prom (unsigned char *palette, unsigned short *colortable,const unsigned char *color_prom);
void lnc_vh_convert_color_prom (unsigned char *palette, unsigned short *colortable,const unsigned char *color_prom);
void zoar_vh_convert_color_prom(unsigned char *palette, unsigned short *colortable,const unsigned char *color_prom);

void lnc_init_machine (void);

int  bnj_vh_start (void);

void bnj_vh_stop (void);

void btime_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh);
void cookrace_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh);
void bnj_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh);
void lnc_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh);
void eggs_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh);
void zoar_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh);

void btime_paletteram_w(int offset,int data);
void bnj_background_w(int offset, int data);
void bnj_scroll1_w(int offset, int data);
void bnj_scroll2_w(int offset, int data);
int  btime_mirrorvideoram_r(int offset);
void btime_mirrorvideoram_w(int offset,int data);
int  btime_mirrorcolorram_r(int offset);
void btime_mirrorcolorram_w(int offset,int data);
void lnc_videoram_w(int offset,int data);
void lnc_mirrorvideoram_w(int offset,int data);
void zoar_video_control_w(int offset,int data);
void btime_video_control_w(int offset,int data);
void bnj_video_control_w(int offset,int data);
void lnc_video_control_w(int offset,int data);

int lnc_sound_interrupt(void);

static void sound_command_w(int offset,int data);

static void btime_decrypt(void)
{
	int A,A1;
	extern unsigned char *RAM;


	/* the encryption is a simple bit rotation: 76543210 -> 65342710, but */
	/* with a catch: it is only applied if the previous instruction did a */
	/* memory write. Also, only opcodes at addresses with this bit pattern: */
	/* xxxx xxx1 xxxx x1xx are encrypted. */

	/* get the address of the next opcode */
	A = cpu_getpc();

	/* however if the previous instruction was JSR (which caused a write to */
	/* the stack), fetch the address of the next instruction. */
	A1 = cpu_getpreviouspc();
	if (ROM[A1] == 0x20)	/* JSR $xxxx */
		A = cpu_readop_arg(A1+1) + 256 * cpu_readop_arg(A1+2);

	/* If the address of the next instruction is xxxx xxx1 xxxx x1xx, decode it. */
	if ((A & 0x0104) == 0x0104)
	{
		/* 76543210 -> 65342710 bit rotation */
		ROM[A] = (RAM[A] & 0x13) | ((RAM[A] & 0x80) >> 5) | ((RAM[A] & 0x64) << 1)
			   | ((RAM[A] & 0x08) << 2);
	}
}

static void btime_ram_w(int offset,int data)
{
	extern unsigned char *RAM;


	if (offset <= 0x07ff)
		RAM[offset] = data;
	else if (offset >= 0x0c00 && offset <= 0x0c0f)
		btime_paletteram_w(offset - 0x0c00,data);
	else if (offset >= 0x1000 && offset <= 0x13ff)
		videoram_w(offset - 0x1000,data);
	else if (offset >= 0x1400 && offset <= 0x17ff)
		colorram_w(offset - 0x1400,data);
	else if (offset >= 0x1800 && offset <= 0x1bff)
		btime_mirrorvideoram_w(offset - 0x1800,data);
	else if (offset >= 0x1c00 && offset <= 0x1fff)
		btime_mirrorcolorram_w(offset - 0x1c00,data);
	else if (offset == 0x4002)
		btime_video_control_w(0,data);
	else if (offset == 0x4003)
		sound_command_w(0,data);
	else if (offset == 0x4004)
		bnj_scroll1_w(0,data);

	btime_decrypt();
}

static void zoar_ram_w(int offset,int data)
{
	int good = 0;
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];


	if (offset <= 0x07ff)
		{ RAM[offset] = data; good = 1; }
	else if (offset >= 0x8000 && offset <= 0x83ff)
		{ videoram_w(offset - 0x8000,data);	good = 1; }
	else if (offset >= 0x8400 && offset <= 0x87ff)
		{ colorram_w(offset - 0x8400,data); good = 1; }
	else if (offset >= 0x8800 && offset <= 0x8bff)
		{ btime_mirrorvideoram_w(offset - 0x8800,data); good = 1; }
	else if (offset >= 0x8c00 && offset <= 0x8fff)
		{ btime_mirrorcolorram_w(offset - 0x8c00,data); good = 1; }
	else if (offset == 0x9000)
		{ zoar_video_control_w(0, data); good = 1; }
	else if (offset >= 0x9800 && offset <= 0x9803)
		{ zoar_scrollram[offset - 0x9800] = data; good = 1; }
	else if (offset == 0x9804)
		{ bnj_scroll2_w(0,data); good = 1; }
	else if (offset == 0x9805)
		{ bnj_scroll1_w(0,data); good = 1; }
	else if (offset == 0x9806)
		{ sound_command_w(0,data); good = 1; }

	btime_decrypt();

}


static struct MemoryReadAddress btime_readmem[] =
{
	{ 0x0000, 0x07ff, MRA_RAM },
	{ 0x1000, 0x17ff, MRA_RAM },
	{ 0x1800, 0x1bff, btime_mirrorvideoram_r },
	{ 0x1c00, 0x1fff, btime_mirrorcolorram_r },
	{ 0x4000, 0x4000, input_port_0_r },     /* IN0 */
	{ 0x4001, 0x4001, input_port_1_r },     /* IN1 */
	{ 0x4002, 0x4002, input_port_2_r },     /* coin */
	{ 0x4003, 0x4003, input_port_3_r },     /* DSW1 */
	{ 0x4004, 0x4004, input_port_4_r },     /* DSW2 */
	{ 0xb000, 0xffff, MRA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress btime_writemem[] =
{
	{ 0x0000, 0xffff, btime_ram_w },	/* override the following entries to */
										/* support ROM decryption */
	{ 0x0000, 0x07ff, MWA_RAM },
	{ 0x0c00, 0x0c0f, btime_paletteram_w, &paletteram },
	{ 0x1000, 0x13ff, videoram_w, &videoram, &videoram_size },
	{ 0x1400, 0x17ff, colorram_w, &colorram },
	{ 0x1800, 0x1bff, btime_mirrorvideoram_w },
	{ 0x1c00, 0x1fff, btime_mirrorcolorram_w },
	{ 0x4000, 0x4000, MWA_NOP },
	{ 0x4002, 0x4002, btime_video_control_w },
	{ 0x4003, 0x4003, sound_command_w },
	{ 0x4004, 0x4004, bnj_scroll1_w },
	{ -1 }  /* end of table */
};

static struct MemoryReadAddress cookrace_readmem[] =
{
	{ 0x0000, 0x03ff, MRA_RAM },
	{ 0x0500, 0x3fff, MRA_ROM },
	{ 0xc000, 0xc7ff, MRA_RAM },
	{ 0xc800, 0xcbff, btime_mirrorvideoram_r },
	{ 0xcc00, 0xcfff, btime_mirrorcolorram_r },
	{ 0xd000, 0xd0ff, MRA_RAM },	/* background */
	{ 0xd100, 0xd3ff, MRA_RAM },	/* ? */
	{ 0xd400, 0xd7ff, MRA_RAM },	/* background? */
	{ 0xe000, 0xe000, input_port_3_r },     /* DSW1 */
	{ 0xe300, 0xe300, input_port_3_r },     /* mirror address used on high score name enter */
											/* screen - could also be a bad ROM read */
	{ 0xe001, 0xe001, input_port_4_r },     /* DSW2 */
	{ 0xe002, 0xe002, input_port_0_r },     /* IN0 */
	{ 0xe003, 0xe003, input_port_1_r },     /* IN1 */
	{ 0xe004, 0xe004, input_port_2_r },     /* coin */
	{ 0xfff9, 0xffff, MRA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress cookrace_writemem[] =
{
	{ 0x0000, 0x03ff, MWA_RAM },
	{ 0x0500, 0x3fff, MWA_ROM },
	{ 0xc000, 0xc3ff, videoram_w, &videoram, &videoram_size },
	{ 0xc400, 0xc7ff, colorram_w, &colorram },
	{ 0xc800, 0xcbff, btime_mirrorvideoram_w },
	{ 0xcc00, 0xcfff, btime_mirrorcolorram_w },
	{ 0xd000, 0xd0ff, MWA_RAM, &bnj_backgroundram, &bnj_backgroundram_size },
	{ 0xd000, 0xd0ff, MWA_RAM },	/* background? */
	{ 0xd100, 0xd3ff, MWA_RAM },	/* ? */
	{ 0xd400, 0xd7ff, MWA_RAM, &bnj_backgroundram, &bnj_backgroundram_size },
	{ 0xe000, 0xe000, bnj_video_control_w },
	{ 0xe001, 0xe001, sound_command_w },
#if 0
	{ 0x4004, 0x4004, bnj_scroll1_w },
#endif
	{ 0xfff9, 0xffff, MWA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryReadAddress zoar_readmem[] =
{
	{ 0x0000, 0x07ff, MRA_RAM },
	{ 0x9800, 0x9800, input_port_3_r },     /* DSW 1 */
	{ 0x9801, 0x9801, input_port_4_r },     /* DSW 2 */
	{ 0x9802, 0x9802, input_port_0_r },     /* IN 0 */
	{ 0x9803, 0x9803, input_port_1_r },     /* IN 1 */
	{ 0x9804, 0x9804, input_port_2_r },     /* coin */
	{ 0xd000, 0xffff, MRA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress zoar_writemem[] =
{
	{ 0x0000, 0xffff, zoar_ram_w },	/* override the following entries to */
									/* support ROM decryption */
	{ 0x0000, 0x07ff, MWA_RAM },
	{ 0x8000, 0x83ff, videoram_w, &videoram, &videoram_size },
	{ 0x8400, 0x87ff, colorram_w, &colorram },
	{ 0x8800, 0x8bff, btime_mirrorvideoram_w },
	{ 0x8c00, 0x8fff, btime_mirrorcolorram_w },
	{ 0x9000, 0x9000, zoar_video_control_w },
	{ 0x9800, 0x9803, MWA_RAM, &zoar_scrollram },
	{ 0x9805, 0x9805, bnj_scroll2_w },
	{ 0x9805, 0x9805, bnj_scroll1_w },
	{ 0x9806, 0x9806, sound_command_w },
  	{ -1 }  /* end of table */
};


static struct MemoryReadAddress eggs_readmem[] =
{
	{ 0x0000, 0x07ff, MRA_RAM },
	{ 0x1000, 0x17ff, MRA_RAM },
	{ 0x1800, 0x1bff, btime_mirrorvideoram_r },
	{ 0x1c00, 0x1fff, btime_mirrorcolorram_r },
	{ 0x2000, 0x2000, input_port_2_r },     /* DSW1 */
	{ 0x2001, 0x2001, input_port_3_r },     /* DSW2 */
	{ 0x2002, 0x2002, input_port_0_r },     /* IN0 */
	{ 0x2003, 0x2003, input_port_1_r },     /* IN1 */
	{ 0x3000, 0x7fff, MRA_ROM },
	{ 0xf000, 0xffff, MRA_ROM },    /* reset/interrupt vectors */
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress eggs_writemem[] =
{
	{ 0x0000, 0x07ff, MWA_RAM },
	{ 0x1000, 0x13ff, videoram_w, &videoram, &videoram_size },
	{ 0x1400, 0x17ff, colorram_w, &colorram },
	{ 0x1800, 0x1bff, btime_mirrorvideoram_w },
	{ 0x1c00, 0x1fff, btime_mirrorcolorram_w },
	{ 0x2000, 0x2000, btime_video_control_w },
	{ 0x2001, 0x2001, MWA_NOP },
	{ 0x2004, 0x2004, AY8910_control_port_0_w },
	{ 0x2005, 0x2005, AY8910_write_port_0_w },
	{ 0x2006, 0x2006, AY8910_control_port_1_w },
	{ 0x2007, 0x2007, AY8910_write_port_1_w },
	{ 0x3000, 0x7fff, MWA_ROM },
	{ -1 }  /* end of table */
};


static struct MemoryReadAddress lnc_readmem[] =
{
	{ 0x0000, 0x3fff, MRA_RAM },
	{ 0x7c00, 0x7fff, btime_mirrorvideoram_r },
	{ 0x8000, 0x8000, input_port_3_r },     /* DSW1 */
	{ 0x8001, 0x8001, input_port_4_r },     /* DSW2 */
	{ 0x9000, 0x9000, input_port_0_r },     /* IN0 */
	{ 0x9001, 0x9001, input_port_1_r },     /* IN1 */
	{ 0x9002, 0x9002, input_port_2_r },     /* coin */
	{ 0xc000, 0xffff, MRA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress lnc_writemem[] =
{
	{ 0x0000, 0x3bff, MWA_RAM },
	{ 0x3c00, 0x3fff, lnc_videoram_w, &videoram, &videoram_size },
	{ 0x7800, 0x7bff, MWA_RAM, &colorram },  
	{ 0x7c00, 0x7fff, lnc_mirrorvideoram_w },
	{ 0x8000, 0x8000, MWA_NOP },            /* ??? */
	{ 0x8001, 0x8001, lnc_video_control_w },
	{ 0x8003, 0x8003, MWA_RAM, &lnc_charbank },
	{ 0x9000, 0x9000, MWA_NOP },            /* ??? */
	{ 0x9002, 0x9002, sound_command_w },
	{ -1 }  /* end of table */
};


static struct MemoryReadAddress bnj_readmem[] =
{
	{ 0x0000, 0x07ff, MRA_RAM },
	{ 0x1000, 0x1000, input_port_3_r },     /* DSW1 */
	{ 0x1001, 0x1001, input_port_4_r },     /* DSW2 */
	{ 0x1002, 0x1002, input_port_0_r },     /* IN0 */
	{ 0x1003, 0x1003, input_port_1_r },     /* IN1 */
	{ 0x1004, 0x1004, input_port_2_r },     /* coin */
	{ 0x4000, 0x47ff, MRA_RAM },
	{ 0x4800, 0x4bff, btime_mirrorvideoram_r },
	{ 0x4c00, 0x4fff, btime_mirrorcolorram_r },
	{ 0xa000, 0xffff, MRA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress bnj_writemem[] =
{
	{ 0x0000, 0x07ff, MWA_RAM },
	{ 0x1001, 0x1001, bnj_video_control_w },
	{ 0x1002, 0x1002, sound_command_w },
	{ 0x4000, 0x43ff, videoram_w, &videoram, &videoram_size },
	{ 0x4400, 0x47ff, colorram_w, &colorram },
	{ 0x4800, 0x4bff, btime_mirrorvideoram_w },
	{ 0x4c00, 0x4fff, btime_mirrorcolorram_w },
	{ 0x5000, 0x51ff, bnj_background_w, &bnj_backgroundram, &bnj_backgroundram_size },
	{ 0x5400, 0x5400, bnj_scroll1_w },
	{ 0x5800, 0x5800, bnj_scroll2_w },
	{ 0x5c00, 0x5c0f, btime_paletteram_w, &paletteram },
	{ -1 }  /* end of table */
};


static struct MemoryReadAddress sound_readmem[] =
{
	{ 0x0000, 0x03ff, MRA_RAM },
	{ 0x0200, 0x0fff, MRA_ROM },	/* Cook Race */
	{ 0xa000, 0xafff, soundlatch_r },
	{ 0xf000, 0xffff, MRA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress sound_writemem[] =
{
	{ 0x0000, 0x03ff, MWA_RAM },
	{ 0x0200, 0x0fff, MWA_ROM },	/* Cook Race */
	{ 0x2000, 0x2fff, AY8910_write_port_0_w },
	{ 0x4000, 0x4fff, AY8910_control_port_0_w },
	{ 0x6000, 0x6fff, AY8910_write_port_1_w },
	{ 0x8000, 0x8fff, AY8910_control_port_1_w },
	{ 0xc000, 0xcfff, interrupt_enable_w },
	{ 0xf000, 0xffff, MWA_ROM },
	{ -1 }  /* end of table */
};



/***************************************************************************

These games don't have VBlank interrupts.
Interrupts are still used by the game, coin insertion generates an IRQ.

***************************************************************************/
static int btime_interrupt(int(*generated_interrupt)(void), int active_high)
{
	static int coin;
	int port;

	port = readinputport(2) & 0xc0;
	if (active_high) port ^= 0xc0;

	if (port != 0xc0)    /* Coin */
	{
		if (coin == 0)
		{
			coin = 1;
			return generated_interrupt();
		}
	}
	else coin = 0;

	return ignore_interrupt();
}

static int btime_irq_interrupt(void)
{
	return btime_interrupt(interrupt, 1);
}

static int zoar_irq_interrupt(void)
{
	return btime_interrupt(interrupt, 0);
}

static int btime_nmi_interrupt(void)
{
	return btime_interrupt(nmi_interrupt, 0);
}

static void sound_command_w(int offset,int data)
{
	soundlatch_w(offset,data);
	cpu_cause_interrupt(1,M6502_INT_IRQ);
}


INPUT_PORTS_START( btime_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_4WAY )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_4WAY )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_4WAY )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_4WAY )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START      /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START      /* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH,IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH,IPT_COIN2 )

	PORT_START      /* DSW1 */
	PORT_DIPNAME( 0x03, 0x03, "Coin A", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x03, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x02, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x01, "1 Coin/3 Credits" )
	PORT_DIPNAME( 0x0c, 0x0c, "Coin B", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x0c, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x08, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/3 Credits" )
	PORT_BITX(    0x10, 0x10, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
	PORT_DIPSETTING(    0x10, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x20, 0x20, "Cross Hatch Pattern", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x00, "Cabinet", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Upright" )
	PORT_DIPSETTING(    0x40, "Cocktail" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK  )

	PORT_START      /* DSW2 */
	PORT_DIPNAME( 0x01, 0x01, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, "Bonus Life", IP_KEY_NONE )
	PORT_DIPSETTING(    0x06, "10000" )
	PORT_DIPSETTING(    0x04, "15000" )
	PORT_DIPSETTING(    0x02, "20000"  )
	PORT_DIPSETTING(    0x00, "30000"  )
	PORT_DIPNAME( 0x08, 0x08, "Enemies", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x10, 0x10, "End of Level Pepper", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "No" )
	PORT_DIPSETTING(    0x00, "Yes" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
INPUT_PORTS_END

INPUT_PORTS_START( cookrace_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH,IPT_JOYSTICK_RIGHT | IPF_4WAY )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH,IPT_JOYSTICK_LEFT | IPF_4WAY )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH,IPT_JOYSTICK_UP | IPF_4WAY )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH,IPT_JOYSTICK_DOWN | IPF_4WAY )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH,IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH,IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH,IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH,IPT_UNUSED )

	PORT_START      /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH,IPT_JOYSTICK_RIGHT | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH,IPT_JOYSTICK_LEFT | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH,IPT_JOYSTICK_UP | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH,IPT_JOYSTICK_DOWN | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH,IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH,IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH,IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH,IPT_UNUSED )

	PORT_START      /* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH,IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH,IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH,IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH,IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH,IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH,IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START      /* DSW1 */
	PORT_DIPNAME( 0x03, 0x03, "Coin A", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x03, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x02, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x01, "1 Coin/3 Credits" )
	PORT_DIPNAME( 0x0c, 0x0c, "Coin B", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x0c, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x08, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/3 Credits" )
	PORT_DIPNAME( 0x10, 0x10, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x00, "Cabinet", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Upright" )
	PORT_DIPSETTING(    0x40, "Cocktail" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK  )

	PORT_START      /* DSW2 */
	PORT_DIPNAME( 0x01, 0x01, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, "Bonus Life", IP_KEY_NONE )
	PORT_DIPSETTING(    0x06, "20000" )
	PORT_DIPSETTING(    0x04, "30000" )
	PORT_DIPSETTING(    0x02, "40000"  )
	PORT_DIPSETTING(    0x00, "50000"  )
	PORT_DIPNAME( 0x08, 0x08, "Enemies", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x10, 0x10, "End of Level Pepper", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "No" )
	PORT_DIPSETTING(    0x00, "Yes" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
INPUT_PORTS_END

INPUT_PORTS_START( zoar_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START      /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START      /* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_COCKTAIL )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START      /* DSW1 */
	PORT_DIPNAME( 0x03, 0x03, "Coin A", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x03, "1 Coin/1 Credits" )
	PORT_DIPSETTING(    0x02, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x01, "1 Coin/3 Credits" )
	PORT_DIPNAME( 0x0c, 0x0c, "Coin B", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x0c, "1 Coin/1 Credits" )
	PORT_DIPSETTING(    0x08, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/3 Credits" )
	PORT_DIPNAME( 0x10, 0x10, "Unknown", IP_KEY_NONE )   /* almost certainly unused */
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x10, "On" )
	PORT_BITX(    0x20, 0x20, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x00, "Cabinet", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Upright" )
	PORT_DIPSETTING(    0x40, "Cocktail" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_VBLANK  )

	PORT_START      /* DSW2 */
	PORT_DIPNAME( 0x01, 0x01, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, "Bonus Life", IP_KEY_NONE )
	PORT_DIPSETTING(    0x06, "5000" )
	PORT_DIPSETTING(    0x04, "10000" )
	PORT_DIPSETTING(    0x02, "15000"  )
	PORT_DIPSETTING(    0x00, "20000"  )
	PORT_DIPNAME( 0x08, 0x08, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Easy" )
	PORT_DIPSETTING(    0x00, "Hard" )
	PORT_DIPNAME( 0x10, 0x00, "Weapon Select", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Manual" )
	PORT_DIPSETTING(    0x10, "Auto" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown", IP_KEY_NONE )  
	PORT_DIPSETTING(    0x00, "Off" )					
	PORT_DIPSETTING(    0x20, "On" )					
	PORT_DIPNAME( 0x40, 0x40, "Unknown", IP_KEY_NONE )	
	PORT_DIPSETTING(    0x00, "Off" )					
	PORT_DIPSETTING(    0x40, "On" )					
	PORT_DIPNAME( 0x80, 0x80, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x80, "On" )
INPUT_PORTS_END


INPUT_PORTS_START( eggs_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_4WAY )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_4WAY )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_4WAY )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_4WAY )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START      /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START      /* DSW1 */
	PORT_DIPNAME( 0x03, 0x03, "Coin A", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x03, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x01, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x02, "1 Coin/3 Credits" )
	PORT_DIPNAME( 0x0c, 0x0c, "Coin B", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x0c, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x04, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x08, "1 Coin/3 Credits" )
	PORT_BIT( 0x30, 0x30, IPT_UNKNOWN )     /* almost certainly unused */
	PORT_DIPNAME( 0x40, 0x00, "Cabinet", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Upright" )
	PORT_DIPSETTING(    0x40, "Cocktail" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK  )

	PORT_START      /* DSW2 */
	PORT_DIPNAME( 0x01, 0x01, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x04, "Bonus Life", IP_KEY_NONE )
	PORT_DIPSETTING(    0x04, "30000" )
	PORT_DIPSETTING(    0x02, "50000" )
	PORT_DIPSETTING(    0x00, "70000"  )
	PORT_DIPSETTING(    0x06, "Never"  )
	PORT_BIT( 0x78, 0x78, IPT_UNKNOWN )     /* almost certainly unused */
	PORT_DIPNAME( 0x80, 0x80, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Easy" )
	PORT_DIPSETTING(    0x00, "Hard" )
INPUT_PORTS_END


INPUT_PORTS_START( lnc_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_4WAY )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_4WAY )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_4WAY )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_4WAY )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START      /* DSW1 */
	PORT_DIPNAME( 0x03, 0x03, "Coin A", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x03, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x02, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x01, "1 Coin/3 Credits" )
	PORT_DIPNAME( 0x0c, 0x0c, "Coin B", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x0c, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x08, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/3 Credits" )
	PORT_DIPNAME( 0x30, 0x30, "Test Mode", IP_KEY_NONE )
	PORT_DIPSETTING(    0x30, "Off" )
	PORT_DIPSETTING(    0x00, "RAM Test Only" )
	PORT_DIPSETTING(    0x20, "Watchdog Test Only" )
	PORT_DIPSETTING(    0x10, "All Tests" )
	PORT_DIPNAME( 0x40, 0x00, "Cabinet", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Upright" )
	PORT_DIPSETTING(    0x40, "Cocktail" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK  )

	PORT_START      /* DSW2 */
	PORT_DIPNAME( 0x01, 0x01, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, "Bonus Life", IP_KEY_NONE )
	PORT_DIPSETTING(    0x06, "15000" )
	PORT_DIPSETTING(    0x04, "20000" )
	PORT_DIPSETTING(    0x02, "30000" )
	PORT_DIPSETTING(    0x00, "Never" )
	PORT_DIPNAME( 0x08, 0x08, "Game Speed", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Slow" )
	PORT_DIPSETTING(    0x00, "Hard" )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )	 
INPUT_PORTS_END


INPUT_PORTS_START( bnj_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START      /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START      /* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START      /* DSW1 */
	PORT_DIPNAME( 0x03, 0x03, "Coin A", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x03, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x02, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x01, "1 Coin/3 Credits" )
	PORT_DIPNAME( 0x0c, 0x0c, "Coin B", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x0c, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x08, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/3 Credits" )
	PORT_BITX(      0x10, 0x10, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
	PORT_DIPSETTING(    0x10, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x00, "Cabinet", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Upright" )
	PORT_DIPSETTING(    0x40, "Cocktail" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK  )

	PORT_START      /* DSW2 */
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x01, 0x01, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, "Bonus Life", IP_KEY_NONE )
	PORT_DIPSETTING(    0x06, "Every 30000" )
	PORT_DIPSETTING(    0x04, "Every 70000" )
	PORT_DIPSETTING(    0x02, "20000 Only"  )
	PORT_DIPSETTING(    0x00, "30000 Only"  )
	PORT_DIPNAME( 0x08, 0x00, "Allow Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "No" )
	PORT_DIPSETTING(    0x00, "Yes" )
	PORT_DIPNAME( 0x10, 0x10, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Easy" )
	PORT_DIPSETTING(    0x00, "Hard" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
INPUT_PORTS_END



static struct GfxLayout charlayout =
{
	8,8,    /* 8*8 characters */
	1024,   /* 1024 characters */
	3,      /* 3 bits per pixel */
	{ 0, 1024*8*8, 2*1024*8*8 },    /* the bitplanes are separated */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	8*8     /* every char takes 8 consecutive bytes */
};

static struct GfxLayout spritelayout =
{
	16,16,  /* 16*16 sprites */
	256,    /* 256 sprites */
	3,      /* 3 bits per pixel */
	{ 0, 256*16*16, 2*256*16*16 },  /* the bitplanes are separated */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
	  8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	{ 7, 6, 5, 4, 3, 2, 1, 0,
	  16*8+7, 16*8+6, 16*8+5, 16*8+4, 16*8+3, 16*8+2, 16*8+1, 16*8+0 },
	32*8    /* every sprite takes 32 consecutive bytes */
};

static struct GfxLayout zoar_spritelayout =
{
	16,16,  /* 16*16 sprites */
	128,    /* 256 sprites */
	3,      /* 3 bits per pixel */
	{ 0, 128*16*16, 2*128*16*16 },  /* the bitplanes are separated */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
	  8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	{ 7, 6, 5, 4, 3, 2, 1, 0,
	  16*8+7, 16*8+6, 16*8+5, 16*8+4, 16*8+3, 16*8+2, 16*8+1, 16*8+0 },
	32*8    /* every sprite takes 32 consecutive bytes */
};

static struct GfxLayout btime_tilelayout =
{
	16,16,  /* 16*16 characters */
	64,    /* 64 characters */
	3,      /* 3 bits per pixel */
	{ 0, 64*16*16, 64*2*16*16 },    /* the bitplanes are separated */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
	  8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	{ 7, 6, 5, 4, 3, 2, 1, 0,
	  16*8+7, 16*8+6, 16*8+5, 16*8+4, 16*8+3, 16*8+2, 16*8+1, 16*8+0 },
	32*8    /* every tile takes 32 consecutive bytes */
};

static struct GfxLayout cookrace_tilelayout =
{
	8,8,  /* 8*8 characters */
	256,    /* 256 characters */
	3,      /* 3 bits per pixel */
	{ 0, 256*8*8, 2*256*8*8 },    /* the bitplanes are separated */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	8*8    /* every tile takes 8 consecutive bytes */
};

static struct GfxLayout bnj_tilelayout =
{
	16,16,  /* 16*16 characters */
	64, /* 64 characters */
	3,  /* 3 bits per pixel */
	{ 2*64*16*16+4, 0, 4 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
	  8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	{ 3, 2, 1, 0, 16*8+3, 16*8+2, 16*8+1, 16*8+0,
	  2*16*8+3, 2*16*8+2, 2*16*8+1, 2*16*8+0, 3*16*8+3, 3*16*8+2, 3*16*8+1, 3*16*8+0 },
	64*8    /* every tile takes 64 consecutive bytes */
};

static struct GfxDecodeInfo btime_gfxdecodeinfo[] =
{
	{ 1, 0x0000, &charlayout,       0, 1 }, /* char set #1 */
	{ 1, 0x0000, &spritelayout,     0, 1 }, /* sprites */
	{ 1, 0x6000, &btime_tilelayout, 8, 1 }, /* background tiles */
	{ -1 } /* end of array */
};

static struct GfxDecodeInfo cookrace_gfxdecodeinfo[] =
{
	{ 1, 0x0000, &charlayout,          0, 1 }, /* char set #1 */
	{ 1, 0x0000, &spritelayout,        0, 1 }, /* sprites */
	{ 1, 0x6000, &cookrace_tilelayout, 8, 1 }, /* background tiles */
	{ -1 } /* end of array */
};

static struct GfxDecodeInfo lnc_gfxdecodeinfo[] =
{
	{ 1, 0x0000, &charlayout,   0, 1 },     /* char set #1 */
	{ 1, 0x0000, &spritelayout, 0, 1 },     /* sprites */
	{ -1 } /* end of array */
};

static struct GfxDecodeInfo bnj_gfxdecodeinfo[] =
{
	{ 1, 0x0000, &charlayout,       0, 1 }, /* char set #1 */
	{ 1, 0x0000, &spritelayout,     0, 1 }, /* sprites */
	{ 1, 0x6000, &bnj_tilelayout,   8, 1 }, /* background tiles */
	{ -1 } /* end of array */
};

static struct GfxDecodeInfo zoar_gfxdecodeinfo[] =
{
	{ 1, 0x0000, &charlayout,       0, 8 }, /* char set #1 */
	{ 1, 0x7800, &zoar_spritelayout,0, 8 }, /* sprites */
	{ 1, 0x6000, &btime_tilelayout, 0, 8 }, /* background tiles */
	{ -1 } /* end of array */
};

static struct AY8910interface ay8910_interface =
{
	2,      /* 2 chips */
	1500000,        /* 1.5 MHz ? (hand tuned) */
	{ 255, 255 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 }
};

/********************************************************************
*
*  Machine Driver macro
*  ====================
*
*  Abusing the pre-processor.
*
********************************************************************/

#define cookrace_vh_convert_color_prom   btime_vh_convert_color_prom
#define bnj_vh_convert_color_prom        0
#define eggs_vh_convert_color_prom       btime_vh_convert_color_prom
#define zoar_vh_convert_color_prom       btime_vh_convert_color_prom

#define btime_init_machine     0
#define cookrace_init_machine  0
#define bnj_init_machine       0
#define zoar_init_machine      0

#define btime_vh_start     generic_vh_start
#define cookrace_vh_start  generic_vh_start
#define zoar_vh_start      generic_vh_start
#define eggs_vh_start      generic_vh_start
#define lnc_vh_start       generic_vh_start

#define btime_vh_stop      generic_vh_stop
#define cookrace_vh_stop   generic_vh_stop
#define zoar_vh_stop       generic_vh_stop
#define eggs_vh_stop       generic_vh_stop
#define lnc_vh_stop        generic_vh_stop


#define MACHINE_DRIVER(GAMENAME, CLOCK, MAIN_IRQ, SOUND_IRQ, GFX, COLOR)   \
																	\
static struct MachineDriver GAMENAME##_machine_driver =             \
{                                                                   \
	/* basic machine hardware */                                	\
	{		                                                        \
		{	  	                                                    \
			CPU_M6502,                                  			\
			CLOCK,													\
			0,                                          			\
			GAMENAME##_readmem,GAMENAME##_writemem,0,0, 			\
			MAIN_IRQ,1                                  			\
		},		                                                    \
		{		                                                    \
			CPU_M6502 | CPU_AUDIO_CPU,                  			\
			500000, /* 500 kHz */                       			\
			2,      /* memory region #2 */              			\
			sound_readmem,sound_writemem,0,0,           			\
			SOUND_IRQ,16   /* IRQs are triggered by the main CPU */ \
		}                                                   		\
	},                                                          	\
	57, 3072,        /* frames per second, vblank duration */   	\
	1,      /* 1 CPU slice per frame - interleaving is forced when a sound command is written */ \
	GAMENAME##_init_machine,		                               	\
																	\
	/* video hardware */                                        	\
	32*8, 32*8, { 1*8, 31*8-1, 0*8, 32*8-1 },                   	\
	GFX,                                                        	\
	COLOR,COLOR,                                                	\
	GAMENAME##_vh_convert_color_prom,                           	\
																	\
	VIDEO_TYPE_RASTER|VIDEO_MODIFIES_PALETTE,                   	\
	0,                                                          	\
	GAMENAME##_vh_start,                                        	\
	GAMENAME##_vh_stop,                                         	\
	GAMENAME##_vh_screenrefresh,                                   	\
																	\
	/* sound hardware */                                        	\
	0,0,0,0,                                                    	\
	{                                                           	\
		{                                                   		\
			SOUND_AY8910,                               			\
			&ay8910_interface                           			\
		}                                                   		\
	}                                                           	\
}

#define EGGS_MACHINE_DRIVER(GAMENAME, CLOCK, MAIN_IRQ, SOUND_IRQ, GFX, COLOR)   \
																	\
static struct MachineDriver GAMENAME##_machine_driver =             \
{                                                                   \
	/* basic machine hardware */                                	\
	{		                                                        \
		{	  	                                                    \
			CPU_M6502,                                  			\
			CLOCK,													\
			0,                                          			\
			GAMENAME##_readmem,GAMENAME##_writemem,0,0, 			\
			MAIN_IRQ,1                                  			\
		}                                                   		\
	},                                                          	\
	57, 3072,        /* frames per second, vblank duration */   	\
	1,      /* 1 CPU slice per frame - interleaving is forced when a sound command is written */ \
	0,						                                    	\
																	\
	/* video hardware */                                        	\
	32*8, 32*8, { 1*8, 31*8-1, 0*8, 32*8-1 },                   	\
	GFX,                                                        	\
	COLOR,COLOR,                                                	\
	GAMENAME##_vh_convert_color_prom,                           	\
																	\
	VIDEO_TYPE_RASTER|VIDEO_MODIFIES_PALETTE,                   	\
	0,                                                          	\
	GAMENAME##_vh_start,                                        	\
	GAMENAME##_vh_stop,                                         	\
	GAMENAME##_vh_screenrefresh,                                   	\
																	\
	/* sound hardware */                                        	\
	0,0,0,0,                                                    	\
	{                                                           	\
		{                                                   		\
			SOUND_AY8910,                               			\
			&ay8910_interface                           			\
		}                                                   		\
	}                                                           	\
}


MACHINE_DRIVER(btime, 1500000, btime_irq_interrupt, nmi_interrupt, btime_gfxdecodeinfo, 16);

MACHINE_DRIVER(cookrace, 1500000, btime_nmi_interrupt, nmi_interrupt, cookrace_gfxdecodeinfo, 16);

EGGS_MACHINE_DRIVER(eggs, 1500000, interrupt, nmi_interrupt, lnc_gfxdecodeinfo, 8);

MACHINE_DRIVER(lnc, 1500000, btime_nmi_interrupt, lnc_sound_interrupt, lnc_gfxdecodeinfo, 8);

MACHINE_DRIVER(bnj, 750000, btime_nmi_interrupt, nmi_interrupt, bnj_gfxdecodeinfo, 16);

MACHINE_DRIVER(zoar, 1500000, zoar_irq_interrupt, nmi_interrupt, zoar_gfxdecodeinfo, 64);

/***************************************************************************

	Game driver(s)

***************************************************************************/

ROM_START( btime_rom )
	ROM_REGION(0x10000)     /* 64k for code */
	ROM_LOAD( "ab05a1.12b",   0xb000, 0x1000, 0x0a98b230 )
	ROM_LOAD( "ab04.9b",      0xc000, 0x1000, 0x797e5f75 )
	ROM_LOAD( "ab06.13b",     0xd000, 0x1000, 0xc77f3f64 )
	ROM_LOAD( "ab05.10b",     0xe000, 0x1000, 0xb0d3640f )
	ROM_LOAD( "ab07.15b",     0xf000, 0x1000, 0xa142f862 )

	ROM_REGION_DISPOSE(0x7800)      /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "ab8.13k",      0x0000, 0x1000, 0x70b35bbe )    /* charset #1 */
	ROM_LOAD( "ab9.15k",      0x1000, 0x1000, 0x8dec15e6 )
	ROM_LOAD( "ab10.10k",     0x2000, 0x1000, 0x854a872a )
	ROM_LOAD( "ab11.12k",     0x3000, 0x1000, 0xd4848014 )
	ROM_LOAD( "ab12.7k",      0x4000, 0x1000, 0x6c79f79f )
	ROM_LOAD( "ab13.9k",      0x5000, 0x1000, 0xac01042f )
	ROM_LOAD( "ab02.4b",      0x6000, 0x0800, 0xb8ef56c3 )    /* charset #2 */
	ROM_LOAD( "ab01.3b",      0x6800, 0x0800, 0x25b49078 )
	ROM_LOAD( "ab00.1b",      0x7000, 0x0800, 0xc7a14485 )

	ROM_REGION(0x10000)     /* 64k for the audio CPU */
	ROM_LOAD( "ab14.12h",     0xf000, 0x1000, 0xf55e5211 )

	ROM_REGION(0x0800)      /* background graphics */
	ROM_LOAD( "ab03.6b",      0x0000, 0x0800, 0xd26bc1f3 )
ROM_END

ROM_START( btimea_rom )
	ROM_REGION(0x10000)     /* 64k for code */
	ROM_LOAD( "aa04.9b",      0xc000, 0x1000, 0xa041e25b )
	ROM_LOAD( "aa06.13b",     0xd000, 0x1000, 0xb4ba400d )
	ROM_LOAD( "aa05.10b",     0xe000, 0x1000, 0x8005bffa )
	ROM_LOAD( "aa07.15b",     0xf000, 0x1000, 0x086440ad )

	ROM_REGION_DISPOSE(0x7800)      /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "aa8.13k",      0x0000, 0x1000, 0x8650c788 )    /* charset #1 */
	ROM_LOAD( "ab9.15k",      0x1000, 0x1000, 0x8dec15e6 )
	ROM_LOAD( "ab10.10k",     0x2000, 0x1000, 0x854a872a )
	ROM_LOAD( "ab11.12k",     0x3000, 0x1000, 0xd4848014 )
	ROM_LOAD( "aa12.7k",      0x4000, 0x1000, 0xc4617243 )
	ROM_LOAD( "ab13.9k",      0x5000, 0x1000, 0xac01042f )
	ROM_LOAD( "ab02.4b",      0x6000, 0x0800, 0xb8ef56c3 )    /* charset #2 */
	ROM_LOAD( "ab01.3b",      0x6800, 0x0800, 0x25b49078 )
	ROM_LOAD( "ab00.1b",      0x7000, 0x0800, 0xc7a14485 )

	ROM_REGION(0x10000)     /* 64k for the audio CPU */
	ROM_LOAD( "ab14.12h",     0xf000, 0x1000, 0xf55e5211 )

	ROM_REGION(0x0800)      /* background graphics */
	ROM_LOAD( "ab03.6b",      0x0000, 0x0800, 0xd26bc1f3 )
ROM_END

ROM_START( cookrace_rom )
	ROM_REGION(0x10000)     /* 64k for code */
	/* code is in the range 0500-3fff, encrypted */
	ROM_LOAD( "inc.1",        0x0000, 0x2000, 0x68759d32 )
	ROM_LOAD( "inc.2",        0x2000, 0x2000, 0xbe7d72d1 )

	ROM_REGION_DISPOSE(0x7800)      /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "inc.9",        0x0000, 0x2000, 0xd0d94477 ) /* charset #1 */
	ROM_LOAD( "inc.8",        0x2000, 0x2000, 0x1104f497 )
	ROM_LOAD( "inc.7",        0x4000, 0x2000, 0xa1a0d5a6 )
	ROM_LOAD( "inc.5",        0x6000, 0x0800, 0x611c686f ) /* garbage?? */
	ROM_CONTINUE(             0x6000, 0x0800 )             /* charset #2 */
	ROM_LOAD( "inc.4",        0x6800, 0x0800, 0x7742e771 ) /* garbage?? */
	ROM_CONTINUE(             0x6800, 0x0800 )
	ROM_LOAD( "inc.3",        0x7000, 0x0800, 0x28609a75 ) /* garbage?? */
	ROM_CONTINUE(             0x7000, 0x0800 )

	ROM_REGION(0x10000)     /* 64k for the audio CPU */
	ROM_LOAD( "inc.6",        0x0000, 0x1000, 0x6b8e0272 ) /* starts at 0000, not f000; 0000-01ff is RAM */
	ROM_RELOAD(               0xf000, 0x1000 )     /* for the reset/interrupt vectors */

    ROM_REGION(0x0020)
    ROM_LOAD( "hamburge.clr", 0x0000, 0x0020, 0xc2348c1d )
ROM_END

static void cookrace_decode(void)
{
	int A;
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];


	for (A = 0;A < 0x4000;A++)
		ROM[A] = (RAM[A] & 0x9f) | ((RAM[A] & 0x20) << 1) | ((RAM[A] & 0x40) >> 1);

	/* fill in the reset/interrupt vectors */
	ROM[0xfff9] = 0x40;

	RAM[0xfffa] = 0x00;
	RAM[0xfffb] = 0x05;
	RAM[0xfffc] = 0x03;
	RAM[0xfffd] = 0x05;
	RAM[0xfffe] = 0xF9;
	RAM[0xffff] = 0xFF;
}


ROM_START( eggs_rom )
	ROM_REGION(0x10000)     /* 64k for code */
	ROM_LOAD( "e14.bin",      0x3000, 0x1000, 0x4e216f9d )
	ROM_LOAD( "d14.bin",      0x4000, 0x1000, 0x4edb267f )
	ROM_LOAD( "c14.bin",      0x5000, 0x1000, 0x15a5c48c )
	ROM_LOAD( "b14.bin",      0x6000, 0x1000, 0x5c11c00e )
	ROM_LOAD( "a14.bin",      0x7000, 0x1000, 0x953faf07 )
	ROM_RELOAD(               0xf000, 0x1000 )   /* for reset/interrupt vectors */

	ROM_REGION_DISPOSE(0x6000)      /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "g12.bin",      0x0000, 0x1000, 0x679f8af7 )
	ROM_LOAD( "g10.bin",      0x1000, 0x1000, 0x5b58d3b5 )
	ROM_LOAD( "h12.bin",      0x2000, 0x1000, 0x9562836d )
	ROM_LOAD( "h10.bin",      0x3000, 0x1000, 0x3cfb3a8e )
	ROM_LOAD( "j12.bin",      0x4000, 0x1000, 0xce4a2e46 )
	ROM_LOAD( "j10.bin",      0x5000, 0x1000, 0xa1bcaffc )

	ROM_REGION(0x0020)	/* color PROM */
	ROM_LOAD( "screggco.c6",  0x0000, 0x0020, 0xff23bdd6 )
ROM_END

ROM_START( scregg_rom )
	ROM_REGION(0x10000)     /* 64k for code */
	ROM_LOAD( "scregg.e14",   0x3000, 0x1000, 0x29226d77 )
	ROM_LOAD( "scregg.d14",   0x4000, 0x1000, 0xeb143880 )
	ROM_LOAD( "scregg.c14",   0x5000, 0x1000, 0x4455f262 )
	ROM_LOAD( "scregg.b14",   0x6000, 0x1000, 0x044ac5d2 )
	ROM_LOAD( "scregg.a14",   0x7000, 0x1000, 0xb5a0814a )
	ROM_RELOAD(               0xf000, 0x1000 )        /* for reset/interrupt vectors */

	ROM_REGION_DISPOSE(0x6000)      /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "scregg.g12",   0x0000, 0x1000, 0xff3c2894 )
	ROM_LOAD( "scregg.g10",   0x1000, 0x1000, 0x9c20214a )
	ROM_LOAD( "scregg.h12",   0x2000, 0x1000, 0x8454f4b2 )
	ROM_LOAD( "scregg.h10",   0x3000, 0x1000, 0x72bd89ee )
	ROM_LOAD( "scregg.j12",   0x4000, 0x1000, 0xa485c10c )
	ROM_LOAD( "scregg.j10",   0x5000, 0x1000, 0x1fd4e539 )

	ROM_REGION(0x0020)	/* color PROM */
	ROM_LOAD( "screggco.c6",  0x0000, 0x0020, 0xff23bdd6 )
ROM_END

/* There is a flyer with a screen shot for Lock'n'Chase at:
   http://www.gamearchive.com/flyers/video/taito/locknchase_f.jpg  */

ROM_START( lnc_rom )
	ROM_REGION(0x10000)     /* 64k for code */
	ROM_LOAD( "s3-3d",        0xc000, 0x1000, 0x1ab4f2c2 )
	ROM_LOAD( "s2-3c",        0xd000, 0x1000, 0x5e46b789 )
	ROM_LOAD( "s1-3b",        0xe000, 0x1000, 0x1308a32e )
	ROM_LOAD( "s0-3a",        0xf000, 0x1000, 0xbeb4b1fc )

	ROM_REGION_DISPOSE(0x6000)      /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "s8-15l",       0x0000, 0x1000, 0x672a92d0 )
	ROM_LOAD( "s9-15m",       0x1000, 0x1000, 0x87c8ee9a )
	ROM_LOAD( "s6-13l",       0x2000, 0x1000, 0xd21e2a57 )
	ROM_LOAD( "s7-13m",       0x3000, 0x1000, 0xc4f247cd )
	ROM_LOAD( "s4-11l",       0x4000, 0x1000, 0xa2162a9e )
	ROM_LOAD( "s5-11m",       0x5000, 0x1000, 0x12f1c2db )

	ROM_REGION(0x10000)     /* 64k for the audio CPU */
	ROM_LOAD( "sa-1h",        0xf000, 0x1000, 0x379387ec )

    ROM_REGION(0x0040)	/* PROMs */
    ROM_LOAD( "sc-5m",        0x0000, 0x0020, 0x2a976ebe )	/* palette */
    ROM_LOAD( "sb-4c",        0x0020, 0x0020, 0xa29b4204 )	/* RAS/CAS logic - not used */
ROM_END

ROM_START( bnj_rom )
	ROM_REGION(0x10000)     /* 64k for code */
	ROM_LOAD( "bnj12b.bin",   0xa000, 0x2000, 0xba3e3801 )
	ROM_LOAD( "bnj12c.bin",   0xc000, 0x2000, 0xfb3a2cdd )
	ROM_LOAD( "bnj12d.bin",   0xe000, 0x2000, 0xb88bc99e )

	ROM_REGION_DISPOSE(0x8000)      /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "bnj4h.bin",    0x0000, 0x2000, 0x5824e6fb )
	ROM_LOAD( "bnj4f.bin",    0x2000, 0x2000, 0x6c31d77a )
	ROM_LOAD( "bnj4e.bin",    0x4000, 0x2000, 0xb864d082 )
	ROM_LOAD( "bnj10e.bin",   0x6000, 0x1000, 0xf4e9eb49 )
	ROM_LOAD( "bnj10f.bin",   0x7000, 0x1000, 0xa9ffacb4 )

	ROM_REGION(0x10000)     /* 64k for the audio CPU */
	ROM_LOAD( "bnj6c.bin",    0xf000, 0x1000, 0x8c02f662 )
ROM_END

ROM_START( brubber_rom )
	ROM_REGION(0x10000)     /* 64k for code */
	/* a000-bfff space for the service ROM */
	ROM_LOAD( "brubber.12c",  0xc000, 0x2000, 0xb5279c70 )
	ROM_LOAD( "brubber.12d",  0xe000, 0x2000, 0xb2ce51f5 )

	ROM_REGION_DISPOSE(0x8000)      /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "bnj4h.bin",    0x0000, 0x2000, 0x5824e6fb )
	ROM_LOAD( "bnj4f.bin",    0x2000, 0x2000, 0x6c31d77a )
	ROM_LOAD( "bnj4e.bin",    0x4000, 0x2000, 0xb864d082 )
	ROM_LOAD( "bnj10e.bin",   0x6000, 0x1000, 0xf4e9eb49 )
	ROM_LOAD( "bnj10f.bin",   0x7000, 0x1000, 0xa9ffacb4 )

	ROM_REGION(0x10000)     /* 64k for the audio CPU */
	ROM_LOAD( "bnj6c.bin",    0xf000, 0x1000, 0x8c02f662 )
ROM_END

ROM_START( caractn_rom )
	ROM_REGION(0x10000)     /* 64k for code */
	/* a000-bfff space for the service ROM */
	ROM_LOAD( "brubber.12c",  0xc000, 0x2000, 0xb5279c70 )
	ROM_LOAD( "caractn.a6",   0xe000, 0x2000, 0x1d6957c4 )

	ROM_REGION_DISPOSE(0x8000)      /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "caractn.a2",   0x0000, 0x2000, 0x51dcc111 )
	ROM_LOAD( "caractn.a1",   0x2000, 0x2000, 0x9789f639 )
	ROM_LOAD( "caractn.a0",   0x4000, 0x2000, 0xbf3ea732 )
	ROM_LOAD( "bnj10e.bin",   0x6000, 0x1000, 0xf4e9eb49 )
	ROM_LOAD( "bnj10f.bin",   0x7000, 0x1000, 0xa9ffacb4 )

	ROM_REGION(0x10000)     /* 64k for the audio CPU */
	ROM_LOAD( "bnj6c.bin",    0xf000, 0x1000, 0x8c02f662 )
ROM_END

ROM_START( zoar_rom )
	ROM_REGION(0x10000)     /* 64k for code */
	ROM_LOAD( "zoar15",       0xd000, 0x1000, 0x1f0cfdb7 )
	ROM_LOAD( "zoar16",       0xe000, 0x1000, 0x7685999c )
	ROM_LOAD( "zoar17",       0xf000, 0x1000, 0x619ea867 )

	ROM_REGION_DISPOSE(0xa800)      /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "zoar06",       0x0000, 0x1000, 0x07638c71 )
	ROM_LOAD( "zoar07",       0x1000, 0x1000, 0xf4710f25 )
	ROM_LOAD( "zoar03",       0x2000, 0x1000, 0x77b7df14 )
	ROM_LOAD( "zoar04",       0x3000, 0x1000, 0x9be786de )
	ROM_LOAD( "zoar00",       0x4000, 0x1000, 0xfd2dcb64 )
	ROM_LOAD( "zoar01",       0x5000, 0x1000, 0x74d3ca48 )

	ROM_LOAD( "zoar12",       0x6000, 0x0800, 0xed317e40 )
	ROM_LOAD( "zoar11",       0x6800, 0x0800, 0xdcdad357 )
	ROM_LOAD( "zoar10",       0x7000, 0x0800, 0xaa8bcab8 )

	ROM_LOAD( "zoar08",       0x7800, 0x1000, 0x9a148551 )
	ROM_LOAD( "zoar05",       0x8800, 0x1000, 0x05dc6b09 )
	ROM_LOAD( "zoar02",       0x9800, 0x1000, 0xd8c3c122 )

	ROM_REGION(0x10000)      /* 64k for the audio CPU */
	ROM_LOAD( "zoar09",       0xf000, 0x1000, 0x18d96ff1 )

	ROM_REGION(0x1000)      /* background graphics */
	ROM_LOAD( "zoar13",       0x0000, 0x1000, 0x8fefa960 )

    ROM_REGION(0x40)
    ROM_LOAD( "z20-1l",       0x0000, 0x0020, 0xa63f0a07 )
    ROM_LOAD( "z21-1l",       0x0020, 0x0020, 0x5e1e5788 )
ROM_END


static void btime_decode(void)
{
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];


	/* For now, just copy the RAM array over to ROM. Decryption will happen */
	/* at run time, since the CPU applies the decryption only if the previous */
	/* instruction did a memory write. */
	fast_memcpy(ROM,RAM,0x10000);
}

static void zoar_decode(void)
{
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];


	fast_memset(&RAM[0xd50a],0xea,8);

	/* For now, just copy the RAM array over to ROM. Decryption will happen */
	/* at run time, since the CPU applies the decryption only if the previous */
	/* instruction did a memory write. */
	fast_memcpy(ROM,RAM,0x10000);
}

static void lnc_decode (void)
{
	static int encrypttable[] = { 0x00, 0x10, 0x40, 0x50, 0x20, 0x30, 0x60, 0x70,
								  0x80, 0x90, 0xc0, 0xd0, 0xa0, 0xb0, 0xe0, 0xf0};

	int A;
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];


	for (A = 0;A < 0x10000;A++)
	{
		/*
		 *  The encryption is dead simple.  Just swap bits 5 & 6 for opcodes
		 *  only.
		 */
		ROM[A] = (RAM[A] & 0x0f) | encrypttable[RAM[A] >> 4];
	}
}




static int btime_hiload(void)
{
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];


	/* check if the hi score table has already been initialized */
	if (memcmp(&RAM[0x0036],"\x00\x80\x02",3) == 0 &&
		memcmp(&RAM[0x0042],"\x50\x48\00",3) == 0)
	{
		void *f;

		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
			osd_fread(f,&RAM[0x0036],6*6);
			RAM[0x0033] = RAM[0x0036];
			RAM[0x0034] = RAM[0x0037];
			RAM[0x0035] = RAM[0x0038];
			osd_fclose(f);
		}

		return 1;
	}
	else return 0;  /* we can't load the hi scores yet */
}

static void btime_hisave(void)
{
	void *f;
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];


	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
		osd_fwrite(f,&RAM[0x0036],6*6);
		osd_fclose(f);
	}
}


static int eggs_hiload(void)
{
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];


	/* check if the hi score table has already been initialized */
	if ((memcmp(&RAM[0x0400],"\x17\x25\x19",3)==0) &&
		(memcmp(&RAM[0x041B],"\x00\x47\x00",3) == 0))
	{
		void *f;

		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
			osd_fread(f,&RAM[0x0400],0x1E);
			/* Fix hi score at top */
			fast_memcpy(&RAM[0x0015],&RAM[0x0403],3);
			osd_fclose(f);
		}

		return 1;
	}
	else return 0;  /* we can't load the hi scores yet */
}

static void eggs_hisave(void)
{
	void *f;
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];


	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
		osd_fwrite(f,&RAM[0x0400],0x1E);
		osd_fclose(f);
	}
}

static int lnc_hiload(void)
{
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];
	static int firsttime=0;
	/* check if the hi score table has already been initialized */
	/* the high score table is intialized to all 0, so first of all */
	/* we dirty it, then we wait for it to be cleared again */
	if (firsttime == 0)
	{
		fast_memset(&RAM[0x0008],0xff,3);
		fast_memset(&RAM[0x0294],0xff,16);
		fast_memset(&RAM[0x02a6],0xff,16);
		firsttime = 1;
	}

	/*   Check if the hi score table has already been initialized.
	 */
	if ((memcmp(&RAM[0x0008],"\x00\x00\x00",3)==0) &&
			(memcmp(&RAM[0x02a6],"\x00\x00\x00",3) == 0))

	{
		void *f;

		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
			int banksave, i;

			osd_fread(f,&RAM[0x0008],0x03);
			osd_fread(f,&RAM[0x0294],0x0f);
			osd_fread(f,&RAM[0x02a6],0x0f);

			/* Put high score on screen as we missed when it was done */
			/* by the program */
			banksave = *lnc_charbank;
			*lnc_charbank = 0;

			lnc_videoram_w(0x004c, 0x0b);
			lnc_videoram_w(0x0052, (RAM[0x0008] & 0x0f) + 1);
			lnc_videoram_w(0x0051, (RAM[0x0008] >> 4) + 1);
			lnc_videoram_w(0x0050, (RAM[0x0009] & 0x0f) + 1);
			lnc_videoram_w(0x004f, (RAM[0x0009] >> 4) + 1);
			lnc_videoram_w(0x004e, (RAM[0x000a] & 0x0f) + 1);
			lnc_videoram_w(0x004d, (RAM[0x000a] >> 4) + 1);

			/* Remove leading zeros */
			for (i = 0; i < 5; i++)
			{
				if (videoram_r(0x004d + i) != 0x01) break;

				lnc_videoram_w(0x004c + i, 0x00);
				lnc_videoram_w(0x004d + i, 0x0b);
			}

			*lnc_charbank = banksave;

			osd_fclose(f);
		}
		firsttime = 0;
		return 1;
	}

	/*
	 *  The hi scores can't be loaded yet.
	 */
	return 0;
}

static void lnc_hisave(void)
{
	void *f;
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];


	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
		osd_fwrite(f,&RAM[0x0008],0x03);
		osd_fwrite(f,&RAM[0x0294],0x0f);
		osd_fwrite(f,&RAM[0x02a6],0x0f);
		osd_fclose(f);
	}
}

static int bnj_hiload(void)
{
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];


	/*   Check if the hi score table has already been initialized.
	 */
	if ((memcmp(&RAM[0x0500],"\x01\x00\x12",3)==0) &&
		(memcmp(&RAM[0x050c],"\x00\x19\x82",3) == 0))

	/*if (RAM[0x0640] == 0x4d)*/
	{
		void *f;

		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
			osd_fread(f,&RAM[0x0500],15);
			osd_fread(f,&RAM[0x0640],15);
			osd_fclose(f);
			/* copy top score to top of screen */
			RAM[0x000c] = RAM[0x0500];
			RAM[0x000b] = RAM[0x0501];
			RAM[0x000a] = RAM[0x0502];
		}

		return 1;
	}

	/*
	 *  The hi scores can't be loaded yet.
	 */
	return 0;
}

static void bnj_hisave(void)
{
	void *f;
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];


	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
		osd_fwrite(f,&RAM[0x0500],15);
		osd_fwrite(f,&RAM[0x0640],15);
		osd_fclose(f);
	}
}


static int zoar_hiload(void)
{
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];


	/* check if the hi score table has already been initialized */
	if (memcmp(&RAM[0x034b],"\x20",1) == 0)
	{
		void *f;

		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
			osd_fread(f,&RAM[0x02dd],3);
			osd_fread(f,&RAM[0x02e5],5*3);
			osd_fread(f,&RAM[0x034b],3);
			osd_fread(f,&RAM[0x0356],3);
			osd_fread(f,&RAM[0x0361],3);
			osd_fread(f,&RAM[0x036c],3);
			osd_fread(f,&RAM[0x0377],3);
			osd_fclose(f);
		}

		return 1;
	}
	else return 0;  /* we can't load the hi scores yet */
}

static void zoar_hisave(void)
{
	void *f;
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];


	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
		osd_fwrite(f,&RAM[0x02dd],3);
		osd_fwrite(f,&RAM[0x02e5],5*3);
		osd_fwrite(f,&RAM[0x034b],3);
		osd_fwrite(f,&RAM[0x0356],3);
		osd_fwrite(f,&RAM[0x0361],3);
		osd_fwrite(f,&RAM[0x036c],3);
		osd_fwrite(f,&RAM[0x0377],3);
		osd_fclose(f);
	}
}



struct GameDriver btime_driver =
{
	__FILE__,
	0,
	"btime",
	"Burger Time (Midway)",
	"1982",
	"Data East (Bally Midway license)",
	"Kevin Brisley (Replay emulator)\nMirko Buffoni (MAME driver)\nNicola Salmoria (MAME driver)\nZsolt Vasvari (ROM decryption)",
	0,
	&btime_machine_driver,
	0,

	btime_rom,
	0, btime_decode,
	0,
	0,	/* sound_prom */

	btime_input_ports,

	0, 0, 0,
	ORIENTATION_DEFAULT,

	btime_hiload, btime_hisave
};

struct GameDriver btimea_driver =
{
	__FILE__,
	&btime_driver,
	"btimea",
	"Burger Time (Data East)",
	"1982",
	"Data East Corporation",
	"Kevin Brisley (Replay emulator)\nMirko Buffoni (MAME driver)\nNicola Salmoria (MAME driver)\nZsolt Vasvari (ROM decryption)",
	0,
	&btime_machine_driver,
	0,

	btimea_rom,
	0, btime_decode,
	0,
	0,	/* sound_prom */

	btime_input_ports,

	0, 0, 0,
	ORIENTATION_DEFAULT,

	btime_hiload, btime_hisave
};

struct GameDriver cookrace_driver =
{
	__FILE__,
	&btime_driver,
	"cookrace",
	"Cook Race",
	"1982",
	"bootleg",
	"Kevin Brisley (Replay emulator)\nMirko Buffoni (MAME driver)\nNicola Salmoria (MAME driver)",
	0,
	&cookrace_machine_driver,
	0,

	cookrace_rom,
	0, cookrace_decode,
	0,
	0,	/* sound_prom */

	cookrace_input_ports,

    PROM_MEMORY_REGION(3), 0, 0,
	ORIENTATION_DEFAULT,

	0, 0
};

struct GameDriver eggs_driver =
{
	__FILE__,
	0,
	"eggs",
	"Eggs",
	"1983",
	"Universal USA",
	"Nicola Salmoria",
	0,
	&eggs_machine_driver,
	0,

	eggs_rom,
	0, 0,
	0,
	0,	/* sound_prom */

	eggs_input_ports,

	PROM_MEMORY_REGION(2), 0, 0,
	ORIENTATION_DEFAULT,

	eggs_hiload, eggs_hisave
};

struct GameDriver scregg_driver =
{
	__FILE__,
	&eggs_driver,
	"scregg",
	"Scrambled Egg",
	"1983",
	"Technos",
	"Nicola Salmoria",
	0,
	&eggs_machine_driver,
	0,

	scregg_rom,
	0, 0,
	0,
	0,	/* sound_prom */

	eggs_input_ports,

	PROM_MEMORY_REGION(2), 0, 0,
	ORIENTATION_DEFAULT,

	eggs_hiload, eggs_hisave
};

struct GameDriver lnc_driver =
{
	__FILE__,
	0,
	"lnc",
	"Lock'n'Chase",
	"1981",
	"Data East Corporation",
	"Zsolt Vasvari\nKevin Brisley (Bump 'n' Jump driver)\nMirko Buffoni (Audio/Add. code)",
	0,
	&lnc_machine_driver,
	0,

	lnc_rom,
	0, lnc_decode,
	0,
	0,	/* sound_prom */

	lnc_input_ports,

    PROM_MEMORY_REGION(3), 0, 0,
	ORIENTATION_DEFAULT,

	lnc_hiload, lnc_hisave
};

struct GameDriver bnj_driver =
{
	__FILE__,
	0,
	"bnj",
	"Bump 'n' Jump",
	"1982",
	"Data East USA (Bally Midway license)",
	"Kevin Brisley (MAME driver)\nMirko Buffoni (Audio/Add. code)",
	0,
	&bnj_machine_driver,
	0,

	bnj_rom,
	0, lnc_decode,
	0,
	0,	/* sound_prom */

	bnj_input_ports,

	0, 0, 0,
	ORIENTATION_DEFAULT,

	bnj_hiload, bnj_hisave
};

struct GameDriver brubber_driver =
{
	__FILE__,
	&bnj_driver,
	"brubber",
	"Burnin' Rubber",
	"1982",
	"Data East",
	"Kevin Brisley (MAME driver)\nMirko Buffoni (Audio/Add. code)",
	0,
	&bnj_machine_driver,
	0,

	brubber_rom,
	0, lnc_decode,
	0,
	0,	/* sound_prom */

	bnj_input_ports,

	0, 0, 0,
	ORIENTATION_DEFAULT,

	bnj_hiload, bnj_hisave
};

struct GameDriver caractn_driver =
{
	__FILE__,
	&bnj_driver,
	"caractn",
	"Car Action",
	"1983",
	"bootleg",
	"Ivan Mackintosh\nKevin Brisley (Bump 'n' Jump driver)\nMirko Buffoni (Audio/Add. code)",
	0,
	&bnj_machine_driver,
	0,

	caractn_rom,
	0, lnc_decode,
	0,
	0,	/* sound_prom */

	bnj_input_ports,

	0, 0, 0,
	ORIENTATION_DEFAULT,

	bnj_hiload, bnj_hisave
};

struct GameDriver zoar_driver =
{
	__FILE__,
	0,
	"zoar",
	"Zoar",
	"1982",
	"Data East USA",
	"Zsolt Vasvari\nKevin Brisley (Bump 'n' Jump driver)",
	0,
	&zoar_machine_driver,
	0,

	zoar_rom,
	0, zoar_decode,
	0,
	0,	/* sound_prom */

	zoar_input_ports,

    PROM_MEMORY_REGION(4), 0, 0,
	ORIENTATION_DEFAULT,

	zoar_hiload, zoar_hisave
};

