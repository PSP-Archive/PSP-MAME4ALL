/***************************************************************************

Notes:
- Sprite zoom is probably not 100% accurate (check the table in vidhrdw).
  In pspikes, the zooming text during attract mode is horrible.
- Aero Fighters has graphics for different tiles (Sonic Wings, The Final War)
  but I haven't found a way to display them - different program, maybe.
- Turbo Force bg1 tile maps are screwed. I have to offset the char code by
  0x9c to get the title screen right...

***************************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"
#include "z80/z80.h"


extern unsigned char *aerofgt_rasterram;
extern unsigned char *aerofgt_bg1videoram,*aerofgt_bg2videoram;
extern int aerofgt_bg1videoram_size,aerofgt_bg2videoram_size;

int aerofgt_rasterram_r(int offset);
void aerofgt_rasterram_w(int offset,int data);
int aerofgt_spriteram_2_r(int offset);
void aerofgt_spriteram_2_w(int offset,int data);
int aerofgt_bg1videoram_r(int offset);
int aerofgt_bg2videoram_r(int offset);
void aerofgt_bg1videoram_w(int offset,int data);
void aerofgt_bg2videoram_w(int offset,int data);
void pspikes_gfxbank_w(int offset,int data);
void turbofrc_gfxbank_w(int offset,int data);
void aerofgt_gfxbank_w(int offset,int data);
void aerofgt_bg1scrolly_w(int offset,int data);
void aerofgt_bg2scrolly_w(int offset,int data);
void turbofrc_bg2scrollx_w(int offset,int data);
void pspikes_palette_bank_w(int offset,int data);
int pspikes_vh_start(void);
int turbofrc_vh_start(void);
int aerofgt_vh_start(void);
void aerofgt_vh_stop(void);
void pspikes_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh);
void turbofrc_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh);
void aerofgt_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh);




static unsigned char *aerofgt_workram;

static int aerofgt_workram_r(int offset)
{
	return READ_WORD(&aerofgt_workram[offset]);
}

static void aerofgt_workram_w(int offset,int data)
{
	COMBINE_WORD_MEM(&aerofgt_workram[offset],data);
}



static int pending_command;

static void sound_command_w(int offset,int data)
{
	pending_command = 1;
	soundlatch_w(offset,data & 0xff);
	cpu_cause_interrupt(1,Z80_NMI_INT);
}

static void turbofrc_sound_command_w(int offset,int data)
{
	pending_command = 1;
	soundlatch_w(offset,(data >> 8) & 0xff);
	cpu_cause_interrupt(1,Z80_NMI_INT);
}

static int pending_command_r(int offset)
{
	return pending_command;
}

static void pending_command_clear_w(int offset,int data)
{
	pending_command = 0;
}

static void aerofgt_sh_bankswitch_w(int offset,int data)
{
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[1].memory_region];
	int bankaddress;


	bankaddress = 0x10000 + (data & 0x03) * 0x8000;
	cpu_setbank(1,&RAM[bankaddress]);
}



static struct MemoryReadAddress pspikes_readmem[] =
{
	{ 0x000000, 0x03ffff, MRA_ROM },
	{ 0x100000, 0x10ffff, MRA_BANK6 },
	{ 0xff8000, 0xff8fff, aerofgt_bg1videoram_r },
	{ 0xffd000, 0xffdfff, aerofgt_rasterram_r },	/* different from aero */
	{ 0xffe000, 0xffefff, paletteram_word_r },
	{ 0xfff000, 0xfff001, input_port_0_r },
	{ 0xfff002, 0xfff003, input_port_1_r },
	{ 0xfff004, 0xfff005, input_port_2_r },
	{ 0xfff006, 0xfff007, pending_command_r },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress pspikes_writemem[] =
{
	{ 0x000000, 0x03ffff, MWA_ROM },
	{ 0x100000, 0x10ffff, MWA_BANK6 },	/* work RAM */
	{ 0x200000, 0x203fff, MWA_BANK4, &spriteram },
	{ 0xff8000, 0xff8fff, aerofgt_bg1videoram_w, &aerofgt_bg1videoram, &aerofgt_bg1videoram_size },
	{ 0xffc000, 0xffc3ff, aerofgt_spriteram_2_w, &spriteram_2, &spriteram_2_size },	/* different from aero */
	{ 0xffd000, 0xffdfff, aerofgt_rasterram_w, &aerofgt_rasterram },	/* bg1 scroll registers */
	{ 0xffe000, 0xffefff, paletteram_xRRRRRGGGGGBBBBB_word_w, &paletteram },
	{ 0xfff000, 0xfff001, pspikes_palette_bank_w },
	{ 0xfff002, 0xfff003, pspikes_gfxbank_w },	/* different from aero */
	{ 0xfff004, 0xfff005, aerofgt_bg1scrolly_w },
	{ 0xfff006, 0xfff007, sound_command_w },
	{ -1 }  /* end of table */
};

static struct MemoryReadAddress turbofrc_readmem[] =
{
	{ 0x000000, 0x0bffff, MRA_ROM },
	{ 0x0c0000, 0x0cffff, MRA_BANK6 },
	{ 0x0d0000, 0x0d1fff, aerofgt_bg1videoram_r },
	{ 0x0d2000, 0x0d3fff, aerofgt_bg2videoram_r },
	{ 0x0e0000, 0x0e7fff, MRA_BANK4 },
	{ 0x0f8000, 0x0fbfff, aerofgt_workram_r },	/* work RAM */
	{ 0xff8000, 0xffbfff, aerofgt_workram_r },	/* mirror */
	{ 0x0fc000, 0x0fc7ff, aerofgt_spriteram_2_r },	/* different from aero */
	{ 0xffc000, 0xffc7ff, aerofgt_spriteram_2_r },	/* mirror */
	{ 0x0fd000, 0x0fdfff, aerofgt_rasterram_r },	/* different from aero */
	{ 0xffd000, 0xffdfff, aerofgt_rasterram_r },	/* mirror */
	{ 0x0fe000, 0x0fe7ff, paletteram_word_r },
	{ 0xfff000, 0xfff001, input_port_0_r },
	{ 0xfff002, 0xfff003, input_port_1_r },
	{ 0xfff004, 0xfff005, input_port_2_r },
	{ 0xfff006, 0xfff007, pending_command_r },
	{ 0xfff008, 0xfff009, input_port_3_r },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress turbofrc_writemem[] =
{
	{ 0x000000, 0x0bffff, MWA_ROM },
	{ 0x0c0000, 0x0cffff, MWA_BANK6 },	/* work RAM */
	{ 0x0d0000, 0x0d1fff, aerofgt_bg1videoram_w, &aerofgt_bg1videoram, &aerofgt_bg1videoram_size },
	{ 0x0d2000, 0x0d3fff, aerofgt_bg2videoram_w, &aerofgt_bg2videoram, &aerofgt_bg2videoram_size },
	{ 0x0e0000, 0x0e7fff, MWA_BANK4, &spriteram },
	{ 0x0f8000, 0x0fbfff, aerofgt_workram_w, &aerofgt_workram },	/* work RAM */
	{ 0xff8000, 0xffbfff, aerofgt_workram_w },	/* mirror */
	{ 0x0fc000, 0x0fc7ff, aerofgt_spriteram_2_w, &spriteram_2, &spriteram_2_size },	/* different from aero */
	{ 0xffc000, 0xffc7ff, aerofgt_spriteram_2_w },	/* mirror */
	{ 0x0fd000, 0x0fdfff, aerofgt_rasterram_w, &aerofgt_rasterram },	/* bg1 scroll registers */
	{ 0xffd000, 0xffdfff, aerofgt_rasterram_w },	/* mirror */
	{ 0x0fe000, 0x0fe7ff, paletteram_xRRRRRGGGGGBBBBB_word_w, &paletteram },
	{ 0xfff002, 0xfff003, aerofgt_bg1scrolly_w },
	{ 0xfff004, 0xfff005, turbofrc_bg2scrollx_w },
	{ 0xfff006, 0xfff007, aerofgt_bg2scrolly_w },
	{ 0xfff008, 0xfff00b, turbofrc_gfxbank_w },	/* different from aero */
	{ 0xfff00c, 0xfff00d, MWA_NOP },	/* related to bg2 (written together with the scroll registers) */
	{ 0xfff00e, 0xfff00f, turbofrc_sound_command_w },
	{ -1 }  /* end of table */
};

static struct MemoryReadAddress aerofgt_readmem[] =
{
	{ 0x000000, 0x07ffff, MRA_ROM },
	{ 0x1a0000, 0x1a07ff, paletteram_word_r },
	{ 0x1b0000, 0x1b07ff, aerofgt_rasterram_r },
	{ 0x1b0800, 0x1b0801, MRA_NOP },	/* ??? */
	{ 0x1b0ff0, 0x1b0fff, MRA_BANK7 },	/* stack area during boot */
	{ 0x1b2000, 0x1b3fff, aerofgt_bg1videoram_r },
	{ 0x1b4000, 0x1b5fff, aerofgt_bg2videoram_r },
	{ 0x1c0000, 0x1c7fff, MRA_BANK4 },
	{ 0x1d0000, 0x1d1fff, aerofgt_spriteram_2_r },
	{ 0xfef000, 0xffefff, aerofgt_workram_r },	/* work RAM */
	{ 0xffffa0, 0xffffa1, input_port_0_r },
	{ 0xffffa2, 0xffffa3, input_port_1_r },
	{ 0xffffa4, 0xffffa5, input_port_2_r },
	{ 0xffffa6, 0xffffa7, input_port_3_r },
	{ 0xffffa8, 0xffffa9, input_port_4_r },
	{ 0xffffac, 0xffffad, pending_command_r },
	{ 0xffffae, 0xffffaf, input_port_5_r },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress aerofgt_writemem[] =
{
	{ 0x000000, 0x07ffff, MWA_ROM },
	{ 0x1a0000, 0x1a07ff, paletteram_xRRRRRGGGGGBBBBB_word_w, &paletteram },
	{ 0x1b0000, 0x1b07ff, aerofgt_rasterram_w, &aerofgt_rasterram },	/* used only for the scroll registers */
	{ 0x1b0800, 0x1b0801, MWA_NOP },	/* ??? */
	{ 0x1b0ff0, 0x1b0fff, MWA_BANK7 },	/* stack area during boot */
	{ 0x1b2000, 0x1b3fff, aerofgt_bg1videoram_w, &aerofgt_bg1videoram, &aerofgt_bg1videoram_size },
	{ 0x1b4000, 0x1b5fff, aerofgt_bg2videoram_w, &aerofgt_bg2videoram, &aerofgt_bg2videoram_size },
	{ 0x1c0000, 0x1c7fff, MWA_BANK4, &spriteram },
	{ 0x1d0000, 0x1d1fff, aerofgt_spriteram_2_w, &spriteram_2, &spriteram_2_size },
	{ 0xfef000, 0xffefff, aerofgt_workram_w, &aerofgt_workram },	/* work RAM */
	{ 0xffff80, 0xffff87, aerofgt_gfxbank_w },
	{ 0xffff88, 0xffff89, aerofgt_bg1scrolly_w },	/* + something else in the top byte */
	{ 0xffff90, 0xffff91, aerofgt_bg2scrolly_w },	/* + something else in the top byte */
	{ 0xffffac, 0xffffad, MWA_NOP },	/* ??? */
	{ 0xffffc0, 0xffffc1, sound_command_w },
	{ -1 }  /* end of table */
};

static struct MemoryReadAddress sound_readmem[] =
{
	{ 0x0000, 0x77ff, MRA_ROM },
	{ 0x7800, 0x7fff, MRA_RAM },
	{ 0x8000, 0xffff, MRA_BANK1 },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress sound_writemem[] =
{
	{ 0x0000, 0x77ff, MWA_ROM },
	{ 0x7800, 0x7fff, MWA_RAM },
	{ 0x8000, 0xffff, MWA_ROM },
	{ -1 }  /* end of table */
};

static struct IOReadPort turbofrc_sound_readport[] =
{
	{ 0x14, 0x14, soundlatch_r },
	{ 0x18, 0x18, YM2610_status_port_0_A_r },
	{ 0x1a, 0x1a, YM2610_status_port_0_B_r },
	{ -1 }	/* end of table */
};

static struct IOWritePort turbofrc_sound_writeport[] =
{
	{ 0x00, 0x00, aerofgt_sh_bankswitch_w },
	{ 0x14, 0x14, pending_command_clear_w },
	{ 0x18, 0x18, YM2610_control_port_0_A_w },
	{ 0x19, 0x19, YM2610_data_port_0_A_w },
	{ 0x1a, 0x1a, YM2610_control_port_0_B_w },
	{ 0x1b, 0x1b, YM2610_data_port_0_B_w },
	{ -1 }	/* end of table */
};

static struct IOReadPort aerofgt_sound_readport[] =
{
	{ 0x00, 0x00, YM2610_status_port_0_A_r },
	{ 0x02, 0x02, YM2610_status_port_0_B_r },
	{ 0x0c, 0x0c, soundlatch_r },
	{ -1 }	/* end of table */
};

static struct IOWritePort aerofgt_sound_writeport[] =
{
	{ 0x00, 0x00, YM2610_control_port_0_A_w },
	{ 0x01, 0x01, YM2610_data_port_0_A_w },
	{ 0x02, 0x02, YM2610_control_port_0_B_w },
	{ 0x03, 0x03, YM2610_data_port_0_B_w },
	{ 0x04, 0x04, aerofgt_sh_bankswitch_w },
	{ 0x08, 0x08, pending_command_clear_w },
	{ -1 }	/* end of table */
};



INPUT_PORTS_START( pspikes_input_ports )
	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START
	PORT_DIPNAME( 0x0003, 0x0003, "Coin A", IP_KEY_NONE )
	PORT_DIPSETTING(      0x0001, "3 Coins/1 Credit" )
	PORT_DIPSETTING(      0x0002, "2 Coins/1 Credit" )
	PORT_DIPSETTING(      0x0003, "1 Coin/1 Credit" )
	PORT_DIPSETTING(      0x0000, "1 Coin/2 Credits" )
	PORT_DIPNAME( 0x000c, 0x000c, "Coin B", IP_KEY_NONE )
	PORT_DIPSETTING(      0x0004, "3 Coins/1 Credit" )
	PORT_DIPSETTING(      0x0008, "2 Coins/1 Credit" )
	PORT_DIPSETTING(      0x000c, "1 Coin/1 Credit" )
	PORT_DIPSETTING(      0x0000, "1 Coin/2 Credits" )
	PORT_DIPNAME( 0x0010, 0x0010, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(      0x0010, "Off" )
	PORT_DIPSETTING(      0x0000, "On" )
	PORT_DIPNAME( 0x0020, 0x0020, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(      0x0020, "Off" )
	PORT_DIPSETTING(      0x0000, "On" )
	PORT_DIPNAME( 0x0040, 0x0000, "Demo Sound", IP_KEY_NONE )
	PORT_DIPSETTING(      0x0040, "Off" )
	PORT_DIPSETTING(      0x0000, "On" )
	PORT_DIPNAME( 0x0080, 0x0080, "Flip Screen", IP_KEY_NONE )
	PORT_DIPSETTING(      0x0080, "Off" )
	PORT_DIPSETTING(      0x0000, "On" )
	PORT_BITX(    0x0100, 0x0100, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
	PORT_DIPSETTING(      0x0100, "Off" )
	PORT_DIPSETTING(      0x0000, "On" )
	PORT_DIPNAME( 0x0600, 0x0600, "Initial Score", IP_KEY_NONE )
	PORT_DIPSETTING(      0x0600, "12-12" )
	PORT_DIPSETTING(      0x0400, "11-11" )
	PORT_DIPSETTING(      0x0200, "11-12" )
	PORT_DIPSETTING(      0x0000, "10-12" )
	PORT_DIPNAME( 0x0800, 0x0800, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(      0x0800, "Off" )
	PORT_DIPSETTING(      0x0000, "On" )
	PORT_DIPNAME( 0x1000, 0x1000, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(      0x1000, "Off" )
	PORT_DIPSETTING(      0x0000, "On" )
	PORT_DIPNAME( 0x2000, 0x2000, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(      0x2000, "Off" )
	PORT_DIPSETTING(      0x0000, "On" )
	PORT_DIPNAME( 0x4000, 0x4000, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(      0x4000, "Off" )
	PORT_DIPSETTING(      0x0000, "On" )
	PORT_DIPNAME( 0x8000, 0x8000, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(      0x8000, "Off" )
	PORT_DIPSETTING(      0x0000, "On" )
INPUT_PORTS_END

INPUT_PORTS_START( turbofrc_input_ports )
	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE )	/* "TEST" */
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN3 )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START
	PORT_DIPNAME( 0x0007, 0x0007, "Coinage", IP_KEY_NONE )
	PORT_DIPSETTING(      0x0004, "4 Coins/1 Credit" )
	PORT_DIPSETTING(      0x0005, "3 Coins/1 Credit" )
	PORT_DIPSETTING(      0x0006, "2 Coins/1 Credit" )
	PORT_DIPSETTING(      0x0007, "1 Coin/1 Credit" )
	PORT_DIPSETTING(      0x0003, "1 Coin/2 Credits" )
	PORT_DIPSETTING(      0x0002, "1 Coin/3 Credits" )
	PORT_DIPSETTING(      0x0001, "1 Coin/5 Credits" )
	PORT_DIPSETTING(      0x0000, "1 Coin/6 Credits" )
	PORT_DIPNAME( 0x0008, 0x0008, "2 Coins to Start, 1 to Continue", IP_KEY_NONE )
	PORT_DIPSETTING(      0x0008, "Off" )
	PORT_DIPSETTING(      0x0000, "On" )
	PORT_DIPNAME( 0x0010, 0x0010, "Coin Slot", IP_KEY_NONE )
	PORT_DIPSETTING(      0x0010, "Same" )
	PORT_DIPSETTING(      0x0000, "Individual" )
	PORT_DIPNAME( 0x0020, 0x0000, "Max Players", IP_KEY_NONE )
	PORT_DIPSETTING(      0x0020, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPNAME( 0x0040, 0x0000, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(      0x0040, "Off" )
	PORT_DIPSETTING(      0x0000, "On" )
	PORT_BITX(    0x0080, 0x0080, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
	PORT_DIPSETTING(      0x0080, "Off" )
	PORT_DIPSETTING(      0x0000, "On" )
	PORT_DIPNAME( 0x0100, 0x0100, "Flip Screen", IP_KEY_NONE )
	PORT_DIPSETTING(      0x0100, "Off" )
	PORT_DIPSETTING(      0x0000, "On" )
	PORT_DIPNAME( 0x0200, 0x0200, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(      0x0200, "Off" )
	PORT_DIPSETTING(      0x0000, "On" )
	PORT_DIPNAME( 0x0400, 0x0400, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(      0x0400, "Off" )
	PORT_DIPSETTING(      0x0000, "On" )
	PORT_DIPNAME( 0x0800, 0x0800, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(      0x0800, "Off" )
	PORT_DIPSETTING(      0x0000, "On" )
	PORT_DIPNAME( 0x1000, 0x1000, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x1000, "3" )
	PORT_DIPNAME( 0x2000, 0x2000, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(      0x2000, "Off" )
	PORT_DIPSETTING(      0x0000, "On" )
	PORT_DIPNAME( 0x4000, 0x4000, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(      0x4000, "Off" )
	PORT_DIPSETTING(      0x0000, "On" )
	PORT_DIPNAME( 0x8000, 0x8000, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(      0x8000, "Off" )
	PORT_DIPSETTING(      0x0000, "On" )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER3 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER3 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER3 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER3 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START3 )
INPUT_PORTS_END

INPUT_PORTS_START( aerofgt_input_ports )
	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START
	PORT_DIPNAME( 0x0001, 0x0001, "Coin Slot", IP_KEY_NONE )
	PORT_DIPSETTING(      0x0001, "Same" )
	PORT_DIPSETTING(      0x0000, "Individual" )
	PORT_DIPNAME( 0x000e, 0x000e, "Coin A", IP_KEY_NONE )
	PORT_DIPSETTING(      0x000a, "3 Coins/1 Credit" )
	PORT_DIPSETTING(      0x000c, "2 Coins/1 Credit" )
	PORT_DIPSETTING(      0x000e, "1 Coin/1 Credit" )
	PORT_DIPSETTING(      0x0008, "1 Coin/2 Credits" )
	PORT_DIPSETTING(      0x0006, "1 Coin/3 Credits" )
	PORT_DIPSETTING(      0x0004, "1 Coin/4 Credits" )
	PORT_DIPSETTING(      0x0002, "1 Coin/5 Credits" )
	PORT_DIPSETTING(      0x0000, "1 Coin/6 Credits" )
	PORT_DIPNAME( 0x0070, 0x0070, "Coin B", IP_KEY_NONE )
	PORT_DIPSETTING(      0x0050, "3 Coins/1 Credit" )
	PORT_DIPSETTING(      0x0060, "2 Coins/1 Credit" )
	PORT_DIPSETTING(      0x0070, "1 Coin/1 Credit" )
	PORT_DIPSETTING(      0x0040, "1 Coin/2 Credits" )
	PORT_DIPSETTING(      0x0030, "1 Coin/3 Credits" )
	PORT_DIPSETTING(      0x0020, "1 Coin/4 Credits" )
	PORT_DIPSETTING(      0x0010, "1 Coin/5 Credits" )
	PORT_DIPSETTING(      0x0000, "1 Coin/6 Credits" )
	PORT_DIPNAME( 0x0080, 0x0080, "2 Coins to Start, 1 to Continue", IP_KEY_NONE )
	PORT_DIPSETTING(      0x0080, "Off" )
	PORT_DIPSETTING(      0x0000, "On" )

	PORT_START
	PORT_DIPNAME( 0x0001, 0x0001, "Flip Screen", IP_KEY_NONE )
	PORT_DIPSETTING(      0x0001, "Off" )
	PORT_DIPSETTING(      0x0000, "On" )
	PORT_DIPNAME( 0x0002, 0x0000, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(      0x0002, "Off" )
	PORT_DIPSETTING(      0x0000, "On" )
	PORT_DIPNAME( 0x000c, 0x000c, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(      0x0008, "Easy" )
	PORT_DIPSETTING(      0x000c, "Normal" )
	PORT_DIPSETTING(      0x0004, "Hard" )
	PORT_DIPSETTING(      0x0000, "Hardest" )
	PORT_DIPNAME( 0x0030, 0x0030, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(      0x0020, "1" )
	PORT_DIPSETTING(      0x0010, "2" )
	PORT_DIPSETTING(      0x0030, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0040, 0x0040, "Bonus Life", IP_KEY_NONE )
	PORT_DIPSETTING(      0x0040, "200000" )
	PORT_DIPSETTING(      0x0000, "300000" )
	PORT_BITX(    0x0080, 0x0080, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
	PORT_DIPSETTING(      0x0080, "Off" )
	PORT_DIPSETTING(      0x0000, "On" )

	PORT_START
	PORT_DIPNAME( 0x000f, 0x0000, "Country", IP_KEY_NONE )
	PORT_DIPSETTING(      0x0000, "Any" )
	PORT_DIPSETTING(      0x000f, "USA" )
	PORT_DIPSETTING(      0x000e, "Korea" )
	PORT_DIPSETTING(      0x000d, "Hong Kong" )
	PORT_DIPSETTING(      0x000b, "Taiwan" )
INPUT_PORTS_END



static struct GfxLayout pspikes_charlayout =
{
	8,8,	/* 8*8 characters */
	16384,	/* 16384 characters */
	4,	/* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8	/* every char takes 32 consecutive bytes */
};

static struct GfxLayout pspikes_spritelayout =
{
	16,16,	/* 16*16 sprites */
	8192,	/* 8192 sprites */
	4,	/* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, 3*4, 2*4, 8192*64*8+1*4, 8192*64*8+0*4, 8192*64*8+3*4, 8192*64*8+2*4,
			5*4, 4*4, 7*4, 6*4, 8192*64*8+5*4, 8192*64*8+4*4, 8192*64*8+7*4, 8192*64*8+6*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	64*8	/* every char takes 32 consecutive bytes */
};

static struct GfxDecodeInfo pspikes_gfxdecodeinfo[] =
{
	{ 1, 0x000000, &pspikes_charlayout,      0, 64 },	/* colors    0-1023 in 8 banks */
	{ 1, 0x080000, &pspikes_spritelayout, 1024, 64 },	/* colors 1024-2047 in 4 banks */
	{ -1 } /* end of array */
};


static struct GfxLayout turbofrc_charlayout =
{
	8,8,	/* 8*8 characters */
	40960,	/* 40960 characters */
	4,	/* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8	/* every char takes 32 consecutive bytes */
};

static struct GfxLayout turbofrc_spritelayout =
{
	16,16,	/* 16*16 sprites */
	12288,	/* 12288 sprites */
	4,	/* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, 3*4, 2*4, 12288*64*8+1*4, 12288*64*8+0*4, 12288*64*8+3*4, 12288*64*8+2*4,
			5*4, 4*4, 7*4, 6*4, 12288*64*8+5*4, 12288*64*8+4*4, 12288*64*8+7*4, 12288*64*8+6*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	64*8	/* every char takes 32 consecutive bytes */
};

static struct GfxDecodeInfo turbofrc_gfxdecodeinfo[] =
{
	{ 1, 0x000000, &turbofrc_charlayout,      0, 32 },	/* I could split this one, first half is bg1 second half bg2 */
	{ 1, 0x140000, &turbofrc_spritelayout,  512, 16 },
	{ 1, 0x2c0000, &pspikes_spritelayout,   768, 16 },
	{ -1 } /* end of array */
};


static struct GfxLayout aerofgt_charlayout =
{
	8,8,	/* 8*8 characters */
	32768,	/* 32768 characters */
	4,	/* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8	/* every char takes 32 consecutive bytes */
};

static struct GfxLayout aerofgt_spritelayout1 =
{
	16,16,	/* 16*16 sprites */
	8192,	/* 8192 sprites */
	4,	/* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4,
			10*4, 11*4, 8*4, 9*4, 14*4, 15*4, 12*4, 13*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
			8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8	/* every sprite takes 128 consecutive bytes */
};

static struct GfxLayout aerofgt_spritelayout2 =
{
	16,16,	/* 16*16 sprites */
	4096,	/* 4096 sprites */
	4,	/* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4,
			10*4, 11*4, 8*4, 9*4, 14*4, 15*4, 12*4, 13*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
			8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8	/* every sprite takes 128 consecutive bytes */
};

static struct GfxDecodeInfo aerofgt_gfxdecodeinfo[] =
{
	{ 1, 0x000000, &aerofgt_charlayout,      0, 32 },
	{ 1, 0x100000, &aerofgt_spritelayout1, 512, 16 },
	{ 1, 0x200000, &aerofgt_spritelayout2, 768, 16 },
	{ -1 } /* end of array */
};


static struct GfxLayout unkvsys_charlayout =
{
	8,8,	/* 8*8 characters */
	8192,	/* 32768 characters */
	4,	/* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8	/* every char takes 32 consecutive bytes */
};

static struct GfxLayout unkvsys_spritelayout1 =
{
	16,16,	/* 16*16 sprites */
	4096,	/* 4096 sprites */
	4,	/* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, 3*4, 2*4, 4096*64*8+1*4, 4096*64*8+0*4, 4096*64*8+3*4, 4096*64*8+2*4,
			5*4, 4*4, 7*4, 6*4, 4096*64*8+5*4, 4096*64*8+4*4, 4096*64*8+7*4, 4096*64*8+6*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	64*8	/* every char takes 32 consecutive bytes */
};

static struct GfxLayout unkvsys_spritelayout2 =
{
	16,16,	/* 16*16 sprites */
	4096,	/* 4096 sprites */
	4,	/* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 10*4, 11*4, 8*4, 9*4, 14*4, 15*4, 12*4, 13*4,
			2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
			8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8	/* every sprite takes 128 consecutive bytes */
};

static struct GfxDecodeInfo unkvsys_gfxdecodeinfo[] =
{
	{ 1, 0x000000, &unkvsys_charlayout,      0, 32 },
	{ 1, 0x040000, &unkvsys_charlayout,      0, 32 },
	{ 1, 0x080000, &unkvsys_charlayout,      0, 32 },
	{ 1, 0x0c0000, &unkvsys_spritelayout1,   0, 32 },
	{ 1, 0x140000, &unkvsys_spritelayout2,   0, 32 },
	{ -1 } /* end of array */
};



static void irqhandler(void)
{
	cpu_cause_interrupt(1,0xff);
}

struct YM2610interface ym2610_interface =
{
	1,
	8000000,	/* 8 MHz??? */
	{ YM2203_VOL(50,50) },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ irqhandler },
	{ 4 },
	{ 3 }
};



static struct MachineDriver pspikes_machine_driver =
{
	/* basic machine hardware */
	{
		{
			CPU_M68000,
			10000000,	/* 10 MHz (?) */
			0,
			pspikes_readmem,pspikes_writemem,0,0,
			m68_level1_irq,1	/* all inrq vectors are the same */
		},
		{
			CPU_Z80 | CPU_AUDIO_CPU,
			4000000,	/* 4 Mhz ??? */
			2,	/* memory region #2 */
			sound_readmem,sound_writemem,turbofrc_sound_readport,turbofrc_sound_writeport,
			ignore_interrupt,0	/* NMIs are triggered by the main CPU */
								/* IRQs are triggered by the YM2610 */
		}
	},
	60, DEFAULT_60HZ_VBLANK_DURATION,       /* frames per second, vblank duration */
	1,	/* 1 CPU slice per frame - interleaving is forced when a sound command is written */
	0,

	/* video hardware */
	64*8, 64*8, { 2*8-4, 44*8-4-1, 1*8, 29*8-1 },
	pspikes_gfxdecodeinfo,
	2048, 2048,
	0,

	VIDEO_TYPE_RASTER | VIDEO_MODIFIES_PALETTE,
	0,
	pspikes_vh_start,
	aerofgt_vh_stop,
	pspikes_vh_screenrefresh,

	/* sound hardware */
	/*SOUND_SUPPORTS_STEREO*/0,0,0,0,
	{
		{
			SOUND_YM2610,
			&ym2610_interface,
		}
	}
};

static struct MachineDriver turbofrc_machine_driver =
{
	/* basic machine hardware */
	{
		{
			CPU_M68000,
			10000000,	/* 10 MHz (?) */
			0,
			turbofrc_readmem,turbofrc_writemem,0,0,
			m68_level1_irq,1	/* all inrq vectors are the same */
		},
		{
			CPU_Z80 | CPU_AUDIO_CPU,
			4000000,	/* 4 Mhz ??? */
			2,	/* memory region #2 */
			sound_readmem,sound_writemem,turbofrc_sound_readport,turbofrc_sound_writeport,
			ignore_interrupt,0	/* NMIs are triggered by the main CPU */
								/* IRQs are triggered by the YM2610 */
		}
	},
	60, DEFAULT_60HZ_VBLANK_DURATION,       /* frames per second, vblank duration */
	1,	/* 1 CPU slice per frame - interleaving is forced when a sound command is written */
	0,

	/* video hardware */
	64*8, 32*8, { 1*8, 44*8-1, 0*8, 30*8-1 },
	turbofrc_gfxdecodeinfo,
	1024, 1024,
	0,

	VIDEO_TYPE_RASTER | VIDEO_MODIFIES_PALETTE,
	0,
	turbofrc_vh_start,
	aerofgt_vh_stop,
	turbofrc_vh_screenrefresh,

	/* sound hardware */
	/*SOUND_SUPPORTS_STEREO*/0,0,0,0,
	{
		{
			SOUND_YM2610,
			&ym2610_interface,
		}
	}
};

static struct MachineDriver aerofgt_machine_driver =
{
	/* basic machine hardware */
	{
		{
			CPU_M68000,
			10000000,	/* 10 MHz ??? (slows down a lot at 8MHz) */
			0,
			aerofgt_readmem,aerofgt_writemem,0,0,
			m68_level1_irq,1	/* all irq vectors are the same */
		},
		{
			CPU_Z80 | CPU_AUDIO_CPU,
			4000000,	/* 4 Mhz ??? */
			2,	/* memory region #2 */
			sound_readmem,sound_writemem,aerofgt_sound_readport,aerofgt_sound_writeport,
			ignore_interrupt,0	/* NMIs are triggered by the main CPU */
								/* IRQs are triggered by the YM2610 */
		}
	},
	60, 500,	/* frames per second, vblank duration */
				/* wrong but improves sprite-background synchronization */
	1,	/* 1 CPU slice per frame - interleaving is forced when a sound command is written */
	0,

	/* video hardware */
	64*8, 32*8, { 0*8, 40*8-1, 0*8, 28*8-1 },
	aerofgt_gfxdecodeinfo,
	1024, 1024,
	0,

	VIDEO_TYPE_RASTER | VIDEO_MODIFIES_PALETTE,
	0,
	aerofgt_vh_start,
	aerofgt_vh_stop,
	aerofgt_vh_screenrefresh,

	/* sound hardware */
	/*SOUND_SUPPORTS_STEREO*/0,0,0,0,
	{
		{
			SOUND_YM2610,
			&ym2610_interface,
		}
	}
};

static struct MachineDriver unkvsys_machine_driver =
{
	/* basic machine hardware */
	{
		{
			CPU_M68000,
			10000000,	/* 10 MHz (?) */
			0,
			aerofgt_readmem,aerofgt_writemem,0,0,
			m68_level1_irq,1	/* all irq vectors are the same */
		},
		{
			CPU_Z80 | CPU_AUDIO_CPU,
			4000000,	/* 4 Mhz ??? */
			2,	/* memory region #2 */
			sound_readmem,sound_writemem,aerofgt_sound_readport,aerofgt_sound_writeport,
			ignore_interrupt,0	/* NMIs are triggered by the main CPU */
								/* IRQs are triggered by the YM2610 */
		}
	},
	60, DEFAULT_60HZ_VBLANK_DURATION,       /* frames per second, vblank duration */
	1,	/* 1 CPU slice per frame - interleaving is forced when a sound command is written */
	0,

	/* video hardware */
	64*8, 32*8, { 0*8, 40*8-1, 0*8, 28*8-1 },
	unkvsys_gfxdecodeinfo,
	1024, 1024,
	0,

	VIDEO_TYPE_RASTER | VIDEO_MODIFIES_PALETTE,
	0,
	aerofgt_vh_start,
	aerofgt_vh_stop,
	aerofgt_vh_screenrefresh,

	/* sound hardware */
	/*SOUND_SUPPORTS_STEREO*/0,0,0,0,
	{
		{
			SOUND_YM2610,
			&ym2610_interface,
		}
	}
};



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( pspikes_rom )
	ROM_REGION(0xc0000)	/* 68000 code */
	ROM_LOAD_WIDE_SWAP( "20",           0x00000, 0x40000, 0x75cdcee2 )

	ROM_REGION_DISPOSE(0x180000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "g7h",          0x000000, 0x80000, 0x74c23c3d )
	ROM_LOAD( "g7j",          0x080000, 0x80000, 0x0b9e4739 )
	ROM_LOAD( "g7l",          0x100000, 0x80000, 0x943139ff )

	ROM_REGION(0x30000)	/* 64k for the audio CPU + banks */
	ROM_LOAD( "19",           0x00000, 0x20000, 0x7e8ed6e5 )
	ROM_RELOAD(               0x10000, 0x20000 )

	ROM_REGION(0x100000) /* sound samples */
	ROM_LOAD( "o5b",          0x000000, 0x100000, 0x07d6cbac )

	ROM_REGION(0x40000) /* sound samples */
	ROM_LOAD( "a47",          0x00000, 0x40000, 0xc6779dfa )
ROM_END

ROM_START( turbofrc_rom )
	ROM_REGION(0xc0000)	/* 68000 code */
	ROM_LOAD_WIDE_SWAP( "tfrc2.bin",    0x00000, 0x40000, 0x721300ee )
	ROM_LOAD_WIDE_SWAP( "tfrc1.bin",    0x40000, 0x40000, 0x6cd5312b )
	ROM_LOAD_WIDE_SWAP( "tfrc3.bin",    0x80000, 0x40000, 0x63f50557 )

	ROM_REGION_DISPOSE(0x3c0000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "tfrcu94.bin",  0x000000, 0x080000, 0xbaa53978 )
	ROM_LOAD( "tfrcu95.bin",  0x080000, 0x020000, 0x71a6c573 )
	/* 0a0000-11ffff - u105 - missing */
	ROM_LOAD( "tfrcu106.bin", 0x120000, 0x020000, 0xc6479eb5 )
	ROM_LOAD( "tfrcu116.bin", 0x140000, 0x080000, 0xdf210f3b )
	ROM_LOAD( "tfrcu118.bin", 0x1c0000, 0x040000, 0xf61d1d79 )
	ROM_LOAD( "tfrcu117.bin", 0x200000, 0x080000, 0xf70812fd )
	ROM_LOAD( "tfrcu119.bin", 0x280000, 0x040000, 0x474ea716 )
	ROM_LOAD( "tfrcu134.bin", 0x2c0000, 0x080000, 0x487330a2 )
	ROM_LOAD( "tfrcu135.bin", 0x340000, 0x080000, 0x3a7e5b6d )

	ROM_REGION(0x30000)	/* 64k for the audio CPU + banks */
	ROM_LOAD( "tfrcu166.bin", 0x00000, 0x20000, 0x2ca14a65 )
	ROM_RELOAD(               0x10000, 0x20000 )

	ROM_REGION(0x100000) /* sound samples */
	ROM_LOAD( "tfrcu179.bin", 0x000000, 0x100000, 0x60ca0333 )

	ROM_REGION(0x20000) /* sound samples */
	ROM_LOAD( "tfrcu180.bin",   0x00000, 0x20000, 0x39c7c7d5 )
ROM_END

ROM_START( aerofgt_rom )
	ROM_REGION(0x80000)	/* 68000 code */
	ROM_LOAD_WIDE_SWAP( "1.u4",         0x00000, 0x80000, 0x6fdff0a2 )

	ROM_REGION_DISPOSE(0x280000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "538a54.124",   0x000000, 0x080000, 0x4d2c4df2 )
	ROM_LOAD( "1538a54.124",  0x080000, 0x080000, 0x286d109e )
	ROM_LOAD( "538a53.u9",    0x100000, 0x100000, 0x630d8e0b )
	ROM_LOAD( "534g8f.u18",   0x200000, 0x080000, 0x76ce0926 )

	ROM_REGION(0x30000)	/* 64k for the audio CPU + banks */
	ROM_LOAD( "2.153",        0x00000, 0x20000, 0xa1ef64ec )
	ROM_RELOAD(               0x10000, 0x20000 )

	ROM_REGION(0x100000) /* sound samples */
	ROM_LOAD( "it1906.137",   0x000000, 0x80000, 0x748b7e9e )
	ROM_LOAD( "1ti1906.137",  0x080000, 0x80000, 0xd39ced76 )

	ROM_REGION(0x40000) /* sound samples */
	ROM_LOAD( "it1901.104",   0x00000, 0x40000, 0x6d42723d )
ROM_END

ROM_START( unkvsys_rom )
	ROM_REGION(0x60000)	/* 68000 code */
	ROM_LOAD_EVEN( "v4",           0x00000, 0x10000, 0x1d4240c2 )
	ROM_LOAD_ODD ( "v7",           0x00000, 0x10000, 0x0fb70066 )
	ROM_LOAD_EVEN( "v5",           0x20000, 0x10000, 0xa9fe15a1 )	/* not sure */
	ROM_LOAD_ODD ( "v8",           0x20000, 0x10000, 0x4fb6a43e )	/* not sure */
	ROM_LOAD_EVEN( "v3",           0x40000, 0x10000, 0xe2e0abad )	/* not sure */
	ROM_LOAD_ODD ( "v6",           0x40000, 0x10000, 0x069817a7 )	/* not sure */

	ROM_REGION_DISPOSE(0x1c0000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "a24",               0x000000, 0x80000, 0xb1e9de43 )
	ROM_LOAD( "o1s",               0x080000, 0x40000, 0x00000000 )
	ROM_LOAD( "oj1",               0x0c0000, 0x40000, 0x39c36b35 )
	ROM_LOAD( "oj2",               0x100000, 0x40000, 0x77ccaea2 )
	ROM_LOAD( "a23",               0x140000, 0x80000, 0xd851cf04 )

	ROM_REGION(0x30000)	/* 64k for the audio CPU + banks */
	ROM_LOAD( "v2",                0x00000, 0x08000, 0x920d8920 )
	ROM_LOAD( "v1",                0x10000, 0x10000, 0xbf35c1a4 )	/* not sure */

	ROM_REGION(0x20000) /* sound samples */
	ROM_LOAD( "osb",               0x00000, 0x20000, 0xd49ab2f5 )

	ROM_REGION(0x00001) /* sound samples */
	/* missing */
ROM_END



struct GameDriver pspikes_driver =
{
	__FILE__,
	0,
	"pspikes",
	"Power Spikes (Korea)",
	"1991",
	"Video System Co.",
	"Nicola Salmoria",
	0,
	&pspikes_machine_driver,
	0,

	pspikes_rom,
	0, 0,
	0,
	0,	/* sound_prom */

	pspikes_input_ports,

	0, 0, 0,
	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver turbofrc_driver =
{
	__FILE__,
	0,
	"turbofrc",
	"Turbo Force",
	"1991",
	"Video System Co.",
	"Nicola Salmoria",
	0,
	&turbofrc_machine_driver,
	0,

	turbofrc_rom,
	0, 0,
	0,
	0,	/* sound_prom */

	turbofrc_input_ports,

	0, 0, 0,
	ORIENTATION_ROTATE_270,
	0, 0
};

struct GameDriver aerofgt_driver =
{
	__FILE__,
	0,
	"aerofgt",
	"Aero Fighters",
	"1992",
	"Video System Co.",
	"Nicola Salmoria",
	0,
	&aerofgt_machine_driver,
	0,

	aerofgt_rom,
	0, 0,
	0,
	0,	/* sound_prom */

	aerofgt_input_ports,

	0, 0, 0,
	ORIENTATION_ROTATE_270,
	0, 0
};

struct GameDriver unkvsys_driver =
{
	__FILE__,
	0,
	"unkvsys",
	"unknown",
	"????",
	"Video System Co.",
	"Nicola Salmoria",
	0,
	&unkvsys_machine_driver,
	0,

	unkvsys_rom,
	0, 0,
	0,
	0,	/* sound_prom */

	aerofgt_input_ports,

	0, 0, 0,
	ORIENTATION_ROTATE_90,
	0, 0
};
