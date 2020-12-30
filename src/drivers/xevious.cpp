/***************************************************************************

Pendings.

background   color mapping table
sprite       color mapping table
sprite , background , font draeing position adjust
exchange sprite set 1 and 2
1P/2P start switch , blaster button (custom_ic)
sound support ( custom_ic sound command )
palette prom (I dont have)
sprite , font & background & scroll position adjust

general map
-----------+---+-----------------+-------------------------
   hex     |r/w| D D D D D D D D |
 location  |   | 7 6 5 4 3 2 1 0 | function
-----------+---+-----------------+-------------------------
0000-3FFF  | R | D D D D D D D D | CPU 1 master rom (16k)
0000-1FFF  | R | D D D D D D D D | CPU 2 motion rom (8k)
0000-0FFF  | R | D D D D D D D D | CPU 3 sound  rom (4k)
-----------+---+-----------------+-------------------------
6800-680F  | W | - - - - D D D D | Audio control
6810-681F  | W | - - - - D D D D | Audio control
-----------+---+-----------------+-------------------------
6820       | W | - - - - - - - D | 0 = Reset IRQ1(latched)
6821       | W | - - - - - - - D | 0 = Reset IRQ2(latched)
6822       | W | - - - - - - - D | 0 = Reset NMI3(latched)
6823       | W | - - - - - - - D | 0 = Reset #2,#3 CPU
-----------+---+-----------------+-------------------------
6830       | W |                 | watchdog reset
-----------+---+-----------------+-------------------------
7000       |R/W| D D D D D D D D | custom 06 Data
7100       |R/W| D D D D D D D D | custom 06 Command
7???                               CPU #1 NMI REQUEST
-----------+---+-----------------+-------------------------
7800-7FFF  |R/W| D D D D D D D D | 2k work ram
-----------+---+-----------------+-------------------------
8000-87FF  |R/W| D D D D D D D D | 2k playfeild RAM
-----------+---+-----------------+-------------------------
8000-877F  |R/W| D D D D D D D D | RAM
8780-87FF  |R/W| D D D D D D D D | 2k sprite RAM (X-POS)
9000-977F  |R/W| D D D D D D D D | RAM
9780-97FF  |R/W| D D D D D D D D | 2k sprite RAM (HSIZE,MODE ??)
A000-A77F  |R/W| D D D D D D D D | RAM
A780-A7FF  |R/W| D D D D D D D D | 2k sprite RAM (PIC)
-----------+---+-----------------+-------------------------
B000-BFFF  |R/W| D D D D D D D D | 4k playfeild RAM ( ATTRIB)
C000-CFFF  |R/W| D D D D D D D D | 4k playfeild RAM ( PIC )
-----------+---+-----------------+-------------------------
D000-D00F  | W | D D D D D D D D | A0->D8:background Y scroll position
D010-D01F  | W | D D D D D D D D | A0->D8:font Y scroll position ??
D020-D02F  | W | D D D D D D D D | A0->D8:background X scroll position ??
D030-D03F  | W | D D D D D D D D | A0->D8:font X scroll position ?
D070-D07F  | W |               D | display flip mode ?
-----------+---+-----------------+-------------------------
F000       | W | D D D D D D D D | planet map position low ?
F001       | W | D D D D D D D D | planet map position high ?
F000       | R | D D D D D D D D | planet map data low ?
F001       | R | D D D D D D D D | planet map data high ?
-----------+---+-----------------+-------------------------
Xevious memory map (preliminary)

  Z80-1:MASTER CPU
    0000H-3FFFH R   P-ROM
    4000H-FFFFH R/W shared area
  Z80-2:MOTION CUP
    0000H-1FFFH R   P-ROM
    4000H-FFFFH R/W shared area
  Z80-3:SOUND CPU
    0000H-1FFFH R   P-ROM
    4000H-FFFFH R/W shared area
  shared area.
    6800H-6807H R   DIP SWITCH READ
                      A0-2 : bit select(6800H=bit0 , 6801=bit1,6807=bit7)
                      D0   : DIP SW.A bit read
                      D1   : DIP SW.B bit read
                    DIP SW.A
                      Bit0 = BLASTER SWITCH
    6800H-680FH R   shadow(6800H-6807H)
    6800H-681FH W   Sound RAM0 (same to DIGDUG ?)
                      D0-3:data
    6810H-681FH W   Sound RAM1
                      D0-3:data
    6820H       W   MASTER-INTERRUPT CLEAR & ENABLE
                      D0 = 0:CLEAR & DISABLE / 1:ENABLE
    6821H       W   MOTION-INTERRUPT CLEAR & ENABLE
                      D0 = 0:CLEAR & DISABLE / 1:ENABLE
    6822H       W   SOUND -NMI CONTROLL
                      D0 = 0:NMI ON / 1:NMI OFF
    6823H       W   CPU 2,3 RESET CONTROLL
                      D0 = 0:RESET
    6830H-683F  W   WDR Watch dock timer clear
    6840H-6FFFH R/W shadow(6800H-603FH)
    7000H       R   custom-io data read ( after CPU #1 NMI )
    7000H       W   custom-io data write( after CPU #1 NMI )
    7100H       R   custom-io timming port
                     bit7  :1=busy ?
                     bit6  :1=busy ?
                     bit5  :1=busy ?
    7100H       W   custom-io command write
    7???H       W   sound ganarator (controll by custom_ic ?)
    7800H-7FFFH R/W S-RAM
    8000H-87FFh R/W ram ( master cpu )
    8780H-87FFH R/W sprite X position ( 80-bf:used by master,c0-ff:used by motion)
                      A0=0:sprite Y position (OFF = 0xef )
                      A0=1:sprite X position
    8800H-8FFFH R/W shadow(8000H-87FFH)
    9000H-97FFH R/W ram ( motion cpu )
    9780H-97FFH R/W sprite attribute
                      A0=0:attribute Y
                        bit0  :HSIZE  :sprite Y size ?
                        bit1  :HUNDZ  :?
                        bit2  :FLOP   :Y flip
                        bit3  :HUKAIRS:?
                        bit4-6:MD4-6  :?
                        bit7  :BIT3   :select sprite set (0=4M,4P,4R 3bit:1=4N 2bit )
                      A0=0:attribute X
                        bit0  :HSIZE  :sprite X size ?
                        bit1  :HUNDZ  :?
                        bit2  :FLOP   :X flip
                        bit3  :HUKAIRS:?
                        bit4-6:MD4-6  :?
                        bit7  :BIT3   :?
    9800H-9FFFH R/W shadow(9000H-97FFH)
    A000H-A7FFH R/W ram ( sound cpu , master cpu )
    A780H-a7FFH R/W sprite character nnumber
                      A0=0 character pattern name
                      A0=1 color map select
    A800H-AFFFH R/W shadow(A000H-A7FFH)
    B000H-BFFFH R/W background attrivute ( have 2 scroll planes ? )
                    D0-D1:COL0,1 palette set ?
                    D1-D5:ANI0-3 color code  ?( font,bg use )
                    D6   :ANF    X flip
                    D7   :PFF    Y flip
    C000H-CFFFH R/W background character ( have 2 scroll planes ? )
    D000H-D07FH  W  CRTC access? (custom-13)
    D???H        W  display flip select (TABLE 1P/2P mode)
                      bit0  :FLIP
    F000H        W  BS0 xevious planet map select low ?
    F001H        W  BB1 xeviosu planet map select high ?
    F000H        R  BB0 xevious planet map get low ?
    F001H        R  BS1 xevious planet map get high ?
    F002-FFFFH      shodow(F000H-F001H)

3)schematic diagram block.

   Sheet 4A : main cpu , address decoder(4000h-7fffh) , sram
   Sheet 4B : motion cpu
   Sheet 5A : sound cpu
   Sheet 5B : clock generator , 'digdug' sound , irq controller
   Sheet 6A : joystick read , dip switch , audio(TONE?) , NMI to master cpu
   Sheet 6B : address decoder (8000h-ffffh) , sprite ram r/w , disp. flip latch
   Sheet 7A : sprite drawing engine
   Sheet 7B : background ram , CRTC ?,graphic drawing engene ?
   Sheet 8A : background(&font) drawing engene
   Sheet 8B : display line buffer ?
   Sheet 9A : video dac ( palette rom )
   Sheet 9B : xevious planet map rom ?

P-ROMS in schematic

1M  master cpu rom 0000H-1FFFH
1L  master cpu rom 2000H-3FFFH
4C  motion cpu rom 0000H-1FFFH
2C  sound  cpu rom 0000H-1FFFH
8M  sound pcm rom ?
6M  sound pcm decode rom ?
4M  spright pattern low (BIT3=0,000-127)
4P  spright pattern low (BIT3=0,128-255)
4R  spright pattern high(BIT3=0)
      D0-3:000-127
      D4-7:128-255
4N  spright pattern (BIT3=1)
3L  spright color map table
3M  spright color map table
3D  background pattern bit0
3C  background pattern bit1
3B  background font pattern
4H  background color map table
4F  background color map table
6E  palette rom blue
6D  palette rom green
6A  palette rom red
2A  xevious planet map table ??
2B  xevious planet map table ??
2C  xevious planet map table ??

S-RAMS in schematic

1H  sram           7800H-7FFFH
7L  sound sram0    6800H-680FH
7K  sound sram1    6810H-681FH
2S  spright line   8000H-87FFH
2A  spright att    9000H-97FFH
2P  spright chr    A000H-A7FFH
2J  background ram B000H-B7FFH
2H  background ram B800H-BFFFH
2F  background ram C000H-C7FFH
2E  background ram C800H-CFFFH
5N,5M sprite display line buffer ? (even disp,odd draw)
6N,6M sprite display line buffer ? (even draw,odd disp)

*****************************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"


extern unsigned char *xevious_sharedram;
int  xevious_sharedram_r(int offset);
void xevious_sharedram_w(int offset,int data);
int  xevious_dsw_r(int offset);
void xevious_interrupt_enable_1_w(int offset,int data);
void xevious_interrupt_enable_2_w(int offset,int data);
void xevious_interrupt_enable_3_w(int offset,int data);
void xevious_bs_w(int offset,int data);
int  xevious_bb_r(int offset);
int xevious_customio_r(int offset);
int xevious_customio_data_r(int offset);
void xevious_customio_w(int offset,int data);
void xevious_customio_data_w(int offset,int data);
void xevious_halt_w(int offset,int data);
int  xevious_interrupt_1(void);
int  xevious_interrupt_2(void);
int  xevious_interrupt_3(void);
void xevious_init_machine(void);

extern unsigned char *xevious_vlatches;
void xevious_vh_latch_w(int offset, int data);

extern unsigned char *xevious_videoram2,*xevious_colorram2;
void xevious_videoram2_w(int offset,int data);
void xevious_colorram2_w(int offset,int data);
int  xevious_vh_start( void );
void xevious_vh_stop( void );
void xevious_vh_convert_color_prom(unsigned char *palette, unsigned short *colortable,const unsigned char *color_prom);
void xevious_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh);

void pengo_sound_w(int offset,int data);
extern unsigned char *namco_soundregs;



static struct MemoryReadAddress readmem_cpu1[] =
{
	{ 0x0000, 0x3fff, MRA_ROM },
	{ 0x6800, 0x6807, xevious_dsw_r },
	{ 0x7000, 0x700f, xevious_customio_data_r },
	{ 0x7100, 0x7100, xevious_customio_r },
	{ 0x7800, 0xcfff, xevious_sharedram_r },
	{ 0xf000, 0xffff, xevious_bb_r },
	{ -1 }	/* end of table */
};

static struct MemoryReadAddress readmem_cpu2[] =
{
	{ 0x0000, 0x1fff, MRA_ROM },
	{ 0x6800, 0x6807, xevious_dsw_r },
	{ 0x7800, 0xcfff, xevious_sharedram_r },
	{ 0xf000, 0xffff, xevious_bb_r },
	{ -1 }	/* end of table */
};

static struct MemoryReadAddress readmem_cpu3[] =
{
	{ 0x0000, 0x0fff, MRA_ROM },
	{ 0x7800, 0xcfff, xevious_sharedram_r },
	{ -1 }	/* end of table */
};

static struct MemoryWriteAddress writemem_cpu1[] =
{
	{ 0x0000, 0x3fff, MWA_ROM },
	{ 0x6820, 0x6820, xevious_interrupt_enable_1_w },
	{ 0x6821, 0x6821, xevious_interrupt_enable_2_w },
	{ 0x6822, 0x6822, xevious_interrupt_enable_3_w },
	{ 0x6823, 0x6823, xevious_halt_w },			/* reset controll */
	{ 0x6830, 0x683f, MWA_NOP },				/* watch dock reset */
	{ 0x7000, 0x700f, xevious_customio_data_w },
	{ 0x7100, 0x7100, xevious_customio_w },
	{ 0x7800, 0xafff, xevious_sharedram_w, &xevious_sharedram },
	{ 0xb000, 0xb7ff, colorram_w, &colorram },
	{ 0xb800, 0xbfff, xevious_colorram2_w, &xevious_colorram2 },
	{ 0xc000, 0xc7ff, videoram_w, &videoram, &videoram_size },
	{ 0xc800, 0xcfff, xevious_videoram2_w, &xevious_videoram2 },
	{ 0xd000, 0xd07f, xevious_vh_latch_w, &xevious_vlatches }, /* ?? */
	{ 0xf000, 0xffff, xevious_bs_w },
	{ 0x8780, 0x87ff, MWA_RAM, &spriteram_2 },	/* here only */
	{ 0x9780, 0x97ff, MWA_RAM, &spriteram_3 },	/* to initialize */
	{ 0xa780, 0xa7ff, MWA_RAM, &spriteram, &spriteram_size },	/* the pointers */
	{ -1 }	/* end of table */
};

static struct MemoryWriteAddress writemem_cpu2[] =
{
	{ 0x0000, 0x1fff, MWA_ROM },
	{ 0x6823, 0x6823, xevious_halt_w },			/* reset control */
	{ 0x6830, 0x683f, MWA_NOP },				/* watch dog reset */
	{ 0x7800, 0xafff, xevious_sharedram_w },
	{ 0xb000, 0xb7ff, colorram_w },
	{ 0xb800, 0xbfff, xevious_colorram2_w },
	{ 0xc000, 0xc7ff, videoram_w },
	{ 0xc800, 0xcfff, xevious_videoram2_w },
	{ 0xd000, 0xd07f, xevious_vh_latch_w, &xevious_vlatches }, /* ?? */
	{ 0xf000, 0xffff, xevious_bs_w },
	{ -1 }	/* end of table */
};

static struct MemoryWriteAddress writemem_cpu3[] =
{
	{ 0x0000, 0x0fff, MWA_ROM },
	{ 0x6800, 0x681f, pengo_sound_w, &namco_soundregs },
	{ 0x6822, 0x6822, xevious_interrupt_enable_3_w },
	{ 0x7800, 0xcfff, xevious_sharedram_w },
	{ -1 }	/* end of table */
};



INPUT_PORTS_START( xevious_input_ports )
	PORT_START	/* DSW0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_DIPNAME( 0x02, 0x02, "Flags Award Bonus Life", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "No" )
	PORT_DIPSETTING(    0x02, "Yes" )
	PORT_DIPNAME( 0x0c, 0x0c, "Right Coin", IP_KEY_NONE )
	PORT_DIPSETTING(    0x0c, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x08, "1 Coin/2 Credit" )
	PORT_DIPSETTING(    0x04, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin/6 Credits" )
	PORT_DIPNAME( 0x10, 0x10, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x60, 0x60, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Easy" )
	PORT_DIPSETTING(    0x60, "Normal" )
	PORT_DIPSETTING(    0x20, "Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0x80, 0x80, "Freeze?", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START	/* DSW1 */
	PORT_DIPNAME( 0x03, 0x03, "Left Coin", IP_KEY_NONE )
	PORT_DIPSETTING(    0x01, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x03, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x00, "2 Coins/3 Credits" )
	PORT_DIPSETTING(    0x02, "1 Coin/2 Credits" )
	/* TODO: bonus scores are different for 5 lives */
	PORT_DIPNAME( 0x1c, 0x1c, "Bonus Life", IP_KEY_NONE )
	PORT_DIPSETTING(    0x18, "10000 40000" )
	PORT_DIPSETTING(    0x14, "10000 50000" )
	PORT_DIPSETTING(    0x10, "20000 50000" )
	PORT_DIPSETTING(    0x0c, "20000 70000" )
	PORT_DIPSETTING(    0x08, "20000 80000" )
	PORT_DIPSETTING(    0x1c, "20000 60000" )
	PORT_DIPSETTING(    0x04, "20000 and 60000" )
	PORT_DIPSETTING(    0x00, "None" )
	PORT_DIPNAME( 0x60, 0x60, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x60, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x80, 0x80, "Cabinet", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Upright" )
	PORT_DIPSETTING(    0x00, "Cocktail" )

	PORT_START	/* FAKE */
	/* The player inputs are not memory mapped, they are handled by an I/O chip. */
	/* These fake input ports are read by galaga_customio_data_r() */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY )
	PORT_BITX(0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_IMPULSE,
			IP_NAME_DEFAULT, IP_KEY_DEFAULT, IP_JOY_DEFAULT, 1 )
	PORT_BITX(0x20, IP_ACTIVE_LOW, IPT_BUTTON1, 0, IP_KEY_PREVIOUS, IP_JOY_PREVIOUS, 0 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* FAKE */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_COCKTAIL)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_COCKTAIL)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_COCKTAIL)
	PORT_BITX(0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_IMPULSE | IPF_COCKTAIL,
			IP_NAME_DEFAULT, IP_KEY_DEFAULT, IP_JOY_DEFAULT, 1 )
	PORT_BITX(0x20, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL, 0, IP_KEY_PREVIOUS, IP_JOY_PREVIOUS, 0 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* FAKE */
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BITX(0x04, IP_ACTIVE_LOW, IPT_START1 | IPF_IMPULSE,
			IP_NAME_DEFAULT, IP_KEY_DEFAULT, IP_JOY_DEFAULT, 1 )
	PORT_BITX(0x08, IP_ACTIVE_LOW, IPT_START2 | IPF_IMPULSE,
			IP_NAME_DEFAULT, IP_KEY_DEFAULT, IP_JOY_DEFAULT, 1 )
	PORT_BITX(0x10, IP_ACTIVE_LOW, IPT_COIN1 | IPF_IMPULSE,
			IP_NAME_DEFAULT, IP_KEY_DEFAULT, IP_JOY_DEFAULT, 1 )
	PORT_BITX(0x20, IP_ACTIVE_LOW, IPT_COIN2 | IPF_IMPULSE,
			IP_NAME_DEFAULT, IP_KEY_DEFAULT, IP_JOY_DEFAULT, 1 )
	PORT_BITX(0x40, IP_ACTIVE_LOW, IPT_COIN3 | IPF_IMPULSE,
			IP_NAME_DEFAULT, IP_KEY_DEFAULT, IP_JOY_DEFAULT, 1 )
	PORT_BITX(    0x80, 0x80, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
INPUT_PORTS_END

/* same as xevious, the only difference is DSW0 bit 7 */
INPUT_PORTS_START( xeviousa_input_ports )
	PORT_START	/* DSW0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_DIPNAME( 0x02, 0x02, "Flags Award Bonus Life", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "No" )
	PORT_DIPSETTING(    0x02, "Yes" )
	PORT_DIPNAME( 0x0c, 0x0c, "Right Coin", IP_KEY_NONE )
	PORT_DIPSETTING(    0x0c, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x08, "1 Coin/2 Credit" )
	PORT_DIPSETTING(    0x04, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin/6 Credits" )
	PORT_DIPNAME( 0x10, 0x10, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x60, 0x60, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Easy" )
	PORT_DIPSETTING(    0x60, "Normal" )
	PORT_DIPSETTING(    0x20, "Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	/* when switch is on Namco, high score names are 10 letters long */
	PORT_DIPNAME( 0x80, 0x80, "Copyright", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Namco" )
	PORT_DIPSETTING(    0x80, "Atari/Namco" )

	PORT_START	/* DSW1 */
	PORT_DIPNAME( 0x03, 0x03, "Left Coin", IP_KEY_NONE )
	PORT_DIPSETTING(    0x01, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x03, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x00, "2 Coins/3 Credits" )
	PORT_DIPSETTING(    0x02, "1 Coin/2 Credits" )
	/* TODO: bonus scores are different for 5 lives */
	PORT_DIPNAME( 0x1c, 0x1c, "Bonus Life", IP_KEY_NONE )
	PORT_DIPSETTING(    0x18, "10000 40000" )
	PORT_DIPSETTING(    0x14, "10000 50000" )
	PORT_DIPSETTING(    0x10, "20000 50000" )
	PORT_DIPSETTING(    0x0c, "20000 70000" )
	PORT_DIPSETTING(    0x08, "20000 80000" )
	PORT_DIPSETTING(    0x1c, "20000 60000" )
	PORT_DIPSETTING(    0x04, "20000 and 60000" )
	PORT_DIPSETTING(    0x00, "None" )
	PORT_DIPNAME( 0x60, 0x60, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x60, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x80, 0x80, "Cabinet", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Upright" )
	PORT_DIPSETTING(    0x00, "Cocktail" )

	PORT_START	/* FAKE */
	/* The player inputs are not memory mapped, they are handled by an I/O chip. */
	/* These fake input ports are read by galaga_customio_data_r() */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY )
	PORT_BITX(0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_IMPULSE,
			IP_NAME_DEFAULT, IP_KEY_DEFAULT, IP_JOY_DEFAULT, 1 )
	PORT_BITX(0x20, IP_ACTIVE_LOW, IPT_BUTTON1, 0, IP_KEY_PREVIOUS, IP_JOY_PREVIOUS, 0 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* FAKE */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_COCKTAIL)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_COCKTAIL)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_COCKTAIL)
	PORT_BITX(0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_IMPULSE | IPF_COCKTAIL,
			IP_NAME_DEFAULT, IP_KEY_DEFAULT, IP_JOY_DEFAULT, 1 )
	PORT_BITX(0x20, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL, 0, IP_KEY_PREVIOUS, IP_JOY_PREVIOUS, 0 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* FAKE */
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BITX(0x04, IP_ACTIVE_LOW, IPT_START1 | IPF_IMPULSE,
			IP_NAME_DEFAULT, IP_KEY_DEFAULT, IP_JOY_DEFAULT, 1 )
	PORT_BITX(0x08, IP_ACTIVE_LOW, IPT_START2 | IPF_IMPULSE,
			IP_NAME_DEFAULT, IP_KEY_DEFAULT, IP_JOY_DEFAULT, 1 )
	PORT_BITX(0x10, IP_ACTIVE_LOW, IPT_COIN1 | IPF_IMPULSE,
			IP_NAME_DEFAULT, IP_KEY_DEFAULT, IP_JOY_DEFAULT, 1 )
	PORT_BITX(0x20, IP_ACTIVE_LOW, IPT_COIN2 | IPF_IMPULSE,
			IP_NAME_DEFAULT, IP_KEY_DEFAULT, IP_JOY_DEFAULT, 1 )
	PORT_BITX(0x40, IP_ACTIVE_LOW, IPT_COIN3 | IPF_IMPULSE,
			IP_NAME_DEFAULT, IP_KEY_DEFAULT, IP_JOY_DEFAULT, 1 )
	PORT_BITX(    0x80, 0x80, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
INPUT_PORTS_END

/* same as xevious, the only difference is DSW0 bit 7. Note that the bit is */
/* inverted wrt xevious. */
INPUT_PORTS_START( sxevious_input_ports )
	PORT_START	/* DSW0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_DIPNAME( 0x02, 0x02, "Flags Award Bonus Life", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "No" )
	PORT_DIPSETTING(    0x02, "Yes" )
	PORT_DIPNAME( 0x0c, 0x0c, "Right Coin", IP_KEY_NONE )
	PORT_DIPSETTING(    0x0c, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x08, "1 Coin/2 Credit" )
	PORT_DIPSETTING(    0x04, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin/6 Credits" )
	PORT_DIPNAME( 0x10, 0x10, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x60, 0x60, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Easy" )
	PORT_DIPSETTING(    0x60, "Normal" )
	PORT_DIPSETTING(    0x20, "Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0x80, 0x00, "Freeze?", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x80, "On" )

	PORT_START	/* DSW1 */
	PORT_DIPNAME( 0x03, 0x03, "Left Coin", IP_KEY_NONE )
	PORT_DIPSETTING(    0x01, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x03, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x00, "2 Coins/3 Credits" )
	PORT_DIPSETTING(    0x02, "1 Coin/2 Credits" )
	/* TODO: bonus scores are different for 5 lives */
	PORT_DIPNAME( 0x1c, 0x1c, "Bonus Life", IP_KEY_NONE )
	PORT_DIPSETTING(    0x18, "10000 40000" )
	PORT_DIPSETTING(    0x14, "10000 50000" )
	PORT_DIPSETTING(    0x10, "20000 50000" )
	PORT_DIPSETTING(    0x0c, "20000 70000" )
	PORT_DIPSETTING(    0x08, "20000 80000" )
	PORT_DIPSETTING(    0x1c, "20000 60000" )
	PORT_DIPSETTING(    0x04, "20000 and 60000" )
	PORT_DIPSETTING(    0x00, "None" )
	PORT_DIPNAME( 0x60, 0x60, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x60, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x80, 0x80, "Cabinet", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Upright" )
	PORT_DIPSETTING(    0x00, "Cocktail" )

	PORT_START	/* FAKE */
	/* The player inputs are not memory mapped, they are handled by an I/O chip. */
	/* These fake input ports are read by galaga_customio_data_r() */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY )
	PORT_BITX(0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_IMPULSE,
			IP_NAME_DEFAULT, IP_KEY_DEFAULT, IP_JOY_DEFAULT, 1 )
	PORT_BITX(0x20, IP_ACTIVE_LOW, IPT_BUTTON1, 0, IP_KEY_PREVIOUS, IP_JOY_PREVIOUS, 0 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* FAKE */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_COCKTAIL)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_COCKTAIL)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_COCKTAIL)
	PORT_BITX(0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_IMPULSE | IPF_COCKTAIL,
			IP_NAME_DEFAULT, IP_KEY_DEFAULT, IP_JOY_DEFAULT, 1 )
	PORT_BITX(0x20, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL, 0, IP_KEY_PREVIOUS, IP_JOY_PREVIOUS, 0 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* FAKE */
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BITX(0x04, IP_ACTIVE_LOW, IPT_START1 | IPF_IMPULSE,
			IP_NAME_DEFAULT, IP_KEY_DEFAULT, IP_JOY_DEFAULT, 1 )
	PORT_BITX(0x08, IP_ACTIVE_LOW, IPT_START2 | IPF_IMPULSE,
			IP_NAME_DEFAULT, IP_KEY_DEFAULT, IP_JOY_DEFAULT, 1 )
	PORT_BITX(0x10, IP_ACTIVE_LOW, IPT_COIN1 | IPF_IMPULSE,
			IP_NAME_DEFAULT, IP_KEY_DEFAULT, IP_JOY_DEFAULT, 1 )
	PORT_BITX(0x20, IP_ACTIVE_LOW, IPT_COIN2 | IPF_IMPULSE,
			IP_NAME_DEFAULT, IP_KEY_DEFAULT, IP_JOY_DEFAULT, 1 )
	PORT_BITX(0x40, IP_ACTIVE_LOW, IPT_COIN3 | IPF_IMPULSE,
			IP_NAME_DEFAULT, IP_KEY_DEFAULT, IP_JOY_DEFAULT, 1 )
	PORT_BITX(    0x80, 0x80, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
INPUT_PORTS_END



/* foreground characters */
static struct GfxLayout charlayout =
{
	8,8,	/* 8*8 characters */
	512,	/* 512 characters */
	1,	/* 1 bit per pixel */
	{ 0 },
	{ 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	8*8	/* every char takes 8 consecutive bytes */
};
/* background tiles */
static struct GfxLayout bgcharlayout =
{
	8,8,	/* 8*8 characters */
	512,	/* 512 characters */
	2,	/* 2 bits per pixel */
	{ 0, 512*8*8 },
	{ 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	8*8	/* every char takes 8 consecutive bytes */
};
/* sprite set #1 */
static struct GfxLayout spritelayout1 =
{
	16,16,	/* 16*16 sprites */
	128,	/* 128 sprites */
	3,	/* 3 bits per pixel */
	{ 128*64*8+4, 0, 4 },
	{ 39*8, 38*8, 37*8, 36*8, 35*8, 34*8, 33*8, 32*8,
		7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3 },
	64*8	/* every sprite takes 128 consecutive bytes */
};
/* sprite set #2 */
static struct GfxLayout spritelayout2 =
{
	16,16,	/* 16*16 sprites */
	128,	/* 128 sprites */
	3,	/* 3 bits per pixel */
	{ 0, 128*64*8, 128*64*8+4 },
	{ 39*8, 38*8, 37*8, 36*8, 35*8, 34*8, 33*8, 32*8,
		7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3 },
	64*8	/* every sprite takes 128 consecutive bytes */
};
/* sprite set #3 */
static struct GfxLayout spritelayout3 =
{
	16,16,	/* 16*16 sprites */
	64,	/* 64 sprites */
	3,	/* 3 bits per pixel (one is always 0) */
	{ 64*64*8, 0, 4 },
	{ 39*8, 38*8, 37*8, 36*8, 35*8, 34*8, 33*8, 32*8,
		7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3 },
	64*8	/* every sprite takes 64 consecutive bytes */
};



static struct GfxDecodeInfo gfxdecodeinfo[] =
{
	{ 1, 0x0000, &charlayout, 128*4+64*8,  64 },
	{ 1, 0x1000, &bgcharlayout,        0, 128 },
	{ 1, 0x3000, &spritelayout1,   128*4,  64 },
	{ 1, 0x5000, &spritelayout2,   128*4,  64 },
	{ 1, 0x9000, &spritelayout3,   128*4,  64 },
	{ -1 } /* end of array */
};



static struct namco_interface namco_interface =
{
	3072000/32,	/* sample rate */
	3,			/* number of voices */
	32,			/* gain adjustment */
	255,		/* playback volume */
	6			/* memory region */
};

struct Samplesinterface samples_interface =
{
	1	/* one channel */
};



static struct MachineDriver machine_driver =
{
	/* basic machine hardware */
	{
		{
			CPU_Z80,
			3125000,	/* 3.125 Mhz (?) */
			0,
			readmem_cpu1,writemem_cpu1,0,0,
			xevious_interrupt_1,1
		},
		{
			CPU_Z80,
			3125000,	/* 3.125 Mhz */
			3,	/* memory region #3 */
			readmem_cpu2,writemem_cpu2,0,0,
			xevious_interrupt_2,1
		},
		{
			CPU_Z80,
			3125000,	/* 3.125 Mhz */
			4,	/* memory region #4 */
			readmem_cpu3,writemem_cpu3,0,0,
			xevious_interrupt_3,2
		}
	},
	60, DEFAULT_60HZ_VBLANK_DURATION,	/* frames per second, vblank duration */
	100,	/* 100 CPU slices per frame - an high value to ensure proper */
			/* synchronization of the CPUs */
	xevious_init_machine,

	/* video hardware */
	28*8, 36*8, { 0*8, 28*8-1, 0*8, 36*8-1 },
	gfxdecodeinfo,
	128+1,128*4+64*8+64*2,
	xevious_vh_convert_color_prom,

	VIDEO_TYPE_RASTER,
	0,
	xevious_vh_start,
	xevious_vh_stop,
	xevious_vh_screenrefresh,

	/* sound hardware */
	0,0,0,0,
	{
		{
			SOUND_NAMCO,
			&namco_interface
		},
		{
			SOUND_SAMPLES,
			&samples_interface
		}
	}
};



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( xevious_rom )
	ROM_REGION(0x10000)	/* 64k for the first CPU */
	ROM_LOAD( "xe-1m-a.bin",  0x0000, 0x2000, 0x92fa6cae )
	ROM_LOAD( "xe-1l-a.bin",  0x2000, 0x2000, 0xbd74609a )

	ROM_REGION_DISPOSE(0xb000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "xe-3b.bin",    0x0000, 0x1000, 0x088c8b26 )	/* foreground characters */
	ROM_LOAD( "xe-3c.bin",    0x1000, 0x1000, 0xde60ba25 )	/* bg pattern B0 */
	ROM_LOAD( "xe-3d.bin",    0x2000, 0x1000, 0x535cdbbc )	/* bg pattern B1 */
	ROM_LOAD( "xe-4m.bin",    0x3000, 0x2000, 0xdc2c0ecb )	/* sprite set #1, planes 0/1 */
	ROM_LOAD( "xe-4r.bin",    0x5000, 0x2000, 0x02417d19 )	/* sprite set #1, plane 2, set #2, plane 0 */
	ROM_LOAD( "xe-4p.bin",    0x7000, 0x2000, 0xdfb587ce )	/* sprite set #2, planes 1/2 */
	ROM_LOAD( "xe-4n.bin",    0x9000, 0x1000, 0x605ca889 )	/* sprite set #3, planes 0/1 */
	/* 0xa000-0xafff empty space to decode sprite set #3 as 3 bits per pixel */

	ROM_REGION(0x0700)	/* color proms */
	ROM_LOAD( "xevious.6a",   0x0000, 0x0100, 0xa0334cef ) /* palette red component */
	ROM_LOAD( "xevious.6d",   0x0100, 0x0100, 0xa076a85c ) /* palette green component */
	ROM_LOAD( "xevious.6e",   0x0200, 0x0100, 0xc04737e5 ) /* palette blue component */
	ROM_LOAD( "4h-4f",        0x0300, 0x0200, 0xc21a65ff ) /* background tiles lookup table */
	/* These are probably not the original PROMs. The transparency information */
	/* (bit 7) is missing. The 0x80 bytes are mine, needed to fix the targeting cursor */
	ROM_LOAD( "3l-3m",        0x0500, 0x0200, 0x48d96000 ) /* sprite lookup table */

	ROM_REGION(0x10000)	/* 64k for the second CPU */
	ROM_LOAD( "xe-4c-a.bin",  0x0000, 0x2000, 0x0ede5706 )

	ROM_REGION(0x10000)	/* 64k for the audio CPU */
	ROM_LOAD( "xe-2c-a.bin",  0x0000, 0x1000, 0xdd35cf1c )

	ROM_REGION(0x4000)	/* gfx map */
	ROM_LOAD( "xe-2a.bin",    0x0000, 0x1000, 0x57ed9879 )
	ROM_LOAD( "xe-2b.bin",    0x1000, 0x2000, 0xae3ba9e5 )
	ROM_LOAD( "xe-2c.bin",    0x3000, 0x1000, 0x31e244dd )

	ROM_REGION(0x0100)	/* sound prom */
	ROM_LOAD( "xevious.spr",  0x0000, 0x0100, 0xa9fe382c )
ROM_END

ROM_START( xeviousa_rom )
	ROM_REGION(0x10000)	/* 64k for the first CPU */
	ROM_LOAD( "xea-1m-a.bin", 0x0000, 0x2000, 0x8c2b50ec )
	ROM_LOAD( "xea-1l-a.bin", 0x2000, 0x2000, 0x0821642b )

	ROM_REGION_DISPOSE(0xb000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "xe-3b.bin",    0x0000, 0x1000, 0x088c8b26 )	/* foreground characters */
	ROM_LOAD( "xe-3c.bin",    0x1000, 0x1000, 0xde60ba25 )	/* bg pattern B0 */
	ROM_LOAD( "xe-3d.bin",    0x2000, 0x1000, 0x535cdbbc )	/* bg pattern B1 */
	ROM_LOAD( "xe-4m.bin",    0x3000, 0x2000, 0xdc2c0ecb )	/* sprite set #1, planes 0/1 */
	ROM_LOAD( "xe-4r.bin",    0x5000, 0x2000, 0x02417d19 )	/* sprite set #1, plane 2, set #2, plane 0 */
	ROM_LOAD( "xe-4p.bin",    0x7000, 0x2000, 0xdfb587ce )	/* sprite set #2, planes 1/2 */
	ROM_LOAD( "xe-4n.bin",    0x9000, 0x1000, 0x605ca889 )	/* sprite set #3, planes 0/1 */
	/* 0xa000-0xafff empty space to decode sprite set #3 as 3 bits per pixel */

	ROM_REGION(0x0700)	/* color proms */
	ROM_LOAD( "xevious.6a",   0x0000, 0x0100, 0xa0334cef ) /* palette red component */
	ROM_LOAD( "xevious.6d",   0x0100, 0x0100, 0xa076a85c ) /* palette green component */
	ROM_LOAD( "xevious.6e",   0x0200, 0x0100, 0xc04737e5 ) /* palette blue component */
	ROM_LOAD( "4h-4f",        0x0300, 0x0200, 0xc21a65ff ) /* background tiles lookup table */
	/* These are probably not the original PROMs. The transparency information */
	/* (bit 7) is missing. The 0x80 bytes are mine, needed to fix the targeting cursor */
	ROM_LOAD( "3l-3m",        0x0500, 0x0200, 0x48d96000 ) /* sprite lookup table */

	ROM_REGION(0x10000)	/* 64k for the second CPU */
	ROM_LOAD( "xea-4c-a.bin", 0x0000, 0x2000, 0x14d8fa03 )

	ROM_REGION(0x10000)	/* 64k for the audio CPU */
	ROM_LOAD( "xe-2c-a.bin",  0x0000, 0x1000, 0xdd35cf1c )

	ROM_REGION(0x4000)	/* gfx map */
	ROM_LOAD( "xe-2a.bin",    0x0000, 0x1000, 0x57ed9879 )
	ROM_LOAD( "xe-2b.bin",    0x1000, 0x2000, 0xae3ba9e5 )
	ROM_LOAD( "xe-2c.bin",    0x3000, 0x1000, 0x31e244dd )

	ROM_REGION(0x0100)	/* sound prom */
	ROM_LOAD( "xevious.spr",  0x0000, 0x0100, 0xa9fe382c )
ROM_END

ROM_START( xevios_rom )
	ROM_REGION(0x10000)	/* 64k for the first CPU */
	ROM_LOAD( "4.7h",         0x0000, 0x1000, 0x1f8ca4c0 )
	ROM_LOAD( "5.6h",         0x1000, 0x1000, 0x2e47ce8f )
	ROM_LOAD( "6.5h",         0x2000, 0x1000, 0x79754b7d )
	ROM_LOAD( "7.4h",         0x3000, 0x1000, 0x7033f2e3 )

	ROM_REGION_DISPOSE(0xb000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "xe-3b.bin",    0x0000, 0x1000, 0x088c8b26 )	/* foreground characters */
	ROM_LOAD( "xe-3c.bin",    0x1000, 0x1000, 0xde60ba25 )	/* bg pattern B0 */
	ROM_LOAD( "xe-3d.bin",    0x2000, 0x1000, 0x535cdbbc )	/* bg pattern B1 */
	ROM_LOAD( "xe-4m.bin",    0x3000, 0x2000, 0xdc2c0ecb )	/* sprite set #1, planes 0/1 */
	ROM_LOAD( "16.8d",        0x5000, 0x2000, 0x44262c04 )	/* sprite set #1, plane 2, set #2, plane 0 */
	ROM_LOAD( "xe-4p.bin",    0x7000, 0x2000, 0xdfb587ce )	/* sprite set #2, planes 1/2 */
	ROM_LOAD( "xe-4n.bin",    0x9000, 0x1000, 0x605ca889 )	/* sprite set #3, planes 0/1 */
	/* 0xa000-0xafff empty space to decode sprite set #3 as 3 bits per pixel */

	ROM_REGION(0x0700)	/* color proms */
	ROM_LOAD( "xevious.6a",   0x0000, 0x0100, 0xa0334cef ) /* palette red component */
	ROM_LOAD( "xevious.6d",   0x0100, 0x0100, 0xa076a85c ) /* palette green component */
	ROM_LOAD( "xevious.6e",   0x0200, 0x0100, 0xc04737e5 ) /* palette blue component */
	ROM_LOAD( "4h-4f",        0x0300, 0x0200, 0xc21a65ff ) /* background tiles lookup table */
	/* These are probably not the original PROMs. The transparency information */
	/* (bit 7) is missing. The 0x80 bytes are mine, needed to fix the targeting cursor */
	ROM_LOAD( "3l-3m",        0x0500, 0x0200, 0x48d96000 ) /* sprite lookup table */

	ROM_REGION(0x10000)	/* 64k for the second CPU */
	ROM_LOAD( "xe-4c-a.bin",  0x0000, 0x2000, 0x0ede5706 )

	ROM_REGION(0x10000)	/* 64k for the audio CPU */
	ROM_LOAD( "xe-2c-a.bin",  0x0000, 0x1000, 0xdd35cf1c )

	ROM_REGION(0x4000)	/* gfx map */
	ROM_LOAD( "10.1d",        0x0000, 0x1000, 0x10baeebb )
	ROM_LOAD( "xe-2b.bin",    0x1000, 0x2000, 0xae3ba9e5 )
	ROM_LOAD( "12.3d",        0x3000, 0x1000, 0x51a4e83b )

	ROM_REGION(0x0100)	/* sound prom */
	ROM_LOAD( "xevious.spr",  0x0000, 0x0100, 0xa9fe382c )

	/* extra ROMs (function unknown, could be emulation of the custom I/O */
	/* chip with a Z80): */
	/* 1.16J */
	/* 2.17B */
ROM_END

ROM_START( sxevious_rom )
	ROM_REGION(0x10000)	/* 64k for the first CPU */
	ROM_LOAD( "cpu_3p.rom",   0x0000, 0x1000, 0x1c8d27d5 )
	ROM_LOAD( "cpu_3m.rom",   0x1000, 0x1000, 0xfd04e615 )
	ROM_LOAD( "cpu_2m.rom",   0x2000, 0x1000, 0x294d5404 )
	ROM_LOAD( "cpu_2l.rom",   0x3000, 0x1000, 0x6a44bf92 )

	ROM_REGION_DISPOSE(0xb000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "xe-3b.bin",    0x0000, 0x1000, 0x088c8b26 )	/* foreground characters */
	ROM_LOAD( "xe-3c.bin",    0x1000, 0x1000, 0xde60ba25 )	/* bg pattern B0 */
	ROM_LOAD( "xe-3d.bin",    0x2000, 0x1000, 0x535cdbbc )	/* bg pattern B1 */
	ROM_LOAD( "xe-4m.bin",    0x3000, 0x2000, 0xdc2c0ecb )	/* sprite set #1, planes 0/1 */
	ROM_LOAD( "xe-4r.bin",    0x5000, 0x2000, 0x02417d19 )	/* sprite set #1, plane 2, set #2, plane 0 */
	ROM_LOAD( "xe-4p.bin",    0x7000, 0x2000, 0xdfb587ce )	/* sprite set #2, planes 1/2 */
	ROM_LOAD( "xe-4n.bin",    0x9000, 0x1000, 0x605ca889 )	/* sprite set #3, planes 0/1 */
	/* 0xa000-0xafff empty space to decode sprite set #3 as 3 bits per pixel */

	ROM_REGION(0x0700)	/* color proms */
	ROM_LOAD( "xevious.6a",   0x0000, 0x0100, 0xa0334cef ) /* palette red component */
	ROM_LOAD( "xevious.6d",   0x0100, 0x0100, 0xa076a85c ) /* palette green component */
	ROM_LOAD( "xevious.6e",   0x0200, 0x0100, 0xc04737e5 ) /* palette blue component */
	ROM_LOAD( "4h-4f",        0x0300, 0x0200, 0xc21a65ff ) /* background tiles lookup table */
	/* These are probably not the original PROMs. The transparency information */
	/* (bit 7) is missing. The 0x80 bytes are mine, needed to fix the targeting cursor */
	ROM_LOAD( "3l-3m",        0x0500, 0x0200, 0x48d96000 ) /* sprite lookup table */

	ROM_REGION(0x10000)	/* 64k for the second CPU */
	ROM_LOAD( "cpu_3f.rom",   0x0000, 0x1000, 0xd4bd3d81 )
	ROM_LOAD( "cpu_3j.rom",   0x1000, 0x1000, 0xaf06be5f )

	ROM_REGION(0x10000)	/* 64k for the audio CPU */
	ROM_LOAD( "xe-2c-a.bin",  0x0000, 0x1000, 0xdd35cf1c )

	ROM_REGION(0x4000)	/* gfx map */
	ROM_LOAD( "xe-2a.bin",    0x0000, 0x1000, 0x57ed9879 )
	ROM_LOAD( "xe-2b.bin",    0x1000, 0x2000, 0xae3ba9e5 )
	ROM_LOAD( "xe-2c.bin",    0x3000, 0x1000, 0x31e244dd )

	ROM_REGION(0x0100)	/* sound prom */
	ROM_LOAD( "xevious.spr",  0x0000, 0x0100, 0xa9fe382c )
ROM_END



static void xevios_decode(void)
{
	int A,i;


	/* convert one of the sprite ROMs to the format used by Xevious */
	for (A = 0x5000;A < 0x7000;A++)
	{
		int bit[8];
		unsigned char *RAM = Machine->memory_region[1];

		/* 76543210 -> 13570246 bit rotation */
		for (i = 0;i < 8;i++)
			bit[i] = (RAM[A] >> i) & 1;

		RAM[A] =
			(bit[6] << 0) +
			(bit[4] << 1) +
			(bit[2] << 2) +
			(bit[0] << 3) +
			(bit[7] << 4) +
			(bit[5] << 5) +
			(bit[3] << 6) +
			(bit[1] << 7);
	}

	/* convert one of tile map ROMs to the format used by Xevious */
	for (A = 0x0000;A < 0x1000;A++)
	{
		int bit[8];
		unsigned char *RAM = Machine->memory_region[5];

		/* 76543210 -> 37512640 bit rotation */
		for (i = 0;i < 8;i++)
			bit[i] = (RAM[A] >> i) & 1;

		RAM[A] =
			(bit[0] << 0) +
			(bit[4] << 1) +
			(bit[6] << 2) +
			(bit[2] << 3) +
			(bit[1] << 4) +
			(bit[5] << 5) +
			(bit[7] << 6) +
			(bit[3] << 7);
	}
}



static const char *xevious_sample_names[] =
{
	"*xevious",
	"explo1.sam",	/* ground target explosion */
	"explo2.sam",	/* Solvalou explosion */
	0	/* end of array */
};



static int hiload(void) /* V.V */
{
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];


	/* check if the hi score table has already been initialized */
	if (memcmp(&RAM[0x8024],"\x00\x40\x00",3) == 0 &&
	    memcmp(&RAM[0x8510],"\x00\x40\x00",3) == 0)
	{
		void *f;


		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
			osd_fread(f,&RAM[0x8024],3);
			osd_fread(f,&RAM[0x8510],16*5);
			osd_fclose(f);
		}

		return 1;
	}
	else return 0;  /* we can't load the hi scores yet */
}



static void hisave(void) /* V.V */
{
	void *f;
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];


	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
		osd_fwrite(f,&RAM[0x8024],3);
		osd_fwrite(f,&RAM[0x8510],16*5);
		osd_fclose(f);
	}
}



struct GameDriver xevious_driver =
{
	__FILE__,
	0,
	"xevious",
	"Xevious (Namco)",
	"1982",
	"Namco",
	"Mirko Buffoni\nTatsuyuki Satoh\nNicola Salmoria",
	0,
	&machine_driver,
	0,

	xevious_rom,
	0, 0,
	xevious_sample_names,
	0,	/* sound_prom */

	xevious_input_ports,

	PROM_MEMORY_REGION(2), 0, 0,
	ORIENTATION_DEFAULT,

	hiload, hisave
};

struct GameDriver xeviousa_driver =
{
	__FILE__,
	&xevious_driver,
	"xeviousa",
	"Xevious (Atari)",
	"1982",
	"Namco (Atari license)",
	"Mirko Buffoni\nTatsuyuki Satoh\nNicola Salmoria",
	0,
	&machine_driver,
	0,

	xeviousa_rom,
	0, 0,
	xevious_sample_names,
	0,	/* sound_prom */

	xeviousa_input_ports,

	PROM_MEMORY_REGION(2), 0, 0,
	ORIENTATION_DEFAULT,

	hiload, hisave
};

struct GameDriver xevios_driver =
{
	__FILE__,
	&xevious_driver,
	"xevios",
	"Xevios",
	"1983",
	"bootleg",
	"Mirko Buffoni\nTatsuyuki Satoh\nNicola Salmoria",
	0,
	&machine_driver,
	0,

	xevios_rom,
	xevios_decode, 0,
	xevious_sample_names,
	0,	/* sound_prom */

	xevious_input_ports,

	PROM_MEMORY_REGION(2), 0, 0,
	ORIENTATION_DEFAULT,

	hiload, hisave
};

struct GameDriver sxevious_driver =
{
	__FILE__,
	&xevious_driver,
	"sxevious",
	"Super Xevious",
	"1984",
	"Namco",
	"Mirko Buffoni\nTatsuyuki Satoh\nNicola Salmoria",
	0,
	&machine_driver,
	0,

	sxevious_rom,
	0, 0,
	xevious_sample_names,
	0,	/* sound_prom */

	sxevious_input_ports,

	PROM_MEMORY_REGION(2), 0, 0,
	ORIENTATION_DEFAULT,

	hiload, hisave
};
