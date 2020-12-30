/******************************************************************
Cabal Bootleg
(c)1998 Red Corp

68000 + Z80

The original uses 2xYM3931 for sound
The bootleg uses YM2151 + 2xZ80 used as ADPCM players

TODO:
- fix ADPCM mapping

MEMORY MAP
0x000000 - 0x03ffff   ROM
0x040000 - 0x0437ff   RAM
0x043800 - 0x0437ff   VRAM (Sprites)
0x060000 - 0x0607ff   VRAM (Tiles)
0x080000 - 0x0803ff   VRAM (Background)
0x0A0000 - 0xA0000f   Input Ports
0x0C0040 - 0x0c0040   Watchdog??
0x0C0080 - 0x0C0080   Watchdog??
0x0E0000 - 0x0E07ff   COLORRAM (----BBBBGGGGRRRR)
0x0E8000 - 0x0E800f   Output Ports/Input Ports

VRAM(Background)
0x80000 - 32 bytes (16 tiles)
0x80040 - 32 bytes (16 tiles)
0x80080 - 32 bytes (16 tiles)
0x800c0 - 32 bytes (16 tiles)
0x80100 - 32 bytes (16 tiles)
...
0x803c0 - 32 bytes (16 tiles)

VRAM(Tiles)
0x60000-0x607ff (1024 tiles 8x8 tiles, 2 bytes every tile)

VRAM(Sprites)
0x43800-0x43bff (128 sprites, 8 bytes every sprite)

COLORRAM(Colors)
0xe0000-0xe07ff (1024 colors, ----BBBBGGGGRRRR)

******************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"
#include "z80/z80.h"

extern void cabal_vh_screenrefresh( struct osd_bitmap *bitmap, int fullrefresh );


static unsigned char cabal_snd_status;

static void cabal_init_machine( void ) {
	cabal_snd_status = 0xff;
}

static int cabal_background_r( int offset ){
	return READ_WORD (&videoram[offset]);
}

static void cabal_background_w( int offset, int data ){
	int oldword = READ_WORD(&videoram[offset]);
	int newword = COMBINE_WORD(oldword,data);
	if( oldword != newword ){
		WRITE_WORD(&videoram[offset],newword);
		dirtybuffer[offset/2] = 1;
	}
}

static void play_adpcm( int channel, int which ){
	static int j = 0;

	unsigned char *RAM = Machine->memory_region[3];
	int offset = channel*0x10000;
	which+=j;
	which = which*2+0x100;
	{
		int start = RAM[offset+which] + RAM[offset+which+1]*256;
		int len = (RAM[offset+start]*256 + RAM[offset+start+1])*2;
		start+=2;
		ADPCM_play( channel,offset+start,len );
	}
}

static int cabal_coin_r( int offset ) {
	static int coin = 0;
	int val = readinputport( 3 );

	if ( !( val & 0x04 ) ) {
		if ( coin == 0 ) {
			coin = 1;
			return val;
		}
	} else
		coin = 0;

 	return val | 0x04;
}

static int cabal_io_r( int offset ) {
	switch (offset){
		case 0x0:  /* DIPSW */
			return readinputport(4) + (readinputport(5)<<8);

		case 0x8: /* IN0 */
			return 0xff + (readinputport(0)<<8);

		case 0x10: /* IN1,IN2 */
			return readinputport(1) + (readinputport(2)<<8);

		default:
			return (0xffff);
	}
}

static void cabal_snd_w( int offset,int data ) {
	switch (offset) {
		case 0x0:
			soundlatch_w(offset,data);
			cpu_cause_interrupt( 1, Z80_NMI_INT );
		break;

		case 0x2:
			cabal_snd_status = data & 0xff;
		break;

		case 0x8:
		break;
	}
}

static struct MemoryReadAddress readmem_cpu[] = {
	{ 0x00000, 0x3ffff, MRA_ROM },
	{ 0x40000, 0x437ff, MRA_RAM },
	{ 0x43c00, 0x4ffff, MRA_RAM },
	{ 0x43800, 0x43bff, MRA_RAM, &spriteram, &spriteram_size },	/* Sprite RAM */
	{ 0x60000, 0x607ff, MRA_BANK1,&colorram },  /* Text   RAM */
	{ 0x80000, 0x801ff, cabal_background_r, &videoram, &videoram_size },   /* Background RAM */
	{ 0x80200, 0x803ff, MRA_BANK2 },
	{ 0xa0000, 0xa0012, cabal_io_r },
	{ 0xe0000, 0xe07ff, paletteram_word_r },    /* Color  RAM */
	{ 0xe8000, 0xe800f, cabal_coin_r },
	{ -1 }
};
static struct MemoryWriteAddress writemem_cpu[] = {
	{ 0x00000, 0x3ffff, MWA_ROM },
	{ 0x40000, 0x43bff, MWA_RAM },
	{ 0x43c00, 0x4ffff, MWA_RAM },
	{ 0x60000, 0x607ff, MWA_BANK1 },
	{ 0x80000, 0x801ff, cabal_background_w },
	{ 0x80200, 0x803ff, MWA_BANK2 },
	{ 0xc0040, 0xc0041, MWA_NOP }, /* ??? */
	{ 0xc0080, 0xc0081, MWA_NOP }, /* ??? */
	{ 0xe0000, 0xe07ff, paletteram_xxxxBBBBGGGGRRRR_word_w, &paletteram},
	{ 0xe8000, 0xe800f, cabal_snd_w },
	{ -1 }
};

/*********************************************************************/

int cabal_snd_read(int offset){
	switch(offset){
		case 0x08: return(cabal_snd_status);
		case 0x0a:
		return (soundlatch_r(offset));
		default:
		return(0xff);
	}
}

void cabal_snd_write(int offset,int data){
	switch( offset ){
		case 0x00:  /* Channel 0 */
			data = data&0x7f;
			if ( data != 2 && data > 0 && data < 16 ) {
				play_adpcm( 0, data );
			}
		break;

		case 0x02:  /* Channel 1 */
			data = data&0x7f;
			if( data > 0 && data < 16 ){
				play_adpcm( 1, data );
			}
		break;
     }
}



static struct MemoryReadAddress readmem_sound[] =
{
	{ 0x0000, 0x1fff, MRA_ROM },
	{ 0x2000, 0x2fff, MRA_RAM },
	{ 0x8000, 0xffff, MRA_ROM },
	{ -1 }
};

static struct MemoryWriteAddress writemem_sound[] =
{
	{ 0x0000, 0x1fff, MWA_ROM },
	{ 0x2000, 0x2fff, MWA_RAM },
	{ 0x8000, 0xffff, MWA_ROM },
 	{ -1 }
};

static struct MemoryReadAddress cabalbl_readmem_sound[] =
{
	{ 0x0000, 0x1fff, MRA_ROM },
	{ 0x2000, 0x2fff, MRA_RAM },
	{ 0x4000, 0x400d, cabal_snd_read },
	{ 0x400f, 0x400f, YM2151_status_port_0_r },
	{ 0x8000, 0xffff, MRA_ROM },
	{ -1 }
};

static struct MemoryWriteAddress cabalbl_writemem_sound[] =
{
	{ 0x0000, 0x1fff, MWA_ROM },
	{ 0x2000, 0x2fff, MWA_RAM },
	{ 0x4000, 0x400d, cabal_snd_write },
	{ 0x400e, 0x400e, YM2151_register_port_0_w },
	{ 0x400f, 0x400f, YM2151_data_port_0_w },
	{ 0x6000, 0x6000, MWA_NOP },  /*???*/
	{ 0x8000, 0xffff, MWA_ROM },
 	{ -1 }
};

/***************************************************************************/

INPUT_PORTS_START( input_ports )
	PORT_START	/* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )

	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* IN2 */
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START	/* IN3 */
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* DIPSW1 */
	PORT_DIPNAME( 0x0f, 0x0f, "Coinage", IP_KEY_NONE )
	PORT_DIPSETTING(    0x0a, "6C_1C" )
	PORT_DIPSETTING(    0x0b, "5C_1C" )
	PORT_DIPSETTING(    0x0c, "4C_1C" )
	PORT_DIPSETTING(    0x0d, "3C_1C" )
	PORT_DIPSETTING(    0x0e, "2C_1C" )
	PORT_DIPSETTING(    0x0f, "1C_1C" )
	PORT_DIPSETTING(    0x09, "1C_2C" )
	PORT_DIPSETTING(    0x08, "1C_3C" )
	PORT_DIPSETTING(    0x07, "1C_4C" )
	PORT_DIPSETTING(    0x06, "1C_5C" )
	PORT_DIPSETTING(    0x05, "1C_6C" )
	PORT_DIPSETTING(    0x04, "1C_7C" )
	PORT_DIPSETTING(    0x03, "1C_8C" )
	PORT_DIPSETTING(    0x02, "1C_9C" )
	PORT_DIPSETTING(    0x01, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x00, "Free_Play" )
	PORT_DIPNAME( 0x10, 0x10, "Coin Mode", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Type 1" )
	PORT_DIPSETTING(    0x00, "Type 2" )
	PORT_DIPNAME( 0x20, 0x20, "Buttons", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Normal" )
	PORT_DIPSETTING(    0x00, "Invert" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(	0x40, "Off" )
	PORT_DIPSETTING(	0x00, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Trackball", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Small" )
	PORT_DIPSETTING(    0x00, "Large" )

	PORT_START	/* DIPSW2 */
	PORT_DIPNAME( 0x03, 0x03, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_BITX( 0,       0x00, IPT_DIPSWITCH_SETTING | IPF_CHEAT, "Infinite", IP_KEY_NONE, IP_JOY_NONE,0 )
	PORT_DIPNAME( 0x0c, 0x0c, "Bonus_Life", IP_KEY_NONE )
	PORT_DIPSETTING(    0x0c, "20k 50k" )
	PORT_DIPSETTING(    0x08, "30k 100k" )
	PORT_DIPSETTING(    0x04, "50k 150k" )
	PORT_DIPSETTING(    0x00, "70K" )
	PORT_DIPNAME( 0x30, 0x30, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x30, "Easy" )
	PORT_DIPSETTING(    0x20, "Normal" )
	PORT_DIPSETTING(    0x10, "Hard" )
	PORT_DIPSETTING(    0x00, "Very Hard" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(	0x40, "Off" )
	PORT_DIPSETTING(	0x00, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Demo_Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x80, "On" )
INPUT_PORTS_END



static struct GfxLayout text_layout =
{
	8,8,	/* 8*8 characters */
	1024,	/* 1024 characters */
	2,	/* 4 bits per pixel */
	{ 0,4 },
	{ 3, 2, 1, 0, 8+3, 8+2, 8+1, 8+0},
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8	/* every char takes 16 consecutive bytes */
};

static struct GfxLayout tile_layout =
{
	16,16,  /* 16*16 tiles */
	4*1024,   /* 1024 tiles */
	4,      /* 4 bits per pixel */
	{ 0, 4, 0x40000*8+0, 0x40000*8+4 },
	{ 3, 2, 1, 0, 8+3, 8+2, 8+1, 8+0,
			32*8+3, 32*8+2, 32*8+1, 32*8+0, 33*8+3, 33*8+2, 33*8+1, 33*8+0 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8
};

static struct GfxLayout sprite_layout =
{
	16,16,	/* 16*16 sprites */
	4*1024,	/* 1024 sprites */
	4,	/* 4 bits per pixel */
	{ 0, 4, 0x40000*8+0, 0x40000*8+4 },	/* the two bitplanes for 4 pixels are packed into one byte */
	{ 3, 2, 1, 0, 8+3, 8+2, 8+1, 8+0,
			16+3, 16+2, 16+1, 16+0, 24+3, 24+2, 24+1, 24+0 },
	{ 30*16, 28*16, 26*16, 24*16, 22*16, 20*16, 18*16, 16*16,
			14*16, 12*16, 10*16,  8*16,  6*16,  4*16,  2*16,  0*16 },
	64*8	/* every sprite takes 64 consecutive bytes */
};

static struct GfxDecodeInfo cabal_gfxdecodeinfo[] = {
	{ 2,      0x100000, &text_layout,		0, 1024/4 },
	{ 2,      0x000000, &tile_layout,		32*16, 16 },
	{ 2,      0x080000, &sprite_layout,		16*16, 16 },
	{ -1 }
};

static struct YM2151interface ym2151_interface =
{
 	1,			/* 1 chip */
 	3579580,	/* 3.58 MHZ ? */ /* 4 MHZ in raine */
 	{ 40 },
 	{ 0 }
};

static struct ADPCMinterface adpcm_interface =
{
	2,			/* 2 channel */
	8000,		/* 8000Hz playback */
	3,			/* memory region */
	0,			/* init function */
	{ 128,128 }		/* volume */
};

static struct MachineDriver cabal_machine_driver =
{
	{
		{
			CPU_M68000,
			12000000, /* 12 Mhz */
			0,
			readmem_cpu,writemem_cpu,0,0,
			m68_level1_irq,1
		},
		{
			CPU_Z80 | CPU_AUDIO_CPU,
			4000000,	/* 4 Mhz */
			1,
			readmem_sound,writemem_sound,0,0,
			interrupt,1
		},
	},
	60, DEFAULT_REAL_60HZ_VBLANK_DURATION,
	1, /* CPU slices per frame */
	cabal_init_machine, /* init machine */

	/* video hardware */
	256, 256, { 0, 255, 16, 255-16 },

	cabal_gfxdecodeinfo,
	1024,1024,
	0,

	VIDEO_TYPE_RASTER | VIDEO_MODIFIES_PALETTE,
	0,
	generic_vh_start,
	generic_vh_stop,
	cabal_vh_screenrefresh,

	/* sound hardware */
	0,0,0,0,
};

static struct MachineDriver cabalbl_machine_driver =
{
	{
		{
			CPU_M68000,
			12000000, /* 12 Mhz */
			0,
			readmem_cpu,writemem_cpu,0,0,
			m68_level1_irq,1
		},
		{
			CPU_Z80 | CPU_AUDIO_CPU,
			4000000,	/* 4 Mhz */
			1,
			cabalbl_readmem_sound,cabalbl_writemem_sound,0,0,
			interrupt,1
		},
	},
	60, DEFAULT_REAL_60HZ_VBLANK_DURATION,
	1, /* CPU slices per frame */
	cabal_init_machine, /* init machine */

	/* video hardware */
	256, 256, { 0, 255, 16, 255-16 },

	cabal_gfxdecodeinfo,
	1024,1024,
	0,

	VIDEO_TYPE_RASTER | VIDEO_MODIFIES_PALETTE,
	0,
	generic_vh_start,
	generic_vh_stop,
	cabal_vh_screenrefresh,

	/* sound hardware */
	0,0,0,0,
	{
		{
			SOUND_YM2151_ALT,
			&ym2151_interface
		},
		{
			SOUND_ADPCM,
			&adpcm_interface
		}
	}
};



ROM_START( cabal_rom )
	ROM_REGION(0x50000)	/* 64k for cpu code */
	ROM_LOAD_EVEN( "h7_512.bin",      0x00000, 0x10000, 0x8fe16fb4 )
	ROM_LOAD_ODD ( "h6_512.bin",      0x00000, 0x10000, 0x6968101c )
	ROM_LOAD_EVEN( "k7_512.bin",      0x20000, 0x10000, 0x562031a2 )
	ROM_LOAD_ODD ( "k6_512.bin",      0x20000, 0x10000, 0x4fda2856 )

	ROM_REGION(0x10000)	/* 64k for sound cpu code */
	ROM_LOAD( "4-3n",           0x0000, 0x2000, 0x4038eff2 )
	ROM_LOAD( "3-3p",           0x8000, 0x8000, 0xd9defcbf )

	ROM_REGION_DISPOSE(0x104000)
	/* the gfx ROMs were missing in this set, I'm using the ones from */
	/* the bootleg (apart from the characters) */
	ROM_LOAD( "cabal_17.bin",   0x000000, 0x10000, 0x3b6d2b09 ) /* tiles, planes0,1 */
	ROM_LOAD( "cabal_16.bin",   0x010000, 0x10000, 0x77bc7a60 )
	ROM_LOAD( "cabal_18.bin",   0x020000, 0x10000, 0x0bc50075 )
	ROM_LOAD( "cabal_19.bin",   0x030000, 0x10000, 0x67e4fe47 )
	ROM_LOAD( "cabal_15.bin",   0x040000, 0x10000, 0x1023319b ) /* tiles, planes2,3 */
	ROM_LOAD( "cabal_14.bin",   0x050000, 0x10000, 0x420b0801 )
	ROM_LOAD( "cabal_12.bin",   0x060000, 0x10000, 0x543fcb37 )
	ROM_LOAD( "cabal_13.bin",   0x070000, 0x10000, 0xd28d921e )
	ROM_LOAD( "cabal_05.bin",   0x080000, 0x10000, 0x4e49c28e ) /* sprites, planes0,1 */
	ROM_LOAD( "cabal_06.bin",   0x090000, 0x10000, 0x6a0e739d )
	ROM_LOAD( "cabal_07.bin",   0x0a0000, 0x10000, 0x581a50c1 )
	ROM_LOAD( "cabal_08.bin",   0x0b0000, 0x10000, 0x702735c9 )
	ROM_LOAD( "cabal_04.bin",   0x0c0000, 0x10000, 0x34d3cac8 ) /* sprites, planes2,3 */
	ROM_LOAD( "cabal_03.bin",   0x0d0000, 0x10000, 0x7065e840 )
	ROM_LOAD( "cabal_02.bin",   0x0e0000, 0x10000, 0x0e1ec30e )
	ROM_LOAD( "cabal_01.bin",   0x0f0000, 0x10000, 0x55c44764 )
	ROM_LOAD( "t6_128.bin",     0x100000, 0x04000, 0x1ccee214 ) /* characters */

	ROM_REGION(0x20000)	/* Samples? */
	ROM_LOAD( "2-1s",            0x00000, 0x10000, 0x850406b4 )
	ROM_LOAD( "1-1u",            0x10000, 0x10000, 0x8b3e0789 )
ROM_END

ROM_START( cabal2_rom )
	ROM_REGION(0x50000)	/* 64k for cpu code */
	ROM_LOAD_EVEN( "9-7h",            0x00000, 0x10000, 0xebbb9484 )
	ROM_LOAD_ODD ( "7-6h",            0x00000, 0x10000, 0x51aeb49e )
	ROM_LOAD_EVEN( "8-7k",            0x20000, 0x10000, 0x4c24ed9a )
	ROM_LOAD_ODD ( "6-6k",            0x20000, 0x10000, 0x681620e8 )

	ROM_REGION(0x10000)	/* 64k for sound cpu code */
	ROM_LOAD( "4-3n",           0x0000, 0x2000, 0x4038eff2 )
	ROM_LOAD( "3-3p",           0x8000, 0x8000, 0xd9defcbf )

	ROM_REGION_DISPOSE(0x104000)
	/* the gfx ROMs were missing in this set, I'm using the ones from */
	/* the bootleg (apart from the characters) */
	ROM_LOAD( "cabal_17.bin",   0x000000, 0x10000, 0x3b6d2b09 ) /* tiles, planes0,1 */
	ROM_LOAD( "cabal_16.bin",   0x010000, 0x10000, 0x77bc7a60 )
	ROM_LOAD( "cabal_18.bin",   0x020000, 0x10000, 0x0bc50075 )
	ROM_LOAD( "cabal_19.bin",   0x030000, 0x10000, 0x67e4fe47 )
	ROM_LOAD( "cabal_15.bin",   0x040000, 0x10000, 0x1023319b ) /* tiles, planes2,3 */
	ROM_LOAD( "cabal_14.bin",   0x050000, 0x10000, 0x420b0801 )
	ROM_LOAD( "cabal_12.bin",   0x060000, 0x10000, 0x543fcb37 )
	ROM_LOAD( "cabal_13.bin",   0x070000, 0x10000, 0xd28d921e )
	ROM_LOAD( "cabal_05.bin",   0x080000, 0x10000, 0x4e49c28e ) /* sprites, planes0,1 */
	ROM_LOAD( "cabal_06.bin",   0x090000, 0x10000, 0x6a0e739d )
	ROM_LOAD( "cabal_07.bin",   0x0a0000, 0x10000, 0x581a50c1 )
	ROM_LOAD( "cabal_08.bin",   0x0b0000, 0x10000, 0x702735c9 )
	ROM_LOAD( "cabal_04.bin",   0x0c0000, 0x10000, 0x34d3cac8 ) /* sprites, planes2,3 */
	ROM_LOAD( "cabal_03.bin",   0x0d0000, 0x10000, 0x7065e840 )
	ROM_LOAD( "cabal_02.bin",   0x0e0000, 0x10000, 0x0e1ec30e )
	ROM_LOAD( "cabal_01.bin",   0x0f0000, 0x10000, 0x55c44764 )
	ROM_LOAD( "5-6s",           0x100000, 0x04000, 0x6a76955a ) /* characters */

	ROM_REGION(0x20000)	/* Samples? */
	ROM_LOAD( "2-1s",            0x00000, 0x10000, 0x850406b4 )
	ROM_LOAD( "1-1u",            0x10000, 0x10000, 0x8b3e0789 )
ROM_END

ROM_START( cabalbl_rom )
	ROM_REGION(0x50000)	/* 64k for cpu code */
	ROM_LOAD_EVEN( "cabal_24.bin",    0x00000, 0x10000, 0x00abbe0c )
	ROM_LOAD_ODD ( "cabal_22.bin",    0x00000, 0x10000, 0x78c4af27 )
	ROM_LOAD_EVEN( "cabal_23.bin",    0x20000, 0x10000, 0xd763a47c )
	ROM_LOAD_ODD ( "cabal_21.bin",    0x20000, 0x10000, 0x96d5e8af )

	ROM_REGION(0x10000)	/* 64k for sound cpu code */
	ROM_LOAD( "cabal_11.bin",    0x0000, 0x10000, 0xd308a543 )

	ROM_REGION_DISPOSE(0x104000)
	ROM_LOAD( "cabal_17.bin",   0x000000, 0x10000, 0x3b6d2b09 ) /* tiles, planes0,1 */
	ROM_LOAD( "cabal_16.bin",   0x010000, 0x10000, 0x77bc7a60 )
	ROM_LOAD( "cabal_18.bin",   0x020000, 0x10000, 0x0bc50075 )
	ROM_LOAD( "cabal_19.bin",   0x030000, 0x10000, 0x67e4fe47 )
	ROM_LOAD( "cabal_15.bin",   0x040000, 0x10000, 0x1023319b ) /* tiles, planes2,3 */
	ROM_LOAD( "cabal_14.bin",   0x050000, 0x10000, 0x420b0801 )
	ROM_LOAD( "cabal_12.bin",   0x060000, 0x10000, 0x543fcb37 )
	ROM_LOAD( "cabal_13.bin",   0x070000, 0x10000, 0xd28d921e )
	ROM_LOAD( "cabal_05.bin",   0x080000, 0x10000, 0x4e49c28e ) /* sprites, planes0,1 */
	ROM_LOAD( "cabal_06.bin",   0x090000, 0x10000, 0x6a0e739d )
	ROM_LOAD( "cabal_07.bin",   0x0a0000, 0x10000, 0x581a50c1 )
	ROM_LOAD( "cabal_08.bin",   0x0b0000, 0x10000, 0x702735c9 )
	ROM_LOAD( "cabal_04.bin",   0x0c0000, 0x10000, 0x34d3cac8 ) /* sprites, planes2,3 */
	ROM_LOAD( "cabal_03.bin",   0x0d0000, 0x10000, 0x7065e840 )
	ROM_LOAD( "cabal_02.bin",   0x0e0000, 0x10000, 0x0e1ec30e )
	ROM_LOAD( "cabal_01.bin",   0x0f0000, 0x10000, 0x55c44764 )
	ROM_LOAD( "5-6s",           0x100000, 0x04000, 0x6a76955a ) /* characters */

	ROM_REGION(0x20000)	/* ADPCM Samples */
	ROM_LOAD( "cabal_09.bin",    0x00000, 0x10000, 0x4ffa7fe3 )
	ROM_LOAD( "cabal_10.bin",    0x10000, 0x10000, 0x958789b6 )
ROM_END



struct GameDriver cabal_driver =
{
	__FILE__,
	0,
	"cabal",
	"Cabal (US set 1)",
	"1988",
	"Tad (Fabtek license)",
	"Carlos A. Lozano Baides\nPhil Stroffolino\nRichard Bush\nErnesto Corvi\n",
	GAME_NOT_WORKING,
	&cabal_machine_driver,
	0,

	cabal_rom,
	0,0,
	0,
	0, /* sound_prom */

	input_ports,

	0,0,0,
	ORIENTATION_DEFAULT,

	0,0
};

struct GameDriver cabal2_driver =
{
	__FILE__,
	&cabal_driver,
	"cabal2",
	"Cabal (US set 2)",
	"1988",
	"Tad (Fabtek license)",
	"Carlos A. Lozano Baides\nPhil Stroffolino\nRichard Bush\nErnesto Corvi\n",
	GAME_NOT_WORKING,
	&cabal_machine_driver,
	0,

	cabal2_rom,
	0,0,
	0,
	0, /* sound_prom */

	input_ports,

	0,0,0,
	ORIENTATION_DEFAULT,

	0,0
};

struct GameDriver cabalbl_driver =
{
	__FILE__,
	&cabal_driver,
	"cabalbl",
	"Cabal (bootleg)",
	"1988",
	"bootleg",
	"Carlos A. Lozano Baides\nPhil Stroffolino\nRichard Bush\nErnesto Corvi\n",
	0,
	&cabalbl_machine_driver,
	0,

	cabalbl_rom,
	0,0,
	0,
	0, /* sound_prom */

	input_ports,

	0,0,0,
	ORIENTATION_DEFAULT,

	0,0
};
