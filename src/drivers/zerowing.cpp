/***************************************************************************
 Zero Wing (ToaPlan)
***************************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"
#include "m68000/m68000.h"
#include "z80/z80.h"

/* change this to gp2x_malloc */

int vblank_r(int offset);
int framedone_r(int offset);
void framedone_w(int offset, int data);

void video_ofs_w(int offset, int data);
void video_ofs3_w(int offset, int data);
int video_ofs_r(int offset);
int video_ofs3_r(int offset);

int scrollregs_r(int offset);
void scrollregs_w(int offset, int data);

void zerowing_videoram1_w(int offset, int data);
void zerowing_videoram2_w(int offset, int data);
void zerowing_videoram3_w(int offset, int data);

int zerowing_videoram1_r(int offset);
int zerowing_videoram2_r(int offset);
int zerowing_videoram3_r(int offset);

void zerowing_colorram1_w(int offset, int data);
int zerowing_colorram1_r(int offset);
void zerowing_colorram2_w(int offset, int data);
int zerowing_colorram2_r(int offset);

void zerowing_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh);
void vimana_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh);
void outzone_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh);

void zerowing_vh_stop(void);
int zerowing_vh_start(void);

int unk;
int int_enable;
int credits;
int latch;

extern int vblank ;
extern int framedone  ;

extern int video_ofs  ;
extern int video_ofs3 ;

unsigned char *shared_ram;

extern unsigned char *zerowing_colorram1;
extern unsigned char *zerowing_colorram2;
extern int colorram1_size;
extern int colorram2_size;

static int zerowing_interrupt(void)
{
	if ( int_enable )
		return MC68000_IRQ_4;
	return MC68000_INT_NONE;
}

static int unk_r(int offset)
{
	return unk ^= 1;
}

static int vm_input_port_4_r(int offset)
{
	int data, p ;

	p = input_port_4_r(0) ;

	latch ^= p ;
	data = (latch & p ) ;

	/* simulate the mcu keeping track of credits */
	/* latch is so is doesn't add more than one */
	/* credit per keypress */

	if ( data & 0x18 )
		{
		credits++ ;
		}

	latch = p ;

	return p ;
}

static int vm_mcu_r(int offset)
{
	int data = 0 ;
	switch ( offset >> 1 )
		{
		case 0:
			data = 0xff ;
			break ;
		case 1:
			data = 0 ;
			break ;
		case 2:
			data = credits ;
			break ;
		}
	return data ;
}

static void vm_mcu_w(int offset, int data)
{
	switch ( offset >> 1 )
		{
		case 0:
			break ;
		case 1:
			break ;
		case 2:
			credits = data ;
			break ;
		}
}

static int shared_r(int offset)
{
	return shared_ram[offset>>1] ;
}

static void shared_w(int offset, int data)
{
	shared_ram[offset>>1] = data ;
}

static void int_enable_w(int offset, int data )
{
	int_enable = data ;
}

static void toaplan_init_machine (void)
{
	unk = 0;
	int_enable = 0;
	credits = 0;
	latch = 0;
}

/* Sound Routines */

static void zw_opl_w(int offset, int data)
{
	if ( (offset & 0xfe) == 0xa8 )
		{
		if ( offset == 0xa8 )
			{
			YM3812_control_port_0_w( 0, data ) ;
			}
		else
			{
			YM3812_write_port_0_w(0, data);
			}
		}
}

static void hf_opl_w(int offset, int data)
{
	if ( (offset & 0xfe) == 0x70 )
		{
		if ( offset == 0x70 )
			{
			YM3812_control_port_0_w( 0, data ) ;
			}
		else
			{
			YM3812_write_port_0_w(0, data);
			}
		}
}

static void truxton_opl_w(int offset, int data)
{
	if ( (offset & 0xfe) == 0x60 )
		{
		if ( offset == 0x60 )
			{
			YM3812_control_port_0_w( 0, data ) ;
			}
		else
			{
			YM3812_write_port_0_w(0, data);
			}
		}
}

static void outzone_opl_w(int offset, int data)
{
	if ( (offset & 0xfe) == 0x00 )
		{
		if ( offset == 0x00 )
			{
			YM3812_control_port_0_w( 0, data ) ;
			}
		else
			{
			YM3812_write_port_0_w(0, data);
			}
		}
}

static struct MemoryReadAddress vm_readmem[] =
{
	{ 0x000000, 0x03ffff, MRA_ROM },
	{ 0x0c0000, 0x0c0001, input_port_6_r },
	{ 0x0c0002, 0x0c0003, video_ofs_r },
	{ 0x0c0004, 0x0c0005, zerowing_videoram1_r },		/* sprites info */
	{ 0x0c0006, 0x0c0007, zerowing_videoram2_r },	/* sprite size ? */
	{ 0x400000, 0x400001, vblank_r },
	{ 0x404000, 0x4047ff, zerowing_colorram1_r },
	{ 0x406000, 0x4067ff, zerowing_colorram2_r },
	{ 0x440000, 0x440005, vm_mcu_r },
	{ 0x440006, 0x440007, input_port_2_r },
	{ 0x440008, 0x440009, vm_input_port_4_r },
	{ 0x44000a, 0x44000b, input_port_0_r },
	{ 0x44000c, 0x44000d, input_port_1_r },
	{ 0x44000e, 0x44000f, input_port_3_r },
	{ 0x440010, 0x440011, input_port_5_r },
	{ 0x480000, 0x480147, MRA_BANK1 },
	{ 0x480148, 0x487fff, MRA_BANK2 },
	{ 0x4c0000, 0x4c0001, unk_r },
	{ 0x4c0002, 0x4c0003, video_ofs3_r },
	{ 0x4c0004, 0x4c0007, zerowing_videoram3_r },	/* tile layers */
	{ 0x4c0010, 0x4c001f, scrollregs_r },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress vm_writemem[] =
{
	{ 0x000000, 0x03ffff, MWA_ROM },
	{ 0x0c0002, 0x0c0003, video_ofs_w },
	{ 0x0c0004, 0x0c0005, zerowing_videoram1_w },		/* sprites info */
	{ 0x0c0006, 0x0c0007, zerowing_videoram2_w },	/* sprite size ? */
	{ 0x400000, 0x400001, int_enable_w },
	{ 0x400002, 0x400003, MWA_NOP }, /* IRQACK? */
	{ 0x404000, 0x4047ff, zerowing_colorram1_w, &zerowing_colorram1, &colorram1_size },
	{ 0x406000, 0x4067ff, zerowing_colorram2_w, &zerowing_colorram2, &colorram2_size },
	{ 0x440000, 0x440005, vm_mcu_w },
	{ 0x480000, 0x480147, MWA_BANK1 },
	{ 0x480148, 0x487fff, MWA_BANK2 },
	{ 0x4c0002, 0x4c0003, video_ofs3_w },
	{ 0x4c0004, 0x4c0007, zerowing_videoram3_w },	/* tile layers */
	{ 0x4c0010, 0x4c001f, scrollregs_w },
	{ -1 }  /* end of table */
};

static struct MemoryReadAddress zw_readmem[] =
{
	{ 0x000000, 0x07ffff, MRA_ROM },
	{ 0x080000, 0x08176d, MRA_BANK1 },
	{ 0x08176e, 0x08176f, framedone_r },
	{ 0x081770, 0x087fff, MRA_BANK2 },
	{ 0x400000, 0x400005, unk_r },
	{ 0x404000, 0x4047ff, zerowing_colorram1_r },
	{ 0x406000, 0x4067ff, zerowing_colorram2_r },
	{ 0x440000, 0x440fff, shared_r },
	{ 0x480002, 0x480003, video_ofs3_r },
	{ 0x480004, 0x480007, zerowing_videoram3_r },	/* tile layers */
	{ 0x480010, 0x48001f, scrollregs_r },
	{ 0x4c0000, 0x4c0001, vblank_r },
	{ 0x4c0002, 0x4c0003, video_ofs_r },
	{ 0x4c0004, 0x4c0005, zerowing_videoram1_r },	/* sprites info */
	{ 0x4c0006, 0x4c0007, zerowing_videoram2_r },	/* sprite size ? */
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress zw_writemem[] =
{
	{ 0x000000, 0x07ffff, MWA_ROM },
	{ 0x080000, 0x08176d, MWA_BANK1 },
	{ 0x08176e, 0x08176f, framedone_w },
	{ 0x081770, 0x087fff, MWA_BANK2 },
	{ 0x400002, 0x400003, int_enable_w },
	{ 0x404000, 0x4047ff, zerowing_colorram1_w, &zerowing_colorram1, &colorram1_size },
	{ 0x406000, 0x4067ff, zerowing_colorram2_w, &zerowing_colorram2, &colorram2_size },
	{ 0x440000, 0x440fff, shared_w, &shared_ram },
	{ 0x480002, 0x480003, video_ofs3_w },
	{ 0x480004, 0x480007, zerowing_videoram3_w },	/* tile layers */
	{ 0x480010, 0x48001f, scrollregs_w },
	{ 0x4c0002, 0x4c0003, video_ofs_w },
	{ 0x4c0004, 0x4c0005, zerowing_videoram1_w },	/* sprites info */
	{ 0x4c0006, 0x4c0007, zerowing_videoram2_w },	/* sprite size ? */
	{ -1 }  /* end of table */
};

static struct MemoryReadAddress hf_readmem[] =
{
	{ 0x000000, 0x03ffff, MRA_ROM },
	{ 0x040000, 0x0422f1, MRA_BANK1 },
	{ 0x0422f2, 0x0422f3, framedone_r },
	{ 0x0422f4, 0x047fff, MRA_BANK2 },
	{ 0x084000, 0x0847ff, zerowing_colorram1_r },
	{ 0x086000, 0x0867ff, zerowing_colorram2_r },
	{ 0x0c0000, 0x0c0fff, shared_r },
	{ 0x100002, 0x100003, video_ofs3_r },
	{ 0x100004, 0x100007, zerowing_videoram3_r },	/* tile layers */
	{ 0x100010, 0x10001f, scrollregs_r },
	{ 0x140000, 0x140001, vblank_r },
	{ 0x140002, 0x140003, video_ofs_r },
	{ 0x140004, 0x140005, zerowing_videoram1_r },	/* sprites info */
	{ 0x140006, 0x140007, zerowing_videoram2_r },	/* sprite size ? */
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress hf_writemem[] =
{
	{ 0x000000, 0x03ffff, MWA_ROM },
	{ 0x040000, 0x0422f1, MWA_BANK1 },
	{ 0x0422f2, 0x0422f3, framedone_w },
	{ 0x0422f4, 0x047fff, MWA_BANK2 },
	{ 0x080002, 0x080003, int_enable_w },
	{ 0x084000, 0x0847ff, zerowing_colorram1_w, &zerowing_colorram1, &colorram1_size },
	{ 0x086000, 0x0867ff, zerowing_colorram2_w, &zerowing_colorram2, &colorram2_size },
	{ 0x0c0000, 0x0c0fff, shared_w, &shared_ram },
	{ 0x100002, 0x100003, video_ofs3_w },
	{ 0x100004, 0x100007, zerowing_videoram3_w },	/* tile layers */
	{ 0x100010, 0x10001f, scrollregs_w },
	{ 0x140002, 0x140003, video_ofs_w },
	{ 0x140004, 0x140005, zerowing_videoram1_w },	/* sprites info */
	{ 0x140006, 0x140007, zerowing_videoram2_w },	/* sprite size ? */
	{ -1 }  /* end of table */
};

static struct MemoryReadAddress truxton_readmem[] =
{
	{ 0x000000, 0x07ffff, MRA_ROM },
	{ 0x080000, 0x0819d5, MRA_BANK1 },
	{ 0x0819d6, 0x0819d7, framedone_r },
	{ 0x0819d8, 0x083fff, MRA_BANK1 },
	{ 0x0c0000, 0x0c0001, input_port_6_r },
	{ 0x0c0002, 0x0c0003, video_ofs_r },
	{ 0x0c0004, 0x0c0005, zerowing_videoram1_r },		/* sprites info */
	{ 0x0c0006, 0x0c0007, zerowing_videoram2_r },	/* sprite size ? */
	{ 0x100002, 0x100003, video_ofs3_r },
	{ 0x100004, 0x100007, zerowing_videoram3_r },	/* tile layers */
	{ 0x100010, 0x10001f, scrollregs_r },
	{ 0x144000, 0x1447ff, zerowing_colorram1_r },
	{ 0x146000, 0x1467ff, zerowing_colorram2_r },
	{ 0x180000, 0x180fff, shared_r },
   { -1 }
};

static struct MemoryWriteAddress truxton_writemem[] =
{
	{ 0x000000, 0x07ffff, MWA_ROM },
	{ 0x080000, 0x0819d5, MWA_BANK1 },
	{ 0x0819d6, 0x0819d7, framedone_w },
	{ 0x0819d8, 0x083fff, MWA_BANK2 },
	{ 0x0c0002, 0x0c0003, video_ofs_w },
	{ 0x0c0004, 0x0c0005, zerowing_videoram1_w },		/* sprites info */
	{ 0x0c0006, 0x0c0007, zerowing_videoram2_w },	/* sprite size ? */
	{ 0x100002, 0x100003, video_ofs3_w },
	{ 0x100004, 0x100007, zerowing_videoram3_w },	/* tile layers */
	{ 0x100010, 0x10001f, scrollregs_w },
	{ 0x140000, 0x140001, int_enable_w },
	{ 0x144000, 0x1447ff, zerowing_colorram1_w, &zerowing_colorram1, &colorram1_size },
	{ 0x146000, 0x1467ff, zerowing_colorram2_w, &zerowing_colorram2, &colorram2_size },
	{ 0x180000, 0x180fff, shared_w, &shared_ram },
   { -1 }
};

static struct MemoryReadAddress outzone_readmem[] =
{
	{ 0x000000, 0x07ffff, MRA_ROM },
	{ 0x100000, 0x100001, input_port_6_r },
	{ 0x100002, 0x100003, video_ofs_r },
	{ 0x100004, 0x100005, zerowing_videoram1_r },		/* sprites info */
	{ 0x100006, 0x100007, zerowing_videoram2_r },
	{ 0x140000, 0x140fff, shared_r },
	{ 0x200002, 0x200003, video_ofs3_r },
	{ 0x200004, 0x200007, zerowing_videoram3_r },	/* tile layers */
	{ 0x200010, 0x20001f, scrollregs_r },
	{ 0x240000, 0x243fff, MRA_BANK1 },
	{ 0x300000, 0x300001, vblank_r },
	{ 0x304000, 0x3047ff, zerowing_colorram1_r },
	{ 0x306000, 0x3067ff, zerowing_colorram2_r },
   { -1 }
};

static struct MemoryWriteAddress outzone_writemem[] =
{
	{ 0x000000, 0x07ffff, MWA_ROM },
	{ 0x100002, 0x100003, video_ofs_w },
	{ 0x100004, 0x100005, zerowing_videoram1_w },		/* sprites info */
	{ 0x100006, 0x100007, zerowing_videoram2_w },
	{ 0x140000, 0x140fff, shared_w, &shared_ram },
	{ 0x200002, 0x200003, video_ofs3_w },
	{ 0x200004, 0x200007, zerowing_videoram3_w },	/* tile layers */
	{ 0x200010, 0x20001f, scrollregs_w },
	{ 0x240000, 0x243fff, MWA_BANK1 },
	{ 0x300000, 0x300001, int_enable_w },
	{ 0x304000, 0x3047ff, zerowing_colorram1_w, &zerowing_colorram1, &colorram1_size },
	{ 0x306000, 0x3067ff, zerowing_colorram2_w, &zerowing_colorram2, &colorram2_size },
   { -1 }
};

static struct MemoryReadAddress sound_readmem[] =
{
	{ 0x0000, 0x7fff, MRA_ROM },
	{ 0x8000, 0xffff, MRA_RAM },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress sound_writemem[] =
{
	{ 0x0000, 0x7fff, MWA_ROM },
	{ 0x8000, 0xffff, MWA_RAM, &shared_ram },
	{ -1 }  /* end of table */
};

static struct IOReadPort zw_sound_readport[] =
{
	{ 0x00, 0x00, input_port_0_r }, /* player 1 */
	{ 0x08, 0x08, input_port_1_r }, /* player 2 */
	{ 0x20, 0x20, input_port_2_r }, /* DSW1 */
	{ 0x28, 0x28, input_port_3_r }, /* DSW2 */
	{ 0x80, 0x80, input_port_4_r }, /* DSWX */
	{ 0x88, 0x88, input_port_5_r }, /* DSW3 */
	{ 0xa8, 0xa8, YM3812_status_port_0_r },
	{ -1 }	/* end of table */
};

static struct IOWritePort zw_sound_writeport[] =
{
	{ 0x00, 0xff, zw_opl_w },
	{ -1 }	/* end of table */
};

static struct IOReadPort hf_sound_readport[] =
{
	{ 0x00, 0x00, input_port_2_r }, /* DSW1 */
	{ 0x10, 0x10, input_port_3_r }, /* DSW2 */
	{ 0x40, 0x40, input_port_0_r }, /* player 1 */
	{ 0x50, 0x50, input_port_1_r }, /* player 2 */
	{ 0x60, 0x60, input_port_4_r }, /* DSWX */
	{ 0x20, 0x20, input_port_5_r }, /* DSW3 */
	{ 0x70, 0x70, YM3812_status_port_0_r },
	{ -1 }	/* end of table */
};

static struct IOWritePort hf_sound_writeport[] =
{
	{ 0x00, 0xff, hf_opl_w },
	{ -1 }	/* end of table */
};

static struct IOReadPort outzone_sound_readport[] =
{
	{ 0x08, 0x08, input_port_2_r },
	{ 0x0c, 0x0c, input_port_3_r },
	{ 0x10, 0x10, input_port_4_r },
	{ 0x14, 0x14, input_port_0_r },
	{ 0x18, 0x18, input_port_1_r },
	{ 0x1c, 0x1c, input_port_5_r },
	{ 0x00, 0x00, YM3812_status_port_0_r },
	{ -1 }	/* end of table */
};

static struct IOWritePort outzone_sound_writeport[] =
{
	{ 0x00, 0xff, outzone_opl_w },
	{ -1 }	/* end of table */
};

static struct IOReadPort truxton_sound_readport[] =
{
	{ 0x00, 0x00, input_port_2_r }, /* DSW1 */
	{ 0x10, 0x10, input_port_3_r }, /* DSW2 */
	{ 0x40, 0x40, input_port_0_r }, /* player 1 */
	{ 0x50, 0x50, input_port_1_r }, /* player 2 */
	{ 0x70, 0x70, input_port_4_r }, /* DSWX */
	{ 0x20, 0x20, input_port_5_r }, /* DSW3 */
	{ 0x60, 0x60, YM3812_status_port_0_r },
	{ -1 }	/* end of table */
};

static struct IOWritePort truxton_sound_writeport[] =
{
	{ 0x00, 0xff, truxton_opl_w },
	{ -1 }	/* end of table */
};

INPUT_PORTS_START( outzone_input_ports )
	PORT_START      /* (0) PLAYER 1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    | IPF_PLAYER1 | IPF_8WAY )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  | IPF_PLAYER1 | IPF_8WAY )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  | IPF_PLAYER1 | IPF_8WAY )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT | IPF_PLAYER1 | IPF_8WAY )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 | IPF_PLAYER1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START      /* (1) PLAYER 2 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    | IPF_PLAYER2 | IPF_8WAY )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  | IPF_PLAYER2 | IPF_8WAY )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  | IPF_PLAYER2 | IPF_8WAY )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT | IPF_PLAYER2 | IPF_8WAY )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 | IPF_PLAYER1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START	/* (2) DSWA */
	PORT_DIPNAME( 0x01, 0x00, "Unused", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x01, "On" )
	PORT_DIPNAME( 0x02, 0x00, "Flip_Screen", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x02, "On" )
	PORT_BITX(    0x04, 0x00, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service_Mode", OSD_KEY_F2, IP_JOY_NONE,0 )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x04, "On" )
	PORT_DIPNAME( 0x08, 0x00, "Demo_Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x30, 0x00, "Coin_A", IP_KEY_NONE )
	PORT_DIPSETTING(    0x30, "4C_1C" )
	PORT_DIPSETTING(    0x20, "3C_1C" )
	PORT_DIPSETTING(    0x10, "2C_1C" )
	PORT_DIPSETTING(    0x00, "1C_1C" )
	PORT_DIPNAME( 0xc0, 0x00, "Coin_B", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "1C_2C" )
	PORT_DIPSETTING(    0x40, "1C_3C" )
	PORT_DIPSETTING(    0x80, "1C_4C" )
	PORT_DIPSETTING(    0xc0, "1C_6C" )

	PORT_START	/* (3) DSWB */
	PORT_DIPNAME( 0x03, 0x00, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x01, "Easy" )
	PORT_DIPSETTING(    0x00, "Medium" )
	PORT_DIPSETTING(    0x02, "Hard" )
	PORT_DIPSETTING(    0x03, "Hardest" )
	PORT_DIPNAME( 0x0c, 0x00, "Bonus_Life", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Every 300k" )
	PORT_DIPSETTING(    0x04, "200k and 500k" )
	PORT_DIPSETTING(    0x08, "300k only" )
	PORT_DIPSETTING(    0x0c, "None" )
	PORT_DIPNAME( 0x30, 0x00, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_BITX( 0x40,    0x00, IPT_DIPSWITCH_NAME | IPF_CHEAT, "Invulnerability", 0 ,0,0 )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x40, "On" )
	PORT_DIPNAME( 0x80, 0x00, "Unused", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /*  (4) DSWX */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN3 ) /* Service switch */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START	/* (5) DSW3 */
	PORT_DIPNAME( 0x0f, 0x02, "Territory", IP_KEY_NONE )
	PORT_DIPSETTING(    0x02, "Europe" )
	PORT_DIPSETTING(    0x01, "US" )
	PORT_DIPSETTING(    0x00, "Japan" )
	PORT_DIPSETTING(    0x03, "Hong Kong" )
	PORT_DIPSETTING(    0x04, "Korea" )
	PORT_DIPSETTING(    0x05, "Taiwan" )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START	/* vblank */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


INPUT_PORTS_START( zerowing_input_ports )
	PORT_START      /* PLAYER 1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    | IPF_PLAYER1 | IPF_8WAY )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  | IPF_PLAYER1 | IPF_8WAY )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  | IPF_PLAYER1 | IPF_8WAY )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT | IPF_PLAYER1 | IPF_8WAY )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START      /* PLAYER 2 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    | IPF_PLAYER2 | IPF_8WAY )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  | IPF_PLAYER2 | IPF_8WAY )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  | IPF_PLAYER2 | IPF_8WAY )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT | IPF_PLAYER2 | IPF_8WAY )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START	/* DSWA */
	PORT_DIPNAME( 0x01, 0x01, "Cabinet", IP_KEY_NONE )
	PORT_DIPSETTING(    0x01, "Upright" )
	PORT_DIPSETTING(    0x00, "Cocktail" )
	PORT_DIPNAME( 0x02, 0x00, "Flip_Screen", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x02, "On" )
	PORT_BITX(    0x04, 0x00, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service_Mode", OSD_KEY_F2, IP_JOY_NONE,0 )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x04, "On" )
	PORT_DIPNAME( 0x08, 0x00, "Demo_Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x30, 0x00, "Coin_A", IP_KEY_NONE )
	PORT_DIPSETTING(    0x30, "4C_1C" )
	PORT_DIPSETTING(    0x20, "3C_1C" )
	PORT_DIPSETTING(    0x10, "2C_1C" )
	PORT_DIPSETTING(    0x00, "1C_1C" )
	PORT_DIPNAME( 0xc0, 0x00, "Coin_B", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "1C_2C" )
	PORT_DIPSETTING(    0x40, "1C_3C" )
	PORT_DIPSETTING(    0x80, "1C_4C" )
	PORT_DIPSETTING(    0xc0, "1C_6C" )

	PORT_START	/* DSWB */
	PORT_DIPNAME( 0x03, 0x00, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x01, "Easy" )
	PORT_DIPSETTING(    0x00, "Medium" )
	PORT_DIPSETTING(    0x02, "Hard" )
	PORT_DIPSETTING(    0x03, "Hardest" )
	PORT_DIPNAME( 0x0c, 0x00, "Bonus_Life", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "200K and 500K" )
	PORT_DIPSETTING(    0x04, "500K and 1M" )
	PORT_DIPSETTING(    0x08, "500K" )
	PORT_DIPSETTING(    0x0c, "None" )
	PORT_DIPNAME( 0x30, 0x00, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_BITX(    0x40, 0x00, IPT_DIPSWITCH_NAME | IPF_CHEAT, "Invulnerability", IP_KEY_NONE, IP_JOY_NONE,0 )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x40, "On" )
	PORT_DIPNAME( 0x80, 0x00, "Allow Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "No" )
	PORT_DIPSETTING(    0x00, "Yes" )

	PORT_START      /* DSWX */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN3 ) /* Service switch */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START	/* DSW3 */
	PORT_DIPNAME( 0x03, 0x03, "Territory", IP_KEY_NONE )
	PORT_DIPSETTING(    0x03, "Europe" )
	PORT_DIPSETTING(    0x01, "US" )
	PORT_DIPSETTING(    0x00, "Japan" )
	PORT_DIPNAME( 0x04, 0x00, "Unused", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x04, "On" )
	PORT_DIPNAME( 0x08, 0x00, "Unused", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x08, "On" )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START	/* vblank */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNKNOWN )

INPUT_PORTS_END

INPUT_PORTS_START( hellfire_input_ports )
	PORT_START      /* PLAYER 1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    | IPF_PLAYER1 | IPF_8WAY )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  | IPF_PLAYER1 | IPF_8WAY )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  | IPF_PLAYER1 | IPF_8WAY )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT | IPF_PLAYER1 | IPF_8WAY )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START      /* PLAYER 2 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    | IPF_PLAYER2 | IPF_8WAY )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  | IPF_PLAYER2 | IPF_8WAY )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  | IPF_PLAYER2 | IPF_8WAY )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT | IPF_PLAYER2 | IPF_8WAY )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START	/* DSWA */
	PORT_DIPNAME( 0x01, 0x00, "Unused", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x01, "On" )
	PORT_DIPNAME( 0x02, 0x00, "Flip_Screen", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x02, "On" )
	PORT_BITX(    0x04, 0x00, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service_Mode", OSD_KEY_F2, IP_JOY_NONE,0 )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x04, "On" )
	PORT_DIPNAME( 0x08, 0x00, "Demo_Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x30, 0x00, "Coin_A", IP_KEY_NONE )
	PORT_DIPSETTING(    0x30, "4C_1C" )
	PORT_DIPSETTING(    0x20, "3C_1C" )
	PORT_DIPSETTING(    0x10, "2C_1C" )
	PORT_DIPSETTING(    0x00, "1C_1C" )
	PORT_DIPNAME( 0xc0, 0x00, "Coin_B", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "1C_2C" )
	PORT_DIPSETTING(    0x40, "1C_3C" )
	PORT_DIPSETTING(    0x80, "1C_4C" )
	PORT_DIPSETTING(    0xc0, "1C_6C" )

	PORT_START	/* DSWB */
	PORT_DIPNAME( 0x03, 0x00, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x01, "Easy" )
	PORT_DIPSETTING(    0x00, "Medium" )
	PORT_DIPSETTING(    0x02, "Hard" )
	PORT_DIPSETTING(    0x03, "Hardest" )
	PORT_DIPNAME( 0x0c, 0x00, "Bonus_Life", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "70K and 200K" )
	PORT_DIPSETTING(    0x04, "100K and 250K" )
	PORT_DIPSETTING(    0x08, "100K" )
	PORT_DIPSETTING(    0x0c, "200K" )
	PORT_DIPNAME( 0x30, 0x00, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_BITX(    0x40, 0x00, IPT_DIPSWITCH_NAME | IPF_CHEAT, "Invulnerability", IP_KEY_NONE, IP_JOY_NONE,0 )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x40, "On" )
	PORT_DIPNAME( 0x80, 0x00, "Unused", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x80, "On" )

	PORT_START      /* DSWX */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN3 ) /* Service switch */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START	/* DSW3 */
	PORT_DIPNAME( 0x03, 0x03, "Territory", IP_KEY_NONE )
	PORT_DIPSETTING(    0x03, "Europe" )
	PORT_DIPSETTING(    0x01, "US" )
	PORT_DIPSETTING(    0x00, "Japan" )
	PORT_DIPNAME( 0x04, 0x00, "Unused", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x04, "On" )
	PORT_DIPNAME( 0x08, 0x00, "Unused", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x08, "On" )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START	/* vblank */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNKNOWN )

INPUT_PORTS_END

INPUT_PORTS_START( truxton_input_ports )
	PORT_START	/* (0) DSWA */
	PORT_DIPNAME( 0x01, 0x00, "Unused", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x01, "On" )
	PORT_DIPNAME( 0x02, 0x00, "Flip_Screen", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x02, "On" )
	PORT_BITX(    0x04, 0x00, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service_Mode", OSD_KEY_F2, IP_JOY_NONE,0 )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x04, "On" )
	PORT_DIPNAME( 0x08, 0x00, "Demo_Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x30, 0x00, "Coin_A", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "1C_2C" )
	PORT_DIPSETTING(    0x30, "2C_3C" )
	PORT_DIPSETTING(    0x00, "1C_1C" )
	PORT_DIPSETTING(    0x20, "2C_1C" )
	PORT_DIPNAME( 0xc0, 0x00, "Coin_B", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "1C_2C" )
	PORT_DIPSETTING(    0xc0, "2C_3C" )
	PORT_DIPSETTING(    0x00, "1C_1C" )
	PORT_DIPSETTING(    0x80, "2C_1C" )

	PORT_START	/* (3) DSWB */
	PORT_DIPNAME( 0x03, 0x00, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x01, "Easy" )
	PORT_DIPSETTING(    0x00, "Medium" )
	PORT_DIPSETTING(    0x02, "Hard" )
	PORT_DIPSETTING(    0x03, "Hardest" )
	PORT_DIPNAME( 0x0c, 0x00, "Bonus_Life", IP_KEY_NONE )
	PORT_DIPSETTING(    0x04, "50k and every 150k" )
	PORT_DIPSETTING(    0x00, "70k and every 200k" )
	PORT_DIPSETTING(    0x08, "100k only" )
	PORT_DIPSETTING(    0x0c, "None" )
	PORT_DIPNAME( 0x30, 0x00, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_BITX(    0x40, 0x00, IPT_DIPSWITCH_NAME | IPF_CHEAT, "Invulnerability", IP_KEY_NONE, IP_JOY_NONE,0 )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x40, "On" )
	PORT_DIPNAME( 0x80, 0x00, "Allow Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "No" )
	PORT_DIPSETTING(    0x00, "Yes" )

	PORT_START      /* PLAYER 1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    | IPF_PLAYER1 | IPF_8WAY )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  | IPF_PLAYER1 | IPF_8WAY )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  | IPF_PLAYER1 | IPF_8WAY )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT | IPF_PLAYER1 | IPF_8WAY )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START      /* PLAYER 2 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    | IPF_PLAYER2 | IPF_8WAY )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  | IPF_PLAYER2 | IPF_8WAY )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  | IPF_PLAYER2 | IPF_8WAY )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT | IPF_PLAYER2 | IPF_8WAY )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START	/* DSWA */
	PORT_DIPNAME( 0x07, 0x02, "Territory/Copyright", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Japan/Taito Corp" )
	PORT_DIPSETTING(    0x01, "US/Romstar" )
	PORT_DIPSETTING(    0x04, "US/Taito America" )
	PORT_DIPSETTING(    0x02, "World/Taito Corp" )
	PORT_DIPSETTING(    0x06, "World/Taito America" )
	PORT_DIPNAME( 0x08, 0x00, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x08, "On" )
	PORT_DIPNAME( 0xf0, 0x00, "Unused", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0xf0, "On" )

	PORT_START      /* DSWX */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN3 ) /* Service switch */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START	/* vblank */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNKNOWN )

INPUT_PORTS_END


INPUT_PORTS_START( vimana_input_ports )
	PORT_START      /* (0) PLAYER 1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    | IPF_PLAYER1 | IPF_8WAY )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  | IPF_PLAYER1 | IPF_8WAY )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  | IPF_PLAYER1 | IPF_8WAY )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT | IPF_PLAYER1 | IPF_8WAY )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 | IPF_PLAYER1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START      /* (1) PLAYER 2 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    | IPF_PLAYER2 | IPF_8WAY )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  | IPF_PLAYER2 | IPF_8WAY )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  | IPF_PLAYER2 | IPF_8WAY )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT | IPF_PLAYER2 | IPF_8WAY )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 | IPF_PLAYER1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START	/* (2) DSWA */
	PORT_DIPNAME( 0x01, 0x00, "Unused", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x01, "On" )
	PORT_DIPNAME( 0x02, 0x00, "Flip_Screen", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x02, "On" )
	PORT_BITX(    0x04, 0x00, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service_Mode", OSD_KEY_F2, IP_JOY_NONE,0 )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x04, "On" )
	PORT_DIPNAME( 0x08, 0x00, "Demo_Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x30, 0x00, "Coin_A", IP_KEY_NONE )
	PORT_DIPSETTING(    0x30, "4C_1C" )
	PORT_DIPSETTING(    0x20, "3C_1C" )
	PORT_DIPSETTING(    0x10, "2C_1C" )
	PORT_DIPSETTING(    0x00, "1C_1C" )
	PORT_DIPNAME( 0xc0, 0x00, "Coin_B", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "1C_2C" )
	PORT_DIPSETTING(    0x40, "1C_3C" )
	PORT_DIPSETTING(    0x80, "1C_4C" )
	PORT_DIPSETTING(    0xc0, "1C_6C" )

	PORT_START	/* (3) DSWB */
	PORT_DIPNAME( 0x03, 0x00, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x01, "Easy" )
	PORT_DIPSETTING(    0x00, "Medium" )
	PORT_DIPSETTING(    0x02, "Hard" )
	PORT_DIPSETTING(    0x03, "Hardest" )
	PORT_DIPNAME( 0x0c, 0x00, "Bonus_Life" , IP_KEY_NONE)
	PORT_DIPSETTING(    0x00, "70K and 200K" )
	PORT_DIPSETTING(    0x04, "100K and 250K" )
	PORT_DIPSETTING(    0x08, "100K" )
	PORT_DIPSETTING(    0x0c, "200K" )
	PORT_DIPNAME( 0x30, 0x00, "Lives" , IP_KEY_NONE)
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_BITX(    0x40, 0x00, IPT_DIPSWITCH_NAME | IPF_CHEAT, "Invulnerability", IP_KEY_NONE, IP_JOY_NONE,0 )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x40, "On" )
	PORT_DIPNAME( 0x80, 0x00, "Allow Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "No" )
	PORT_DIPSETTING(    0x00, "Yes" )

	PORT_START      /*  (4) DSWX */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN3 ) /* Service switch */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START	/* (5) DSW3 */
	PORT_DIPNAME( 0x0f, 0x02, "Territory", IP_KEY_NONE )
	PORT_DIPSETTING(    0x02, "Europe" )
	PORT_DIPSETTING(    0x01, "US" )
	PORT_DIPSETTING(    0x00, "Japan" )
	PORT_DIPSETTING(    0x03, "Hong Kong" )
	PORT_DIPSETTING(    0x04, "Korea" )
	PORT_DIPSETTING(    0x05, "Taiwan" )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START	/* vblank */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static struct GfxLayout tilelayout =
{
	8,8,	/* 8x8 */
	16384,/* 16384 tiles */
	4,	/* 4 bits per pixel */
	{ 3*8*0x20000, 2*8*0x20000, 1*8*0x20000, 0*8*0x20000 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38 },
	64
};

static struct GfxLayout vm_tilelayout =
{
	8,8,	/* 8x8 */
	32768,/* 16384 tiles */
	4,	/* 4 bits per pixel */
	{ 8*0x80000+8, 8*0x80000, 8, 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70 },
	128
};

static struct GfxDecodeInfo gfxdecodeinfo[] =
{
	{ 1, 0x00000, &tilelayout,      0, 64 },
	{ 1, 0x80000, &tilelayout,  64*16, 64 },
	{ -1 } /* end of array */
};

static struct GfxDecodeInfo vm_gfxdecodeinfo[] =
{
	{ 1, 0x00000, &tilelayout,        0, 64 },
	{ 1, 0x80000, &vm_tilelayout, 64*16, 64 },
	{ -1 } /* end of array */
};

static struct GfxDecodeInfo outzone_gfxdecodeinfo[] =
{
	{ 1, 0x80000, &vm_tilelayout,        0, 64 },
	{ 1, 0x00000, &tilelayout, 64*16, 64 },
	{ -1 } /* end of array */
};

static void irqhandler(void)
{
	cpu_cause_interrupt(1,0xff);
}

static struct YM3812interface ym3812_interface =
{
	1,
	3500000,
	{ 255 },
	irqhandler,
};

static struct MachineDriver zw_machine_driver =
{
	/* basic machine hardware */
	{
		{
			CPU_M68000,
			10000000,
			0,
			zw_readmem,zw_writemem,0,0,
			zerowing_interrupt,1
		},
		{
			CPU_Z80 ,
			3500000,
			2,
			sound_readmem,sound_writemem,zw_sound_readport,zw_sound_writeport,
			ignore_interrupt,0
		}
	},
	60, DEFAULT_REAL_60HZ_VBLANK_DURATION,	/* frames per second, vblank duration */
	1,
	toaplan_init_machine,

	/* video hardware */
	320, 240, { 0, 319, 0, 239 },
	gfxdecodeinfo,
	64*16+64*16, 64*16+64*16,
	0,

	VIDEO_TYPE_RASTER | VIDEO_MODIFIES_PALETTE,
	0,
	zerowing_vh_start,
	zerowing_vh_stop,
	zerowing_vh_screenrefresh,

	/* sound hardware */
	0,0,0,0,
	{
		{
			SOUND_YM3812,
			&ym3812_interface
		},
	}
};

static struct MachineDriver vm_machine_driver =
{
	/* basic machine hardware */
	{
		{
			CPU_M68000,
			10000000,
			0,
			vm_readmem,vm_writemem,0,0,
			zerowing_interrupt,1
		}
	},
	60, DEFAULT_REAL_60HZ_VBLANK_DURATION,	/* frames per second, vblank duration */
	1,
	toaplan_init_machine,

	/* video hardware */
	320, 240, { 0, 319, 0, 239 },
	vm_gfxdecodeinfo,
	64*16+64*16, 64*16+64*16,
	0,

	VIDEO_TYPE_RASTER | VIDEO_MODIFIES_PALETTE,
	0,
	zerowing_vh_start,
	zerowing_vh_stop,
	vimana_vh_screenrefresh,

	/* sound hardware */
	0,0,0,0,
	{
		{
			SOUND_YM3812,
			&ym3812_interface
		},
	}
};

static struct MachineDriver hf_machine_driver =
{
	/* basic machine hardware */
	{
		{
			CPU_M68000,
			10000000,
			0,
			hf_readmem,hf_writemem,0,0,
			zerowing_interrupt,1
		},
		{
			CPU_Z80 ,
			3500000,
			2,
			sound_readmem,sound_writemem,hf_sound_readport,hf_sound_writeport,
			ignore_interrupt,0
		}
	},
	60, DEFAULT_REAL_60HZ_VBLANK_DURATION,	/* frames per second, vblank duration */
	1,
	toaplan_init_machine,

	/* video hardware */
	320, 240, { 0, 319, 0, 239 },
	gfxdecodeinfo,
	64*16+64*16, 64*16+64*16,
	0,

	VIDEO_TYPE_RASTER | VIDEO_MODIFIES_PALETTE,
	0,
	zerowing_vh_start,
	zerowing_vh_stop,
	zerowing_vh_screenrefresh,

	/* sound hardware */
	0,0,0,0,
	{
		{
			SOUND_YM3812,
			&ym3812_interface
		},
	}
};

static struct MachineDriver truxton_machine_driver =
{
	/* basic machine hardware */
	{
		{
			CPU_M68000,
			10000000,
			0,
			truxton_readmem,truxton_writemem,0,0,
			zerowing_interrupt,1
		},
		{
			CPU_Z80 ,
			3500000,
			2,
			sound_readmem,sound_writemem,truxton_sound_readport,truxton_sound_writeport,
			ignore_interrupt,0
		}
	},
	60, DEFAULT_REAL_60HZ_VBLANK_DURATION,	/* frames per second, vblank duration */
	1,
	toaplan_init_machine,

	/* video hardware */
	320, 240, { 0, 319, 0, 239 },
	gfxdecodeinfo,
	64*16+64*16, 64*16+64*16,
	0,

	VIDEO_TYPE_RASTER | VIDEO_MODIFIES_PALETTE,
	0,
	zerowing_vh_start,
	zerowing_vh_stop,
	zerowing_vh_screenrefresh,

	/* sound hardware */
	0,0,0,0,
	{
		{
			SOUND_YM3812,
			&ym3812_interface
		},
	}
};


static struct MachineDriver outzone_machine_driver =
{
	/* basic machine hardware */
	{
		{
			CPU_M68000,
			10000000,
			0,
			outzone_readmem,outzone_writemem,0,0,
			zerowing_interrupt,1
		},
		{
			CPU_Z80 ,
			3500000,
			2,
			sound_readmem,sound_writemem,outzone_sound_readport,outzone_sound_writeport,
			ignore_interrupt,0
		}
	},
	60, DEFAULT_REAL_60HZ_VBLANK_DURATION,	/* frames per second, vblank duration */
	1,
	toaplan_init_machine,

	/* video hardware */
	320, 240, {0, 319, 0, 239 },
	outzone_gfxdecodeinfo,
	64*16+64*16, 64*16+64*16,
	0,

	VIDEO_TYPE_RASTER | VIDEO_MODIFIES_PALETTE,
	0,
	zerowing_vh_start,
	zerowing_vh_stop,
	outzone_vh_screenrefresh,

	/* sound hardware */
	0,0,0,0,
	{
		{
			SOUND_YM3812,
			&ym3812_interface
		},
	}
};


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( zerowing_rom )
	ROM_REGION(0x80000)	/* 8*64k for 68000 code */
	ROM_LOAD_EVEN( "o15-11.rom",  0x00000, 0x08000, 0x6ff2b9a0 )
	ROM_LOAD_ODD ( "o15-12.rom",  0x00000, 0x08000, 0x9773e60b )
	ROM_LOAD_EVEN( "o15-09.rom",  0x40000, 0x20000, 0x13764e95 )
	ROM_LOAD_ODD ( "o15-10.rom",  0x40000, 0x20000, 0x351ba71a )

	ROM_REGION_DISPOSE(0x100000)
	ROM_LOAD( "o15-05.rom", 0x00000, 0x20000, 0x4e5dd246 )
	ROM_LOAD( "o15-06.rom", 0x20000, 0x20000, 0xc8c6d428 )
	ROM_LOAD( "o15-07.rom", 0x40000, 0x20000, 0xefc40e99 )
	ROM_LOAD( "o15-08.rom", 0x60000, 0x20000, 0x1b019eab )

	ROM_LOAD( "o15-03.rom", 0x80000, 0x20000, 0x7f245fd3 )
	ROM_LOAD( "o15-04.rom", 0xa0000, 0x20000, 0x0b1a1289 )
	ROM_LOAD( "o15-01.rom", 0xc0000, 0x20000, 0x70570e43 )
	ROM_LOAD( "o15-02.rom", 0xe0000, 0x20000, 0x724b487f )

	ROM_REGION(0x10000)	/* 64k for z80 sound code */
	ROM_LOAD( "o15-13.rom",   0x0000, 0x8000, 0xe7b72383 )
ROM_END

ROM_START( truxton_rom )
	ROM_REGION(0x40000)	/* 8*64k for 68000 code */
	ROM_LOAD_EVEN( "b65_11.bin",  0x00000, 0x20000, 0x1a62379a )
	ROM_LOAD_ODD ( "b65_10.bin",  0x00000, 0x20000, 0xaff5195d )

	ROM_REGION_DISPOSE(0x100000)
	ROM_LOAD( "b65_08.bin", 0x00000, 0x20000, 0xd2315b37 )
	ROM_LOAD( "b65_07.bin", 0x20000, 0x20000, 0xfb83252a )
	ROM_LOAD( "b65_06.bin", 0x40000, 0x20000, 0x36cedcbe )
	ROM_LOAD( "b65_05.bin", 0x60000, 0x20000, 0x81cd95f1 )

	ROM_LOAD( "b65_04.bin", 0x80000, 0x20000, 0x8c6ff461 )
	ROM_LOAD( "b65_03.bin", 0xa0000, 0x20000, 0x58b1350b )
	ROM_LOAD( "b65_02.bin", 0xc0000, 0x20000, 0x1dd55161 )
	ROM_LOAD( "b65_01.bin", 0xe0000, 0x20000, 0xe974937f )

	ROM_REGION(0x10000)	/* 64k for z80 sound code */
	ROM_LOAD( "b65_09.bin",   0x0000, 0x8000, 0xf1c0f410 )
ROM_END

ROM_START( hellfire_rom )
	ROM_REGION(0x40000)	/* 8*64k for 68000 code */
	ROM_LOAD_EVEN( "b90-14.bin",  0x00000, 0x20000, 0x101df9f5 )
	ROM_LOAD_ODD ( "b90-15.bin",  0x00000, 0x20000, 0xe67fd452 )

	ROM_REGION_DISPOSE(0x100000)
	ROM_LOAD( "b90-04.bin", 0x00000, 0x20000, 0xea6150fc )
	ROM_LOAD( "b90-05.bin", 0x20000, 0x20000, 0xbb52c507 )
	ROM_LOAD( "b90-06.bin", 0x40000, 0x20000, 0xcf5b0252 )
	ROM_LOAD( "b90-07.bin", 0x60000, 0x20000, 0xb98af263 )

	ROM_LOAD( "b90-11.bin", 0x80000, 0x20000, 0xc33e543c )
	ROM_LOAD( "b90-10.bin", 0xa0000, 0x20000, 0x35fd1092 )
	ROM_LOAD( "b90-09.bin", 0xc0000, 0x20000, 0xcf01009e )
	ROM_LOAD( "b90-08.bin", 0xe0000, 0x20000, 0x3404a5e3 )

	ROM_REGION(0x10000)	/* 64k for z80 sound code */
	ROM_LOAD( "b90-03.bin",   0x0000, 0x8000, 0x4058fa67 )
ROM_END

ROM_START( outzone_rom )
	ROM_REGION(0x40000)	/* 8*64k for 68000 code */
	ROM_LOAD_EVEN( "rom7.bin",  0x00000, 0x20000, 0x936e25d8 )
	ROM_LOAD_ODD ( "rom8.bin",  0x00000, 0x20000, 0xd19b3ecf )

	ROM_REGION_DISPOSE(0x180000)
	ROM_LOAD( "rom2.bin", 0x00000, 0x20000, 0x6bb72d16 )
	ROM_LOAD( "rom1.bin", 0x20000, 0x20000, 0x0934782d )
	ROM_LOAD( "rom3.bin", 0x40000, 0x20000, 0xec903c07 )
	ROM_LOAD( "rom4.bin", 0x60000, 0x20000, 0x50cbf1a8 )

	ROM_LOAD( "rom5.bin", 0x080000, 0x80000, 0xc64ec7b6 )
	ROM_LOAD( "rom6.bin", 0x100000, 0x80000, 0x64b6c5ac )

	ROM_REGION(0x10000)	/* 64k for z80 sound code */
	ROM_LOAD( "rom9.bin",   0x0000, 0x8000, 0x73d8e235 )
ROM_END

ROM_START( vimana_rom )
	ROM_REGION(0x40000)	/* 8*64k for 68000 code */
	ROM_LOAD_EVEN( "vim07.bin",  0x00000, 0x20000, 0x1efaea84 )
	ROM_LOAD_ODD ( "vim08.bin",  0x00000, 0x20000, 0xe45b7def )

	ROM_REGION_DISPOSE(0x180000)
	ROM_LOAD( "vim6.bin", 0x00000, 0x20000, 0x2886878d )
	ROM_LOAD( "vim5.bin", 0x20000, 0x20000, 0x61a63d7a )
	ROM_LOAD( "vim4.bin", 0x40000, 0x20000, 0xb0515768 )
	ROM_LOAD( "vim3.bin", 0x60000, 0x20000, 0x0b539131 )

	ROM_LOAD( "vim1.bin", 0x080000, 0x80000, 0xcdde26cd )
	ROM_LOAD( "vim2.bin", 0x100000, 0x80000, 0x1dbfc118 )

	/* sound CPU is a Z180 (not supported) */
ROM_END

ROM_START( vimana2_rom )
	ROM_REGION(0x40000)	/* 8*64k for 68000 code */
	ROM_LOAD_EVEN( "vimana07.bin",  0x00000, 0x20000, 0x5a4bf73e )
	ROM_LOAD_ODD ( "vimana08.bin",  0x00000, 0x20000, 0x03ba27e8 )

	ROM_REGION_DISPOSE(0x180000)
	ROM_LOAD( "vim6.bin", 0x00000, 0x20000, 0x2886878d )
	ROM_LOAD( "vim5.bin", 0x20000, 0x20000, 0x61a63d7a )
	ROM_LOAD( "vim4.bin", 0x40000, 0x20000, 0xb0515768 )
	ROM_LOAD( "vim3.bin", 0x60000, 0x20000, 0x0b539131 )

	ROM_LOAD( "vim1.bin", 0x080000, 0x80000, 0xcdde26cd )
	ROM_LOAD( "vim2.bin", 0x100000, 0x80000, 0x1dbfc118 )

	/* sound CPU is a Z180 (not supported) */
ROM_END



struct GameDriver zerowing_driver =
{
	__FILE__,
	0,
	"zerowing",
	"Zero Wing",
	"1989",
	"Toaplan",
	"Darren Olafson (MAME driver)\nCarl-Henrik Sk�rstedt (Technical)\nMagnus Danielsson (Technical)\n",
	0,
	&zw_machine_driver,
	0,

	zerowing_rom,
	0, 0,
	0,
	0,	/* sound_prom */

	zerowing_input_ports,

	0, 0, 0,   /* colors, palette, colortable */
	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver hellfire_driver =
{
	__FILE__,
	0,
	"hellfire",
	"Hellfire",
	"1989",
	"Toaplan (Taito license)",
	"Darren Olafson (MAME driver)\nCarl-Henrik Sk�rstedt (Technical)\nMagnus Danielsson (Technical)\n",
	0,
	&hf_machine_driver,
	0,

	hellfire_rom,
	0, 0,
	0,
	0,	/* sound_prom */

	hellfire_input_ports,

	0, 0, 0,   /* colors, palette, colortable */
	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver truxton_driver =
{
	__FILE__,
	0,
	"truxton",
	"Truxton / Tatsujin",
	"1988",
	"[Toaplan] (Taito license)",
	"Darren Olafson (MAME driver)\nCarl-Henrik Sk�rstedt (Technical)\nMagnus Danielsson (Technical)\n",
	0,
	&truxton_machine_driver,
	0,

	truxton_rom,
	0, 0,
	0,
	0,	/* sound_prom */

	truxton_input_ports,

	0, 0, 0,   /* colors, palette, colortable */
	ORIENTATION_ROTATE_270,
	0, 0
};

struct GameDriver outzone_driver =
{
	__FILE__,
	0,
	"outzone",
	"Out Zone",
	"1990",
	"Toaplan",
	"Darren Olafson (MAME driver)\nCarl-Henrik Sk�rstedt (Technical)\nMagnus Danielsson (Technical)\n",
	0,
	&outzone_machine_driver,
	0,

	outzone_rom,
	0, 0,
	0,
	0,	/* sound_prom */

	outzone_input_ports,

	0, 0, 0,   /* colors, palette, colortable */
	ORIENTATION_ROTATE_270,
	0, 0
};

struct GameDriver vimana_driver =
{
	__FILE__,
	0,
	"vimana",
	"Vimana (set 1)",
	"1991",
	"Toaplan",
	"Darren Olafson (MAME driver)\nCarl-Henrik Sk�rstedt (Technical)\nMagnus Danielsson (Technical)\n",
	0,
	&vm_machine_driver,
	0,

	vimana_rom,
	0, 0,
	0,
	0,	/* sound_prom */

	vimana_input_ports,

	0, 0, 0,   /* colors, palette, colortable */
	ORIENTATION_ROTATE_270,
	0, 0
};

struct GameDriver vimana2_driver =
{
	__FILE__,
	&vimana_driver,
	"vimana2",
	"Vimana (set 2)",
	"1991",
	"Toaplan",
	"Darren Olafson (MAME driver)\nCarl-Henrik Sk�rstedt (Technical)\nMagnus Danielsson (Technical)\n",
	0,
	&vm_machine_driver,
	0,

	vimana2_rom,
	0, 0,
	0,
	0,	/* sound_prom */

	vimana_input_ports,

	0, 0, 0,   /* colors, palette, colortable */
	ORIENTATION_ROTATE_270,
	0, 0
};
