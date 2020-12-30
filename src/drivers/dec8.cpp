/***************************************************************************

Various Data East 8 bit games:

	Cobra Command               (c) 1988 Data East Corporation (6809)
	The Real Ghostbusters (2p)  (c) 1987 Data East USA (6809 + I8751)
	The Real Ghostbusters (3p)  (c) 1987 Data East USA (6809 + I8751)
	Mazehunter                  (c) 1987 Data East Corporation (6809 + I8751)
	Super Real Darwin           (c) 1987 Data East Corporation (6809 + I8751)
	Psycho-Nics Oscar           (c) 1988 Data East USA (2*6809 + I8751)
	Psycho-Nics Oscar (Japan)   (c) 1987 Data East Corporation (2*6809 + I8751)
	Gondomania					(c) 1987 Data East USA (6809 + I8751)
	Mekyo Sensi					(c) 1987 Data East Corporation (6809 + I8751)
	Last Mission (rev 6)        (c) 1986 Data East USA (2*6809 + I8751)
	Last Mission (rev 5)        (c) 1986 Data East USA (2*6809 + I8751)
	Shackled                    (c) 1986 Data East USA (2*6809 + I8751)
    Breywood                    (c) 1986 Data East Corporation (2*6809 + I8751)
    Captain Silver (Japan)      (c) 1987 Data East Corporation (2*6809 + I8751)

	All games use a 6502 for sound (some are encrypted), all games except Cobracom
	use an Intel 8751 for protection & coinage.  For these games the coinage dip
	switch is not currently supported, they are fixed at 1 coin 1 credit.

	Emulation by Bryan McPhail, mish@tendril.force9.net

Recent fixes:
	Ghostbusters kludge removed
	Priority added to Oscar
	Oscar speed fixed.
	SRD fixed
	New games added

To do:
	Slight graphics glitches in Captain Silver, Breywood, Shackled.
	Weird cpu race condition in Last Mission.
	Support coinage options for all i8751 emulations.
	Captain Silver/Cobra Command probably have some sprite/playfield priorities
	Wrong sprite colours in Super Real Darwin
	Cobra Command probably has missing graphics later on in the game.
	Dips needed to be worked on several games

Emulation Notes:

* Dip switches confirmed for Oscar, Ghostbusters & Gondomania, the others seem reasonable.
* Maze Hunter is using Ghostbusters colour proms for now...
* Breywood sprites are not currently dumped, a Breywood Rev 2 rom set is known
to exist..

***************************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"
#include "m6809/m6809.h"
#include "m6502/m6502.h"

int dec8_video_r(int offset);
void dec8_video_w(int offset, int data);
void dec8_pf1_w(int offset, int data);
void dec8_pf2_w(int offset, int data);
void dec8_scroll1_w(int offset, int data);
void dec8_scroll2_w(int offset, int data);
void dec8_vh_screenrefresh(struct osd_bitmap *bitmap, int full_refresh);
void ghostb_vh_screenrefresh(struct osd_bitmap *bitmap, int full_refresh);
void srdarwin_vh_screenrefresh(struct osd_bitmap *bitmap, int full_refresh);
void gondo_vh_screenrefresh(struct osd_bitmap *bitmap, int full_refresh);
void lastmiss_vh_screenrefresh(struct osd_bitmap *bitmap, int full_refresh);
void oscar_vh_screenrefresh(struct osd_bitmap *bitmap, int full_refresh);
int dec8_vh_start(void);
void dec8_vh_stop(void);


void srdarwin_control_w(int offset, int data);

void gondo_scroll_w(int offset, int data);

void lastmiss_control_w(int offset, int data);
void lastmiss_scrollx_w(int offset, int data);
void lastmiss_scrolly_w(int offset, int data);


void dec8_bac06_0_w(int offset, int data);
void dec8_bac06_1_w(int offset, int data);

void ghostb_vh_convert_color_prom(unsigned char *palette, unsigned short *colortable,const unsigned char *color_prom);

/******************************************************************************/

static unsigned char *dec8_shared_ram,*dec8_shared2_ram;
extern unsigned char *dec8_row;

static int nmi_enable,int_enable;
static int i8751_return, i8751_value;

/******************************************************************************/

static int i8751_h_r(int offset)
{
	return i8751_return>>8; /* MSB */
}

static int i8751_l_r(int offset)
{
	return i8751_return&0xff; /* LSB */
}

static void i8751_reset_w(int offset, int data)
{
	i8751_return=0;
}

/******************************************************************************/

static int gondo_player_1_r(int offset)
{
	switch (offset) {
		case 0: /* Rotary low byte */
			return ~((1 << (readinputport(5) * 12 / 256))&0xff);
		case 1: /* Joystick = bottom 4 bits, rotary = top 4 */
			return ((~((1 << (readinputport(5) * 12 / 256))>>4))&0xf0) | (readinputport(0)&0xf);
	}
	return 0xff;
}

static int gondo_player_2_r(int offset)
{
	switch (offset) {
		case 0: /* Rotary low byte */
			return ~((1 << (readinputport(6) * 12 / 256))&0xff);
		case 1: /* Joystick = bottom 4 bits, rotary = top 4 */
			return ((~((1 << (readinputport(6) * 12 / 256))>>4))&0xf0) | (readinputport(1)&0xf);
	}
	return 0xff;
}

/******************************************************************************/

static void ghostb_i8751_w(int offset, int data)
{
	i8751_return=0;

	switch (offset) {
	case 0: /* High byte */
		i8751_value=(i8751_value&0xff) | (data<<8);
		break;
	case 1: /* Low byte */
		i8751_value=(i8751_value&0xff00) | data;
		break;
	}

	if (i8751_value==0x00aa) i8751_return=0x655;
	if (i8751_value==0x021a) i8751_return=0x6e5; /* Ghostbusters ID */
	if (i8751_value==0x021b) i8751_return=0x6e4; /* Mazehunter ID */
}

static void srdarwin_i8751_w(int offset, int data)
{
	static int coins,latch;
	i8751_return=0;

	switch (offset) {
	case 0: /* High byte */
		i8751_value=(i8751_value&0xff) | (data<<8);
		break;
	case 1: /* Low byte */
		i8751_value=(i8751_value&0xff00) | data;
		break;
	}

	if (i8751_value==0x0000) {i8751_return=0;coins=0;}
	if (i8751_value==0x3063) i8751_return=0x9c; /* Protection */
	if ((i8751_value&0xff00)==0x4000) i8751_return=i8751_value; /* Coinage settings */
 	if (i8751_value==0x5000) i8751_return=coins; /* Coin request */
 	if (i8751_value==0x6000) {i8751_value=-1; coins--; } /* Coin clear */

	/* Nb:  Command 0x4000 for setting coinage options is not supported */

 	if ((readinputport(1)&1)==1) latch=1;
 	if ((readinputport(1)&1)!=1 && latch) {coins++; latch=0;}

	/* This next value is the index to a series of tables,
	each table controls the end of level bad guy, wrong values crash the
	cpu right away via a bogus jump.

	Level number requested is in low byte, I'm assuming the
	table values are stored in sequential order (ie, level 1 is the
	first table, level 2 is the second, etc).

	See location 0xf580 in rom dy_01.rom

	A real board would be nice to check against :)

	*/
	if ((i8751_value&0xff00)==0x8000)
		i8751_return=0xf580 + ((i8751_value&0xff)*2);
}

static void gondo_i8751_w(int offset, int data)
{
	static int coin1,coin2,latch,snd;
	i8751_return=0;

	switch (offset) {
	case 0: /* High byte */
		i8751_value=(i8751_value&0xff) | (data<<8);
		if (int_enable) cpu_cause_interrupt (0, M6809_INT_IRQ); /* IRQ on *high* byte only */
		break;
	case 1: /* Low byte */
		i8751_value=(i8751_value&0xff00) | data;
		break;
	}

	/* Coins are controlled by the i8751 */
 	if ((readinputport(4)&3)==3) latch=1;
 	if ((readinputport(4)&1)!=1 && latch) {coin1++; snd=1; latch=0;}
 	if ((readinputport(4)&2)!=2 && latch) {coin2++; snd=1; latch=0;}

	/* Work out return values */
	if (i8751_value==0x0000) {i8751_return=0; coin1=coin2=snd=0;}
	if (i8751_value==0x038a)  i8751_return=0x375; /* Mekyo Sensi ID */
	if (i8751_value==0x038b)  i8751_return=0x374; /* Gondomania ID */
	if ((i8751_value>>8)==0x04)  i8751_return=0x40f; /* Coinage settings (Not supported) */
	if ((i8751_value>>8)==0x05) {i8751_return=0x500 | coin1;  } /* Coin 1 */
	if ((i8751_value>>8)==0x06 && coin1 && !offset) {i8751_return=0x600; coin1--; } /* Coin 1 clear */
	if ((i8751_value>>8)==0x07) {i8751_return=0x700 | coin2;  } /* Coin 2 */
	if ((i8751_value>>8)==0x08 && coin2 && !offset) {i8751_return=0x800; coin2--; } /* Coin 2 clear */
	/* Commands 0x9xx do nothing */
	if ((i8751_value>>8)==0x0a) {i8751_return=0xa00 | snd; if (snd) snd=0; }
}

static void shackled_i8751_w(int offset, int data)
{
	static int coin1,coin2,latch=0;
	i8751_return=0;

	switch (offset) {
	case 0: /* High byte */
		i8751_value=(i8751_value&0xff) | (data<<8);
		cpu_cause_interrupt (1, M6809_INT_FIRQ); /* Signal main cpu */
		break;
	case 1: /* Low byte */
		i8751_value=(i8751_value&0xff00) | data;
		break;
	}

	/* Coins are controlled by the i8751 */
 	if (/*(readinputport(2)&3)==3*/!latch) {latch=1;coin1=coin2=0;}
 	if ((readinputport(2)&1)!=1 && latch) {coin1=1; latch=0;}
 	if ((readinputport(2)&2)!=2 && latch) {coin2=1; latch=0;}

	if (i8751_value==0x0050) i8751_return=0; /* Breywood ID */
	if (i8751_value==0x0051) i8751_return=0; /* Shackled ID */
	if (i8751_value==0x0102) i8751_return=0; /* ?? */
	if (i8751_value==0x0101) i8751_return=0; /* ?? */
	if (i8751_value==0x8101) i8751_return=coin2 | (coin1<<8); /* Coins */
}

static void lastmiss_i8751_w(int offset, int data)
{
	static int coin,latch=0,snd;
	i8751_return=0;

	switch (offset) {
	case 0: /* High byte */
		i8751_value=(i8751_value&0xff) | (data<<8);
		cpu_cause_interrupt (0, M6809_INT_FIRQ); /* Signal main cpu */
		break;
	case 1: /* Low byte */
		i8751_value=(i8751_value&0xff00) | data;
		break;
	}

	/* Coins are controlled by the i8751 */
 	if ((readinputport(2)&3)==3 && !latch) latch=1;
 	if ((readinputport(2)&3)!=3 && latch) {coin++; latch=0;snd=0x400;}

	if (i8751_value==0x007b) i8751_return=0x0184; 
	if (i8751_value==0x0000) {i8751_return=0x0184; coin=snd=0;}
	if (i8751_value==0x0401) i8751_return=0x0184; 
	if ((i8751_value>>8)==0x01) i8751_return=0x0184; /* Coinage setup */
	if ((i8751_value>>8)==0x02) {i8751_return=snd | coin; snd=0;} /* Coin return */
	if ((i8751_value>>8)==0x03) {i8751_return=0; coin--; } /* Coin clear */
}

static void csilver_i8751_w(int offset, int data)
{
	static int coin,latch=0,snd;
	i8751_return=0;

	switch (offset) {
	case 0: /* High byte */
		i8751_value=(i8751_value&0xff) | (data<<8);
		cpu_cause_interrupt (0, M6809_INT_FIRQ); /* Signal main cpu */
		break;
	case 1: /* Low byte */
		i8751_value=(i8751_value&0xff00) | data;
		break;
	}

	/* Coins are controlled by the i8751 */
 	if ((readinputport(2)&3)==3 && !latch) latch=1;
 	if ((readinputport(2)&3)!=3 && latch) {coin++; latch=0;snd=0x1200;}

	if (i8751_value==0x054a) {i8751_return=~(0x4a); coin=0; snd=0;} /* Captain Silver ID */
	if ((i8751_value>>8)==0x01) i8751_return=0; /* Coinage - Not Supported */
	if ((i8751_value>>8)==0x02) {i8751_return=snd | coin; snd=0; } /* Coin Return */
	if (i8751_value==0x0003 && coin) {i8751_return=0; coin--;} /* Coin Clear */
/* Todo:  Coin insert sound doesn't seem to work...*/
}

/******************************************************************************/

static void dec8_bank_w(int offset, int data)
{
 	int bankaddress;
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];

	bankaddress = 0x10000 + (data & 0x0f) * 0x4000;
	cpu_setbank(1,&RAM[bankaddress]);
}

/* Used by Ghostbusters, Mazehunter & Gondomania */
static void ghostb_bank_w(int offset, int data)
{
 	int bankaddress;
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];

	/* Bit 0: Interrupt enable/disable (I think..)
	   Bit 1: NMI enable/disable
	   Bit 2: ??
	   Bit 3: Screen flip (not supported)
	   Bits 4-7: Bank switch
	*/

	bankaddress = 0x10000 + (data >> 4) * 0x4000;
	cpu_setbank(1,&RAM[bankaddress]);

	if (data&1) int_enable=1; else int_enable=0;
	if (data&2) nmi_enable=1; else nmi_enable=0;


}

void csilver_control_w(int offset, int data)
{
	int bankaddress;
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];

	/* Bottom 4 bits - bank switch */
	bankaddress = 0x10000 + (data & 0x0f) * 0x4000;
	cpu_setbank(1,&RAM[bankaddress]);

	/* There are unknown bits in the top half of the byte! */
 
}

static void dec8_sound_w(int offset, int data)
{
 	soundlatch_w(0,data);
	cpu_cause_interrupt(1,M6502_INT_NMI);
}

static void oscar_sound_w(int offset, int data)
{
 	soundlatch_w(0,data);
	cpu_cause_interrupt(2,M6502_INT_NMI);
}

static void csilver_adpcm_int(int data)
{
	static int toggle;

	toggle = 1 - toggle;
	if (toggle)
		cpu_cause_interrupt(2,M6502_INT_IRQ);
}

static int csilver_adpcm_reset_r(int offset)
{
	MSM5205_reset_w(0,0);
	return 0;
}

static void csilver_adpcm_data_w(int offset,int data)
{
	MSM5205_data_w(offset,data >> 4);
	MSM5205_data_w(offset,data);
}

static void csilver_sound_bank_w(int offset,int data)
{
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[2].memory_region];

	if (data&8) { cpu_setbank(3,&RAM[0x14000]); }
	else { cpu_setbank(3,&RAM[0x10000]); }
}

/******************************************************************************/

static void oscar_int_w(int offset, int data)
{
	/* Deal with interrupts, coins also generate NMI to CPU 0 */
	switch (offset) {
		case 0: /* IRQ2 */
			cpu_cause_interrupt (1, M6809_INT_IRQ);
			return;
		case 1: /* IRC 1 */
			return;
		case 2: /* IRQ 1 */
			cpu_cause_interrupt (0, M6809_INT_IRQ);
			return;
		case 3: /* IRC 2 */
			return;
	}
}

/* Used by Shackled, Last Mission, Captain Silver */
static void shackled_int_w(int offset, int data)
{
	switch (offset) {
		case 0: /* CPU 2 - IRQ acknowledge */
            return;
        case 1: /* CPU 1 - IRQ acknowledge */
        	return;
        case 2: /* i8751 - FIRQ acknowledge */
            return;
        case 3: /* IRQ 1 */
			cpu_cause_interrupt (0, M6809_INT_IRQ);
			return;
        case 4: /* IRQ 2 */
            cpu_cause_interrupt (1, M6809_INT_IRQ);
            return;
	}
}

/******************************************************************************/

static int dec8_share_r(int offset) { return dec8_shared_ram[offset]; }
static int dec8_share2_r(int offset) { return dec8_shared2_ram[offset]; }
static void dec8_share_w(int offset,int data) { dec8_shared_ram[offset]=data; }
static void dec8_share2_w(int offset,int data) { dec8_shared2_ram[offset]=data; }
static int shackled_sprite_r(int offset) { return spriteram[offset]; }
static void shackled_sprite_w(int offset,int data) { spriteram[offset]=data; }
static void shackled_video_w(int offset,int data) { videoram[offset]=data; }
static int shackled_video_r(int offset) { return videoram[offset]; }

/******************************************************************************/

static struct MemoryReadAddress cobra_readmem[] =
{
	{ 0x0000, 0x0fff, MRA_RAM },
	{ 0x1000, 0x1fff, dec8_video_r },
	{ 0x2000, 0x27ff, MRA_RAM },
	{ 0x2800, 0x2fff, MRA_RAM },
	{ 0x3000, 0x31ff, paletteram_r },
	{ 0x3800, 0x3800, input_port_0_r }, /* Player 1 */
	{ 0x3801, 0x3801, input_port_1_r }, /* Player 2 */
	{ 0x3802, 0x3802, input_port_3_r }, /* Dip 1 */
	{ 0x3803, 0x3803, input_port_4_r }, /* Dip 2 */
	{ 0x3a00, 0x3a00, input_port_2_r }, /* VBL & coins */
	{ 0x4000, 0x7fff, MRA_BANK1 },
	{ 0x8000, 0xffff, MRA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress cobra_writemem[] =
{
 	{ 0x0000, 0x0fff, MWA_RAM },
 	{ 0x1000, 0x1fff, dec8_video_w },
	{ 0x2000, 0x27ff, MWA_RAM, &videoram, &videoram_size },
	{ 0x2800, 0x2fff, MWA_RAM, &spriteram },
	{ 0x3000, 0x31ff, paletteram_xxxxBBBBGGGGRRRR_swap_w, &paletteram },
	{ 0x3200, 0x37ff, MWA_RAM }, /* Unknown, seemingly unused */
 	{ 0x3800, 0x381f, dec8_bac06_0_w },
	{ 0x3a00, 0x3a1f, dec8_bac06_1_w },
	{ 0x3c00, 0x3c00, dec8_bank_w },
	{ 0x3c02, 0x3c02, MWA_NOP }, /* DMA flag? */
 	{ 0x3e00, 0x3e00, dec8_sound_w },
	{ 0x4000, 0xffff, MWA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryReadAddress ghostb_readmem[] =
{
	{ 0x0000, 0x1fff, MRA_RAM },
	{ 0x2000, 0x27ff, dec8_video_r },
	{ 0x2800, 0x2dff, MRA_RAM },
	{ 0x3000, 0x37ff, MRA_RAM },
	{ 0x3800, 0x3800, input_port_0_r }, /* Player 1 */
	{ 0x3801, 0x3801, input_port_1_r }, /* Player 2 */
	{ 0x3802, 0x3802, input_port_2_r }, /* Player 3 */
	{ 0x3803, 0x3803, input_port_3_r }, /* Start buttons + VBL */
 	{ 0x3820, 0x3820, input_port_5_r }, /* Dip */
	{ 0x3840, 0x3840, i8751_h_r },
	{ 0x3860, 0x3860, i8751_l_r },
	{ 0x4000, 0x7fff, MRA_BANK1 },
	{ 0x8000, 0xffff, MRA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress ghostb_writemem[] =
{
	{ 0x0000, 0x0fff, MWA_RAM },
	{ 0x1000, 0x17ff, MWA_RAM },
	{ 0x1800, 0x1fff, MWA_RAM, &videoram },
	{ 0x2000, 0x27ff, dec8_video_w },
	{ 0x2800, 0x2bff, MWA_RAM }, /* Scratch ram for rowscroll? */
	{ 0x2c00, 0x2dff, MWA_RAM, &dec8_row },
	{ 0x2e00, 0x2fff, MWA_RAM }, /* Unused */
	{ 0x3000, 0x37ff, MWA_RAM, &spriteram },
	{ 0x3800, 0x3800, dec8_sound_w },
	{ 0x3820, 0x3827, dec8_pf2_w },
	{ 0x3830, 0x3833, dec8_scroll2_w },
	{ 0x3840, 0x3840, ghostb_bank_w },
	{ 0x3860, 0x3861, ghostb_i8751_w },
	{ 0x4000, 0xffff, MWA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryReadAddress srdarwin_readmem[] =
{
	{ 0x0000, 0x13ff, MRA_RAM },
	{ 0x1400, 0x17ff, dec8_video_r },
	{ 0x2000, 0x2000, i8751_h_r },
	{ 0x2001, 0x2001, i8751_l_r },
	{ 0x3800, 0x3800, input_port_2_r }, /* Dip */
	{ 0x3801, 0x3801, input_port_0_r }, /* Player */
	{ 0x3802, 0x3802, input_port_1_r }, /* VBL */
	{ 0x3803, 0x3803, input_port_3_r }, /* Dip */
 	{ 0x4000, 0x7fff, MRA_BANK1 },
	{ 0x8000, 0xffff, MRA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress srdarwin_writemem[] =
{
	{ 0x0000, 0x05ff, MWA_RAM },
	{ 0x0600, 0x07ff, MWA_RAM, &spriteram },
	{ 0x0800, 0x0fff, MWA_RAM, &videoram },
	{ 0x1000, 0x13ff, MWA_RAM },
	{ 0x1400, 0x17ff, dec8_video_w },
	{ 0x1800, 0x1801, srdarwin_i8751_w },
	{ 0x1802, 0x1802, i8751_reset_w },
	{ 0x1803, 0x1803, MWA_NOP },            /* NMI ack */
	{ 0x1804, 0x1804, MWA_NOP },            /* DMA */
	{ 0x1805, 0x1806, srdarwin_control_w }, /* Scroll & Bank */
	{ 0x2000, 0x2000, dec8_sound_w },       /* Sound */
	{ 0x2001, 0x2001, MWA_NOP },            /* Flipscreen (not supported) */
	{ 0x2800, 0x288f, paletteram_xxxxBBBBGGGGRRRR_split1_w, &paletteram },
	{ 0x3000, 0x308f, paletteram_xxxxBBBBGGGGRRRR_split2_w, &paletteram_2 },
	{ 0x4000, 0xffff, MWA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryReadAddress gondo_readmem[] =
{
	{ 0x0000, 0x1fff, MRA_RAM },
	{ 0x2000, 0x27ff, dec8_video_r },
	{ 0x2800, 0x2fff, MRA_RAM },
	{ 0x3000, 0x37ff, MRA_RAM },          /* Sprites */
	{ 0x3800, 0x3800, input_port_7_r },   /* Dip 1 */
	{ 0x3801, 0x3801, input_port_8_r },   /* Dip 2 */
	{ 0x380a, 0x380b, gondo_player_1_r }, /* Player 1 rotary */
	{ 0x380c, 0x380d, gondo_player_2_r }, /* Player 2 rotary */
	{ 0x380e, 0x380e, input_port_3_r },   /* VBL */
	{ 0x380f, 0x380f, input_port_2_r },   /* Fire buttons */
	{ 0x3838, 0x3838, i8751_h_r },
	{ 0x3839, 0x3839, i8751_l_r },
	{ 0x4000, 0x7fff, MRA_BANK1 },
	{ 0x8000, 0xffff, MRA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress gondo_writemem[] =
{
	{ 0x0000, 0x17ff, MWA_RAM },
	{ 0x1800, 0x1fff, MWA_RAM, &videoram, &videoram_size },
  	{ 0x2000, 0x27ff, dec8_video_w },
	{ 0x2800, 0x2bff, paletteram_xxxxBBBBGGGGRRRR_split1_w, &paletteram },
	{ 0x2c00, 0x2fff, paletteram_xxxxBBBBGGGGRRRR_split2_w, &paletteram_2 },
	{ 0x3000, 0x37ff, MWA_RAM, &spriteram },
	{ 0x3810, 0x3810, dec8_sound_w },
	{ 0x3818, 0x382f, gondo_scroll_w },
	{ 0x3830, 0x3830, ghostb_bank_w }, /* Bank + NMI enable */
	{ 0x383a, 0x383b, gondo_i8751_w },
	{ 0x4000, 0xffff, MWA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryReadAddress oscar_readmem[] =
{
	{ 0x0000, 0x0eff, dec8_share_r },
 	{ 0x0f00, 0x0fff, MRA_RAM },
	{ 0x1000, 0x1fff, dec8_share2_r },
	{ 0x2000, 0x27ff, MRA_RAM },
	{ 0x2800, 0x2fff, dec8_video_r },
	{ 0x3000, 0x37ff, MRA_RAM }, /* Sprites */
	{ 0x3800, 0x3bff, paletteram_r },
  	{ 0x3c00, 0x3c00, input_port_0_r },
  	{ 0x3c01, 0x3c01, input_port_1_r },
  	{ 0x3c02, 0x3c02, input_port_2_r }, /* VBL & coins */
  	{ 0x3c03, 0x3c03, input_port_3_r }, /* Dip 1 */
  	{ 0x3c04, 0x3c04, input_port_4_r },
	{ 0x4000, 0x7fff, MRA_BANK1 },
	{ 0x8000, 0xffff, MRA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress oscar_writemem[] =
{
	{ 0x0000, 0x0eff, dec8_share_w, &dec8_shared_ram },
  	{ 0x0f00, 0x0fff, MWA_RAM },
 	{ 0x1000, 0x1fff, dec8_share2_w, &dec8_shared2_ram },
	{ 0x2000, 0x27ff, MWA_RAM, &videoram, &videoram_size },
 	{ 0x2800, 0x2fff, dec8_video_w },
	{ 0x3000, 0x37ff, MWA_RAM, &spriteram },
	{ 0x3800, 0x3bff, paletteram_xxxxBBBBGGGGRRRR_swap_w, &paletteram },
	{ 0x3c10, 0x3c13, dec8_scroll2_w },
	{ 0x3c80, 0x3c80, MWA_NOP },       /* DMA */
	{ 0x3d00, 0x3d00, dec8_bank_w },   /* BNKS */
	{ 0x3d80, 0x3d80, oscar_sound_w }, /* SOUN */
	{ 0x3e00, 0x3e00, MWA_NOP },       /* COINCL */
	{ 0x3e80, 0x3e83, oscar_int_w },
	{ 0x4000, 0xffff, MWA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryReadAddress oscar_sub_readmem[] =
{
	{ 0x0000, 0x0eff, dec8_share_r },
  	{ 0x0f00, 0x0fff, MRA_RAM },
  	{ 0x1000, 0x1fff, dec8_share2_r },
	{ 0x4000, 0xffff, MRA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress oscar_sub_writemem[] =
{
	{ 0x0000, 0x0eff, dec8_share_w },
 	{ 0x0f00, 0x0fff, MWA_RAM },
  	{ 0x1000, 0x1fff, dec8_share2_w },
 	{ 0x3e80, 0x3e83, oscar_int_w },
	{ 0x4000, 0xffff, MWA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryReadAddress lastmiss_readmem[] =
{
	{ 0x0000, 0x0fff, dec8_share_r },
	{ 0x1000, 0x17ff, MRA_RAM },
    { 0x1800, 0x1800, input_port_0_r },
	{ 0x1801, 0x1801, input_port_1_r },
	{ 0x1802, 0x1802, input_port_2_r },
	{ 0x1803, 0x1803, input_port_3_r }, /* Dip 1 */
	{ 0x1804, 0x1804, input_port_4_r }, /* Dip 2 */
	{ 0x1806, 0x1806, i8751_h_r },
	{ 0x1807, 0x1807, i8751_l_r },
	{ 0x2000, 0x27ff, MRA_RAM },
	{ 0x2800, 0x2fff, MRA_RAM },
    { 0x3000, 0x37ff, dec8_share2_r },
	{ 0x3800, 0x3fff, dec8_video_r },
	{ 0x4000, 0x7fff, MRA_BANK1 },
	{ 0x8000, 0xffff, MRA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress lastmiss_writemem[] =
{
	{ 0x0000, 0x0fff, dec8_share_w, &dec8_shared_ram },
	{ 0x1000, 0x13ff, paletteram_xxxxBBBBGGGGRRRR_split1_w, &paletteram },
	{ 0x1400, 0x17ff, paletteram_xxxxBBBBGGGGRRRR_split2_w, &paletteram_2 },
   	{ 0x1800, 0x1804, shackled_int_w },
	{ 0x1805, 0x1805, MWA_NOP }, /* DMA */
	{ 0x1807, 0x1807, MWA_NOP }, /* Flipscreen */
	{ 0x1809, 0x1809, lastmiss_scrollx_w }, /* Scroll LSB */
	{ 0x180b, 0x180b, lastmiss_scrolly_w }, /* Scroll LSB */
	{ 0x180c, 0x180c, oscar_sound_w },
	{ 0x180d, 0x180d, lastmiss_control_w }, /* Bank switch + Scroll MSB */
	{ 0x180e, 0x180f, lastmiss_i8751_w },
	{ 0x2000, 0x27ff, MWA_RAM, &videoram, &videoram_size },
	{ 0x2800, 0x2fff, MWA_RAM, &spriteram },
	{ 0x3000, 0x37ff, dec8_share2_w, &dec8_shared2_ram },
	{ 0x3800, 0x3fff, dec8_video_w },
	{ 0x4000, 0xffff, MWA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryReadAddress lastmiss_sub_readmem[] =
{
	{ 0x0000, 0x0fff, dec8_share_r },
    { 0x1800, 0x1800, input_port_0_r },
	{ 0x1801, 0x1801, input_port_1_r },
	{ 0x1802, 0x1802, input_port_2_r },
	{ 0x1803, 0x1803, input_port_3_r }, /* Dip 1 */
	{ 0x1804, 0x1804, input_port_4_r }, /* Dip 2 */
    { 0x3000, 0x37ff, dec8_share2_r },
	{ 0x3800, 0x3fff, dec8_video_r },
	{ 0x4000, 0xffff, MRA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress lastmiss_sub_writemem[] =
{
	{ 0x0000, 0x0fff, dec8_share_w },
	{ 0x1000, 0x13ff, paletteram_xxxxBBBBGGGGRRRR_split1_w },
	{ 0x1400, 0x17ff, paletteram_xxxxBBBBGGGGRRRR_split2_w },
   	{ 0x1800, 0x1804, shackled_int_w },
	{ 0x1805, 0x1805, MWA_NOP }, /* DMA */
	{ 0x1807, 0x1807, MWA_NOP }, /* Flipscreen */
	{ 0x180c, 0x180c, oscar_sound_w },
	{ 0x2000, 0x27ff, shackled_video_w },
	{ 0x2800, 0x2fff, shackled_sprite_w },
    { 0x3000, 0x37ff, dec8_share2_w },
	{ 0x3800, 0x3fff, dec8_video_w },
	{ 0x4000, 0xffff, MWA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryReadAddress shackled_readmem[] =
{
	{ 0x0000, 0x0fff, dec8_share_r },
   	{ 0x1000, 0x17ff, MRA_RAM },
 	{ 0x1800, 0x1800, input_port_0_r },
	{ 0x1801, 0x1801, input_port_1_r },
	{ 0x1802, 0x1802, input_port_2_r },
  	{ 0x1803, 0x1803, input_port_3_r },
  	{ 0x1804, 0x1804, input_port_4_r },
	{ 0x2000, 0x27ff, shackled_video_r },
	{ 0x2800, 0x2fff, shackled_sprite_r },
	{ 0x3000, 0x37ff, dec8_share2_r },
	{ 0x3800, 0x3fff, dec8_video_r },
	{ 0x4000, 0x7fff, MRA_BANK1 },
	{ 0x8000, 0xffff, MRA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress shackled_writemem[] =
{
	{ 0x0000, 0x0fff, dec8_share_w, &dec8_shared_ram },
	{ 0x1000, 0x13ff, paletteram_xxxxBBBBGGGGRRRR_split1_w, &paletteram },
	{ 0x1400, 0x17ff, paletteram_xxxxBBBBGGGGRRRR_split2_w, &paletteram_2 },
   	{ 0x1800, 0x1804, shackled_int_w },
	{ 0x1805, 0x1805, MWA_NOP }, /* DMA */
   	{ 0x1809, 0x1809, lastmiss_scrollx_w }, /* Scroll LSB */
	{ 0x180b, 0x180b, lastmiss_scrolly_w }, /* Scroll LSB */
	{ 0x180c, 0x180c, oscar_sound_w },
	{ 0x180d, 0x180d, lastmiss_control_w }, /* Bank switch + Scroll MSB */
	{ 0x2000, 0x27ff, shackled_video_w },
	{ 0x2800, 0x2fff, shackled_sprite_w },
	{ 0x3000, 0x37ff, dec8_share2_w, &dec8_shared2_ram },
	{ 0x3800, 0x3fff, dec8_video_w },
	{ 0x4000, 0xffff, MWA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryReadAddress shackled_sub_readmem[] =
{
	{ 0x0000, 0x0fff, dec8_share_r },
   	{ 0x1000, 0x17ff, MRA_RAM },
   	{ 0x1800, 0x1800, input_port_0_r },
	{ 0x1801, 0x1801, input_port_1_r },
	{ 0x1802, 0x1802, input_port_2_r },
   	{ 0x1803, 0x1803, input_port_3_r },
	{ 0x1804, 0x1804, input_port_4_r },
	{ 0x1806, 0x1806, i8751_h_r },
	{ 0x1807, 0x1807, i8751_l_r },
	{ 0x2000, 0x27ff, MRA_RAM },
	{ 0x2800, 0x2fff, MRA_RAM },
	{ 0x3000, 0x37ff, dec8_share2_r },
	{ 0x3800, 0x3fff, dec8_video_r },
	{ 0x4000, 0xffff, MRA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress shackled_sub_writemem[] =
{
	{ 0x0000, 0x0fff, dec8_share_w },
	{ 0x1000, 0x13ff, paletteram_xxxxBBBBGGGGRRRR_split1_w, &paletteram },
	{ 0x1400, 0x17ff, paletteram_xxxxBBBBGGGGRRRR_split2_w, &paletteram_2 },
   	{ 0x1800, 0x1804, shackled_int_w },
	{ 0x1805, 0x1805, MWA_NOP }, /* DMA */
	{ 0x1809, 0x1809, lastmiss_scrollx_w }, /* Scroll LSB */
	{ 0x180b, 0x180b, lastmiss_scrolly_w }, /* Scroll LSB */
	{ 0x180c, 0x180c, oscar_sound_w },
	{ 0x180d, 0x180d, lastmiss_control_w }, /* Bank switch + Scroll MSB */
	{ 0x180e, 0x180f, shackled_i8751_w },
	{ 0x2000, 0x27ff, MWA_RAM, &videoram, &videoram_size },
	{ 0x2800, 0x2fff, MWA_RAM, &spriteram },
	{ 0x3000, 0x37ff, dec8_share2_w },
	{ 0x3800, 0x3fff, dec8_video_w },
	{ 0x4000, 0xffff, MWA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryReadAddress csilver_readmem[] =
{
	{ 0x0000, 0x0fff, dec8_share_r },
   	{ 0x1000, 0x17ff, MRA_RAM },
 
	{ 0x1801, 0x1801, input_port_0_r },
	{ 0x1803, 0x1803, input_port_2_r },

	{ 0x1804, 0x1804, input_port_4_r }, /* Dip 2 */
	{ 0x1805, 0x1805, input_port_3_r }, /* Dip 1 */
	{ 0x1c00, 0x1c00, i8751_h_r },
	{ 0x1e00, 0x1e00, i8751_l_r },

	{ 0x2000, 0x27ff, shackled_video_r },
	{ 0x2800, 0x2fff, shackled_sprite_r },
	{ 0x3000, 0x37ff, dec8_share2_r },
	{ 0x3800, 0x3fff, dec8_video_r },
	{ 0x4000, 0x7fff, MRA_BANK1 },
	{ 0x8000, 0xffff, MRA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress csilver_writemem[] =
{
	{ 0x0000, 0x0fff, dec8_share_w, &dec8_shared_ram },
	{ 0x1000, 0x13ff, paletteram_xxxxBBBBGGGGRRRR_split1_w, &paletteram },
	{ 0x1400, 0x17ff, paletteram_xxxxBBBBGGGGRRRR_split2_w, &paletteram_2 },
   	{ 0x1800, 0x1804, shackled_int_w },
	{ 0x1805, 0x1805, MWA_NOP }, /* DMA */
	{ 0x1807, 0x1807, MWA_NOP }, /* Flipscreen */

   	{ 0x1808, 0x180b, dec8_scroll2_w },
	{ 0x180c, 0x180c, oscar_sound_w },
	{ 0x180d, 0x180d, csilver_control_w },
	{ 0x180e, 0x180f, csilver_i8751_w },

	{ 0x2000, 0x27ff, shackled_video_w },
	{ 0x2800, 0x2fff, shackled_sprite_w },
	{ 0x3000, 0x37ff, dec8_share2_w, &dec8_shared2_ram },
	{ 0x3800, 0x3fff, dec8_video_w },
	{ 0x4000, 0xffff, MWA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryReadAddress csilver_sub_readmem[] =
{
	{ 0x0000, 0x0fff, dec8_share_r },
	{ 0x1000, 0x17ff, MRA_RAM },
  

	{ 0x1803, 0x1803, input_port_2_r },
	{ 0x1804, 0x1804, input_port_4_r },
	{ 0x1805, 0x1805, input_port_3_r },

	{ 0x2000, 0x27ff, MRA_RAM },
	{ 0x2800, 0x2fff, MRA_RAM },
	{ 0x3000, 0x37ff, dec8_share2_r },
	{ 0x3800, 0x3fff, dec8_video_r },
	{ 0x4000, 0xffff, MRA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress csilver_sub_writemem[] =
{
	{ 0x0000, 0x0fff, dec8_share_w },
	{ 0x1000, 0x13ff, paletteram_xxxxBBBBGGGGRRRR_split1_w, &paletteram },
	{ 0x1400, 0x17ff, paletteram_xxxxBBBBGGGGRRRR_split2_w, &paletteram_2 },
   	{ 0x1800, 0x1804, shackled_int_w },
	{ 0x1805, 0x1805, MWA_NOP }, /* DMA */
	{ 0x180c, 0x180c, oscar_sound_w },
	{ 0x180d, 0x180d, lastmiss_control_w }, /* Bank switch + Scroll MSB */
	{ 0x2000, 0x27ff, MWA_RAM, &videoram, &videoram_size },
	{ 0x2800, 0x2fff, MWA_RAM, &spriteram },
	{ 0x3000, 0x37ff, dec8_share2_w },
	{ 0x3800, 0x3fff, dec8_video_w },
	{ 0x4000, 0xffff, MWA_ROM },
	{ -1 }  /* end of table */
};

/******************************************************************************/

/* Used for Cobra Command, Maze Hunter, Super Real Darwin, Gondomania, etc */
static struct MemoryReadAddress dec8_s_readmem[] =
{
	{ 0x0000, 0x05ff, MRA_RAM},
	{ 0x6000, 0x6000, soundlatch_r },
	{ 0x8000, 0xffff, MRA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress dec8_s_writemem[] =
{
 	{ 0x0000, 0x05ff, MWA_RAM},
	{ 0x2000, 0x2000, YM2203_control_port_0_w }, /* OPN */
	{ 0x2001, 0x2001, YM2203_write_port_0_w },
	{ 0x4000, 0x4000, YM3812_control_port_0_w }, /* OPL */
	{ 0x4001, 0x4001, YM3812_write_port_0_w },
 	{ 0x8000, 0xffff, MWA_ROM },
	{ -1 }  /* end of table */
};

/* Used by Last Mission, Shackled & Breywood */
static struct MemoryReadAddress ym3526_s_readmem[] =
{
	{ 0x0000, 0x05ff, MRA_RAM},
	{ 0x3000, 0x3000, soundlatch_r },
	{ 0x8000, 0xffff, MRA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress ym3526_s_writemem[] =
{
 	{ 0x0000, 0x05ff, MWA_RAM},
	{ 0x0800, 0x0800, YM2203_control_port_0_w }, /* OPN */
	{ 0x0801, 0x0801, YM2203_write_port_0_w },
	{ 0x1000, 0x1000, YM3526_control_port_0_w }, /* ? */
	{ 0x1001, 0x1001, YM3526_write_port_0_w },
 	{ 0x8000, 0xffff, MWA_ROM },
	{ -1 }  /* end of table */
};

/* Captain Silver - same sound system as Pocket Gal */
static struct MemoryReadAddress csilver_s_readmem[] =
{
	{ 0x0000, 0x07ff, MRA_RAM },
	{ 0x3000, 0x3000, soundlatch_r },
	{ 0x3400, 0x3400, csilver_adpcm_reset_r },	/* ? not sure */
	{ 0x4000, 0x7fff, MRA_BANK3 },
	{ 0x8000, 0xffff, MRA_ROM },
	{ -1 }	/* end of table */
};

static struct MemoryWriteAddress csilver_s_writemem[] =
{
	{ 0x0000, 0x07ff, MWA_RAM },
    { 0x0800, 0x0800, YM2203_control_port_0_w },
    { 0x0801, 0x0801, YM2203_write_port_0_w },
	{ 0x1000, 0x1000, YM3812_control_port_0_w },
	{ 0x1001, 0x1001, YM3812_write_port_0_w },
	{ 0x1800, 0x1800, csilver_adpcm_data_w },	/* ADPCM data for the MSM5205 chip */
	{ 0x2000, 0x2000, csilver_sound_bank_w },
	{ 0x4000, 0xffff, MWA_ROM },
	{ -1 }	/* end of table */
};

/******************************************************************************/

#define PLAYER1_JOYSTICK /* Player 1 controls */ \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY )				\
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY )			\
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY )			\
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )

#define PLAYER2_JOYSTICK /* Player 2 controls */ \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_COCKTAIL )		\
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_COCKTAIL )	\
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_COCKTAIL )	\
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )

INPUT_PORTS_START( input_ports )
	PORT_START	/* Player 1 controls */
	PLAYER1_JOYSTICK
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

 	PORT_START	/* Player 2 controls */
	PLAYER2_JOYSTICK
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_VBLANK )

	PORT_START	/* Dip switch bank 1 */
	PORT_DIPNAME( 0x03, 0x03, "Coin A", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x01, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x03, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x02, "1 Coin/2 Credits" )
	PORT_DIPNAME( 0x0c, 0x0c, "Coin B", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x04, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x0c, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x08, "1 Coin/2 Credits" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* Test mode on other games */
	PORT_DIPNAME( 0x20, 0x20, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x20, "On" )
	PORT_DIPNAME( 0x40, 0x40, "Screen Rotation", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Normal" )
	PORT_DIPSETTING(    0x00, "Reverse" )
	PORT_DIPNAME( 0x80, 0x80, "Cabinet", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Cocktail" )
	PORT_DIPSETTING(    0x00, "Upright" )

	PORT_START	/* Dip switch bank 2 */
	PORT_DIPNAME( 0x03, 0x03, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "Infinite" )
	PORT_BITX( 0,       0x00, IPT_DIPSWITCH_SETTING | IPF_CHEAT, "Infinite", IP_KEY_NONE, IP_JOY_NONE, 0 )
	PORT_DIPNAME( 0x0c, 0x0c, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x04, "Easy" )
	PORT_DIPSETTING(    0x0c, "Normal" )
	PORT_DIPSETTING(    0x08, "Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0x10, 0x10, "Allow Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "No" )
	PORT_DIPSETTING(    0x10, "Yes" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

INPUT_PORTS_START( ghostb_input_ports )
	PORT_START	/* Player 1 controls */
	PLAYER1_JOYSTICK
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

 	PORT_START	/* Player 2 controls */
	PLAYER2_JOYSTICK
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

 	PORT_START	/* Player 3 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
 	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
 	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x20, 0x20, "Attract Mode Sound", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Yes" )
	PORT_DIPSETTING(    0x00, "No" )
	PORT_DIPNAME( 0x40, 0x40, "Screen Flip", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Normal" )
	PORT_DIPSETTING(    0x00, "Inverted" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* Dummy input for i8751 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START	/* Dip switch */
	PORT_DIPNAME( 0x03, 0x03, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, "Infinite" )
	PORT_DIPNAME( 0x0c, 0x0c, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x04, "Easy" )
	PORT_DIPSETTING(    0x0c, "Normal" )
	PORT_DIPSETTING(    0x08, "Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0x30, 0x30, "Scene Time", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "4.00" )
	PORT_DIPSETTING(    0x10, "4.30" )
	PORT_DIPSETTING(    0x30, "5.00" )
	PORT_DIPSETTING(    0x20, "6.00" )
	PORT_DIPNAME( 0x40, 0x00, "Allow Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "No" )
	PORT_DIPSETTING(    0x00, "Yes" )
	PORT_DIPNAME( 0x80, 0x80, "Beam Energy Pickup", IP_KEY_NONE ) /* Ghostb only */
	PORT_DIPSETTING(    0x00, "Up 1.5%" )
	PORT_DIPSETTING(    0x80, "Normal" )



INPUT_PORTS_END

INPUT_PORTS_START( darwin_input_ports )
	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) /* Fake */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START
	/* The bottom bits of this dip (coinage) are for the i8751 */
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) /* Unused dip */
	PORT_DIPNAME( 0x20, 0x20, "Attract Mode Sound", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "On" )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPNAME( 0x40, 0x40, "Screen Flip", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Normal" )
	PORT_DIPSETTING(    0x00, "Inverted" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 7", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START
	PORT_DIPNAME( 0x03, 0x03, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "27" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 1", IP_KEY_NONE )
	PORT_DIPSETTING(    0x04, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 2", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 3", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x10, "On" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 4", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 5", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x40, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 6", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x80, "On" )
INPUT_PORTS_END

INPUT_PORTS_START( gondo_input_ports )
	PORT_START	/* Player 1 controls */
	PLAYER1_JOYSTICK
	/* Top 4 bits are rotary controller */

 	PORT_START	/* Player 2 controls */
	PLAYER2_JOYSTICK
	/* Top 4 bits are rotary controller */

 	PORT_START	/* Player 1 & 2 fire buttons */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK )

	PORT_START /* Fake port for the i8751 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START	/* player 1 12-way rotary control */
	PORT_ANALOGX( 0xff, 0x00, IPT_DIAL | IPF_REVERSE, 25, 0, 0, 0, OSD_KEY_Z, OSD_KEY_X, 0, 0, 8 )

	PORT_START	/* player 2 12-way rotary control */
	PORT_ANALOGX( 0xff, 0x00, IPT_DIAL | IPF_REVERSE | IPF_PLAYER2, 25, 0, 0, 0, OSD_KEY_N, OSD_KEY_M, 0, 0, 8 )

	PORT_START	/* Dip switch bank 1 */
	/* Coinage not currently supported */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) /* Test mode on other games */
	PORT_DIPNAME( 0x20, 0x20, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x20, "On" )
	PORT_DIPNAME( 0x40, 0x40, "Screen Rotation", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Normal" )
	PORT_DIPSETTING(    0x00, "Reverse" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) /* Cabinet on other games */

	PORT_START	/* Dip switch bank 2 */
	PORT_DIPNAME( 0x03, 0x03, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, "Infinite" )
	PORT_BITX( 0,       0x00, IPT_DIPSWITCH_SETTING | IPF_CHEAT, "Infinite", IP_KEY_NONE, IP_JOY_NONE, 0 )
	PORT_DIPNAME( 0x0c, 0x0c, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x04, "Easy" )
	PORT_DIPSETTING(    0x0c, "Normal" )
	PORT_DIPSETTING(    0x08, "Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0x10, 0x00, "Allow Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Yes" )
	PORT_DIPSETTING(    0x10, "No" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

INPUT_PORTS_START( oscar_input_ports )
	PORT_START	/* Player 1 controls */
	PLAYER1_JOYSTICK
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

 	PORT_START	/* Player 2 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK )

	PORT_START	/* Dip switch bank 1 */
	PORT_DIPNAME( 0x03, 0x03, "Coin A", IP_KEY_NONE )
	PORT_DIPSETTING(    0x03, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x02, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x01, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x00, "2 Coins/1 Credit" )
	PORT_DIPNAME( 0x0c, 0x0c, "Coin B", IP_KEY_NONE )
	PORT_DIPSETTING(    0x0c, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x08, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x00, "2 Coins/1 Credit" )
	PORT_DIPNAME( 0x20, 0x20, "Demo Freeze Mode", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "No" )
	PORT_DIPSETTING(    0x00, "Yes" )
	PORT_DIPNAME( 0x20, 0x20, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x20, "On" )
	PORT_DIPNAME( 0x40, 0x40, "Screen Rotation", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Normal" )
	PORT_DIPSETTING(    0x00, "Reverse" )
	PORT_DIPNAME( 0x80, 0x80, "Cabinet", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Cocktail" )
	PORT_DIPSETTING(    0x00, "Upright" )

	PORT_START	/* Dip switch bank 2 */
	PORT_DIPNAME( 0x03, 0x03, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, "Infinite" )
	PORT_BITX( 0,       0x00, IPT_DIPSWITCH_SETTING | IPF_CHEAT, "Infinite", IP_KEY_NONE, IP_JOY_NONE, 0 )
	PORT_DIPNAME( 0x0c, 0x0c, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x04, "Easy" )
	PORT_DIPSETTING(    0x0c, "Normal" )
	PORT_DIPSETTING(    0x08, "Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0x30, 0x30, "Bonus Life", IP_KEY_NONE )
	PORT_DIPSETTING(    0x30, "Every 40000" )
	PORT_DIPSETTING(    0x20, "Every 60000" )
	PORT_DIPSETTING(    0x10, "Every 90000" )
	PORT_DIPSETTING(    0x00, "50000 only" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x80, 0x80, "Allow Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "No" )
	PORT_DIPSETTING(    0x80, "Yes" )
INPUT_PORTS_END

INPUT_PORTS_START( lastmiss_input_ports )
	PORT_START	/* Player 1 controls */
	PLAYER1_JOYSTICK
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

 	PORT_START	/* Player 2 controls */
	PLAYER2_JOYSTICK
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK )

	PORT_START	/* Dip switch bank 1 */
	/* Coinage options not supported (controlled by the i8751) */
	PORT_DIPNAME( 0x10, 0x00, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPSETTING(    0x10, "Off" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 1", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x20, "On" )
	PORT_DIPNAME( 0x40, 0x40, "Invincible", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "No" )
	PORT_DIPSETTING(    0x00, "Yes" )
	PORT_DIPNAME( 0x80, 0x80, "Cabinet?", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Cocktail" )
	PORT_DIPSETTING(    0x00, "Upright" )

	PORT_START	/* Dip switch bank 2 */
	PORT_DIPNAME( 0x03, 0x03, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x00, "Infinite" )
	PORT_BITX( 0,       0x00, IPT_DIPSWITCH_SETTING | IPF_CHEAT, "Infinite", IP_KEY_NONE, IP_JOY_NONE, 0 )
	PORT_DIPNAME( 0x0c, 0x0c, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x04, "Easy" )
	PORT_DIPSETTING(    0x0c, "Normal" )
	PORT_DIPSETTING(    0x08, "Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0x10, 0x10, "Allow Continue?", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "No" )
	PORT_DIPSETTING(    0x10, "Yes" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

INPUT_PORTS_START( shackled_input_ports )
	PORT_START	/* Player 1 controls */
	PLAYER1_JOYSTICK
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

 	PORT_START	/* Player 2 controls */
	PLAYER2_JOYSTICK
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK )

	PORT_START	/* Dip switch bank 1 */
	/* Coinage not supported */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* Test mode on other games */
	PORT_DIPNAME( 0x20, 0x20, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x20, "On" )
	PORT_DIPNAME( 0x40, 0x40, "Screen Rotation", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Normal" )
	PORT_DIPSETTING(    0x00, "Reverse" )
	PORT_DIPNAME( 0x80, 0x80, "Cabinet", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Cocktail" )
	PORT_DIPSETTING(    0x00, "Upright" )

	PORT_START	/* Dip switch bank 2 */
	PORT_DIPNAME( 0x03, 0x03, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x00, "Infinite" )
	PORT_BITX( 0,       0x00, IPT_DIPSWITCH_SETTING | IPF_CHEAT, "Infinite", IP_KEY_NONE, IP_JOY_NONE, 0 )
	PORT_DIPNAME( 0x0c, 0x0c, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x04, "Easy" )
	PORT_DIPSETTING(    0x0c, "Normal" )
	PORT_DIPSETTING(    0x08, "Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0x10, 0x10, "Allow Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "No" )
	PORT_DIPSETTING(    0x10, "Yes" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

INPUT_PORTS_START( csilver_input_ports )
	PORT_START	/* Player 1 controls */
	PLAYER1_JOYSTICK
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 )

 	PORT_START	/* Player 2 controls */
	PLAYER2_JOYSTICK
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK )

	PORT_START	/* Dip switch bank 1 */
	/* Coinage not supported */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* Test mode on other games */
	PORT_DIPNAME( 0x20, 0x20, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x20, "On" )
	PORT_DIPNAME( 0x40, 0x40, "Screen Rotation", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Normal" )
	PORT_DIPSETTING(    0x00, "Reverse" )
	PORT_DIPNAME( 0x80, 0x80, "Cabinet", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Cocktail" )
	PORT_DIPSETTING(    0x00, "Upright" )

	PORT_START	/* Dip switch bank 2 */
	PORT_DIPNAME( 0x03, 0x03, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x00, "Infinite" )
	PORT_BITX( 0,       0x00, IPT_DIPSWITCH_SETTING | IPF_CHEAT, "Infinite", IP_KEY_NONE, IP_JOY_NONE, 0 )
	PORT_DIPNAME( 0x0c, 0x0c, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x04, "Easy" )
	PORT_DIPSETTING(    0x0c, "Normal" )
	PORT_DIPSETTING(    0x08, "Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0x10, 0x10, "Allow Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "No" )
	PORT_DIPSETTING(    0x10, "Yes" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/******************************************************************************/

static struct GfxLayout charlayout_32k =
{
	8,8,
	1024,
	2,
	{ 0x4000*8,0x0000*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8	/* every sprite takes 8 consecutive bytes */
};

static struct GfxLayout chars_3bpp =
{
	8,8,
	1024,
	3,
	{ 0x6000*8,0x4000*8,0x2000*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8	/* every sprite takes 8 consecutive bytes */
};

/* SRDarwin characters - very unusual layout for Data East */
static struct GfxLayout charlayout_16k =
{
	8,8,	/* 8*8 characters */
	1024,
	2,	/* 2 bits per pixel */
	{ 0, 4 },	/* the two bitplanes for 4 pixels are packed into one byte */
	{ 0x2000*8+0, 0x2000*8+1, 0x2000*8+2, 0x2000*8+3, 0, 1, 2, 3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8	/* every char takes 8 consecutive bytes */
};

static struct GfxLayout oscar_charlayout =
{
	8,8,
	1024,
	3,
	{ 0x3000*8,0x2000*8,0x1000*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8	/* every sprite takes 8 consecutive bytes */
};

/* Darwin sprites - only 3bpp */
static struct GfxLayout sr_sprites =
{
	16,16,
	2048,
	3,
 	{ 0x10000*8,0x20000*8,0x00000*8 },
	{ 16*8, 1+(16*8), 2+(16*8), 3+(16*8), 4+(16*8), 5+(16*8), 6+(16*8), 7+(16*8),
		0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 ,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8

},
	16*16
};

static struct GfxLayout srdarwin_tiles =
{
	16,16,
	256,
	4,
	{ 0x8000*8, 0x8000*8+4, 0, 4 },
	{ 0, 1, 2, 3, 1024*8*8+0, 1024*8*8+1, 1024*8*8+2, 1024*8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+1024*8*8+0, 16*8+1024*8*8+1, 16*8+1024*8*8+2, 16*8+1024*8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8	/* every tile takes 32 consecutive bytes */
};

static struct GfxLayout tiles =
{
	16,16,
	4096,
	4,
 	{ 0x60000*8,0x40000*8,0x20000*8,0x00000*8 },
	{ 16*8, 1+(16*8), 2+(16*8), 3+(16*8), 4+(16*8), 5+(16*8), 6+(16*8), 7+(16*8),
		0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 ,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8},
	16*16
};

/* X flipped on Ghostbusters tiles */
static struct GfxLayout tiles_r =
{
	16,16,
	2048,
	4,
 	{ 0x20000*8,0x00000*8,0x30000*8,0x10000*8 },
	{ 7,6,5,4,3,2,1,0,
		7+(16*8), 6+(16*8), 5+(16*8), 4+(16*8), 3+(16*8), 2+(16*8), 1+(16*8), 0+(16*8) },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 ,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8},
	16*16
};

static struct GfxDecodeInfo gfxdecodeinfo[] =
{
	{ 1, 0x000000, &charlayout_32k,     0, 4  },
	{ 1, 0x008000, &tiles,   64, 4 },
	{ 1, 0x088000, &tiles,  192, 4 },
	{ 1, 0x108000, &tiles,  128, 4 },
	{ -1 } /* end of array */
};

static struct GfxDecodeInfo ghostb_gfxdecodeinfo[] =
{
	{ 1, 0x00000, &chars_3bpp,	0,  4 },
	{ 1, 0x08000, &tiles,     256, 16 },
	{ 1, 0x88000, &tiles_r,   512, 16 },
	{ -1 } /* end of array */
};

static struct GfxDecodeInfo srdarwin_gfxdecodeinfo[] =
{
	{ 1, 0x00000, &charlayout_16k,128, 32  },
	{ 1, 0x08000, &sr_sprites,     64, 8 }, 
	{ 1, 0x38000, &srdarwin_tiles,  0, 8 },
  	{ 1, 0x48000, &srdarwin_tiles,  0, 8 },
    { 1, 0x58000, &srdarwin_tiles,  0, 8 }, 
    { 1, 0x68000, &srdarwin_tiles,  0, 8 },
	{ -1 } /* end of array */
};

static struct GfxDecodeInfo gondo_gfxdecodeinfo[] =
{
	{ 1, 0x00000, &chars_3bpp,  0, 16 }, /* Chars */
	{ 1, 0x08000, &tiles,     256, 16 }, /* Sprites */
	{ 1, 0x88000, &tiles,     768, 16 }, /* Tiles */
 	{ -1 } /* end of array */
};

static struct GfxDecodeInfo oscar_gfxdecodeinfo[] =
{
	{ 1, 0x00000, &oscar_charlayout, 256,  8 }, /* Chars */
	{ 1, 0x08000, &tiles,              0, 16 }, /* Sprites */
	{ 1, 0x88000, &tiles,            384,  8 }, /* Tiles */
 	{ -1 } /* end of array */
};

static struct GfxDecodeInfo lastmiss_gfxdecodeinfo[] =
{
	{ 1, 0x00000, &chars_3bpp,0, 4 },
	{ 1, 0x08000, &tiles,  256, 16 },
	{ 1, 0x88000, &tiles,  768, 16 },
 	{ -1 } /* end of array */
};

/******************************************************************************/

static struct YM2203interface ym2203_interface =
{
	1,
	1500000,	/* Unknown */
	{ YM2203_VOL(30,40) },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 }
};

/* handler called by the 3812 emulator when the internal timers cause an IRQ */
static void irqhandler(void)
{
	cpu_cause_interrupt(1,M6502_INT_IRQ);
}

static void oscar_irqhandler(void)
{
	cpu_cause_interrupt(2,M6502_INT_IRQ);
}

static struct YM3526interface ym3526_interface =
{
	1,			/* 1 chip (no more supported) */
	3000000,	/* 3 MHz ? */
	{ 255 },	/* (not supported) */
	irqhandler,
};

static struct YM3526interface oscar_ym3526_interface =
{
	1,			/* 1 chip (no more supported) */
	3000000,	/* 3 MHz ? */
	{ 255 },		/* (not supported) */
	oscar_irqhandler,
};

static struct YM3812interface ym3812_interface =
{
	1,			/* 1 chip (no more supported) */
	3000000,	/* 3 MHz ? */
	{ 255 },		/* (not supported) */
	irqhandler,
};

static struct MSM5205interface msm5205_interface =
{
	1,		/* 1 chip */
	8000,	/* 8000Hz playback ? */
	csilver_adpcm_int,		/* interrupt function */
	{ 99 }
};

/******************************************************************************/

static int ghostb_interrupt(void)
{
	static int latch[4];
	int i8751_out=readinputport(4);

	/* Ghostbusters coins are controlled by the i8751 */
	if ((i8751_out & 0x8) == 0x8) latch[0]=1;
	if ((i8751_out & 0x4) == 0x4) latch[1]=1;
	if ((i8751_out & 0x2) == 0x2) latch[2]=1;
	if ((i8751_out & 0x1) == 0x1) latch[3]=1;

	if (((i8751_out & 0x8) != 0x8) && latch[0]) {latch[0]=0; cpu_cause_interrupt(0,M6809_INT_IRQ); i8751_return=0x8001; } /* Player 1 coin */
	if (((i8751_out & 0x4) != 0x4) && latch[1]) {latch[1]=0; cpu_cause_interrupt(0,M6809_INT_IRQ); i8751_return=0x4001; } /* Player 2 coin */
	if (((i8751_out & 0x2) != 0x2) && latch[2]) {latch[2]=0; cpu_cause_interrupt(0,M6809_INT_IRQ); i8751_return=0x2001; } /* Player 3 coin */
	if (((i8751_out & 0x1) != 0x1) && latch[3]) {latch[3]=0; cpu_cause_interrupt(0,M6809_INT_IRQ); i8751_return=0x1001; } /* Service */

	if (nmi_enable) return M6809_INT_NMI; /* VBL */

	return 0; /* VBL */
}

static int gondo_interrupt(void)
{
	if (nmi_enable)
		return M6809_INT_NMI; /* VBL */

	return 0; /* VBL */
}

/* Coins generate NMI's */
static int oscar_interrupt(void)
{
	static int latch=1;

	if ((readinputport(2) & 0x7) == 0x7) latch=1;
	if (latch && (readinputport(2) & 0x7) != 0x7) {
		latch=0;
    	cpu_cause_interrupt (0, M6809_INT_NMI);
    }

	return 0;
}

/******************************************************************************/

static struct MachineDriver cobra_machine_driver =
{
	/* basic machine hardware */
	{
 		{
			CPU_M6809,
			1250000,
			0,
			cobra_readmem,cobra_writemem,0,0,
			nmi_interrupt,1
		},
		{
			CPU_M6502 | CPU_AUDIO_CPU,
			1250000,        /* 1.25 Mhz ? */
			2,	/* memory region #2 */
			dec8_s_readmem,dec8_s_writemem,0,0,
			ignore_interrupt,0	/* IRQs are caused by the YM3812 */
								/* NMIs are caused by the main CPU */
		}
	},
	60, DEFAULT_REAL_60HZ_VBLANK_DURATION,
	1,
	0,	/* init machine */

	/* video hardware */
	32*8, 32*8, { 0*8, 32*8-1, 1*8, 31*8-1 },
  

	gfxdecodeinfo,
	256,256,
	0,

	VIDEO_TYPE_RASTER | VIDEO_MODIFIES_PALETTE | VIDEO_UPDATE_BEFORE_VBLANK,
	0,
	dec8_vh_start,
	dec8_vh_stop,
	dec8_vh_screenrefresh,

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
		}
	}
};

static struct MachineDriver ghostb_machine_driver =
{
	/* basic machine hardware */
	{
		{
			CPU_M6809,  /* Really HD6309 */
			2000000,
			0,
			ghostb_readmem,ghostb_writemem,0,0,
			ghostb_interrupt,1
		},
		{
			CPU_M6502 | CPU_AUDIO_CPU,
			1250000,        /* 1.25 Mhz ? */
			2,	/* memory region #2 */
			dec8_s_readmem,dec8_s_writemem,0,0,
			ignore_interrupt,0	/* IRQs are caused by the YM3812 */
								/* NMIs are caused by the main CPU */
		}
	},
	60, DEFAULT_REAL_60HZ_VBLANK_DURATION,	/* frames per second, vblank duration */
	1,	/* 1 CPU slice per frame - interleaving is forced when a sound command is written */
	0,	/* init machine */

	/* video hardware */
  	32*8, 32*8, { 0*8, 32*8-1, 1*8, 31*8-1 },

	ghostb_gfxdecodeinfo,
	1024,1024,
	ghostb_vh_convert_color_prom,

	VIDEO_TYPE_RASTER | VIDEO_UPDATE_BEFORE_VBLANK,
	0,
	dec8_vh_start,
	dec8_vh_stop,
	ghostb_vh_screenrefresh,

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
		}
	}
};

static struct MachineDriver srdarwin_machine_driver =
{
	/* basic machine hardware */
	{
		{
			CPU_M6809,  /* MC68A09EP */
			2000000,
			0,
			srdarwin_readmem,srdarwin_writemem,0,0,
			nmi_interrupt,1
		},
		{
			CPU_M6502 | CPU_AUDIO_CPU,
			1250000,        /* 1.25 Mhz ? */
			2,	/* memory region #2 */
			dec8_s_readmem,dec8_s_writemem,0,0,
			ignore_interrupt,0	/* IRQs are caused by the YM3812 */
								/* NMIs are caused by the main CPU */
		}
	},
	60, DEFAULT_REAL_60HZ_VBLANK_DURATION,	/* frames per second, vblank duration */
	1,	/* 1 CPU slice per frame - interleaving is forced when a sound command is written */
	0,	/* init machine */

	/* video hardware */
  	32*8, 32*8, { 0*8, 32*8-1, 1*8, 31*8-1 },

	srdarwin_gfxdecodeinfo,
	144,144,
	0,

	VIDEO_TYPE_RASTER | VIDEO_MODIFIES_PALETTE | VIDEO_UPDATE_BEFORE_VBLANK,
	0,
	dec8_vh_start,
	dec8_vh_stop,
	srdarwin_vh_screenrefresh,

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
		}
	}
};

static struct MachineDriver gondo_machine_driver =
{
	/* basic machine hardware */
	{
 		{
			CPU_M6809,
			3000000,
			0,
			gondo_readmem,gondo_writemem,0,0,
			gondo_interrupt,1
		},
		{
			CPU_M6502 | CPU_AUDIO_CPU,
			1250000,        /* 1.25 Mhz ? */
			2,	/* memory region #2 */
			dec8_s_readmem,dec8_s_writemem,0,0,
			ignore_interrupt,0	/* IRQs are caused by the YM3526 */
								/* NMIs are caused by the main CPU */
		}
	},
	60, DEFAULT_REAL_60HZ_VBLANK_DURATION,	/* frames per second, vblank duration */
	1,	/* 1 CPU slice per frame - interleaving is forced when a sound command is written */
	0,	/* init machine */

	/* video hardware */
	32*8, 32*8, { 0*8, 32*8-1, 1*8, 31*8-1 },

	gondo_gfxdecodeinfo,
	1024,1024,
	0,

	VIDEO_TYPE_RASTER | VIDEO_MODIFIES_PALETTE | VIDEO_UPDATE_BEFORE_VBLANK,
	0,
	dec8_vh_start,
	dec8_vh_stop,
	gondo_vh_screenrefresh,

	/* sound hardware */
	0,0,0,0,
	{
		{
			SOUND_YM2203,
			&ym2203_interface
		},
		{
			SOUND_YM3526,
			&ym3526_interface
		}
	}
};

static struct MachineDriver oscar_machine_driver =
{
	/* basic machine hardware */
	{
	  	{
			CPU_M6809,
			2000000,
			0,
			oscar_readmem,oscar_writemem,0,0,
			oscar_interrupt,1
		},
	 	{
			CPU_M6809,
			2000000,
			3,
			oscar_sub_readmem,oscar_sub_writemem,0,0,
			ignore_interrupt,0
		},
		{
			CPU_M6502 | CPU_AUDIO_CPU,
			1250000,        /* 1.25 Mhz ? */
			2,	/* memory region #2 */
			dec8_s_readmem,dec8_s_writemem,0,0,
			ignore_interrupt,0	/* IRQs are caused by the YM3526 */
								/* NMIs are caused by the main CPU */
		}
	},
	60, DEFAULT_REAL_60HZ_VBLANK_DURATION,	/* frames per second, vblank duration */
	1,	/* 1 CPU slice per frame - interleaving is forced when a sound command is written */
	/* 100, // 100 CPU slices per frame */
	0,	/* init machine */

	/* video hardware */
  	32*8, 32*8, { 0*8, 32*8-1, 1*8, 31*8-1 },

	oscar_gfxdecodeinfo,
	512,512,
	0,

	VIDEO_TYPE_RASTER | VIDEO_MODIFIES_PALETTE | VIDEO_UPDATE_BEFORE_VBLANK,
	0,
	dec8_vh_start,
	dec8_vh_stop,
	oscar_vh_screenrefresh,

	/* sound hardware */
	0,0,0,0,
	{
		{
			SOUND_YM2203,
			&ym2203_interface
		},
		{
			SOUND_YM3526,
			&oscar_ym3526_interface
		}
	}
};

static struct MachineDriver lastmiss_machine_driver =
{
	/* basic machine hardware */
	{
  		{
			CPU_M6809,
			2000000,
			0,
			lastmiss_readmem,lastmiss_writemem,0,0,
			ignore_interrupt,0
		},
     	{
			CPU_M6809,
			2000000,
			3,
			lastmiss_sub_readmem,lastmiss_sub_writemem,0,0,
			ignore_interrupt,0
		},
		{
			CPU_M6502 | CPU_AUDIO_CPU,
			1250000,        /* 1.25 Mhz ? */
			2,	/* memory region #2 */
			ym3526_s_readmem,ym3526_s_writemem,0,0,
			ignore_interrupt,0	/* IRQs are caused by the YM3526 */
								/* NMIs are caused by the main CPU */
		}
	},
	60, DEFAULT_REAL_60HZ_VBLANK_DURATION,	/* frames per second, vblank duration */
	200,
	0,	/* init machine */

	/* video hardware */
  	32*8, 32*8, { 0*8, 32*8-1, 1*8, 31*8-1 },

	lastmiss_gfxdecodeinfo,
	1024,1024,
	0,

	VIDEO_TYPE_RASTER | VIDEO_MODIFIES_PALETTE | VIDEO_UPDATE_BEFORE_VBLANK,
	0,
	dec8_vh_start,
	dec8_vh_stop,
	lastmiss_vh_screenrefresh,

	/* sound hardware */
	0,0,0,0,
	{
		{
			SOUND_YM2203,
			&ym2203_interface
		},
		{
			SOUND_YM3526,
			&oscar_ym3526_interface
		}
	}
};

static struct MachineDriver shackled_machine_driver =
{
	/* basic machine hardware */
	{
  		{
			CPU_M6809,
			2000000,
			0,
			shackled_readmem,shackled_writemem,0,0,
		   	ignore_interrupt,0
		},
     	{
			CPU_M6809,
			2000000,
			3,
			shackled_sub_readmem,shackled_sub_writemem,0,0,
			ignore_interrupt,0
		},
		{
			CPU_M6502 | CPU_AUDIO_CPU,
			1250000,        /* 1.25 Mhz ? */
			2,	/* memory region #2 */
			ym3526_s_readmem,ym3526_s_writemem,0,0,
			ignore_interrupt,0	/* IRQs are caused by the YM3526 */
								/* NMIs are caused by the main CPU */
		}
	},
	60, DEFAULT_REAL_60HZ_VBLANK_DURATION,	/* frames per second, vblank duration */
	80,
	0,	/* init machine */

	/* video hardware */
	32*8, 32*8, { 0*8, 32*8-1, 1*8, 31*8-1 },

	lastmiss_gfxdecodeinfo,
	1024,1024,
	0,

	VIDEO_TYPE_RASTER | VIDEO_MODIFIES_PALETTE | VIDEO_UPDATE_BEFORE_VBLANK,
	0,
	dec8_vh_start,
	dec8_vh_stop,
	lastmiss_vh_screenrefresh,

	/* sound hardware */
	0,0,0,0,
	{
		{
			SOUND_YM2203,
			&ym2203_interface
		},
		{
			SOUND_YM3526,
			&oscar_ym3526_interface
		}
	}
};

static struct MachineDriver csilver_machine_driver =
{
	/* basic machine hardware */
	{
  		{
			CPU_M6809,
			2000000,
			0,
			csilver_readmem,csilver_writemem,0,0,
		   	ignore_interrupt,0
		},
     	{
			CPU_M6809,
			2000000,
			3,
			csilver_sub_readmem,csilver_sub_writemem,0,0,
			ignore_interrupt,0
		},
		{
			CPU_M6502 | CPU_AUDIO_CPU,
			1250000,        /* 1.25 Mhz ? */
			2,	/* memory region #2 */
			csilver_s_readmem,csilver_s_writemem,0,0,
			ignore_interrupt,0	/* IRQs are caused by the MSM5205 */
								/* NMIs are caused by the main CPU */
		}
	},
	60, DEFAULT_REAL_60HZ_VBLANK_DURATION,	/* frames per second, vblank duration */
	80,
	0,	/* init machine */

	/* video hardware */
	32*8, 32*8, { 0*8, 32*8-1, 1*8, 31*8-1 },

	lastmiss_gfxdecodeinfo,
	1024,1024,
	0,

	VIDEO_TYPE_RASTER | VIDEO_MODIFIES_PALETTE | VIDEO_UPDATE_BEFORE_VBLANK,
	0,
	dec8_vh_start,
	dec8_vh_stop,
	lastmiss_vh_screenrefresh,

	/* sound hardware */
	0,0,0,0,
	{
		{
			SOUND_YM2203,
			&ym2203_interface
		},
		{
			SOUND_YM3526,
			&oscar_ym3526_interface
		},
		{
			SOUND_MSM5205,
			&msm5205_interface
	    }
	}
};

/******************************************************************************/

ROM_START( cobracom_rom )
	ROM_REGION(0x30000)
 	ROM_LOAD( "eh-11.rom",    0x08000, 0x08000, 0x868637e1 )
 	ROM_LOAD( "eh-12.rom",    0x10000, 0x10000, 0x7c878a83 )
 	ROM_LOAD( "eh-13.rom",    0x20000, 0x10000, 0x04505acb )

	ROM_REGION_DISPOSE(0x188000)	/* temporary space for graphics */
	ROM_LOAD( "eh-14.rom",    0x00000, 0x08000, 0x47246177 ) /* Characters */
	ROM_LOAD( "eh-00.rom",    0x08000, 0x10000, 0xd96b6797 ) /* Sprites */
	ROM_LOAD( "eh-01.rom",    0x28000, 0x10000, 0x3fef9c02 )
	ROM_LOAD( "eh-02.rom",    0x48000, 0x10000, 0xbfae6c34 )
	ROM_LOAD( "eh-03.rom",    0x68000, 0x10000, 0xd56790f8 )
	ROM_LOAD( "eh-05.rom",    0x88000, 0x10000, 0x1c4f6033 ) /* Tiles */
	ROM_LOAD( "eh-06.rom",    0xa8000, 0x10000, 0xd24ba794 )
	ROM_LOAD( "eh-04.rom",    0xc8000, 0x10000, 0xd80a49ce )
	ROM_LOAD( "eh-07.rom",    0xe8000, 0x10000, 0x6d771fc3 )
	ROM_LOAD( "eh-08.rom",    0x108000,0x08000, 0xcb0dcf4c ) /* Tiles 2 */
	ROM_CONTINUE(             0x128000,0x8000 )
	ROM_LOAD( "eh-09.rom",    0x148000,0x08000, 0x1fae5be7 )
	ROM_CONTINUE(             0x168000,0x8000)

	ROM_REGION(0x10000)	/* 64K for sound CPU */
	ROM_LOAD( "eh-10.rom",    0x8000,  0x8000,  0x62ca5e89 )
ROM_END

ROM_START( ghostb_rom )
	ROM_REGION(0x50000)
 	ROM_LOAD( "dz-01.rom", 0x08000, 0x08000, 0x7c5bb4b1 )
 	ROM_LOAD( "dz-02.rom", 0x10000, 0x10000, 0x8e117541 )
	ROM_LOAD( "dz-03.rom", 0x20000, 0x10000, 0x5606a8f4 )
	ROM_LOAD( "dz-04.rom", 0x30000, 0x10000, 0xd09bad99 )
	ROM_LOAD( "dz-05.rom", 0x40000, 0x10000, 0x0315f691 )

	ROM_REGION_DISPOSE(0xc8000)	/* temporary space for graphics */
	ROM_LOAD( "dz-00.rom", 0x00000, 0x08000, 0x992b4f31 ) /* Characters */

	ROM_LOAD( "dz-15.rom", 0x08000, 0x10000, 0xa01a5fd9 ) /* Sprites */
	ROM_LOAD( "dz-16.rom", 0x18000, 0x10000, 0x5a9a344a )
	ROM_LOAD( "dz-12.rom", 0x28000, 0x10000, 0x817fae99 )
	ROM_LOAD( "dz-14.rom", 0x38000, 0x10000, 0x0abbf76d )
	ROM_LOAD( "dz-11.rom", 0x48000, 0x10000, 0xa5e19c24 )
	ROM_LOAD( "dz-13.rom", 0x58000, 0x10000, 0x3e7c0405 )
	ROM_LOAD( "dz-17.rom", 0x68000, 0x10000, 0x40361b8b )
	ROM_LOAD( "dz-18.rom", 0x78000, 0x10000, 0x8d219489 )

	ROM_LOAD( "dz-07.rom", 0x88000, 0x10000, 0xe7455167 ) /* Tiles */
 	ROM_LOAD( "dz-08.rom", 0x98000, 0x10000, 0x32f9ddfe )
 	ROM_LOAD( "dz-09.rom", 0xa8000, 0x10000, 0xbb6efc02 )
	ROM_LOAD( "dz-10.rom", 0xb8000, 0x10000, 0x6ef9963b )

	ROM_REGION(0x10000)	/* 64K for sound CPU */
	ROM_LOAD( "dz-06.rom", 0x8000, 0x8000, 0x798f56df )

	ROM_REGION_DISPOSE(0x800)	/* Colour proms */
	ROM_LOAD( "dz19a.10d", 0x0000, 0x0400, 0x47e1f83b )
	ROM_LOAD( "dz20a.11d", 0x0400, 0x0400, 0xd8fe2d99 )
ROM_END

ROM_START( ghostb3_rom )
	ROM_REGION(0x50000)
 	ROM_LOAD( "dz01-3b",   0x08000, 0x08000, 0xc8cc862a )
 	ROM_LOAD( "dz-02.rom", 0x10000, 0x10000, 0x8e117541 )
	ROM_LOAD( "dz-03.rom", 0x20000, 0x10000, 0x5606a8f4 )
	ROM_LOAD( "dz04-1",    0x30000, 0x10000, 0x3c3eb09f )
	ROM_LOAD( "dz05",      0x40000, 0x10000, 0xb4971d33 )

	ROM_REGION_DISPOSE(0xc8000)	/* temporary space for graphics */
	ROM_LOAD( "dz-00.rom", 0x00000, 0x08000, 0x992b4f31 )  /* Characters */

	ROM_LOAD( "dz-15.rom", 0x08000, 0x10000, 0xa01a5fd9 ) /* Sprites */
	ROM_LOAD( "dz-16.rom", 0x18000, 0x10000, 0x5a9a344a )
	ROM_LOAD( "dz-12.rom", 0x28000, 0x10000, 0x817fae99 )
	ROM_LOAD( "dz-14.rom", 0x38000, 0x10000, 0x0abbf76d )
	ROM_LOAD( "dz-11.rom", 0x48000, 0x10000, 0xa5e19c24 )
	ROM_LOAD( "dz-13.rom", 0x58000, 0x10000, 0x3e7c0405 )
	ROM_LOAD( "dz-17.rom", 0x68000, 0x10000, 0x40361b8b )
	ROM_LOAD( "dz-18.rom", 0x78000, 0x10000, 0x8d219489 )

	ROM_LOAD( "dz-07.rom", 0x88000, 0x10000, 0xe7455167 ) /* Tiles */
 	ROM_LOAD( "dz-08.rom", 0x98000, 0x10000, 0x32f9ddfe )
 	ROM_LOAD( "dz-09.rom", 0xa8000, 0x10000, 0xbb6efc02 )
	ROM_LOAD( "dz-10.rom", 0xb8000, 0x10000, 0x6ef9963b )

	ROM_REGION(0x10000)	/* 64K for sound CPU */
	ROM_LOAD( "dz-06.rom", 0x8000, 0x8000, 0x798f56df )

	ROM_REGION_DISPOSE(0x800)	/* Colour proms */
	ROM_LOAD( "dz19a.10d", 0x0000, 0x0400, 0x47e1f83b )
	ROM_LOAD( "dz20a.11d", 0x0400, 0x0400, 0xd8fe2d99 )
ROM_END

ROM_START( mazeh_rom )
	ROM_REGION(0x40000)
 	ROM_LOAD( "dw-01.rom", 0x08000, 0x08000, 0x87610c39 )
 	ROM_LOAD( "dw-02.rom", 0x10000, 0x10000, 0x40c9b0b8 )
 	ROM_LOAD( "dw-03.rom", 0x20000, 0x10000, 0x5606a8f4 )
 	ROM_LOAD( "dw-04.rom", 0x30000, 0x10000, 0x235c0c36 )

	ROM_REGION_DISPOSE(0xc8000)	/* temporary space for graphics */
	ROM_LOAD( "dw-00.rom", 0x00000, 0x8000, 0x3d25f15c ) /* Characters */

	ROM_LOAD( "dw-14.rom", 0x08000, 0x10000, 0x9b0dbfa9 ) /* Sprites */
	ROM_LOAD( "dw-15.rom", 0x18000, 0x10000, 0x95683fda )
	ROM_LOAD( "dw-11.rom", 0x28000, 0x10000, 0x1b1fcca7 )
	ROM_LOAD( "dw-13.rom", 0x38000, 0x10000, 0xe7413056 )
	ROM_LOAD( "dw-10.rom", 0x48000, 0x10000, 0x57667546 )
	ROM_LOAD( "dw-12.rom", 0x58000, 0x10000, 0x4c548db8 )
	ROM_LOAD( "dw-16.rom", 0x68000, 0x10000, 0xe5bcf927 )
	ROM_LOAD( "dw-17.rom", 0x78000, 0x10000, 0x9e10f723 )

	ROM_LOAD( "dw-06.rom", 0x88000, 0x10000, 0xb65e029d ) /* Tiles */
	ROM_LOAD( "dw-07.rom", 0x98000, 0x10000, 0x668d995d )
	ROM_LOAD( "dw-08.rom", 0xa8000, 0x10000, 0xbb2cf4a0 )
	ROM_LOAD( "dw-09.rom", 0xb8000, 0x10000, 0x6a528d13 )

	ROM_REGION(0x10000)	/* 64K for sound CPU */
	ROM_LOAD( "dw-05.rom", 0x8000, 0x8000, 0xc28c4d82 )

	ROM_REGION_DISPOSE(0x800)	/* Colour proms */
	ROM_LOAD( "dz19a.10d", 0x0000, 0x0400, 0x00000000 ) /* Not the real ones! */
	ROM_LOAD( "dz20a.11d", 0x0400, 0x0400, 0x00000000 ) /* These are from ghostbusters */
ROM_END

ROM_START( srdarwin_rom )
	ROM_REGION(0x28000)
 	ROM_LOAD( "dy_01.rom", 0x20000, 0x08000, 0x1eeee4ff )
	ROM_CONTINUE(          0x08000, 0x08000 )
 	ROM_LOAD( "dy_00.rom", 0x10000, 0x10000, 0x2bf6b461 )

	ROM_REGION_DISPOSE(0xd0000)	/* temporary space for graphics */
	ROM_LOAD( "dy_05.rom", 0x00000, 0x4000, 0x8780e8a3 ) /* Characters */

	ROM_LOAD( "dy_06.rom", 0x10000, 0x8000, 0xc279541b ) /* Sprites */
	ROM_LOAD( "dy_07.rom", 0x08000, 0x8000, 0x97eaba60 )
	ROM_LOAD( "dy_08.rom", 0x20000, 0x8000, 0x71d645fd )
	ROM_LOAD( "dy_09.rom", 0x18000, 0x8000, 0xd30d1745 )
	ROM_LOAD( "dy_10.rom", 0x30000, 0x8000, 0x88770ab8 )
	ROM_LOAD( "dy_11.rom", 0x28000, 0x8000, 0xfd9ccc5b )

	ROM_LOAD( "dy_03.rom", 0x38000, 0x4000, 0x44f2a4f9 )
	ROM_CONTINUE(0x48000,0x4000)
	ROM_CONTINUE(0x58000,0x4000)
	ROM_CONTINUE(0x68000,0x4000)

	ROM_LOAD( "dy_02.rom", 0x40000, 0x4000, 0x522d9a9e )
	ROM_CONTINUE(0x50000,0x4000)
	ROM_CONTINUE(0x60000,0x4000)
	ROM_CONTINUE(0x70000,0x4000)

	ROM_REGION(0x10000)	/* 64K for sound CPU */
	ROM_LOAD( "dy_04.rom", 0x8000, 0x8000, 0x2ae3591c )
ROM_END

ROM_START( gondo_rom )
	ROM_REGION(0x40000)
 	ROM_LOAD( "dt-00.256", 0x08000, 0x08000, 0xa8cf9118 )
 	ROM_LOAD( "dt-01.512", 0x10000, 0x10000, 0xc39bb877 )
	ROM_LOAD( "dt-02.512", 0x20000, 0x10000, 0xbb5e674b )
	ROM_LOAD( "dt-03.512", 0x30000, 0x10000, 0x99c32b13 )

	ROM_REGION_DISPOSE(0x108000)	/* temporary space for graphics */
	ROM_LOAD( "dt-14.256", 0x00000, 0x08000, 0x4bef16e1 )	/* Chars */

	ROM_LOAD( "dt-19.512", 0x08000, 0x10000, 0xda2abe4b )	/* Sprites */
	ROM_LOAD( "dt-20.256", 0x18000, 0x08000, 0x42d01002 )
	ROM_LOAD( "dt-16.512", 0x28000, 0x10000, 0xe9955d8f )
	ROM_LOAD( "dt-18.256", 0x38000, 0x08000, 0xc0c5df1c )
	ROM_LOAD( "dt-15.512", 0x48000, 0x10000, 0xa54b2eb6 )
	ROM_LOAD( "dt-17.256", 0x58000, 0x08000, 0x3bbcff0d )
	ROM_LOAD( "dt-21.512", 0x68000, 0x10000, 0x1c5f682d )
	ROM_LOAD( "dt-22.256", 0x78000, 0x08000, 0xc1876a5f )

	ROM_LOAD( "dt-08.512", 0x88000, 0x08000, 0xaec483f5 )	/* Tiles */
	ROM_CONTINUE(          0x98000, 0x08000 )
	ROM_LOAD( "dt-09.256", 0x90000, 0x08000, 0x446f0ce0 )
	ROM_LOAD( "dt-06.512", 0xa8000, 0x08000, 0x3fe1527f )
	ROM_CONTINUE(          0xb8000, 0x08000 )
	ROM_LOAD( "dt-07.256", 0xb0000, 0x08000, 0x61f9bce5 )
	ROM_LOAD( "dt-12.512", 0xc8000, 0x08000, 0x1a72ca8d )
	ROM_CONTINUE(          0xd8000, 0x08000 )
	ROM_LOAD( "dt-13.256", 0xd0000, 0x08000, 0xccb81aec )
	ROM_LOAD( "dt-10.512", 0xe8000, 0x08000, 0xcfcfc9ed )
	ROM_CONTINUE(          0xf8000, 0x08000 )
	ROM_LOAD( "dt-11.256", 0xf0000, 0x08000, 0x53e9cf17 )

	ROM_REGION(0x10000)	/* 64K for sound CPU */
	ROM_LOAD( "dt-05.256", 0x8000, 0x8000, 0xec08aa29 )
ROM_END

ROM_START( mekyosen_rom )
	ROM_REGION(0x40000)
 	ROM_LOAD( "ds00",      0x08000, 0x08000, 0x33bb16fe )
 	ROM_LOAD( "dt-01.512", 0x10000, 0x10000, 0xc39bb877 )
	ROM_LOAD( "ds02",      0x20000, 0x10000, 0x925307a4 )
	ROM_LOAD( "ds03",      0x30000, 0x10000, 0x9c0fcbf6 )

	ROM_REGION_DISPOSE(0x108000)	/* temporary space for graphics */
	ROM_LOAD( "ds14", 0x00000, 0x08000, 0x00cbe9c8 )		/* Chars */

	ROM_LOAD( "dt-19.512", 0x08000, 0x10000, 0xda2abe4b )	/* Sprites */
	ROM_LOAD( "ds20",      0x18000, 0x08000, 0x0eef7f56 )
	ROM_LOAD( "dt-16.512", 0x28000, 0x10000, 0xe9955d8f )
	ROM_LOAD( "ds18",      0x38000, 0x08000, 0x2b2d1468 )
	ROM_LOAD( "dt-15.512", 0x48000, 0x10000, 0xa54b2eb6 )
	ROM_LOAD( "ds17",      0x58000, 0x08000, 0x75ae349a )
	ROM_LOAD( "dt-21.512", 0x68000, 0x10000, 0x1c5f682d )
	ROM_LOAD( "ds22",      0x78000, 0x08000, 0xc8ffb148 )

	ROM_LOAD( "dt-08.512", 0x88000, 0x08000, 0xaec483f5 )	/* Tiles */
	ROM_CONTINUE(          0x98000, 0x08000 )
	ROM_LOAD( "dt-09.256", 0x90000, 0x08000, 0x446f0ce0 )
	ROM_LOAD( "dt-06.512", 0xa8000, 0x08000, 0x3fe1527f )
	ROM_CONTINUE(          0xb8000, 0x08000 )
	ROM_LOAD( "dt-07.256", 0xb0000, 0x08000, 0x61f9bce5 )
	ROM_LOAD( "dt-12.512", 0xc8000, 0x08000, 0x1a72ca8d )
	ROM_CONTINUE(          0xd8000, 0x08000 )
	ROM_LOAD( "dt-13.256", 0xd0000, 0x08000, 0xccb81aec )
	ROM_LOAD( "dt-10.512", 0xe8000, 0x08000, 0xcfcfc9ed )
	ROM_CONTINUE(          0xf8000, 0x08000 )
	ROM_LOAD( "dt-11.256", 0xf0000, 0x08000, 0x53e9cf17 )

	ROM_REGION(0x10000)	/* 64K for sound CPU */
	ROM_LOAD( "ds05", 0x8000, 0x8000, 0xe6e28ca9 )
ROM_END

ROM_START( oscar_rom )
	ROM_REGION(0x20000)
 	ROM_LOAD( "ed10", 0x08000, 0x08000, 0xf9b0d4d4 )
 	ROM_LOAD( "ed09", 0x10000, 0x10000, 0xe2d4bba9 )

	ROM_REGION_DISPOSE(0x108000)	/* temporary space for graphics */
	ROM_LOAD( "ed08", 0x00000, 0x04000, 0x308ac264 )	/* Characters */

	ROM_LOAD( "ed04", 0x08000, 0x10000, 0x416a791b )	/* Sprites */
	ROM_LOAD( "ed05", 0x28000, 0x10000, 0xfcdba431 )
	ROM_LOAD( "ed06", 0x48000, 0x10000, 0x7d50bebc )
	ROM_LOAD( "ed07", 0x68000, 0x10000, 0x8fdf0fa5 )

	ROM_LOAD( "ed00", 0xc8000, 0x10000, 0xac201f2d )	/* Tiles */
	ROM_LOAD( "ed01", 0x88000, 0x10000, 0xd3a58e9e )
	ROM_LOAD( "ed02", 0xe8000, 0x10000, 0x7ddc5651 )
	ROM_LOAD( "ed03", 0xa8000, 0x10000, 0x4fc4fb0f )

	ROM_REGION(0x10000)	/* 64K for sound CPU */
	ROM_LOAD( "ed12", 0x8000, 0x8000,  0x432031c5 )

	ROM_REGION(0x10000)	/* CPU 2, 1st 16k is empty */
	ROM_LOAD( "ed11", 0x0000, 0x10000,  0x10e5d919 )
ROM_END

ROM_START( oscarj_rom )
	ROM_REGION(0x20000)
 	ROM_LOAD( "du10", 0x08000, 0x08000, 0x120040d8 )
 	ROM_LOAD( "ed09", 0x10000, 0x10000, 0xe2d4bba9 )

	ROM_REGION_DISPOSE(0x108000)	/* temporary space for graphics */
	ROM_LOAD( "ed08", 0x00000, 0x04000, 0x308ac264 )	/* Characters */

	ROM_LOAD( "ed04", 0x08000, 0x10000, 0x416a791b )	/* Sprites */
	ROM_LOAD( "ed05", 0x28000, 0x10000, 0xfcdba431 )
	ROM_LOAD( "ed06", 0x48000, 0x10000, 0x7d50bebc )
	ROM_LOAD( "ed07", 0x68000, 0x10000, 0x8fdf0fa5 )

	ROM_LOAD( "ed00", 0xc8000, 0x10000, 0xac201f2d )	/* Tiles */
	ROM_LOAD( "ed01", 0x88000, 0x10000, 0xd3a58e9e )
	ROM_LOAD( "ed02", 0xe8000, 0x10000, 0x7ddc5651 )
	ROM_LOAD( "ed03", 0xa8000, 0x10000, 0x4fc4fb0f )

	ROM_REGION(0x10000)	/* 64K for sound CPU */
	ROM_LOAD( "ed12", 0x8000, 0x8000, 0x432031c5 )

	ROM_REGION(0x10000)	/* CPU 2, 1st 16k is empty */
	ROM_LOAD( "du11", 0x0000, 0x10000, 0xff45c440 )
ROM_END

ROM_START( lastmiss_rom )
	ROM_REGION(0x20000)
 	ROM_LOAD( "dl03-6",      0x08000, 0x08000, 0x47751a5e ) /* Rev 6 roms */
 	ROM_LOAD( "lm_dl04.rom", 0x10000, 0x10000, 0x7dea1552 )

	ROM_REGION_DISPOSE(0x108000)	/* temporary space for graphics */
	ROM_LOAD( "lm_dl01.rom", 0x00000, 0x8000, 0xf3787a5d )	/* Characters */

	ROM_LOAD( "lm_dl11.rom", 0x08000, 0x08000, 0x36579d3b ) /* Sprites */
	ROM_LOAD( "lm_dl12.rom", 0x28000, 0x08000, 0x2ba6737e )
	ROM_LOAD( "lm_dl13.rom", 0x48000, 0x08000, 0x39a7dc93 )
	ROM_LOAD( "lm_dl10.rom", 0x68000, 0x08000, 0xfe275ea8 )

	ROM_LOAD( "lm_dl09.rom", 0x88000, 0x10000, 0x6a5a0c5d ) /* Tiles */
	ROM_LOAD( "lm_dl08.rom", 0xa8000, 0x10000, 0x3b38cfce )
	ROM_LOAD( "lm_dl07.rom", 0xc8000, 0x10000, 0x1b60604d )
	ROM_LOAD( "lm_dl06.rom", 0xe8000, 0x10000, 0xc43c26a7 )

	ROM_REGION(0x10000)	/* 64K for sound CPU */
	ROM_LOAD( "lm_dl05.rom", 0x8000, 0x8000, 0x1a5df8c0 )

	ROM_REGION(0x10000)	/* CPU 2, 1st 16k is empty */
	ROM_LOAD( "lm_dl02.rom", 0x0000, 0x10000, 0xec9b5daf )
ROM_END

ROM_START( lastmss2_rom )
	ROM_REGION(0x20000)
 	ROM_LOAD( "lm_dl03.rom", 0x08000, 0x08000, 0x357f5f6b ) /* Rev 5 roms */
 	ROM_LOAD( "lm_dl04.rom", 0x10000, 0x10000, 0x7dea1552 )

	ROM_REGION_DISPOSE(0x108000)	/* temporary space for graphics */
	ROM_LOAD( "lm_dl01.rom", 0x00000, 0x8000, 0xf3787a5d )	/* Characters */

	ROM_LOAD( "lm_dl11.rom", 0x08000, 0x08000, 0x36579d3b ) /* Sprites */
	ROM_LOAD( "lm_dl12.rom", 0x28000, 0x08000, 0x2ba6737e )
	ROM_LOAD( "lm_dl13.rom", 0x48000, 0x08000, 0x39a7dc93 )
	ROM_LOAD( "lm_dl10.rom", 0x68000, 0x08000, 0xfe275ea8 )

	ROM_LOAD( "lm_dl09.rom", 0x88000, 0x10000, 0x6a5a0c5d )	/* Tiles */
	ROM_LOAD( "lm_dl08.rom", 0xa8000, 0x10000, 0x3b38cfce )
	ROM_LOAD( "lm_dl07.rom", 0xc8000, 0x10000, 0x1b60604d )
	ROM_LOAD( "lm_dl06.rom", 0xe8000, 0x10000, 0xc43c26a7 )

	ROM_REGION(0x10000)	/* 64K for sound CPU */
	ROM_LOAD( "lm_dl05.rom", 0x8000, 0x8000, 0x1a5df8c0 )

	ROM_REGION(0x10000)	/* CPU 2, 1st 16k is empty */
	ROM_LOAD( "lm_dl02.rom", 0x0000, 0x10000, 0xec9b5daf )
ROM_END

ROM_START( shackled_rom )
	ROM_REGION(0x48000)
 	ROM_LOAD( "dk-02.rom", 0x08000, 0x08000, 0x87f8fa85 )
	ROM_LOAD( "dk-06.rom", 0x10000, 0x10000, 0x69ad62d1 )
	ROM_LOAD( "dk-05.rom", 0x20000, 0x10000, 0x598dd128 )
	ROM_LOAD( "dk-04.rom", 0x30000, 0x10000, 0x36d305d4 )
    ROM_LOAD( "dk-03.rom", 0x40000, 0x08000, 0x6fd90fd1 )

	ROM_REGION_DISPOSE(0x108000)	/* temporary space for graphics */
	ROM_LOAD( "dk-00.rom", 0x00000, 0x08000, 0x69b975aa )	/* Characters */

	ROM_LOAD( "dk-12.rom", 0x08000, 0x10000, 0x615c2371 )	/* Sprites */
	ROM_LOAD( "dk-13.rom", 0x18000, 0x10000, 0x479aa503 )
	ROM_LOAD( "dk-14.rom", 0x28000, 0x10000, 0xcdc24246 )
	ROM_LOAD( "dk-15.rom", 0x38000, 0x10000, 0x88db811b )
	ROM_LOAD( "dk-16.rom", 0x48000, 0x10000, 0x061a76bd )
	ROM_LOAD( "dk-17.rom", 0x58000, 0x10000, 0xa6c5d8af )
	ROM_LOAD( "dk-18.rom", 0x68000, 0x10000, 0x4d466757 )
	ROM_LOAD( "dk-19.rom", 0x78000, 0x10000, 0x1911e83e )

	ROM_LOAD( "dk-11.rom", 0x88000, 0x10000, 0x5cf5719f )	/* Tiles */
	ROM_LOAD( "dk-10.rom", 0xa8000, 0x10000, 0x408e6d08 )
	ROM_LOAD( "dk-09.rom", 0xc8000, 0x10000, 0xc1557fac )
	ROM_LOAD( "dk-08.rom", 0xe8000, 0x10000, 0x5e54e9f5 )

	ROM_REGION(0x10000)	/* 64K for sound CPU */
	ROM_LOAD( "dk-07.rom", 0x08000, 0x08000, 0x887e4bcc )

	ROM_REGION(0x10000)	/* CPU 2, 1st 16k is empty */
	ROM_LOAD( "dk-01.rom", 0x00000, 0x10000, 0x71fe3bda )
ROM_END

ROM_START( breywood_rom )
	ROM_REGION(0x48000)
 	ROM_LOAD( "7.bin", 0x08000, 0x08000, 0xc19856b9 )
   	ROM_LOAD( "3.bin", 0x10000, 0x10000, 0x2860ea02 )
	ROM_LOAD( "4.bin", 0x20000, 0x10000, 0x0fdd915e )
	ROM_LOAD( "5.bin", 0x30000, 0x10000, 0x71036579 )
    ROM_LOAD( "6.bin", 0x40000, 0x08000, 0x308f4893 )

	ROM_REGION_DISPOSE(0x108000)	/* temporary space for graphics */
	ROM_LOAD( "1.bin",  0x00000, 0x8000, 0x815a891a )	/* Characters */

	ROM_LOAD( "dk-12.rom", 0x08000, 0x10000, 0x0 )	/* Shackled sprites */
	ROM_LOAD( "dk-13.rom", 0x18000, 0x10000, 0x0 )
	ROM_LOAD( "dk-14.rom", 0x28000, 0x10000, 0x0 )
	ROM_LOAD( "dk-15.rom", 0x38000, 0x10000, 0x0 )
	ROM_LOAD( "dk-16.rom", 0x48000, 0x10000, 0x0 )
	ROM_LOAD( "dk-17.rom", 0x58000, 0x10000, 0x0 )
	ROM_LOAD( "dk-18.rom", 0x68000, 0x10000, 0x0 )
	ROM_LOAD( "dk-19.rom", 0x78000, 0x10000, 0x0 )

	ROM_LOAD( "9.bin",  0x88000, 0x10000, 0x067e2a43 )	/* Tiles */
	ROM_LOAD( "10.bin", 0xa8000, 0x10000, 0xc19733aa )
	ROM_LOAD( "11.bin", 0xc8000, 0x10000, 0xe37d5dbe )
	ROM_LOAD( "12.bin", 0xe8000, 0x10000, 0xbeee880f )

	ROM_REGION(0x10000)	/* 64K for sound CPU */
	ROM_LOAD( "2.bin", 0x8000, 0x8000,  0x4a471c38 )

	ROM_REGION(0x10000)	/* CPU 2, 1st 16k is empty */
	ROM_LOAD( "8.bin", 0x0000, 0x10000,  0x3d9fb623 )
ROM_END

ROM_START( csilver_rom )
	ROM_REGION(0x48000)
 	ROM_LOAD( "a4", 0x08000, 0x08000, 0x02dd8cfc )
   	ROM_LOAD( "a2", 0x10000, 0x10000, 0x570fb50c )
	ROM_LOAD( "a3", 0x20000, 0x10000, 0x58625890 )

	ROM_REGION_DISPOSE(0x108000)	/* temporary space for graphics */
	ROM_LOAD( "a1",  0x00000, 0x8000, 0xf01ef985 )	/* Characters */

	ROM_LOAD( "b5",  0x08000, 0x10000, 0x80f07915 )	/* Sprites (3bpp) */
	ROM_LOAD( "b4",  0x28000, 0x10000, 0xd32c02e7 )
	ROM_LOAD( "b3",  0x48000, 0x10000, 0xac78b76b )

	ROM_LOAD( "a7",  0x88000, 0x10000, 0xb6fb208c )	/* Tiles (3bpp) */
	ROM_LOAD( "a8",  0x98000, 0x10000, 0xee3e1817 )
	ROM_LOAD( "a9",  0xa8000, 0x10000, 0x705900fe )
	ROM_LOAD( "a10", 0xb8000, 0x10000, 0x3192571d )
	ROM_LOAD( "b1",  0xc8000, 0x10000, 0x3ef77a32 )
	ROM_LOAD( "b2",  0xd8000, 0x10000, 0x9cf3d5b8 )

	ROM_REGION(0x18000)	/* 64K for sound CPU */
	ROM_LOAD( "a6", 0x10000, 0x08000,  0xeb32cf25 )
	ROM_CONTINUE(   0x08000, 0x08000 )

	ROM_REGION(0x10000)	/* CPU 2, 1st 16k is empty */
	ROM_LOAD( "a5", 0x0000, 0x10000,  0x29432691 )
ROM_END

/******************************************************************************/

/* Ghostbusters, Darwin, Oscar use a "Deco 222" custom 6502 for sound. */
static void deco222_decode(void)
{
	int A,sound_cpu;
	unsigned char *RAM;
	extern int encrypted_cpu;

	sound_cpu = 1;
	/* Oscar has three CPUs */
	if (Machine->drv->cpu[2].cpu_type != 0) sound_cpu = 2;

	/* bits 5 and 6 of the opcodes are swapped */
	RAM = Machine->memory_region[Machine->drv->cpu[sound_cpu].memory_region];
	encrypted_cpu = sound_cpu;
	for (A = 0;A < 0x10000;A++)
		ROM[A] = (RAM[A] & 0x9f) | ((RAM[A] & 0x20) << 1) | ((RAM[A] & 0x40) >> 1);
}

static void mazeh_patch(void)
{
	/* Blank out garbage in colour prom to avoid colour overflow */
	unsigned char *RAM = Machine->memory_region[3];
	fast_memset(RAM+0x20,0,0xe0);
}

static void ghostb_decode(void)
{
	deco222_decode();
	mazeh_patch();
}

/******************************************************************************/

static int cobracom_hiload(void)
{
	void *f;
	unsigned char *RAM = Machine->memory_region[0];

	/* check if the hi score table has already been initialized */
	if (memcmp(&RAM[0x06c6],"\x00\x84\x76",3) == 0)
	{
		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
			osd_fread(f,&RAM[0x06c6],30);
			osd_fclose(f);

			/* copy the high score to the work RAM as well */
            RAM[0x0135] = RAM[0x06c6];
            RAM[0x0136] = RAM[0x06c7];
            RAM[0x0137] = RAM[0x06c8];
		}
		return 1;
	}
	else return 0;  /* we can't load the hi scores yet */
}

static void cobracom_hisave(void)
{
	void *f;
	unsigned char *RAM = Machine->memory_region[0];

	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
		osd_fwrite(f,&RAM[0x06c6],30);
		osd_fclose(f);
	}
}

/* Maze Hunter and Real Ghost Busters high score save - DW (Jan 22, 1999) */
static int ghostb_hiload(void)
{
	void *f;
	unsigned char *RAM = Machine->memory_region[0];

	/* check if the hi score table has already been initialized */
	if (memcmp(&RAM[0x01C0],"\x01\x1F\x0F",3) == 0)
	{
		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
			osd_fread(f,&RAM[0x01C0],118);
			osd_fclose(f);
		}
		return 1;
	}
	else return 0;  /* we can't load the hi scores yet */
}

static void ghostb_hisave(void)
{
	void *f;
	unsigned char *RAM = Machine->memory_region[0];

	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
		osd_fwrite(f,&RAM[0x01C0],118);
		osd_fclose(f);
	}
}

static int mazeh_hiload(void)
{
	void *f;
	unsigned char *RAM = Machine->memory_region[0];

	/* check if the hi score table has already been initialized */
	if (memcmp(&RAM[0x0190],"\x01\x1F\x0F",3) == 0)
	{
		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
			osd_fread(f,&RAM[0x0190],80);
			osd_fclose(f);
		}
		return 1;
	}
	else return 0;  /* we can't load the hi scores yet */
}

static void mazeh_hisave(void)
{
	void *f;
	unsigned char *RAM = Machine->memory_region[0];

	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
		osd_fwrite(f,&RAM[0x0190],80);
		osd_fclose(f);
	}
}

static int ghostb3_hiload(void)
{
	void *f;
	unsigned char *RAM = Machine->memory_region[0];

	/* check if the hi score table has already been initialized */
	if (memcmp(&RAM[0x0DA0],"\x01\x1F\x0F",3) == 0)
	{
		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
			osd_fread(f,&RAM[0x0DA0],118);
			osd_fclose(f);
		}
		return 1;
	}
	else return 0;  /* we can't load the hi scores yet */
}

static void ghostb3_hisave(void)
{
	void *f;
	unsigned char *RAM = Machine->memory_region[0];

	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
		osd_fwrite(f,&RAM[0x0DA0],118);
		osd_fclose(f);
	}
}

/* Oscar US/JPN High score save - DW (Jan 22, 1999) */
static int oscar_hiload(void)
{
	void *f;

	/* check if the hi score table has already been initialized */
	if (memcmp(&dec8_shared_ram[0x075A],"\x00\x68\x09",3) == 0)
	{
		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
			osd_fread(f,&dec8_shared_ram[0x075A],70);
			osd_fclose(f);
		}
		return 1;
	}
	else return 0;  /* we can't load the hi scores yet */
}

static void oscar_hisave(void)
{
	void *f;

	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
		osd_fwrite(f,&dec8_shared_ram[0x075A],70);
		osd_fclose(f);
	}
}

/******************************************************************************/

struct GameDriver cobracom_driver =
{
	__FILE__,
	0,
	"cobracom",
	"Cobra Command",
	"1988",
	"Data East Corporation",
	"Bryan McPhail",
	0,
	&cobra_machine_driver,
	0,

	cobracom_rom,
	0, 0,
	0,
	0,

	input_ports,

	0, 0, 0,
	ORIENTATION_DEFAULT,

	cobracom_hiload, cobracom_hisave
};

struct GameDriver ghostb_driver =
{
	__FILE__,
	0,
	"ghostb",
	"The Real Ghostbusters (2 player)",
	"1987",
	"Data East USA",
	"Bryan McPhail",
	0,
	&ghostb_machine_driver,
	0,

	ghostb_rom,
	0, ghostb_decode,
	0,
	0,

	ghostb_input_ports,

	PROM_MEMORY_REGION(3), 0, 0,
	ORIENTATION_DEFAULT,

	ghostb_hiload, ghostb_hisave
};

struct GameDriver ghostb3_driver =
{
	__FILE__,
	&ghostb_driver,
	"ghostb3",
	"The Real Ghostbusters (3 player)",
	"1987",
	"Data East USA",
	"Bryan McPhail",
	0,
	&ghostb_machine_driver,
	0,

	ghostb3_rom,
	0, ghostb_decode,
	0,
	0,

	ghostb_input_ports,

	PROM_MEMORY_REGION(3), 0, 0,
	ORIENTATION_DEFAULT,

	ghostb3_hiload, ghostb3_hisave
};

struct GameDriver mazeh_driver =
{
	__FILE__,
	&ghostb_driver,
	"mazeh",
	"Maze Hunter",
	"1987",
	"Data East Corporation",
	"Bryan McPhail",
	0,
	&ghostb_machine_driver,
	0,

	mazeh_rom,
	mazeh_patch, 0,
	0,
	0,

	ghostb_input_ports,

	PROM_MEMORY_REGION(3), 0, 0,
	ORIENTATION_DEFAULT,

	mazeh_hiload, mazeh_hisave
};

struct GameDriver srdarwin_driver =
{
	__FILE__,
	0,
	"srdarwin",
	"Super Real Darwin",
	"1987",
	"Data East Corporation",
	"Bryan McPhail",
	GAME_IMPERFECT_COLORS,
	&srdarwin_machine_driver,
	0,

	srdarwin_rom,
	0, deco222_decode,
	0,
	0,

	darwin_input_ports,

	0, 0, 0,
	ORIENTATION_ROTATE_270,

	0,0
};

struct GameDriver gondo_driver =
{
	__FILE__,
	0,
	"gondo",
	"Gondomania",
	"1987",
	"Data East USA",
	"Bryan McPhail",
	0,
	&gondo_machine_driver,
	0,

	gondo_rom,
	0, 0,
	0,
	0,

	gondo_input_ports,

	0, 0, 0,
	ORIENTATION_ROTATE_270,

	0,0
};

struct GameDriver mekyosen_driver =
{
	__FILE__,
	&gondo_driver,
	"mekyosen",
	"Mekyo Sensi",
	"1987",
	"Data East Corporation",
	"Bryan McPhail",
	0,
	&gondo_machine_driver,
	0,

	mekyosen_rom,
	0, 0,
	0,
	0,

	gondo_input_ports,

	0, 0, 0,
	ORIENTATION_ROTATE_270,

	0,0
};

struct GameDriver oscar_driver =
{
	__FILE__,
	0,
	"oscar",
	"Psycho-Nics Oscar",
	"1988",
	"Data East USA",
	"Bryan McPhail",
	0,
	&oscar_machine_driver,
	0,

	oscar_rom,
	0, deco222_decode,
	0,
	0,

	oscar_input_ports,

	0, 0, 0,
	ORIENTATION_DEFAULT,

	oscar_hiload, oscar_hisave
};

struct GameDriver oscarj_driver =
{
	__FILE__,
	&oscar_driver,
	"oscarj",
	"Psycho-Nics Oscar (Japan)",
	"1987",
	"Data East Corporation",
	"Bryan McPhail",
	0,
	&oscar_machine_driver,
	0,

	oscarj_rom,
	0, deco222_decode,
	0,
	0,

	oscar_input_ports,

	0, 0, 0,
	ORIENTATION_DEFAULT,

	oscar_hiload, oscar_hisave
};

struct GameDriver lastmiss_driver =
{
	__FILE__,
	0,
	"lastmiss",
	"Last Mission (Rev 6)",
	"1986",
	"Data East USA",
	"Bryan McPhail",
	0,
	&lastmiss_machine_driver,
	0,

	lastmiss_rom,
	0, 0,
	0,
	0,

	lastmiss_input_ports,

	0, 0, 0,
	ORIENTATION_ROTATE_270,

	0,0
};

struct GameDriver lastmss2_driver =
{
	__FILE__,
	&lastmiss_driver,
	"lastmss2",
	"Last Mission (Rev 5)",
	"1986",
	"Data East USA",
	"Bryan McPhail",
	0,
	&lastmiss_machine_driver,
	0,

	lastmss2_rom,
	0, 0,
	0,
	0,

	lastmiss_input_ports,

	0, 0, 0,
	ORIENTATION_ROTATE_270,

	0,0
};

struct GameDriver shackled_driver =
{
	__FILE__,
	0,
	"shackled",
	"Shackled",
	"1986",
	"Data East USA",
	"Bryan McPhail",
	0,
	&shackled_machine_driver,
	0,

	shackled_rom,
	0, 0,
	0,
	0,

	shackled_input_ports,

	0, 0, 0,
	ORIENTATION_DEFAULT,

	0,0
};

struct GameDriver breywood_driver =
{
	__FILE__,
	&shackled_driver,
	"breywood",
	"Breywood",
	"1986",
	"Data East Corporation",
	"Bryan McPhail",
	0,
	&shackled_machine_driver,
	0,

	breywood_rom,
	0, 0,
	0,
	0,

	shackled_input_ports,

	0, 0, 0,
	ORIENTATION_DEFAULT,

	0,0
};

struct GameDriver csilver_driver =
{
	__FILE__,
	0,
	"csilver",
	"Captain Silver",
	"1987",
	"Data East Corporation",
	"Bryan McPhail",
	0,
	&csilver_machine_driver,
	0,

	csilver_rom,
	0, 0,
	0,
	0,

	csilver_input_ports,

	0, 0, 0,
	ORIENTATION_DEFAULT,

	0,0
};
