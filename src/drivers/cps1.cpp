/***************************************************************************

  Capcom System 1
  ===============

  Driver provided by:
	Paul Leaman

  M680000 for game, Z80, YM-2151 and OKIM6295 for sound.

  68000 clock speeds are unknown for all games (except where commented)

***************************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"
#include "sndhrdw/qsound.h"

#include "cps1.h"       /* External CPS1 definitions */

/* in machine/kabuki.c */
void wof_decode(void);
void dino_decode(void);
void punisher_decode(void);
void slammast_decode(void);


#ifndef CPS1_VIDEO_HZ_REDUCTION
#define CPS1_VIDEO_HZ 60
#else
#define CPS1_VIDEO_HZ 50
#endif

int cps1_input2_r(int offset)
{
	  int buttons=readinputport(6);
	  return buttons << 8 | buttons;
}

static int cps1_sound_fade_timer;

void cps1_snd_bankswitch_w(int offset,int data)
{
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[1].memory_region];

	cpu_setbank (1, &RAM[0x10000+(data&0x01)*0x4000]);
}

void cps1_sound_fade_w(int offset, int data)
{
	cps1_sound_fade_timer=data;
}

int cps1_snd_fade_timer_r(int offset)
{
	return cps1_sound_fade_timer;
}


/********************************************************************
*
*  Q Sound
*  =======
*
********************************************************************/

static struct QSound_interface qsound_interface =
{
	QSOUND_CLOCK,
	3,
	{ 100,100 }
};

static unsigned char *qsound_sharedram1, *qsound_sharedram2;;

static int qsound_sharedram1_r(int offset)
{
	return qsound_sharedram1[offset / 2] | 0xff00;
}

static void qsound_sharedram1_w(int offset,int data)
{
	qsound_sharedram1[offset / 2] = data;
}


static int qsound_sharedram2_r(int offset)
{
	return qsound_sharedram2[offset / 2] | 0xff00;
}

static void qsound_sharedram2_w(int offset,int data)
{
	qsound_sharedram2[offset / 2] = data;
}

static int cps1_qsound_interrupt(void)
{
	if (Machine->sample_rate == 0)
		qsound_sharedram1[0xfff] = 0x77;
	return 2;
}

static void qsound_banksw_w(int offset,int data)
{
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[1].memory_region];
	int bankaddress=0x10000+((data&0x0f)*0x4000);
	if (bankaddress >= Machine->memory_region_length[Machine->drv->cpu[1].memory_region])
		bankaddress=0x10000;
	cpu_setbank(1, &RAM[bankaddress]);
}


/********************************************************************
*
*  EEPROM
*  ======
*
*   The EEPROM appears to be accessed by a serial protocol using
*   the register 0xf1c006
*
********************************************************************/

static unsigned char cps1_eeprom[0x40];
static unsigned int  cps1_eeprom_register;

int cps1_eeprom_port_r(int offset)
{
	return (cps1_eeprom_register&0xfe) | 0x01;
}

void cps1_eeprom_port_w(int offset, int data)
{
	cps1_eeprom_register=data;
}

static void cps1_eeprom_save (void)
{
	void *f;

	f = osd_fopen (Machine->gamedrv->name, 0, OSD_FILETYPE_HIGHSCORE, 1);
	if (f)
	{
		osd_fwrite (f, cps1_eeprom, sizeof(cps1_eeprom));
		osd_fclose (f);
	}
}

static int cps1_eeprom_load (void)
{
	void *f;

	f = osd_fopen (Machine->gamedrv->name, 0, OSD_FILETYPE_HIGHSCORE, 0);
	if (f)
	{
		osd_fread (f, cps1_eeprom, sizeof(cps1_eeprom));
		osd_fclose (f);
	}
	else
	{
		fast_memset(cps1_eeprom, 0, sizeof(cps1_eeprom));
	}

	return 1;
}

static int cps1_input_r(int offset)
{
       int control=readinputport (offset/2);
       return (control<<8) | control;
}

static int cps1_player_input_r(int offset)
{
       return (readinputport (offset + 4 )+
		(readinputport (offset+1 + 4 )<<8));
}

static struct MemoryReadAddress cps1_readmem[] =
{
	{ 0x000000, 0x1fffff, MRA_ROM },     /* 68000 ROM */
	{ 0x800000, 0x800003, cps1_player_input_r }, /* Player input ports */
	{ 0x800010, 0x800013, cps1_player_input_r }, /* ?? */
	{ 0x800018, 0x80001f, cps1_input_r }, /* Input ports */
	{ 0x800020, 0x800021, MRA_NOP }, /* ? Used by Rockman ? */
	{ 0x800176, 0x800177, cps1_input2_r }, /* Extra input ports */
	{ 0x8001fc, 0x8001fc, cps1_input2_r }, /* Input ports (SF Rev E) */
	{ 0x800100, 0x8001ff, cps1_output_r },   /* Output ports */
	{ 0x900000, 0x92ffff, cps1_gfxram_r },
	{ 0xf18000, 0xf19fff, qsound_sharedram1_r },    /* Q RAM */
	{ 0xf1c000, 0xf1c005, MRA_NOP }, /* Unknown */
	{ 0xf1c006, 0xf1c007, cps1_eeprom_port_r },
	{ 0xf1e000, 0xf1ffff, qsound_sharedram2_r },    /* Q RAM */
	{ 0xff0000, 0xffffff, MRA_BANK2 },   /* RAM */
	{ -1 }  /* end of table */
};

static int cps1_ram_size;
static unsigned char *cps1_ram;

static struct MemoryWriteAddress cps1_writemem[] =
{
	{ 0x000000, 0x1fffff, MWA_ROM },      /* ROM */
	{ 0x800030, 0x800033, MWA_NOP },      /* ??? Unknown ??? */
	{ 0x800180, 0x800181, soundlatch_w },  /* Sound command */
	{ 0x800188, 0x80018b, cps1_sound_fade_w },
	{ 0x800100, 0x8001ff, cps1_output_w, &cps1_output, &cps1_output_size },  /* Output ports */
	{ 0x900000, 0x92ffff, cps1_gfxram_w, &cps1_gfxram, &cps1_gfxram_size },
	{ 0xf18000, 0xf19fff, qsound_sharedram1_w }, /* Q RAM */
	{ 0xf1c004, 0xf1c005, MWA_NOP }, /* Unknown */
	{ 0xf1c006, 0xf1c007, cps1_eeprom_port_w },
	{ 0xf1e000, 0xf1ffff, qsound_sharedram2_w }, /* Q RAM */
	{ 0xff0000, 0xffffff, MWA_BANK2, &cps1_ram, &cps1_ram_size },        /* RAM */
	{ -1 }  /* end of table */
};


static struct MemoryReadAddress sound_readmem[] =
{
	{ 0x0000, 0x7fff, MRA_ROM },
	{ 0x8000, 0xbfff, MRA_BANK1 },
	{ 0xd000, 0xd7ff, MRA_RAM },
	{ 0xf001, 0xf001, YM2151_status_port_0_r },
	{ 0xf002, 0xf002, OKIM6295_status_0_r },
	{ 0xf008, 0xf008, soundlatch_r },
	{ 0xf00a, 0xf00a, cps1_snd_fade_timer_r }, /* Sound timer fade */
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress sound_writemem[] =
{
	{ 0x0000, 0xbfff, MWA_ROM },
	{ 0xd000, 0xd7ff, MWA_RAM },
	{ 0xf000, 0xf000, YM2151_register_port_0_w },
	{ 0xf001, 0xf001, YM2151_data_port_0_w },
	{ 0xf002, 0xf002, OKIM6295_data_0_w },
	{ 0xf004, 0xf004, cps1_snd_bankswitch_w },
	{ 0xf006, 0xf006, MWA_NOP }, /* ???? Unknown ???? */
	{ -1 }  /* end of table */
};

static struct MemoryReadAddress qsound_readmem[] =
{
	{ 0x0000, 0x7fff, MRA_ROM },
	{ 0x8000, 0xbfff, MRA_BANK1 },
	{ 0xc000, 0xcfff, MRA_RAM },
	{ 0xd007, 0xd007, qsound_status_r },
	{ 0xf000, 0xffff, MRA_RAM },
	{ -1 } 
};

static struct MemoryWriteAddress qsound_writemem[] =
{
	{ 0x0000, 0xbfff, MWA_ROM },
	{ 0xc000, 0xcfff, MWA_RAM, &qsound_sharedram1 },
	{ 0xd000, 0xd000, qsound_data_h_w },
	{ 0xd001, 0xd001, qsound_data_l_w },
	{ 0xd002, 0xd002, qsound_cmd_w },
	{ 0xd003, 0xd003, qsound_banksw_w },
	{ 0xf000, 0xffff, MWA_RAM, &qsound_sharedram2 },
	{ -1 }  
};


/********************************************************************

			Configuration table:

********************************************************************/

static struct CPS1config cps1_config_table[]=
{
	/* DRIVER    BANK   BANK    BANK    START      CPSB CPSB
	  NAME      OBJ   SCRL1   SCRL2   SCRL3    A ADDR VAL */
	{"forgottn",0,      0,      0,      0,       5,0x00,0x0000}, /* ??? */
	{"ghouls",  0,      0,      0,      0,       5,0x00,0x0000, 1},
	{"ghoulsj", 0,      0,      0,      0,       5,0x00,0x0000, 1},
	{"strider", 0,      1,      0,      1,       4,0x00,0x0000},
	{"striderj",0,      1,      0,      1,       4,0x00,0x0000},
	{"dwj",     0,      0,      1,      1,       9,0x00,0x0000},
	{"willow",  0,      0,      1,      0,       1,0x00,0x0000},
	{"willowj", 0,      0,      1,      0,       1,0x00,0x0000},
	{"unsquad", 0,      0,      0,      0,       0,0x00,0x0000},
	{"area88",  0,      0,      0,      0,       0,0x00,0x0000},
	{"ffight",  0,      0,      0,      0,       2,0x60,0x0004},
	{"ffightu", 0,      0,      0,      0,       3,0x00,0x0000},
	{"ffightj", 0,      0,      0,      0,       2,0x60,0x0004},
	{"1941",    0,      0,      0,      0,      19,0x60,0x0005},
	{"1941j",   0,      0,      0,      0,      19,0x60,0x0005},
	{"mercs",   0,      0,      0,      0,      22,0x60,0x0402},
	{"mercsu",  0,      0,      0,      0,      22,0x60,0x0402},
	{"mercsj",  0,      0,      0,      0,      22,0x60,0x0402},
	{"mtwins",  0,      0,      0,      0,      21,0x5e,0x0404},
	{"chikij",  0,      0,      0,      0,      21,0x5e,0x0404},
	{"msword",  0,      0,      0,      0,       7,0x00,0x0000},
	{"mswordu", 0,      0,      0,      0,       7,0x00,0x0000},
	{"mswordj", 0,      0,      0,      0,       7,0x00,0x0000},
	{"cawing",  0,      0,      0,      0,      11,0x00,0x0000},
	{"cawingj", 0,      0,      0,      0,      11,0x00,0x0000},
	{"nemo",    0,      0,      0,      0,       8,0x4e,0x0405},
	{"nemoj",   0,      0,      0,      0,       8,0x4e,0x0405},
	{"sf2",     0,      2,      2,      2,      12,0x48,0x0407},
	{"sf2a",    0,      2,      2,      2,      12,0x48,0x0407},
	{"sf2b",    0,      2,      2,      2,      12,0x48,0x0407},
	{"sf2e",    0,      2,      2,      2,      15,0xd0,0x0408},
	{"sf2j",    0,      2,      2,      2,      13,0x6e,0x0403},
	{"3wonders",0,      0,      1,      1,      16,0x72,0x0800, 2},
	{"3wonderj",0,      0,      1,      1,      16,0x72,0x0800, 2},
	{"kod",     0,      0,      0,      0,      10,0x00,0x0000},
	{"kodj",    0,      0,      0,      0,      10,0x00,0x0000},
	{"kodb",    0,      0,      0,      0,      10,0x00,0x0000},
	{"captcomm",0,      0,      0,      0,      29,0x00,0x0000},
	{"captcomu",0,      0,      0,      0,      29,0x00,0x0000},
	{"captcomj",0,      0,      0,      0,      29,0x00,0x0000},
	{"knights", 0,      0,      0,      0,       6,0x00,0x0000},
	{"knightsj",0,      0,      0,      0,       6,0x00,0x0000},
	{"sf2ce",   0,      2,      2,      2,      14,0x00,0x0000},
	{"sf2cej",  0,      2,      2,      2,      14,0x00,0x0000},
	{"sf2rb",   0,      2,      2,      2,      14,0x00,0x0000},
	{"varth",   0,      0,      0,      0,      17,0x00,0x0000},
	{"varthj",  0,      0,      0,      0,      18,0x00,0x0000},
	{"cworld2j",0,      0,      0,      0,      24,0x00,0x0000},
	{"wof",     0,      0,      0,      0,      30,0x00,0x0000},
	{"wofj",    0,      0,      0,      0,      33,0x00,0x0000},
	{"dino",    0,      0,      0,      0,      27,0x00,0x0000},
	{"punisher",0,      0,      0,      0,      28,0x4e,0x0c00},
	{"punishrj",0,      0,      0,      0,      28,0x4e,0x0c00},
	{"slammast",0,      0,      0,      0,      31,0x6e,0x0c01},
	{"mbomber", 0,      0,      0,      0,      32,0x5e,0x0c02},
	{"mbomberj",0,      0,      0,      0,      32,0x5e,0x0c02},
	{"sf2t",    0,      2,      2,      2,      14,0x00,0x0000},
	{"sf2tj",   0,      2,      2,      2,      14,0x00,0x0000},
	{"pnickj",  0,      0,      0,      0,      25,0x00,0x0000},
	{"qad",     0,      0,      0,      0,      23,0x00,0x0000},
	{"qadj",    0,      0,      0,      0,      26,0x00,0x0000},
	{"megaman", 0,      0,      0,      0,      20,0x00,0x0000},
	{"rockmanj",0,      0,      0,      0,      20,0x00,0x0000},
	{"pang3",   0,      0,      0,      0,      34,0x00,0x0000, 5},
	/* End of table (default values) */
	{0,         0,     0,       0,      0,       0,0x00,0x0000},
};

static void cps1_init_machine(void)
{
	const char *gamename = Machine->gamedrv->name;
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];

	fast_memset((void *)&cps1_eeprom[0],0,sizeof(cps1_eeprom));
	cps1_eeprom_register=0;

	struct CPS1config *pCFG=&cps1_config_table[0];
	while(pCFG->name)
	{
		if (strcmp(pCFG->name, gamename) == 0)
		{
			break;
		}
		pCFG++;
	}
	cps1_game_config=pCFG;

	if (strcmp(gamename, "ghouls" )==0)
	{
		/* Patch out self-test... it takes forever */
		WRITE_WORD(&RAM[0x61964+0], 0x4ef9);
		WRITE_WORD(&RAM[0x61964+2], 0x0000);
		WRITE_WORD(&RAM[0x61964+4], 0x0400);
	}
	else if (strcmp(gamename, "ghoulsj" )==0)
	{
		/* Patch out self-test... it takes forever */
		WRITE_WORD(&RAM[0x61930+0], 0x4ef9);
		WRITE_WORD(&RAM[0x61930+2], 0x0000);
		WRITE_WORD(&RAM[0x61930+4], 0x0400);
	}
	else if (strcmp(gamename, "slammast" )==0)
	{
		/* temporary patch until the protection is understood */
		WRITE_WORD(&RAM[0x0fbe], 0x4e75);
	}
	else if (strcmp(gamename, "mbomber" )==0 || strcmp(gamename, "mbomberj" )==0)
	{
		/* temporary patch until the protection is understood */
		WRITE_WORD(&RAM[0x0f1a], 0x4e75);
	}
}




INPUT_PORTS_START( forgottn_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )  /* Service, but it doesn't give any credit */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* DSWA */
	PORT_DIPNAME( 0xff, 0xff, "DIP A", IP_KEY_NONE )
	PORT_DIPSETTING(    0xff, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* DSWB */
	PORT_DIPNAME( 0xff, 0xff, "DIP B", IP_KEY_NONE )
	PORT_DIPSETTING(    0xff, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* DSWC */
	PORT_DIPNAME( 0xff, 0xff, "DIP C", IP_KEY_NONE )
	PORT_DIPSETTING(    0xff, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* Player 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_4WAY )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_4WAY )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_4WAY )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_4WAY )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* Player 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

INPUT_PORTS_START( ghouls_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )  /* Service, but it doesn't give any credit */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* DSWA */
	PORT_DIPNAME( 0x07, 0x07, "Coin A", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x01, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x02, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x07, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x06, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/4 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin/6 Credits" )
	PORT_DIPNAME( 0x38, 0x38, "Coin B", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x08, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x10, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x38, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x30, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x28, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x20, "1 Coin/4 Credits" )
	PORT_DIPSETTING(    0x18, "1 Coin/6 Credits" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x40, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Cabinet", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Upright" )
	PORT_DIPSETTING(    0x00, "Cocktail" )

	PORT_START      /* DSWB */
	PORT_DIPNAME( 0x01, 0x01, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x01, "On" )
	PORT_DIPNAME( 0x02, 0x02, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x02, "On" )
	PORT_DIPNAME( 0x04, 0x04, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x04, "On" )
	PORT_DIPNAME( 0x08, 0x08, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x08, "On" )
	PORT_DIPNAME( 0x30, 0x30, "Bonus Life", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "10K, 30K and every 30K" )
	PORT_DIPSETTING(    0x10, "20K, 50K and every 70K" )
	PORT_DIPSETTING(    0x30, "30K, 60K and every 70K" )
	PORT_DIPSETTING(    0x00, "40K, 70K and every 80K" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x40, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x80, "On" )

	PORT_START      /* DSWC */
	PORT_DIPNAME( 0x03, 0x03, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x04, 0x04, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x04, "On" )
	PORT_DIPNAME( 0x08, 0x08, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x08, "On" )
	PORT_DIPNAME( 0x10, 0x10, "Flip Screen", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x20, 0x20, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x20, "On" )
	PORT_DIPNAME( 0x40, 0x40, "Allow Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "No" )
	PORT_DIPSETTING(    0x40, "Yes" )
	PORT_BITX(    0x80, 0x80, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* Player 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_4WAY )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_4WAY )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_4WAY )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_4WAY )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* Player 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

INPUT_PORTS_START( strider_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START4 )

	PORT_START      /* DSWA */
	PORT_DIPNAME( 0x07, 0x07, "Coin A", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x01, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x02, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x07, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x06, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/4 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin/6 Credits" )
	PORT_DIPNAME( 0x38, 0x38, "Coin B", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x08, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x10, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x38, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x30, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x28, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x20, "1 Coin/4 Credits" )
	PORT_DIPSETTING(    0x18, "1 Coin/6 Credits" )
	PORT_DIPNAME( 0xc0, 0xc0, "Cabinet", IP_KEY_NONE )
	PORT_DIPSETTING(    0xc0, "Upright 1 Player" )
	PORT_DIPSETTING(    0x80, "Upright 2 Players" )
	PORT_DIPSETTING(    0x00, "Cocktail" )
	/* 0x40 Cocktail */

	PORT_START      /* DSWB */
	PORT_DIPNAME( 0x07, 0x07, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x04, "Easiest" )
	PORT_DIPSETTING(    0x05, "Easier" )
	PORT_DIPSETTING(    0x06, "Easy" )
	PORT_DIPSETTING(    0x07, "Normal" )
	PORT_DIPSETTING(    0x03, "Medium" )
	PORT_DIPSETTING(    0x02, "Hard" )
	PORT_DIPSETTING(    0x01, "Harder" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	/* TODO: this doesn't seem to work */
	PORT_DIPNAME( 0x08, 0x00, "Continue Coinage ?", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "1 Coin" )
	PORT_DIPSETTING(    0x08, "2 Coins" )
	PORT_DIPNAME( 0x30, 0x30, "Bonus Life", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "20000 60000" )
	PORT_DIPSETTING(    0x00, "30000 60000" )
	PORT_DIPSETTING(    0x30, "20000 40000 60000" )
	PORT_DIPSETTING(    0x20, "30000 50000 70000" )
	PORT_DIPNAME( 0x40, 0x00, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x80, 0x00, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* DSWC */
	PORT_DIPNAME( 0x03, 0x03, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x04, 0x04, "Freeze", IP_KEY_NONE )
	PORT_DIPSETTING(    0x04, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x08, 0x08, "Free Play", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x10, 0x10, "Flip Screen", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x20, 0x20, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x20, "On" )
	PORT_DIPNAME( 0x40, 0x40, "Allow Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "No" )
	PORT_DIPSETTING(    0x40, "Yes" )
	PORT_BITX(    0x80, 0x80, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* Player 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* Player 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* Player 3 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* Player 4 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

INPUT_PORTS_START( dwj_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	 PORT_START      /* DSWA */
	PORT_DIPNAME( 0x07, 0x07, "Coin A", IP_KEY_NONE )
	PORT_DIPSETTING(    0x01, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x02, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x03, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x07, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x06, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/4 Credits" )
	PORT_DIPNAME( 0x38, 0x38, "Coin B", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x10, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x18, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x38, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x30, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x28, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x20, "1 Coin/4 Credits" )
	/* 0x00 2 Coins/1 credit for both coin ports */
	PORT_DIPNAME( 0x40, 0x40, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Free Play", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* DSWB */
	PORT_DIPNAME( 0x07, 0x07, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x04, "Very Easy" )
	PORT_DIPSETTING(    0x05, "Easy 2" )
	PORT_DIPSETTING(    0x06, "Easy 1" )
	PORT_DIPSETTING(    0x07, "Normal" )
	PORT_DIPSETTING(    0x03, "Difficult 1" )
	PORT_DIPSETTING(    0x02, "Difficult 2" )
	PORT_DIPSETTING(    0x01, "Difficult 3" )
	PORT_DIPSETTING(    0x00, "Very difficult" )
	PORT_DIPNAME( 0x08, 0x08, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x10, 0x10, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* DSWC */
	PORT_DIPNAME( 0x01, 0x01, "Freeze", IP_KEY_NONE )
	PORT_DIPSETTING(    0x01, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x02, 0x02, "Turbo Mode", IP_KEY_NONE )
	PORT_DIPSETTING(    0x02, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x04, 0x04, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x04, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x08, 0x08, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x10, 0x10, "Flip Screen", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x20, 0x20, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x20, "On" )
	PORT_DIPNAME( 0x40, 0x40, "Allow Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "No" )
	PORT_DIPSETTING(    0x40, "Yes" )
	PORT_BITX(    0x80, 0x80, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* Player 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* Player 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

INPUT_PORTS_START( willow_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* DSWA */
	PORT_DIPNAME( 0x07, 0x07, "Coin A", IP_KEY_NONE )
	PORT_DIPSETTING(    0x01, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x02, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x03, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x00, "2 Coins/1 Credit (1 to continue)" )
	PORT_DIPSETTING(    0x07, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x06, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/4 Credits" )
	PORT_DIPNAME( 0x38, 0x38, "Coin B", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x10, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x18, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x00, "2 Coins/1 Credit (1 to continue)" )
	PORT_DIPSETTING(    0x38, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x30, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x28, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x20, "1 Coin/4 Credits" )
	PORT_DIPNAME( 0xc0, 0xc0, "Cabinet", IP_KEY_NONE )
	PORT_DIPSETTING(    0xc0, "Upright 1 Player" )
	PORT_DIPSETTING(    0x80, "Upright 2 Players" )
	PORT_DIPSETTING(    0x00, "Cocktail" )
	/* 0x40 Cocktail */

	PORT_START      /* DSWB */
	PORT_DIPNAME( 0x07, 0x07, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x04, "Very easy" )
	PORT_DIPSETTING(    0x05, "Easier" )
	PORT_DIPSETTING(    0x06, "Easy" )
	PORT_DIPSETTING(    0x07, "Normal" )
	PORT_DIPSETTING(    0x03, "Medium" )
	PORT_DIPSETTING(    0x02, "Hard" )
	PORT_DIPSETTING(    0x01, "Harder" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0x18, 0x18, "Nando Speed", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Slow" )
	PORT_DIPSETTING(    0x18, "Normal" )
	PORT_DIPSETTING(    0x08, "Fast" )
	PORT_DIPSETTING(    0x00, "Very Fast" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown", IP_KEY_NONE ) /* Unused ? */
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x20, "Unknown", IP_KEY_NONE ) /* Unused ? */
	PORT_DIPSETTING(    0x40, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Stage Magic Continue (power up?)", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	/* The test mode reports Stage Magic, a file with dip says if
	 power up are on the player gets sword and magic item without having
	 to buy them. To test */

	PORT_START      /* DSWC */
	PORT_DIPNAME( 0x03, 0x03, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x0c, 0x0c, "Vitality", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x10, 0x10, "Flip Screen", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x20, 0x20, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x20, "On" )
	PORT_DIPNAME( 0x40, 0x40, "Allow Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "No" )
	PORT_DIPSETTING(    0x40, "Yes" )
	PORT_BITX(    0x80, 0x80, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* Player 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* Player 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

INPUT_PORTS_START( unsquad_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* DSWA */
	PORT_DIPNAME( 0x07, 0x07, "Coin A", IP_KEY_NONE )
	PORT_DIPSETTING(    0x01, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x02, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x03, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x00, "2 Coins/1 Credit (1 to continue)" )
	PORT_DIPSETTING(    0x07, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x06, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/4 Credits" )
	PORT_DIPNAME( 0x38, 0x38, "Coin B", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x10, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x18, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x00, "2 Coins/1 Credit (1 to continue)" )
	PORT_DIPSETTING(    0x38, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x30, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x28, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x20, "1 Coin/4 Credits" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* DSWB */
	PORT_DIPNAME( 0x07, 0x07, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x07, "Super Easy" )
	PORT_DIPSETTING(    0x06, "Very Easy" )
	PORT_DIPSETTING(    0x05, "Easy" )
	PORT_DIPSETTING(    0x04, "Normal" )
	PORT_DIPSETTING(    0x03, "Difficult" )
	PORT_DIPSETTING(    0x02, "Very Difficult" )
	PORT_DIPSETTING(    0x01, "Super Difficult" )
	PORT_DIPSETTING(    0x00, "Ultra Super Difficult" )
	PORT_DIPNAME( 0x08, 0x08, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x10, 0x10, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* DSWC */
	PORT_DIPNAME( 0x01, 0x01, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x01, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x02, 0x02, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x02, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x04, 0x04, "Free Play", IP_KEY_NONE )
	PORT_DIPSETTING(    0x04, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x08, 0x08, "Freeze", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x10, 0x10, "Flip Screen", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x20, 0x20, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x00, "Allow Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "No" )
	PORT_DIPSETTING(    0x00, "Yes" )
	PORT_BITX(    0x80, 0x80, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* Player 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* Player 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

INPUT_PORTS_START( ffight_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* DSWA */
	PORT_DIPNAME( 0x07, 0x07, "Coin A", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x01, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x02, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x07, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x06, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/4 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin/6 Credits" )
	PORT_DIPNAME( 0x38, 0x38, "Coin B", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x08, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x10, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x38, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x30, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x28, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x20, "1 Coin/4 Credits" )
	PORT_DIPSETTING(    0x18, "1 Coin/6 Credits" )
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* DSWB */
	PORT_DIPNAME( 0x07, 0x04, "Difficulty Level 1", IP_KEY_NONE )
	PORT_DIPSETTING(    0x07, "Very easy" )
	PORT_DIPSETTING(    0x06, "Easier" )
	PORT_DIPSETTING(    0x05, "Easy" )
	PORT_DIPSETTING(    0x04, "Normal" )
	PORT_DIPSETTING(    0x03, "Medium" )
	PORT_DIPSETTING(    0x02, "Hard" )
	PORT_DIPSETTING(    0x01, "Harder" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0x18, 0x10, "Difficulty Level 2", IP_KEY_NONE )
	PORT_DIPSETTING(    0x18, "Easy" )
	PORT_DIPSETTING(    0x10, "Normal" )
	PORT_DIPSETTING(    0x08, "Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0x60, 0x60, "Bonus Life", IP_KEY_NONE )
	PORT_DIPSETTING(    0x60, "100000" )
	PORT_DIPSETTING(    0x40, "200000" )
	PORT_DIPSETTING(    0x20, "100000 200000" )
	PORT_DIPSETTING(    0x00, "None" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* DSWC */
	PORT_DIPNAME( 0x03, 0x03, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPNAME( 0x04, 0x04, "Free Play", IP_KEY_NONE )
	PORT_DIPSETTING(    0x04, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x08, 0x08, "Freeze", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x10, 0x10, "Flip screen", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x20, 0x00, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x40, "Allow Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "No" )
	PORT_DIPSETTING(    0x40, "Yes" )
	PORT_BITX(    0x80, 0x80, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* Player 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* Player 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

INPUT_PORTS_START( c1941_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* DSWA */
	PORT_DIPNAME( 0x07, 0x07, "Coin A", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x01, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x02, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x07, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x06, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/4 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin/6 Credits" )
	PORT_DIPNAME( 0x38, 0x38, "Coin B", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x08, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x10, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x38, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x30, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x28, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x20, "1 Coin/4 Credits" )
	PORT_DIPSETTING(    0x18, "1 Coin/6 Credits" )
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* DSWB */
	PORT_DIPNAME( 0x07, 0x07, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x07, "0 (Easier)" )
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x05, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x01, "6" )
	PORT_DIPSETTING(    0x00, "7 (Harder)" )
	PORT_DIPNAME( 0x18, 0x18, "Life Bar", IP_KEY_NONE )
	PORT_DIPSETTING(    0x18, "More Slowly" )
	PORT_DIPSETTING(    0x10, "Slowly" )
	PORT_DIPSETTING(    0x08, "Quickly" )
	PORT_DIPSETTING(    0x00, "More Quickly" )
	PORT_DIPNAME( 0x60, 0x60, "Bullet's Speed", IP_KEY_NONE )
	PORT_DIPSETTING(    0x60, "Very Slow" )
	PORT_DIPSETTING(    0x40, "Slow" )
	PORT_DIPSETTING(    0x20, "Fast" )
	PORT_DIPSETTING(    0x00, "Very Fast" )
	PORT_DIPNAME( 0x80, 0x80, "Initial Vitality", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "3 Bars" )
	PORT_DIPSETTING(    0x00, "4 Bars" )

	PORT_START      /* DSWC */
	PORT_DIPNAME( 0x01, 0x01, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x01, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x02, 0x02, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x02, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x04, 0x04, "Free Play", IP_KEY_NONE )
	PORT_DIPSETTING(    0x04, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x08, 0x08, "Freeze", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x10, 0x10, "Flip Screen", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x20, 0x00, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x00, "Allow Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "No" )
	PORT_DIPSETTING(    0x00, "Yes" )
	PORT_BITX(    0x80, 0x80, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* Player 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* Player 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

INPUT_PORTS_START( mercs_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN4 ) /* Service Coin */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* DSWA */
	PORT_DIPNAME( 0x07, 0x07, "Coinage", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x01, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x02, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x07, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x06, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/4 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin/6 Credits" )
	PORT_DIPNAME( 0x38, 0x38, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x38, "3" )
	PORT_DIPSETTING(    0x30, "4" )
	PORT_DIPSETTING(    0x28, "5" )
	PORT_DIPSETTING(    0x20, "6" )
	PORT_DIPSETTING(    0x18, "7" )
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* DSWB */
	PORT_DIPNAME( 0x07, 0x04, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x07, "Easiest" )
	PORT_DIPSETTING(    0x06, "Very Easy" )
	PORT_DIPSETTING(    0x05, "Easy" )
	PORT_DIPSETTING(    0x04, "Normal" )
	PORT_DIPSETTING(    0x03, "Difficult" )
	PORT_DIPSETTING(    0x02, "Very Difficult" )
	PORT_DIPSETTING(    0x01, "Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0x08, 0x08, "Coin Slots", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPNAME( 0x10, 0x10, "Max Players", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* DSWC */
	PORT_DIPNAME( 0x07, 0x07, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x07, "7" )
	PORT_DIPNAME( 0x08, 0x08, "Freeze", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x10, 0x10, "Flip Screen", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x20, 0x00, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x00, "Allow Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "No" )
	PORT_DIPSETTING(    0x00, "Yes" )
	PORT_BITX(    0x80, 0x80, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* Player 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* Player 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* Player 3 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START3 )
INPUT_PORTS_END

INPUT_PORTS_START( mtwins_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* DSWA */
	PORT_DIPNAME( 0x07, 0x07, "Coin A", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x01, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x02, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x07, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x06, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/4 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin/6 Credits" )
	PORT_DIPNAME( 0x38, 0x38, "Coin B", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x08, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x10, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x38, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x30, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x28, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x20, "1 Coin/4 Credits" )
	PORT_DIPSETTING(    0x18, "1 Coin/6 Credits" )
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* DSWB */
	PORT_DIPNAME( 0x07, 0x07, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x07, "Super Easy" )
	PORT_DIPSETTING(    0x06, "Very Easy" )
	PORT_DIPSETTING(    0x05, "Easy" )
	PORT_DIPSETTING(    0x04, "Normal" )
	PORT_DIPSETTING(    0x03, "Difficult" )
	PORT_DIPSETTING(    0x02, "Very Difficult" )
	PORT_DIPSETTING(    0x01, "Super Difficult" )
	PORT_DIPSETTING(    0x00, "Ultra Super Difficult" )
	PORT_DIPNAME( 0x08, 0x08, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x30, 0x00, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	/*  0x30 gives 1 life */
	PORT_DIPNAME( 0x40, 0x40, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* DSWC */
	PORT_DIPNAME( 0x01, 0x01, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x01, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x02, 0x02, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x02, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x04, 0x04, "Free Play", IP_KEY_NONE )
	PORT_DIPSETTING(    0x04, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x08, 0x08, "Freeze", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x10, 0x10, "Flip Screen", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x20, 0x00, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x00, "Allow Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "No" )
	PORT_DIPSETTING(    0x00, "Yes" )
	PORT_BITX(    0x80, 0x80, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* Player 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* Player 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

INPUT_PORTS_START( msword_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* DSWA */
	PORT_DIPNAME( 0x07, 0x07, "Coin A", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x01, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x02, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x07, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x06, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/4 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin/6 Credits" )
	PORT_DIPNAME( 0x38, 0x38, "Coin B", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x08, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x10, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x38, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x30, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x28, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x20, "1 Coin/4 Credits" )
	PORT_DIPSETTING(    0x18, "1 Coin/6 Credits" )
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* DSWB */
	PORT_DIPNAME( 0x07, 0x04, "Level 1", IP_KEY_NONE )
	PORT_DIPSETTING(    0x07, "Easiest" )
	PORT_DIPSETTING(    0x06, "Very Easy" )
	PORT_DIPSETTING(    0x05, "Easy" )
	PORT_DIPSETTING(    0x04, "Normal" )
	PORT_DIPSETTING(    0x03, "Difficult" )
	PORT_DIPSETTING(    0x02, "Hard" )
	PORT_DIPSETTING(    0x01, "Very Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0x38, 0x38, "Level 2", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Easiest" )
	PORT_DIPSETTING(    0x28, "Very Easy" )
	PORT_DIPSETTING(    0x30, "Easy" )
	PORT_DIPSETTING(    0x38, "Normal" )
	PORT_DIPSETTING(    0x18, "Difficult" )
	PORT_DIPSETTING(    0x10, "Hard" )
	PORT_DIPSETTING(    0x08, "Very Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0x40, 0x00, "Stage Select", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* DSWC */
	PORT_DIPNAME( 0x03, 0x03, "Vitality", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPNAME( 0x04, 0x04, "Free Play", IP_KEY_NONE )
	PORT_DIPSETTING(    0x04, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x08, 0x08, "Freeze", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x10, 0x10, "Flip Screen", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x20, 0x00, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x00, "Allow Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "No" )
	PORT_DIPSETTING(    0x00, "Yes" )
	PORT_BITX(    0x80, 0x80, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* Player 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* Player 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

INPUT_PORTS_START( cawing_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* DSWA */
	PORT_DIPNAME( 0x07, 0x07, "Coin A", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x01, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x02, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x07, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x06, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/4 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin/6 Credits" )
	PORT_DIPNAME( 0x38, 0x38, "Coin B", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x08, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x10, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x38, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x30, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x28, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x20, "1 Coin/4 Credits" )
	PORT_DIPSETTING(    0x18, "1 Coin/6 Credits" )
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* DSWB */
	PORT_DIPNAME( 0x07, 0x07, "Difficulty Level 1", IP_KEY_NONE )
	PORT_DIPSETTING(    0x07, "Very Easy" )
	PORT_DIPSETTING(    0x06, "Easy 2" )
	PORT_DIPSETTING(    0x05, "Easy 1" )
	PORT_DIPSETTING(    0x04, "Normal" )
	PORT_DIPSETTING(    0x03, "Difficult 1" )
	PORT_DIPSETTING(    0x02, "Difficult 2" )
	PORT_DIPSETTING(    0x01, "Difficult 3" )
	PORT_DIPSETTING(    0x00, "Very Difficult" )
	PORT_DIPNAME( 0x18, 0x10, "Difficulty Level 2", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Easy" )
	PORT_DIPSETTING(    0x18, "Normal" )
	PORT_DIPSETTING(    0x08, "Difficult" )
	PORT_DIPSETTING(    0x00, "Very Difficult" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* DSWC */
	PORT_DIPNAME( 0x03, 0x03, "Lives?", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPNAME( 0x04, 0x04, "Free Play", IP_KEY_NONE )
	PORT_DIPSETTING(    0x04, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x08, 0x08, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x10, 0x10, "Flip Screen", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x20, 0x00, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x00, "Allow Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "No" )
	PORT_DIPSETTING(    0x00, "Yes" )
	PORT_BITX(    0x80, 0x80, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* Player 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* Player 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

INPUT_PORTS_START( nemo_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* DSWA */
	PORT_DIPNAME( 0x07, 0x07, "Coin A", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x01, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x02, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x07, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x06, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/4 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin/6 Credits" )
	PORT_DIPNAME( 0x38, 0x38, "Coin B", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x08, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x10, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x38, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x30, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x28, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x20, "1 Coin/4 Credits" )
	PORT_DIPSETTING(    0x18, "1 Coin/6 Credits" )
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* DSWB */
	PORT_DIPNAME( 0x07, 0x07, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x07, "Very Easy" )
	PORT_DIPSETTING(    0x06, "Easy 1" )
	PORT_DIPSETTING(    0x05, "Easy 2" )
	PORT_DIPSETTING(    0x04, "Normal" )
	PORT_DIPSETTING(    0x03, "Difficult 1" )
	PORT_DIPSETTING(    0x02, "Difficult 2" )
	PORT_DIPSETTING(    0x01, "Difficult 3" )
	PORT_DIPSETTING(    0x00, "Very Difficult" )
	PORT_DIPNAME( 0x18, 0x18, "Life Bar", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Minimun" )
	PORT_DIPSETTING(    0x18, "Medium" )
	PORT_DIPSETTING(    0x08, "Maximum" )
	/* 0x10 gives Medium */
	PORT_DIPNAME( 0x20, 0x20, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* DSWC */
	PORT_DIPNAME( 0x03, 0x03, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x04, 0x04, "Free Play", IP_KEY_NONE )
	PORT_DIPSETTING(    0x04, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x08, 0x08, "Freeze", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x10, 0x10, "Flip Screen", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x20, 0x00, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x00, "Allow Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "No" )
	PORT_DIPSETTING(    0x00, "Yes" )
	PORT_BITX(    0x80, 0x80, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* Player 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* Player 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

INPUT_PORTS_START( sf2_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	 PORT_START      /* DSWA */
	PORT_DIPNAME( 0x07, 0x07, "Coin A", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x01, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x02, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x07, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x06, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/4 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin/6 Credits" )
	PORT_DIPNAME( 0x38, 0x38, "Coin B", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x08, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x10, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x38, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x30, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x28, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x20, "1 Coin/4 Credits" )
	PORT_DIPSETTING(    0x18, "1 Coin/6 Credits" )
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* DSWB */
	PORT_DIPNAME( 0x07, 0x04, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPSETTING(    0x01, "Very Hard" )
	PORT_DIPSETTING(    0x02, "Hard" )
	PORT_DIPSETTING(    0x03, "Difficult" )
	PORT_DIPSETTING(    0x04, "Normal" )
	PORT_DIPSETTING(    0x05, "Easy" )
	PORT_DIPSETTING(    0x06, "Very Easy" )
	PORT_DIPSETTING(    0x07, "Easier" )
	PORT_DIPNAME( 0x08, 0x08, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x08, "On" )
	PORT_DIPNAME( 0x10, 0x10, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x10, "On" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x20, "On" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x40, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x80, "On" )

	PORT_START      /* DSWC */
	PORT_DIPNAME( 0x03, 0x03, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPNAME( 0x04, 0x04, "Free Play", IP_KEY_NONE )
	PORT_DIPSETTING(    0x04, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x08, 0x08, "Freeze", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x10, 0x10, "Flip Screen", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x20, 0x00, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x00, "Allow Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "No" )
	PORT_DIPSETTING(    0x00, "Yes" )
	PORT_BITX(    0x80, 0x80, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* Player 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* Player 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* Extra buttons */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 | IPF_PLAYER1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 | IPF_PLAYER1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON6 | IPF_PLAYER2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

INPUT_PORTS_START( c3wonders_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* DSWA */
	PORT_DIPNAME( 0x07, 0x07, "Coin A", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x01, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x02, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x07, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x06, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/4 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin/6 Credits" )
	PORT_DIPNAME( 0x38, 0x38, "Coin B", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x08, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x10, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x38, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x30, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x28, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x20, "1 Coin/4 Credits" )
	PORT_DIPSETTING(    0x18, "1 Coin/6 Credits" )
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Freeze", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* DSWB */
	PORT_DIPNAME( 0x03, 0x03, "Action Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x0c, 0x0c, "Action Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x0c, "Easy" )
	PORT_DIPSETTING(    0x08, "Normal" )
	PORT_DIPSETTING(    0x04, "Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0x30, 0x30, "Shooting Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0xc0, 0xc0, "Shooting Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0xc0, "Easy" )
	PORT_DIPSETTING(    0x80, "Normal" )
	PORT_DIPSETTING(    0x40, "Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )

	PORT_START      /* DSWC */
	PORT_DIPNAME( 0x03, 0x03, "Puzzle Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x0c, 0x0c, "Puzzle Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x0c, "Easy" )
	PORT_DIPSETTING(    0x08, "Normal" )
	PORT_DIPSETTING(    0x04, "Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0x10, 0x10, "Flip Screen", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x20, 0x00, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x00, "Allow Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "No" )
	PORT_DIPSETTING(    0x00, "Yes" )
	PORT_BITX(    0x80, 0x80, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* Player 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* Player 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

INPUT_PORTS_START( kod_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) /* Service Coin, not player 3 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* Test */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* DSWA */
	PORT_DIPNAME( 0x07, 0x07, "Coinage", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x01, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x02, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x07, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x06, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/4 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin/6 Credits" )
	PORT_DIPNAME( 0x08, 0x08, "Coin Slots", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPNAME( 0x10, 0x10, "Max Players", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* DSWB */
	PORT_DIPNAME( 0x07, 0x04, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x07, "Easiest" )
	PORT_DIPSETTING(    0x06, "Very Easy" )
	PORT_DIPSETTING(    0x05, "Easy" )
	PORT_DIPSETTING(    0x04, "Normal" )
	PORT_DIPSETTING(    0x03, "Medium" )
	PORT_DIPSETTING(    0x02, "Hard" )
	PORT_DIPSETTING(    0x01, "Very Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0x38, 0x38, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x08, "7" )
	PORT_DIPSETTING(    0x10, "6" )
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x28, "3" )
	PORT_DIPSETTING(    0x38, "2" )
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* DSWC */
	PORT_DIPNAME( 0x01, 0x01, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x01, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x02, 0x02, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x02, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x04, 0x04, "Free Play", IP_KEY_NONE )
	PORT_DIPSETTING(    0x04, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x08, 0x08, "Freeze", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x10, 0x10, "Flip Screen", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x20, 0x00, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x00, "Allow Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "No" )
	PORT_DIPSETTING(    0x00, "Yes" )
	PORT_BITX(    0x80, 0x80, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* Player 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* Player 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* Player 3 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START3 )
INPUT_PORTS_END

INPUT_PORTS_START( captcomm_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* DSWA */
	PORT_DIPNAME( 0x07, 0x07, "Coin A", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x01, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x02, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x07, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x06, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/4 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin/6 Credits" )
	PORT_DIPNAME( 0x38, 0x38, "Coin B", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x08, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x10, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x38, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x30, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x28, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x20, "1 Coin/4 Credits" )
	PORT_DIPSETTING(    0x18, "1 Coin/6 Credits" )
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* DSWB */
	PORT_DIPNAME( 0x07, 0x07, "Difficulty 1", IP_KEY_NONE )
	PORT_DIPSETTING(    0x07, "Very Easy" )
	PORT_DIPSETTING(    0x06, "Easy 1" )
	PORT_DIPSETTING(    0x05, "Easy 2" )
	PORT_DIPSETTING(    0x04, "Normal" )
	PORT_DIPSETTING(    0x03, "Difficult" )
	PORT_DIPSETTING(    0x02, "Very Difficult" )
	PORT_DIPSETTING(    0x01, "Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0x18, 0x18, "Difficulty 2", IP_KEY_NONE )
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0xc0, 0xc0, "Max Players", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0xc0, "2" )
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0x00, "4" )

	PORT_START      /* DSWC */
	PORT_DIPNAME( 0x03, 0x03, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPNAME( 0x04, 0x04, "Free Play", IP_KEY_NONE )
	PORT_DIPSETTING(    0x04, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x08, 0x08, "Freeze", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x10, 0x10, "Flip Screen", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x20, 0x00, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x00, "Allow Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "No" )
	PORT_DIPSETTING(    0x00, "Yes" )
	PORT_BITX(    0x80, 0x80, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* Player 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* Player 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

INPUT_PORTS_START( knights_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN4 ) /* service */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* TEST */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* DSWA */
	PORT_DIPNAME( 0x07, 0x07, "Coin A", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x01, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x02, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x07, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x06, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/4 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin/6 Credits" )
	PORT_DIPNAME( 0x38, 0x38, "Coin B?", IP_KEY_NONE )
	PORT_DIPSETTING(    0x38, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_START      /* DSWB */
	PORT_DIPNAME( 0x07, 0x04, "Player speed and vitality consumption", IP_KEY_NONE )
	PORT_DIPSETTING(    0x07, "Very easy" )
	PORT_DIPSETTING(    0x06, "Easier" )
	PORT_DIPSETTING(    0x05, "Easy" )
	PORT_DIPSETTING(    0x04, "Normal" )
	PORT_DIPSETTING(    0x03, "Medium" )
	PORT_DIPSETTING(    0x02, "Hard" )
	PORT_DIPSETTING(    0x01, "Harder" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0x38, 0x38, "Enemy's vitality and attack power", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Very Easy" )
	PORT_DIPSETTING(    0x08, "Easier" )
	PORT_DIPSETTING(    0x00, "Easy" )
	PORT_DIPSETTING(    0x38, "Normal" )
	PORT_DIPSETTING(    0x30, "Medium" )
	PORT_DIPSETTING(    0x28, "Hard" )
	PORT_DIPSETTING(    0x20, "Harder" )
	PORT_DIPSETTING(    0x18, "Hardest" )
	PORT_DIPNAME( 0x40, 0x40, "Coin Slots", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPNAME( 0x80, 0x80, "Max Players", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x80, "3" )
	PORT_START      /* DSWC */
	PORT_DIPNAME( 0x03, 0x03, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPNAME( 0x04, 0x04, "Free Play", IP_KEY_NONE )
	PORT_DIPSETTING(    0x04, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x08, 0x08, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x10, 0x10, "Flip Screen", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x20, 0x00, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x00, "Allow Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "No" )
	PORT_DIPSETTING(    0x00, "Yes" )
	PORT_BITX(    0x80, 0x80, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* Player 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* Player 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* Player 3 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START3 )
INPUT_PORTS_END

INPUT_PORTS_START( varth_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* DSWA */
	PORT_DIPNAME( 0x07, 0x07, "Coin A", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x01, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x02, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x07, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x06, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/4 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin/6 Credits" )
	PORT_DIPNAME( 0x38, 0x38, "Coin B", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x08, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x10, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x38, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x30, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x28, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x20, "1 Coin/4 Credits" )
	PORT_DIPSETTING(    0x18, "1 Coin/6 Credits" )
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* DSWB */
	PORT_DIPNAME( 0x07, 0x07, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x07, "Very Easy" )
	PORT_DIPSETTING(    0x06, "Easy 1" )
	PORT_DIPSETTING(    0x05, "Easy 2" )
	PORT_DIPSETTING(    0x04, "Normal" )
	PORT_DIPSETTING(    0x03, "Difficult" )
	PORT_DIPSETTING(    0x02, "Very Difficult" )
	PORT_DIPSETTING(    0x01, "Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0x18, 0x18, "Bonus Life", IP_KEY_NONE )
	PORT_DIPSETTING(    0x18, "500K & every 1.400K ?" )
	PORT_DIPSETTING(    0x10, "500K, 2.000K, +500K ?" )
	PORT_DIPSETTING(    0x08, "1.200K & 3.500K ?" )
	PORT_DIPSETTING(    0x00, "2000K ?" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* DSWC */
	PORT_DIPNAME( 0x03, 0x03, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x04, 0x04, "Free Play", IP_KEY_NONE )
	PORT_DIPSETTING(    0x04, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x08, 0x08, "Freeze", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x10, 0x10, "Flip Screen", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x20, 0x00, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x00, "Allow Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "No" )
	PORT_DIPSETTING(    0x00, "Yes" )
	PORT_BITX(    0x80, 0x80, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* Player 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* Player 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

INPUT_PORTS_START( cworld2j_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START4 )

	PORT_START      /* DSWA */
	PORT_DIPNAME( 0x07, 0x07, "Coinage", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x01, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x02, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x07, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x06, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/4 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin/6 Credits" )
	PORT_DIPNAME( 0x38, 0x38, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x38, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Test Mode (Use with Service Mode)", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* DSWB */
	PORT_DIPNAME( 0x07, 0x06, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x06, "0" )
	PORT_DIPSETTING(    0x05, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPNAME( 0x18, 0x18, "Extend", IP_KEY_NONE )
	PORT_DIPSETTING(    0x18, "N" )
	PORT_DIPSETTING(    0x10, "E" )
	PORT_DIPSETTING(    0x00, "D" )
	PORT_DIPNAME( 0xe0, 0xe0, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0xe0, "3" )
	PORT_DIPSETTING(    0xa0, "4" )
	PORT_DIPSETTING(    0xc0, "5" )

	PORT_START      /* DSWC */
	PORT_DIPNAME( 0x01, 0x01, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x01, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x02, 0x02, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x02, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x04, 0x04, "Free Play", IP_KEY_NONE )
	PORT_DIPSETTING(    0x04, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x08, 0x08, "Freeze", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x10, 0x10, "Flip Screen", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x20, 0x00, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x00, "Allow Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "No" )
	PORT_DIPSETTING(    0x00, "Yes" )
	PORT_BITX(    0x80, 0x80, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* Player 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER1 )

	PORT_START      /* Player 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER2 )
INPUT_PORTS_END

INPUT_PORTS_START( wof_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START4 )

	PORT_START      /* DSWA */
	PORT_DIPNAME( 0xff, 0xff, "", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPSETTING(    0xff, "Off" )

	PORT_START      /* DSWB */
	PORT_DIPNAME( 0xff, 0xff, "", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPSETTING(    0xff, "Off" )

	PORT_START      /* DSWC */
	PORT_DIPNAME( 0xff, 0xff, "", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPSETTING(    0xff, "Off" )

	PORT_START      /* Player 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER1 )

	PORT_START      /* Player 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER2 )
INPUT_PORTS_END

INPUT_PORTS_START( dino_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START4 )

	PORT_START      /* DSWA */
	PORT_DIPNAME( 0xff, 0xff, "", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPSETTING(    0xff, "Off" )

	PORT_START      /* DSWB */
	PORT_DIPNAME( 0xff, 0xff, "", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPSETTING(    0xff, "Off" )

	PORT_START      /* DSWC */
	PORT_DIPNAME( 0xff, 0xff, "", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPSETTING(    0xff, "Off" )

	PORT_START      /* Player 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER1 )

	PORT_START      /* Player 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER2 )
INPUT_PORTS_END

INPUT_PORTS_START( punisher_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START4 )

	PORT_START      /* DSWA */
	PORT_DIPNAME( 0xff, 0xff, "", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPSETTING(    0xff, "Off" )

	PORT_START      /* DSWB */
	PORT_DIPNAME( 0xff, 0xff, "", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPSETTING(    0xff, "Off" )

	PORT_START      /* DSWC */
	PORT_DIPNAME( 0xff, 0xff, "", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPSETTING(    0xff, "Off" )

	PORT_START      /* Player 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER1 )

	PORT_START      /* Player 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER2 )
INPUT_PORTS_END

INPUT_PORTS_START( slammast_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START4 )

	PORT_START      /* DSWA */
	PORT_DIPNAME( 0xff, 0xff, "", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPSETTING(    0xff, "Off" )

	PORT_START      /* DSWB */
	PORT_DIPNAME( 0xff, 0xff, "", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPSETTING(    0xff, "Off" )

	PORT_START      /* DSWC */
	PORT_DIPNAME( 0xff, 0xff, "", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPSETTING(    0xff, "Off" )

	PORT_START      /* Player 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER1 )

	PORT_START      /* Player 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER2 )
INPUT_PORTS_END

INPUT_PORTS_START( mbomber_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START4 )

	PORT_START      /* DSWA */
	PORT_DIPNAME( 0xff, 0xff, "", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPSETTING(    0xff, "Off" )

	PORT_START      /* DSWB */
	PORT_DIPNAME( 0xff, 0xff, "", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPSETTING(    0xff, "Off" )

	PORT_START      /* DSWC */
	PORT_DIPNAME( 0xff, 0xff, "", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPSETTING(    0xff, "Off" )

	PORT_START      /* Player 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER1 )

	PORT_START      /* Player 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER2 )
INPUT_PORTS_END

INPUT_PORTS_START( pnickj_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* DSWA */
	PORT_DIPNAME( 0x07, 0x07, "Coinage", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x01, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x02, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x07, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x06, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/4 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin/6 Credits" )
	PORT_DIPNAME( 0x08, 0x08, "Coin Slots", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x10, 0x10, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* DSWB */
	PORT_DIPNAME( 0x07, 0x04, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x07, "Easiest" )
	PORT_DIPSETTING(    0x06, "Very Easy" )
	PORT_DIPSETTING(    0x05, "Easy" )
	PORT_DIPSETTING(    0x04, "Normal" )
	PORT_DIPSETTING(    0x03, "Hard" )
	PORT_DIPSETTING(    0x02, "Very Hard" )
	PORT_DIPSETTING(    0x01, "Hardest" )
	PORT_DIPSETTING(    0x00, "Master Level" )
	PORT_DIPNAME( 0x08, 0x00, "Unknkown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x30, 0x30, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x30, "4" )
	PORT_DIPNAME( 0xc0, 0xc0, "Vs Play Mode", IP_KEY_NONE )
	PORT_DIPSETTING(    0xc0, "1 Game Match" )
	PORT_DIPSETTING(    0x80, "3 Games Match" )
	PORT_DIPSETTING(    0x40, "5 Games Match" )
	PORT_DIPSETTING(    0x00, "7 Games Match" )

	PORT_START      /* DSWC */
	PORT_DIPNAME( 0x03, 0x03, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x04, 0x04, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x04, "On" )
	PORT_DIPNAME( 0x08, 0x08, "Freeze", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x10, 0x10, "Flip Screen", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x20, 0x00, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x40, "On" )
	PORT_BITX(    0x80, 0x80, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* Player 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_4WAY | IPF_PLAYER1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_4WAY | IPF_PLAYER1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_4WAY | IPF_PLAYER1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_4WAY | IPF_PLAYER1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* Player 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_4WAY | IPF_PLAYER2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_4WAY | IPF_PLAYER2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_4WAY | IPF_PLAYER2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_4WAY | IPF_PLAYER2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

INPUT_PORTS_START( qad_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START4 )

	PORT_START      /* DSWA */
	PORT_DIPNAME( 0x07, 0x07, "Coinage", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x01, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x02, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x07, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x06, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/4 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin/6 Credits" )
	PORT_DIPNAME( 0x08, 0x08, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x10, 0x10, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* DSWB */
	PORT_DIPNAME( 0x07, 0x06, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x06, "Very Easy" )
	PORT_DIPSETTING(    0x05, "Easy" )
	PORT_DIPSETTING(    0x04, "Normal" )
	PORT_DIPSETTING(    0x03, "Hard" )
	PORT_DIPSETTING(    0x02, "Very Hard" )
	PORT_DIPNAME( 0x18, 0x18, "Wisdom", IP_KEY_NONE )
	PORT_DIPSETTING(    0x18, "Low" )
	PORT_DIPSETTING(    0x10, "Normal" )
	PORT_DIPSETTING(    0x08, "High" )
	PORT_DIPSETTING(    0x00, "Brilliant" )
	PORT_DIPNAME( 0xe0, 0xe0, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0xa0, "3" )
	PORT_DIPSETTING(    0xc0, "4" )
	PORT_DIPSETTING(    0xe0, "5" )

	PORT_START      /* DSWC */
	PORT_DIPNAME( 0x04, 0x04, "Free Play", IP_KEY_NONE )
	PORT_DIPSETTING(    0x04, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x08, 0x08, "Freeze", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x10, 0x10, "Flip Screen", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x20, 0x20, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x20, "On" )
	PORT_DIPNAME( 0x40, 0x40, "Allow Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "No" )
	PORT_DIPSETTING(    0x40, "Yes" )
	PORT_BITX(    0x80, 0x80, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* Player 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER1 )

	PORT_START      /* Player 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER2 )
INPUT_PORTS_END

INPUT_PORTS_START( qadj_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START4 )

	PORT_START      /* DSWA */
	PORT_DIPNAME( 0x07, 0x07, "Coinage", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x01, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x02, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x07, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x06, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/4 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin/6 Credits" )
	PORT_DIPNAME( 0x08, 0x08, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x10, 0x10, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* DSWB */
	PORT_DIPNAME( 0x07, 0x07, "Game Level", IP_KEY_NONE )
	PORT_DIPSETTING(    0x07, "0" )
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x05, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPNAME( 0xf8, 0xf8, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0xd8, "2" )
	PORT_DIPSETTING(    0xf8, "3" )

	PORT_START      /* DSWC */
	PORT_DIPNAME( 0x03, 0x03, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x03, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x04, 0x04, "Free Play", IP_KEY_NONE )
	PORT_DIPSETTING(    0x04, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x08, 0x08, "Freeze", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x10, 0x10, "Flip Screen", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x20, 0x00, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x40, "Allow Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "No" )
	PORT_DIPSETTING(    0x40, "Yes" )
	PORT_BITX(    0x80, 0x80, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* Player 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER1 )

	PORT_START      /* Player 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER2 )
INPUT_PORTS_END

INPUT_PORTS_START( megaman_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* DSWA */
	PORT_DIPNAME( 0x1f, 0x1f, "Coinage", IP_KEY_NONE )
	PORT_DIPSETTING(    0x0f, "9 Coins/1 Credit" )
	PORT_DIPSETTING(    0x10, "8 Coins/1 Credit" )
	PORT_DIPSETTING(    0x11, "7 Coins/1 Credit" )
	PORT_DIPSETTING(    0x12, "6 Coins/1 Credit" )
	PORT_DIPSETTING(    0x13, "5 Coins/1 Credit" )
	PORT_DIPSETTING(    0x14, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x15, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x16, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x0e, "2 Coins/1 Credit - 1 to continue (if on)" )
	PORT_DIPSETTING(    0x1f, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x1e, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x1d, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x1c, "1 Coin/4 Credits" )
	PORT_DIPSETTING(    0x1b, "1 Coin/5 Credits" )
	PORT_DIPSETTING(    0x1a, "1 Coin/6 Credits" )
	PORT_DIPSETTING(    0x19, "1 Coin/7 Credits" )
	PORT_DIPSETTING(    0x18, "1 Coin/8 Credits" )
	PORT_DIPSETTING(    0x17, "1 Coin/9 Credits" )
	PORT_DIPSETTING(    0x0d, "Free Play" )
	/* 0x00 to 0x0c 1 Coin/1 Credit */
	PORT_DIPNAME( 0x60, 0x20, "2 Player Game", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "1 Credit" )
	PORT_DIPSETTING(    0x40, "2 Credits" )
	PORT_DIPSETTING(    0x60, "2 Credits - pl1 may play on right" )
	   /* Unused 0x00 */
	PORT_DIPNAME( 0x80, 0x80, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x80, "On" )

	PORT_START      /* DSWB */
	PORT_DIPNAME( 0x03, 0x03, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x03, "Easy" )
	PORT_DIPSETTING(    0x02, "Normal" )
	PORT_DIPSETTING(    0x01, "Difficult" )
	PORT_DIPSETTING(    0x00, "Hard" )
	PORT_DIPNAME( 0x0c, 0x0c, "Time", IP_KEY_NONE )
	PORT_DIPSETTING(    0x0c, "100" )
	PORT_DIPSETTING(    0x08, "90" )
	PORT_DIPSETTING(    0x04, "70" )
	PORT_DIPSETTING(    0x00, "60" )
	PORT_DIPNAME( 0x10, 0x10, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x10, "On" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x20, "On" )
	PORT_DIPNAME( 0x40, 0x40, "Voice", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x40, "On" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x80, "On" )

	PORT_START      /* DSWC */
	PORT_DIPNAME( 0x01, 0x01, "Flip Screen", IP_KEY_NONE )
	PORT_DIPSETTING(    0x01, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x02, 0x02, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x02, "On" )
	PORT_DIPNAME( 0x04, 0x04, "Allow Continue", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "No" )
	PORT_DIPSETTING(    0x04, "Yes" )
	PORT_DIPNAME( 0x08, 0x08, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x08, "On" )
	PORT_DIPNAME( 0x10, 0x10, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x10, "On" )
	PORT_DIPNAME( 0x20, 0x20, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x20, "On" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x40, "On" )
	PORT_BITX(    0x80, 0x80, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* Player 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* Player 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

INPUT_PORTS_START( pang3_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BITX(0x40, 0x40, IPT_SERVICE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE,0 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* DSWA (not used, EEPROM) */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* DSWB (not used, EEPROM) */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* DSWC */
	PORT_DIPNAME( 0x08, 0x08, "Freeze", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* Player 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* Player 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/********************************************************************

			Graphics Layout macros

  These are not really needed, and are used for documentation only.

********************************************************************/

#define SPRITE_LAYOUT(LAYOUT, SPRITES, SPRITE_SEP2, PLANE_SEP) \
static struct GfxLayout LAYOUT = \
{                                               \
	16,16,   /* 16*16 sprites */             \
	SPRITES,  /* ???? sprites */            \
	4,       /* 4 bits per pixel */            \
	{ PLANE_SEP+8,PLANE_SEP,8,0 },            \
	{ SPRITE_SEP2+0,SPRITE_SEP2+1,SPRITE_SEP2+2,SPRITE_SEP2+3, \
	  SPRITE_SEP2+4,SPRITE_SEP2+5,SPRITE_SEP2+6,SPRITE_SEP2+7,  \
	  0,1,2,3,4,5,6,7, },\
	{ 0*8, 2*8, 4*8, 6*8, 8*8, 10*8, 12*8, 14*8, \
	   16*8, 18*8, 20*8, 22*8, 24*8, 26*8, 28*8, 30*8, }, \
	32*8    /* every sprite takes 32*8*2 consecutive bytes */ \
};

#define CHAR_LAYOUT(LAYOUT, CHARS, PLANE_SEP) \
static struct GfxLayout LAYOUT =        \
{                                        \
	8,8,    /* 8*8 chars */             \
	CHARS,  /* ???? chars */        \
	4,       /* 4 bits per pixel */     \
	{ PLANE_SEP+8,PLANE_SEP,8,0 },     \
	{ 0,1,2,3,4,5,6,7, },                         \
	{ 0*8, 2*8, 4*8, 6*8, 8*8, 10*8, 12*8, 14*8,}, \
	16*8    /* every sprite takes 32*8*2 consecutive bytes */\
};

#define TILE32_LAYOUT(LAYOUT, TILES, SEP, PLANE_SEP) \
static struct GfxLayout LAYOUT =                                   \
{                                                                  \
	32,32,   /* 32*32 tiles */                                 \
	TILES,   /* ????  tiles */                                 \
	4,       /* 4 bits per pixel */                            \
	{ PLANE_SEP+8,PLANE_SEP,8,0},                                        \
	{                                                          \
	   SEP+0,SEP+1,SEP+2,SEP+3, SEP+4,SEP+5,SEP+6,SEP+7,       \
	   0,1,2,3,4,5,6,7,                                        \
	   16+SEP+0,16+SEP+1,16+SEP+2,                             \
	   16+SEP+3,16+SEP+4,16+SEP+5,                             \
	   16+SEP+6,16+SEP+7,                                      \
	   16+0,16+1,16+2,16+3,16+4,16+5,16+6,16+7                 \
	},                                                         \
	{                                                          \
	   0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,         \
	   8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32,   \
	   16*32, 17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32, \
	   24*32, 25*32, 26*32, 27*32, 28*32, 29*32, 30*32, 31*32  \
	},                                                         \
	4*32*8    /* every sprite takes 32*8*4 consecutive bytes */\
};

/* Generic layout, no longer needed, but very useful for testing
   Will need to change this constant to reflect the gfx region size
   for the game.
   Also change the number of characters
 */
#define CPS1_ROM_SIZE 0x00000
#define CPS1_CHARS (CPS1_ROM_SIZE/32)
CHAR_LAYOUT(cps1_charlayout,     CPS1_CHARS, CPS1_ROM_SIZE/4*16)
SPRITE_LAYOUT(cps1_spritelayout, CPS1_CHARS/4, CPS1_ROM_SIZE/4*8, CPS1_ROM_SIZE/4*16)
SPRITE_LAYOUT(cps1_tilelayout,   CPS1_CHARS/4, CPS1_ROM_SIZE/4*8, CPS1_ROM_SIZE/4*16)
TILE32_LAYOUT(cps1_tilelayout32, CPS1_CHARS/16, CPS1_ROM_SIZE/4*8, CPS1_ROM_SIZE/4*16)

static struct GfxDecodeInfo cps1_gfxdecodeinfo[] =
{
	/*   start    pointer          colour start   number of colours */
	{ 1, 0x000000, &cps1_charlayout,    32*16,                  32 },
	{ 1, 0x000000, &cps1_spritelayout,  0,                      32 },
	{ 1, 0x000000, &cps1_tilelayout,    32*16+32*16,            32 },
	{ 1, 0x000000, &cps1_tilelayout32,  32*16+32*16+32*16,      32 },
	{ -1 } /* end of array */
};



static void cps1_irq_handler_mus(void)
{
	cpu_cause_interrupt(1,0xff);
}

static struct YM2151interface ym2151_interface =
{
	1,  /* 1 chip */
	3579580,    /* 3.579580 MHz ? */
	{ CPS1_VIDEO_HZ },
	{ cps1_irq_handler_mus }
};

static struct OKIM6295interface okim6295_interface =
{
	1,  /* 1 chip */
	7576,   /* hand tuned to match the real SF2 */
	{ 3 },  /* memory region 3 */
	{ 50 }
};

static struct OKIM6295interface okim6295_interface_6061 =
{
	1,  /* 1 chip */
	6061,   /* Forgotten Worlds */
	{ 3 },  /* memory region 3 */
	{ 50 }
};



/********************************************************************
*
*  Machine Driver macro
*  ====================
*
*  Abusing the pre-processor.
*
********************************************************************/

#define MACHINE_DRIVER(CPS1_DRVNAME, CPS1_IRQ, CPS1_CPU_FRQ) \
	MACHINE_DRV(CPS1_DRVNAME, CPS1_IRQ, 1, CPS1_CPU_FRQ)

#define MACHINE_DRIVER_16BIT(CPS1_DRVNAME, CPS1_IRQ, CPS1_CPU_FRQ) \
	MACHINE_DRV_16BIT(CPS1_DRVNAME, CPS1_IRQ, 1, CPS1_CPU_FRQ)

#define MACHINE_DRV(CPS1_DRVNAME, CPS1_IRQ, CPS1_ICNT, CPS1_CPU_FRQ) \
static struct MachineDriver CPS1_DRVNAME##_machine_driver =           \
{                                                                        \
	/* basic machine hardware */                                     \
	{                                                                \
		{                                                        \
			CPU_M68000,                                      \
			CPS1_CPU_FRQ,                                    \
			0,                                               \
			cps1_readmem,cps1_writemem,0,0,                  \
			CPS1_IRQ, CPS1_ICNT  /* ??? interrupts per frame */   \
		},                                                       \
		{                                                        \
			CPU_Z80 | CPU_AUDIO_CPU,                         \
			4000000,  /* 4 Mhz ??? TODO: find real FRQ */    \
			2,      /* memory region #2 */                   \
			sound_readmem,sound_writemem,0,0,                \
			ignore_interrupt,0                               \
		}                                                        \
	},                                                               \
	CPS1_VIDEO_HZ, 0, /* wrong, but reduces jerkiness */                     \
	1,                                                               \
	0,                                                               \
									 \
	/* video hardware */                                             \
	0x30*8+32*2, 0x1c*8+32*3, { 32, 32+0x30*8-1, 32+16, 32+16+0x1c*8-1 }, \
									 \
	cps1_gfxdecodeinfo,                                              \
	32*16+32*16+32*16+32*16,   /* lotsa colours */                   \
	32*16+32*16+32*16+32*16,   /* Colour table length */             \
	0,                                                               \
									 \
	VIDEO_TYPE_RASTER | VIDEO_MODIFIES_PALETTE,                      \
	0,                                                               \
	cps1_vh_start,                                                   \
	cps1_vh_stop,                                                    \
	cps1_vh_screenrefresh,                                           \
									 \
	/* sound hardware */                                             \
	0 /*FRANXIS 05-04-2006 - SOUND_SUPPORTS_STEREO*/,0,0,0,                                     \
	{ { SOUND_YM2151,  &ym2151_interface },                          \
	  { SOUND_OKIM6295,  &okim6295_interface }                       \
	}                            \
};

#define MACHINE_DRV_16BIT(CPS1_DRVNAME, CPS1_IRQ, CPS1_ICNT, CPS1_CPU_FRQ) \
static struct MachineDriver CPS1_DRVNAME##_machine_driver =           \
{                                                                        \
	/* basic machine hardware */                                     \
	{                                                                \
		{                                                        \
			CPU_M68000,                                      \
			CPS1_CPU_FRQ,                                    \
			0,                                               \
			cps1_readmem,cps1_writemem,0,0,                  \
			CPS1_IRQ, CPS1_ICNT  /* ??? interrupts per frame */   \
		},                                                       \
		{                                                        \
			CPU_Z80 | CPU_AUDIO_CPU,                         \
			4000000,  /* 4 Mhz ??? TODO: find real FRQ */    \
			2,      /* memory region #2 */                   \
			sound_readmem,sound_writemem,0,0,                \
			ignore_interrupt,0                               \
		}                                                        \
	},                                                               \
	CPS1_VIDEO_HZ, 4000, /* wrong, but reduces jerkiness */                     \
	1,                                                               \
	0,                                                               \
									 \
	/* video hardware */                                             \
	0x30*8+32*2, 0x1c*8+32*3, { 32, 32+0x30*8-1, 32+16, 32+16+0x1c*8-1 }, \
									 \
	cps1_gfxdecodeinfo,                                              \
	32*16+32*16+32*16+32*16,   /* lotsa colours */                   \
	32*16+32*16+32*16+32*16,   /* Colour table length */             \
	0,                                                               \
									 \
	VIDEO_TYPE_RASTER | VIDEO_MODIFIES_PALETTE,                      \
	0,                                                               \
	cps1_vh_start,                                                   \
	cps1_vh_stop,                                                    \
	cps1_vh_screenrefresh,                                           \
									 \
	/* sound hardware */                                             \
	0 /*FRANXIS 05-04-2006 - SOUND_SUPPORTS_STEREO*/,0,0,0,                                     \
	{ { SOUND_YM2151,  &ym2151_interface },                          \
	  { SOUND_OKIM6295,  &okim6295_interface }                       \
	}                            \
};

#define MACHINE_DRV_FORGOT(CPS1_DRVNAME, CPS1_IRQ, CPS1_ICNT, CPS1_CPU_FRQ) \
static struct MachineDriver CPS1_DRVNAME##_machine_driver =           \
{                                                                        \
	/* basic machine hardware */                                     \
	{                                                                \
		{                                                        \
			CPU_M68000,                                      \
			CPS1_CPU_FRQ,                                    \
			0,                                               \
			cps1_readmem,cps1_writemem,0,0,                  \
			CPS1_IRQ, CPS1_ICNT  /* ??? interrupts per frame */   \
		},                                                       \
		{                                                        \
			CPU_Z80 | CPU_AUDIO_CPU,                         \
			4000000,  /* 4 Mhz ??? TODO: find real FRQ */    \
			2,      /* memory region #2 */                   \
			sound_readmem,sound_writemem,0,0,                \
			ignore_interrupt,0                               \
		}                                                        \
	},                                                               \
	CPS1_VIDEO_HZ, 0, /* wrong, but reduces jerkiness */                     \
	1,                                                               \
	0,                                                               \
									 \
	/* video hardware */                                             \
	0x30*8+32*2, 0x1c*8+32*3, { 32, 32+0x30*8-1, 32+16, 32+16+0x1c*8-1 }, \
									 \
	cps1_gfxdecodeinfo,                                              \
	32*16+32*16+32*16+32*16,   /* lotsa colours */                   \
	32*16+32*16+32*16+32*16,   /* Colour table length */             \
	0,                                                               \
									 \
	VIDEO_TYPE_RASTER | VIDEO_MODIFIES_PALETTE,                      \
	0,                                                               \
	cps1_vh_start,                                                   \
	cps1_vh_stop,                                                    \
	cps1_vh_screenrefresh,                                           \
									 \
	/* sound hardware */                                             \
	0 /*FRANXIS 05-04-2006 - SOUND_SUPPORTS_STEREO*/,0,0,0,                                     \
	{ { SOUND_YM2151,  &ym2151_interface },                          \
	  { SOUND_OKIM6295,  &okim6295_interface_6061 }                       \
	}                            \
};


#define QSOUND_MACHINE_DRIVER(CPS1_DRVNAME, CPS1_IRQ, CPS1_CPU_FRQ) \
static struct MachineDriver CPS1_DRVNAME##_machine_driver =            \
{                                                                        \
	/* basic machine hardware */                                     \
	{                                                                \
		{                                                        \
			CPU_M68000,                                      \
			CPS1_CPU_FRQ,                                    \
			0,                                               \
			cps1_readmem,cps1_writemem,0,0,                  \
			cps1_qsound_interrupt, 1 \
		},                                                       \
		{                                                        \
			CPU_Z80 | CPU_AUDIO_CPU,                         \
			4000000,  /* 4 Mhz ??? TODO: find real FRQ */    \
			2,      /* memory region #2 */                   \
			qsound_readmem,qsound_writemem,0,0,	         \
			interrupt, 8					 \
		} 							 \
	},                                                               \
	CPS1_VIDEO_HZ, 4000, /* wrong, but reduces jerkiness */          \
	1,                                                               \
	0,                                                               \
									 \
	/* video hardware */                                             \
	0x30*8+32*2, 0x1c*8+32*3, { 32, 32+0x30*8-1, 32+16, 32+16+0x1c*8-1 }, \
									 \
	cps1_gfxdecodeinfo,                                              \
	32*16+32*16+32*16+32*16,   /* lotsa colours */                   \
	32*16+32*16+32*16+32*16,   /* Colour table length */             \
	0,                                                               \
									 \
	VIDEO_TYPE_RASTER | VIDEO_MODIFIES_PALETTE,                      \
	0,                                                               \
	cps1_vh_start,                                                   \
	cps1_vh_stop,                                                    \
	cps1_vh_screenrefresh,                                           \
									 \
	/* sound hardware */                                             \
	0 /* FRANXIS 05-04-2006 - SOUND_SUPPORTS_STEREO */,0,0,0,         \
	{ { SOUND_QSOUND, &qsound_interface } }  \
};

#define CPS1_DEFAULT_CPU_FAST_SPEED     12000000
#define CPS1_DEFAULT_CPU_SPEED          10000000
#define CPS1_DEFAULT_CPU_SLOW_SPEED      8000000

static int cps1_interrupt(void)
{
	/* Strider also has a IRQ4 handler. It is input port related, but the game */
	/* works without it (maybe it's used to multiplex controls). It is the */
	/* *only* game to have that. */
	return 2;
}

MACHINE_DRV_FORGOT   ( forgottn,  cps1_interrupt, 1, CPS1_DEFAULT_CPU_SLOW_SPEED )
MACHINE_DRV   ( ghouls,    cps1_interrupt, 1, CPS1_DEFAULT_CPU_SLOW_SPEED )
MACHINE_DRIVER( strider,   cps1_interrupt, CPS1_DEFAULT_CPU_SPEED )
MACHINE_DRIVER( dwj,       cps1_interrupt, CPS1_DEFAULT_CPU_SPEED )
MACHINE_DRIVER( willow,    cps1_interrupt, CPS1_DEFAULT_CPU_SLOW_SPEED )
MACHINE_DRIVER( unsquad,   cps1_interrupt, CPS1_DEFAULT_CPU_SPEED )
MACHINE_DRIVER( ffight,    cps1_interrupt, 10000000 )  /* 10 MHz */
MACHINE_DRIVER( c1941,     cps1_interrupt, CPS1_DEFAULT_CPU_SPEED )
MACHINE_DRIVER( mercs,     cps1_interrupt, CPS1_DEFAULT_CPU_SPEED )
MACHINE_DRIVER( mtwins,    cps1_interrupt, CPS1_DEFAULT_CPU_SPEED )
MACHINE_DRIVER( msword,    cps1_interrupt, CPS1_DEFAULT_CPU_SPEED )
MACHINE_DRIVER( cawing,    cps1_interrupt, CPS1_DEFAULT_CPU_SPEED )
MACHINE_DRIVER( nemo,      cps1_interrupt, CPS1_DEFAULT_CPU_SPEED )
MACHINE_DRV   ( sf2,       cps1_interrupt, 1, 12000000 )   /* 12 MHz */
MACHINE_DRIVER( c3wonders, cps1_interrupt, CPS1_DEFAULT_CPU_SPEED )
MACHINE_DRIVER( kod,       cps1_interrupt, CPS1_DEFAULT_CPU_SPEED )
MACHINE_DRIVER( captcomm,  cps1_interrupt, CPS1_DEFAULT_CPU_SPEED )
MACHINE_DRIVER( knights,   cps1_interrupt, CPS1_DEFAULT_CPU_SPEED )
MACHINE_DRIVER( varth,     cps1_interrupt, CPS1_DEFAULT_CPU_SPEED )
MACHINE_DRIVER_16BIT( cworld2j,  cps1_interrupt, CPS1_DEFAULT_CPU_SPEED )
QSOUND_MACHINE_DRIVER( wof,      cps1_interrupt, CPS1_DEFAULT_CPU_SPEED )
QSOUND_MACHINE_DRIVER( dino,     cps1_interrupt, CPS1_DEFAULT_CPU_SPEED )
QSOUND_MACHINE_DRIVER( punisher, cps1_interrupt, CPS1_DEFAULT_CPU_SPEED )
QSOUND_MACHINE_DRIVER( slammast, cps1_interrupt, CPS1_DEFAULT_CPU_SPEED )
QSOUND_MACHINE_DRIVER( mbomber,  cps1_interrupt, CPS1_DEFAULT_CPU_SPEED )
MACHINE_DRIVER( pnickj,    cps1_interrupt, CPS1_DEFAULT_CPU_SPEED )
MACHINE_DRIVER( qad,       cps1_interrupt, CPS1_DEFAULT_CPU_SPEED )
MACHINE_DRIVER_16BIT( megaman,   cps1_interrupt, CPS1_DEFAULT_CPU_SPEED )
MACHINE_DRIVER_16BIT( pang3,     cps1_interrupt, CPS1_DEFAULT_CPU_SPEED )



/***************************************************************************

  Game driver(s)

***************************************************************************/

#define CODE_SIZE 0x200000

ROM_START( forgottn_rom )
	ROM_REGION(CODE_SIZE)
	ROM_LOAD_EVEN( "lwu11a.14f",    0x00000, 0x20000, 0xddf78831 ) /* 68000 code */
	ROM_LOAD_ODD ( "lwu15a.14g",    0x00000, 0x20000, 0xf7ce2097 ) /* 68000 code */
	ROM_LOAD_EVEN( "lwu10a.13f",    0x40000, 0x20000, 0x8cb38c81 ) /* 68000 code */
	ROM_LOAD_ODD ( "lwu14a.13g",    0x40000, 0x20000, 0xd70ef9fd ) /* 68000 code */

	ROM_REGION_DISPOSE(0x400000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "lwu.1",     0x080000, 0x80000, 0x00000000 ) /* Sprites / tiles */
	ROM_LOAD( "lwu.3",     0x180000, 0x80000, 0x00000000 )
	ROM_LOAD( "lwu.5",     0x280000, 0x80000, 0x00000000 )
	ROM_LOAD( "lwu.7",     0x380000, 0x80000, 0x00000000 )

	ROM_REGION(0x18000) /* 64k for the audio CPU */
	ROM_LOAD( "lwu-00.14a",    0x000000, 0x08000, 0x59df2a63 )
	ROM_CONTINUE(              0x010000, 0x08000 )
ROM_END

ROM_START( ghouls_rom )
	ROM_REGION(CODE_SIZE)
	ROM_LOAD_EVEN( "ghl29.bin",    0x00000, 0x20000, 0x166a58a2 )
	ROM_LOAD_ODD ( "ghl30.bin",    0x00000, 0x20000, 0x7ac8407a )
	ROM_LOAD_EVEN( "ghl27.bin",    0x40000, 0x20000, 0xf734b2be )
	ROM_LOAD_ODD ( "ghl28.bin",    0x40000, 0x20000, 0x03d3e714 )
	ROM_LOAD_WIDE( "ghl17.bin",    0x80000, 0x80000, 0x3ea1b0f2 )

	ROM_REGION_DISPOSE(0x400000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD         ( "ghl6.bin",     0x000000, 0x80000, 0x4ba90b59 )
	ROM_LOAD_GFX_EVEN( "ghl11.bin",    0x080000, 0x10000, 0x37c9b6c6 )
	ROM_LOAD_GFX_ODD ( "ghl20.bin",    0x080000, 0x10000, 0x2f1345b4 )
	ROM_LOAD_GFX_EVEN( "ghl12.bin",    0x0a0000, 0x10000, 0xda088d61 )
	ROM_LOAD_GFX_ODD ( "ghl21.bin",    0x0a0000, 0x10000, 0x17e11df0 )
	ROM_LOAD         ( "ghl5.bin",     0x100000, 0x80000, 0x0ba9c0b0 )
	ROM_LOAD_GFX_EVEN( "ghl09.bin",    0x180000, 0x10000, 0xae24bb19 )
	ROM_LOAD_GFX_ODD ( "ghl18.bin",    0x180000, 0x10000, 0xd34e271a )
	ROM_LOAD_GFX_EVEN( "ghl10.bin",    0x1a0000, 0x10000, 0xbcc0f28c )
	ROM_LOAD_GFX_ODD ( "ghl19.bin",    0x1a0000, 0x10000, 0x2a40166a )
	ROM_LOAD         ( "ghl8.bin",     0x200000, 0x80000, 0x4bdee9de )
	ROM_LOAD_GFX_EVEN( "ghl15.bin",    0x280000, 0x10000, 0x3c2a212a )
	ROM_LOAD_GFX_ODD ( "ghl24.bin",    0x280000, 0x10000, 0x889aac05 )
	ROM_LOAD_GFX_EVEN( "ghl16.bin",    0x2a0000, 0x10000, 0xf187ba1c )
	ROM_LOAD_GFX_ODD ( "ghl25.bin",    0x2a0000, 0x10000, 0x29f79c78 )
	ROM_LOAD         ( "ghl7.bin",     0x300000, 0x80000, 0x5d760ab9 )
	ROM_LOAD_GFX_EVEN( "ghl13.bin",    0x380000, 0x10000, 0x3f70dd37 )
	ROM_LOAD_GFX_ODD ( "ghl22.bin",    0x380000, 0x10000, 0x7e69e2e6 )
	ROM_LOAD_GFX_EVEN( "ghl14.bin",    0x3a0000, 0x10000, 0x20f85c03 )
	ROM_LOAD_GFX_ODD ( "ghl23.bin",    0x3a0000, 0x10000, 0x8426144b )

	ROM_REGION(0x18000) /* 64k for the audio CPU */
	ROM_LOAD( "ghl26.bin",    0x000000, 0x08000, 0x3692f6e5 )
	ROM_CONTINUE(             0x010000, 0x08000 )
ROM_END

ROM_START( ghoulsj_rom )
	ROM_REGION(CODE_SIZE)
	ROM_LOAD_EVEN( "ghlj29.bin",   0x00000, 0x20000, 0x82fd1798 )
	ROM_LOAD_ODD ( "ghlj30.bin",   0x00000, 0x20000, 0x35366ccc )
	ROM_LOAD_EVEN( "ghlj27.bin",   0x40000, 0x20000, 0xa17c170a )
	ROM_LOAD_ODD ( "ghlj28.bin",   0x40000, 0x20000, 0x6af0b391 )
	ROM_LOAD_WIDE( "ghl17.bin",    0x80000, 0x80000, 0x3ea1b0f2 )

	ROM_REGION_DISPOSE(0x400000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD         ( "ghl6.bin",     0x000000, 0x80000, 0x4ba90b59 )
	ROM_LOAD_GFX_EVEN( "ghl12.bin",    0x080000, 0x10000, 0xda088d61 )
	ROM_LOAD_GFX_ODD ( "ghl21.bin",    0x080000, 0x10000, 0x17e11df0 )
	ROM_LOAD_GFX_EVEN( "ghl11.bin",    0x0a0000, 0x10000, 0x37c9b6c6 )
	ROM_LOAD_GFX_ODD ( "ghl20.bin",    0x0a0000, 0x10000, 0x2f1345b4 )
	ROM_LOAD         ( "ghl5.bin",     0x100000, 0x80000, 0x0ba9c0b0 )
	ROM_LOAD_GFX_EVEN( "ghl10.bin",    0x180000, 0x10000, 0xbcc0f28c )
	ROM_LOAD_GFX_ODD ( "ghl19.bin",    0x180000, 0x10000, 0x2a40166a )
	ROM_LOAD_GFX_EVEN( "ghl09.bin",    0x1a0000, 0x10000, 0xae24bb19 )
	ROM_LOAD_GFX_ODD ( "ghl18.bin",    0x1a0000, 0x10000, 0xd34e271a )
	ROM_LOAD         ( "ghl8.bin",     0x200000, 0x80000, 0x4bdee9de )
	ROM_LOAD_GFX_EVEN( "ghl16.bin",    0x280000, 0x10000, 0xf187ba1c )
	ROM_LOAD_GFX_ODD ( "ghl25.bin",    0x280000, 0x10000, 0x29f79c78 )
	ROM_LOAD_GFX_EVEN( "ghl15.bin",    0x2a0000, 0x10000, 0x3c2a212a )
	ROM_LOAD_GFX_ODD ( "ghl24.bin",    0x2a0000, 0x10000, 0x889aac05 )
	ROM_LOAD         ( "ghl7.bin",     0x300000, 0x80000, 0x5d760ab9 )
	ROM_LOAD_GFX_EVEN( "ghl14.bin",    0x380000, 0x10000, 0x20f85c03 )
	ROM_LOAD_GFX_ODD ( "ghl23.bin",    0x380000, 0x10000, 0x8426144b )
	ROM_LOAD_GFX_EVEN( "ghl13.bin",    0x3a0000, 0x10000, 0x3f70dd37 )
	ROM_LOAD_GFX_ODD ( "ghl22.bin",    0x3a0000, 0x10000, 0x7e69e2e6 )

	ROM_REGION(0x18000) /* 64k for the audio CPU */
	ROM_LOAD( "ghl26.bin",    0x000000, 0x08000, 0x3692f6e5 )
	ROM_CONTINUE(             0x010000, 0x08000 )
ROM_END

ROM_START( strider_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "strider.30",   0x00000, 0x20000, 0xda997474 )
	ROM_LOAD_ODD ( "strider.35",   0x00000, 0x20000, 0x5463aaa3 )
	ROM_LOAD_EVEN( "strider.31",   0x40000, 0x20000, 0xd20786db )
	ROM_LOAD_ODD ( "strider.36",   0x40000, 0x20000, 0x21aa2863 )
	ROM_LOAD_WIDE_SWAP( "strider.32",   0x80000, 0x80000, 0x9b3cfc08 )

	ROM_REGION_DISPOSE(0x400000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "strider.02",   0x000000, 0x80000, 0x7705aa46 )
	ROM_LOAD( "strider.01",   0x080000, 0x80000, 0xb7d04e8b )
	ROM_LOAD( "strider.06",   0x100000, 0x80000, 0x4eee9aea )
	ROM_LOAD( "strider.05",   0x180000, 0x80000, 0x005f000b )
	ROM_LOAD( "strider.04",   0x200000, 0x80000, 0x5b18b722 )
	ROM_LOAD( "strider.03",   0x280000, 0x80000, 0x6b4713b4 )
	ROM_LOAD( "strider.08",   0x300000, 0x80000, 0x2d7f21e4 )
	ROM_LOAD( "strider.07",   0x380000, 0x80000, 0xb9441519 )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "strider.09",   0x00000, 0x10000, 0x2ed403bc )
	ROM_RELOAD(               0x08000, 0x10000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD( "strider.18",   0x00000, 0x20000, 0x4386bc80 )
	ROM_LOAD( "strider.19",   0x20000, 0x20000, 0x444536d7 )
ROM_END

ROM_START( striderj_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_WIDE_SWAP( "sthj23.bin",   0x00000, 0x80000, 0x046e7b12 )
	ROM_LOAD_WIDE_SWAP( "strider.32",   0x80000, 0x80000, 0x9b3cfc08 )

	ROM_REGION_DISPOSE(0x400000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "strider.02",   0x000000, 0x80000, 0x7705aa46 )
	ROM_LOAD( "strider.01",   0x080000, 0x80000, 0xb7d04e8b )
	ROM_LOAD( "strider.06",   0x100000, 0x80000, 0x4eee9aea )
	ROM_LOAD( "strider.05",   0x180000, 0x80000, 0x005f000b )
	ROM_LOAD( "strider.04",   0x200000, 0x80000, 0x5b18b722 )
	ROM_LOAD( "strider.03",   0x280000, 0x80000, 0x6b4713b4 )
	ROM_LOAD( "strider.08",   0x300000, 0x80000, 0x2d7f21e4 )
	ROM_LOAD( "strider.07",   0x380000, 0x80000, 0xb9441519 )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "strider.09",   0x00000, 0x10000, 0x2ed403bc )
	ROM_RELOAD(               0x08000, 0x10000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD( "strider.18",   0x00000, 0x20000, 0x4386bc80 )
	ROM_LOAD( "strider.19",   0x20000, 0x20000, 0x444536d7 )
ROM_END

ROM_START( dwj_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "36.bin",       0x00000, 0x20000, 0x1a516657 )
	ROM_LOAD_ODD ( "42.bin",       0x00000, 0x20000, 0x12a290a0 )
	ROM_LOAD_EVEN( "37.bin",       0x40000, 0x20000, 0x932fc943 )
	ROM_LOAD_ODD ( "43.bin",       0x40000, 0x20000, 0x872ad76d )
	ROM_LOAD_EVEN( "34.bin",       0x80000, 0x20000, 0x8f663d00 )
	ROM_LOAD_ODD ( "40.bin",       0x80000, 0x20000, 0x1586dbf3 )
	ROM_LOAD_EVEN( "35.bin",       0xc0000, 0x20000, 0x9db93d7a )
	ROM_LOAD_ODD ( "41.bin",       0xc0000, 0x20000, 0x1aae69a4 )

	ROM_REGION_DISPOSE(0x400000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD_GFX_EVEN( "24.bin",       0x000000, 0x20000, 0xc6909b6f )
	ROM_LOAD_GFX_ODD ( "17.bin",       0x000000, 0x20000, 0x2e2f8320 )
	ROM_LOAD_GFX_EVEN( "25.bin",       0x040000, 0x20000, 0x152ea74a )
	ROM_LOAD_GFX_ODD ( "18.bin",       0x040000, 0x20000, 0x1833f932 )
	ROM_LOAD_GFX_EVEN( "26.bin",       0x080000, 0x20000, 0x07fc714b )
	ROM_LOAD_GFX_ODD ( "19.bin",       0x080000, 0x20000, 0x7114e5c6 )
	ROM_LOAD_GFX_EVEN( "27.bin",       0x0c0000, 0x20000, 0xa27e81fa )
	ROM_LOAD_GFX_ODD ( "20.bin",       0x0c0000, 0x20000, 0x002796dc )
	ROM_LOAD_GFX_EVEN( "09.bin",       0x100000, 0x20000, 0xc3e83c69 )
	ROM_LOAD_GFX_ODD ( "01.bin",       0x100000, 0x20000, 0x187b2886 )
	ROM_LOAD_GFX_EVEN( "10.bin",       0x140000, 0x20000, 0xff28f8d0 )
	ROM_LOAD_GFX_ODD ( "02.bin",       0x140000, 0x20000, 0xcc83c02f )
	ROM_LOAD_GFX_EVEN( "11.bin",       0x180000, 0x20000, 0x29eaf490 )
	ROM_LOAD_GFX_ODD ( "03.bin",       0x180000, 0x20000, 0x7bf51337 )
	ROM_LOAD_GFX_EVEN( "12.bin",       0x1c0000, 0x20000, 0x38652339 )
	ROM_LOAD_GFX_ODD ( "04.bin",       0x1c0000, 0x20000, 0x4951bc0f )
	ROM_LOAD_GFX_EVEN( "38.bin",       0x200000, 0x20000, 0xcd7923ed )
	ROM_LOAD_GFX_ODD ( "32.bin",       0x200000, 0x20000, 0x21a0a453 )
	ROM_LOAD_GFX_EVEN( "39.bin",       0x240000, 0x20000, 0xbc09b360 )
	ROM_LOAD_GFX_ODD ( "33.bin",       0x240000, 0x20000, 0x89de1533 )
	ROM_LOAD_GFX_EVEN( "28.bin",       0x280000, 0x20000, 0xaf62bf07 )
	ROM_LOAD_GFX_ODD ( "21.bin",       0x280000, 0x20000, 0x523f462a )
	ROM_LOAD_GFX_EVEN( "29.bin",       0x2c0000, 0x20000, 0x6b41f82d )
	ROM_LOAD_GFX_ODD ( "22.bin",       0x2c0000, 0x20000, 0x52145369 )
	ROM_LOAD_GFX_EVEN( "13.bin",       0x300000, 0x20000, 0x0273d87d )
	ROM_LOAD_GFX_ODD ( "05.bin",       0x300000, 0x20000, 0x339378b8 )
	ROM_LOAD_GFX_EVEN( "14.bin",       0x340000, 0x20000, 0x18fb232c )
	ROM_LOAD_GFX_ODD ( "06.bin",       0x340000, 0x20000, 0x6f9edd75 )
	ROM_LOAD_GFX_EVEN( "15.bin",       0x380000, 0x20000, 0xd36cdb91 )
	ROM_LOAD_GFX_ODD ( "07.bin",       0x380000, 0x20000, 0xe04af054 )
	ROM_LOAD_GFX_EVEN( "16.bin",       0x3c0000, 0x20000, 0x381608ae )
	ROM_LOAD_GFX_ODD ( "08.bin",       0x3c0000, 0x20000, 0xb475d4e9 )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "23.bin",       0x000000, 0x08000, 0xb3b79d4f )
	ROM_CONTINUE(             0x010000, 0x08000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD ( "30.bin",       0x00000, 0x20000, 0x7e5f6cb4 )
	ROM_LOAD ( "31.bin",       0x20000, 0x20000, 0x4a30c737 )
ROM_END

ROM_START( willow_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "wlu_30.rom",   0x00000, 0x20000, 0xd604dbb1 )
	ROM_LOAD_ODD ( "wlu_35.rom",   0x00000, 0x20000, 0xdaee72fe )
	ROM_LOAD_EVEN( "wlu_31.rom",   0x40000, 0x20000, 0x0eb48a83 )
	ROM_LOAD_ODD ( "wlu_36.rom",   0x40000, 0x20000, 0x36100209 )
	ROM_LOAD_WIDE_SWAP( "wl_32.rom",    0x80000, 0x80000, 0xdfd9f643 )

	ROM_REGION_DISPOSE(0x400000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD         ( "wl_gfx1.rom",  0x000000, 0x80000, 0xc6f2abce )
	ROM_LOAD_GFX_EVEN( "wl_20.rom",    0x080000, 0x20000, 0x84992350 )
	ROM_LOAD_GFX_ODD ( "wl_10.rom",    0x080000, 0x20000, 0xb87b5a36 )
	ROM_LOAD         ( "wl_gfx5.rom",  0x100000, 0x80000, 0xafa74b73 )
	ROM_LOAD_GFX_EVEN( "wl_24.rom",    0x180000, 0x20000, 0x6f0adee5 )
	ROM_LOAD_GFX_ODD ( "wl_14.rom",    0x180000, 0x20000, 0x9cf3027d )
	ROM_LOAD         ( "wl_gfx3.rom",  0x200000, 0x80000, 0x4aa4c6d3 )
	ROM_LOAD_GFX_EVEN( "wl_22.rom",    0x280000, 0x20000, 0xfd3f89f0 )
	ROM_LOAD_GFX_ODD ( "wl_12.rom",    0x280000, 0x20000, 0x7da49d69 )
	ROM_LOAD         ( "wl_gfx7.rom",  0x300000, 0x80000, 0x12a0dc0b )
	ROM_LOAD_GFX_EVEN( "wl_26.rom",    0x380000, 0x20000, 0xf09c8ecf )
	ROM_LOAD_GFX_ODD ( "wl_16.rom",    0x380000, 0x20000, 0xe35407aa )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "wl_09.rom",    0x00000, 0x08000, 0xf6b3d060 )
	ROM_CONTINUE(             0x10000, 0x08000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD ( "wl_18.rom",    0x00000, 0x20000, 0xbde23d4d )
	ROM_LOAD ( "wl_19.rom",    0x20000, 0x20000, 0x683898f5 )
ROM_END

ROM_START( willowj_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "wl36.bin",     0x00000, 0x20000, 0x2b0d7cbc )
	ROM_LOAD_ODD ( "wl42.bin",     0x00000, 0x20000, 0x1ac39615 )
	ROM_LOAD_EVEN( "wl37.bin",     0x40000, 0x20000, 0x30a717fa )
	ROM_LOAD_ODD ( "wl43.bin",     0x40000, 0x20000, 0xd0dddc9e )
	ROM_LOAD_WIDE_SWAP( "wl_32.rom",    0x80000, 0x80000, 0xdfd9f643 )

	ROM_REGION_DISPOSE(0x400000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD         ( "wl_gfx1.rom",  0x000000, 0x80000, 0xc6f2abce )
	ROM_LOAD_GFX_EVEN( "wl_20.rom",    0x080000, 0x20000, 0x84992350 )
	ROM_LOAD_GFX_ODD ( "wl_10.rom",    0x080000, 0x20000, 0xb87b5a36 )
	ROM_LOAD         ( "wl_gfx5.rom",  0x100000, 0x80000, 0xafa74b73 )
	ROM_LOAD_GFX_EVEN( "wl_24.rom",    0x180000, 0x20000, 0x6f0adee5 )
	ROM_LOAD_GFX_ODD ( "wl_14.rom",    0x180000, 0x20000, 0x9cf3027d )
	ROM_LOAD         ( "wl_gfx3.rom",  0x200000, 0x80000, 0x4aa4c6d3 )
	ROM_LOAD_GFX_EVEN( "wl_22.rom",    0x280000, 0x20000, 0xfd3f89f0 )
	ROM_LOAD_GFX_ODD ( "wl_12.rom",    0x280000, 0x20000, 0x7da49d69 )
	ROM_LOAD         ( "wl_gfx7.rom",  0x300000, 0x80000, 0x12a0dc0b )
	ROM_LOAD_GFX_EVEN( "wl_26.rom",    0x380000, 0x20000, 0xf09c8ecf )
	ROM_LOAD_GFX_ODD ( "wl_16.rom",    0x380000, 0x20000, 0xe35407aa )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "wl_09.rom",    0x00000, 0x08000, 0xf6b3d060 )
	ROM_CONTINUE(             0x10000, 0x08000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD ( "wl_18.rom",    0x00000, 0x20000, 0xbde23d4d )
	ROM_LOAD ( "wl_19.rom",    0x20000, 0x20000, 0x683898f5 )
ROM_END

ROM_START( unsquad_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "unsquad.30",   0x00000, 0x20000, 0x24d8f88d )
	ROM_LOAD_ODD ( "unsquad.35",   0x00000, 0x20000, 0x8b954b59 )
	ROM_LOAD_EVEN( "unsquad.31",   0x40000, 0x20000, 0x33e9694b )
	ROM_LOAD_ODD ( "unsquad.36",   0x40000, 0x20000, 0x7cc8fb9e )
	ROM_LOAD_WIDE_SWAP( "unsquad.32",   0x80000, 0x80000, 0xae1d7fb0 )

	ROM_REGION_DISPOSE(0x200000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "unsquad.01",   0x000000, 0x80000, 0x5965ca8d )
	ROM_LOAD( "unsquad.05",   0x080000, 0x80000, 0xbf4575d8 )
	ROM_LOAD( "unsquad.03",   0x100000, 0x80000, 0xac6db17d )
	ROM_LOAD( "unsquad.07",   0x180000, 0x80000, 0xa02945f4 )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "unsquad.09",   0x00000, 0x10000, 0xf3dd1367 )
	ROM_RELOAD(               0x08000, 0x10000 )

	ROM_REGION(0x20000) /* Samples */
	ROM_LOAD ( "unsquad.18",   0x00000, 0x20000, 0x584b43a9 )
ROM_END

ROM_START( area88_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "ar36.bin",     0x00000, 0x20000, 0x65030392 )
	ROM_LOAD_ODD ( "ar42.bin",     0x00000, 0x20000, 0xc48170de )
	ROM_LOAD_EVEN( "unsquad.31",   0x40000, 0x20000, 0x33e9694b )
	ROM_LOAD_ODD ( "unsquad.36",   0x40000, 0x20000, 0x7cc8fb9e )
	ROM_LOAD_WIDE_SWAP( "unsquad.32",   0x80000, 0x80000, 0xae1d7fb0 )

	ROM_REGION_DISPOSE(0x200000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "unsquad.01",   0x000000, 0x80000, 0x5965ca8d )
	ROM_LOAD( "unsquad.05",   0x080000, 0x80000, 0xbf4575d8 )
	ROM_LOAD( "unsquad.03",   0x100000, 0x80000, 0xac6db17d )
	ROM_LOAD( "unsquad.07",   0x180000, 0x80000, 0xa02945f4 )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "unsquad.09",   0x00000, 0x10000, 0xf3dd1367 )
	ROM_RELOAD(               0x08000, 0x10000 )

	ROM_REGION(0x20000) /* Samples */
	ROM_LOAD ( "unsquad.18",   0x00000, 0x20000, 0x584b43a9 )
ROM_END

ROM_START( ffight_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "ff30-36.bin",  0x00000, 0x20000, 0xf9a5ce83 )
	ROM_LOAD_ODD ( "ff35-42.bin",  0x00000, 0x20000, 0x65f11215 )
	ROM_LOAD_EVEN( "ff31-37.bin",  0x40000, 0x20000, 0xe1033784 )
	ROM_LOAD_ODD ( "ff36-43.bin",  0x40000, 0x20000, 0x995e968a )
	ROM_LOAD_WIDE_SWAP( "ff32-32m.bin", 0x80000, 0x80000, 0xc747696e )

	ROM_REGION_DISPOSE(0x200000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "ff01-01m.bin", 0x000000, 0x80000, 0x0b605e44 )
	ROM_LOAD( "ff05-05m.bin", 0x080000, 0x80000, 0x9c284108 )
	ROM_LOAD( "ff03-03m.bin", 0x100000, 0x80000, 0x52291cd2 )
	ROM_LOAD( "ff07-07m.bin", 0x180000, 0x80000, 0xa7584dfb )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ff09-09.bin",  0x00000, 0x10000, 0xb8367eb5 )
	ROM_RELOAD(               0x08000, 0x10000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD ( "ff18-18.bin",  0x00000, 0x20000, 0x375c66e7 )
	ROM_LOAD ( "ff19-19.bin",  0x20000, 0x20000, 0x1ef137f9 )
ROM_END

ROM_START( ffightu_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "36",           0x00000, 0x20000, 0xe2a48af9 )
	ROM_LOAD_ODD ( "42",           0x00000, 0x20000, 0xf4bb480e )
	ROM_LOAD_EVEN( "37",           0x40000, 0x20000, 0xc371c667 )
	ROM_LOAD_ODD ( "43",           0x40000, 0x20000, 0x2f5771f9 )
	ROM_LOAD_WIDE_SWAP( "ff32-32m.bin", 0x80000, 0x80000, 0xc747696e )

	/* Note: the gfx ROMs were missing from this set. I used the ones from */
	/* the World version, assuming the if the scantily clad woman shouldn't */
	/* be seen in Europe, it shouldn't be seen in the USA as well. */
	ROM_REGION(0x200000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "ff01-01m.bin", 0x000000, 0x80000, 0x0b605e44 )
	ROM_LOAD( "ff05-05m.bin", 0x080000, 0x80000, 0x9c284108 )
	ROM_LOAD( "ff03-03m.bin", 0x100000, 0x80000, 0x52291cd2 )
	ROM_LOAD( "ff07-07m.bin", 0x180000, 0x80000, 0xa7584dfb )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ff09-09.bin",  0x00000, 0x10000, 0xb8367eb5 )
	ROM_RELOAD(               0x08000, 0x10000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD ( "ff18-18.bin",  0x00000, 0x20000, 0x375c66e7 )
	ROM_LOAD ( "ff19-19.bin",  0x20000, 0x20000, 0x1ef137f9 )
ROM_END

ROM_START( ffightj_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "ff30-36.bin",  0x00000, 0x20000, 0xf9a5ce83 )
	ROM_LOAD_ODD ( "ff35-42.bin",  0x00000, 0x20000, 0x65f11215 )
	ROM_LOAD_EVEN( "ff31-37.bin",  0x40000, 0x20000, 0xe1033784 )
	ROM_LOAD_ODD ( "ff43.bin",     0x40000, 0x20000, 0xb6dee1c3 )
	ROM_LOAD_WIDE_SWAP( "ff32-32m.bin", 0x80000, 0x80000, 0xc747696e )

	ROM_REGION_DISPOSE(0x200000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD_GFX_EVEN( "ff24.bin",     0x000000, 0x20000, 0xa1ab607a )
	ROM_LOAD_GFX_ODD ( "ff17.bin",     0x000000, 0x20000, 0x2dc18cf4 )
	ROM_LOAD_GFX_EVEN( "ff25.bin",     0x040000, 0x20000, 0x6e8181ea )
	ROM_LOAD_GFX_ODD ( "ff18.bin",     0x040000, 0x20000, 0xb19ede59 )
	ROM_LOAD_GFX_EVEN( "ff09.bin",     0x080000, 0x20000, 0x5b116d0d )
	ROM_LOAD_GFX_ODD ( "ff01.bin",     0x080000, 0x20000, 0x815b1797 )
	ROM_LOAD_GFX_EVEN( "ff10.bin",     0x0c0000, 0x20000, 0x624a924a )
	ROM_LOAD_GFX_ODD ( "ff02.bin",     0x0c0000, 0x20000, 0x5d91f694 )
	ROM_LOAD_GFX_EVEN( "ff38.bin",     0x100000, 0x20000, 0x6535a57f )
	ROM_LOAD_GFX_ODD ( "ff32.bin",     0x100000, 0x20000, 0xc8bc4a57 )
	ROM_LOAD_GFX_EVEN( "ff39.bin",     0x140000, 0x20000, 0x9416b477 )
	ROM_LOAD_GFX_ODD ( "ff33.bin",     0x140000, 0x20000, 0x7369fa07 )
	ROM_LOAD_GFX_EVEN( "ff13.bin",     0x180000, 0x20000, 0x8721a7da )
	ROM_LOAD_GFX_ODD ( "ff05.bin",     0x180000, 0x20000, 0xd0fcd4b5 )
	ROM_LOAD_GFX_EVEN( "ff14.bin",     0x1c0000, 0x20000, 0x0a2e9101 )
	ROM_LOAD_GFX_ODD ( "ff06.bin",     0x1c0000, 0x20000, 0x1c18f042 )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ff09-09.bin",  0x00000, 0x10000, 0xb8367eb5 )
	ROM_RELOAD(               0x08000, 0x10000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD ( "ff18-18.bin",  0x00000, 0x20000, 0x375c66e7 )
	ROM_LOAD ( "ff19-19.bin",  0x20000, 0x20000, 0x1ef137f9 )
ROM_END

ROM_START( c1941_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "41e_30.rom",   0x00000, 0x20000, 0x9deb1e75 )
	ROM_LOAD_ODD ( "41e_35.rom",   0x00000, 0x20000, 0xd63942b3 )
	ROM_LOAD_EVEN( "41e_31.rom",   0x40000, 0x20000, 0xdf201112 )
	ROM_LOAD_ODD ( "41e_36.rom",   0x40000, 0x20000, 0x816a818f )
	ROM_LOAD_WIDE_SWAP( "41_32.rom",    0x80000, 0x80000, 0x4e9648ca )

	ROM_REGION_DISPOSE(0x200000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "41_gfx1.rom",  0x000000, 0x80000, 0xff77985a )
	ROM_LOAD( "41_gfx5.rom",  0x080000, 0x80000, 0x01d1cb11 )
	ROM_LOAD( "41_gfx3.rom",  0x100000, 0x80000, 0x983be58f )
	ROM_LOAD( "41_gfx7.rom",  0x180000, 0x80000, 0xaeaa3509 )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "41_09.rom",    0x000000, 0x08000, 0x0f9d8527 )
	ROM_CONTINUE(             0x010000, 0x08000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD( "41_18.rom",    0x00000, 0x20000, 0xd1f15aeb )
	ROM_LOAD( "41_19.rom",    0x20000, 0x20000, 0x15aec3a6 )
ROM_END

ROM_START( c1941j_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "4136.bin",     0x00000, 0x20000, 0x7fbd42ab )
	ROM_LOAD_ODD ( "4142.bin",     0x00000, 0x20000, 0xc7781f89 )
	ROM_LOAD_EVEN( "4137.bin",     0x40000, 0x20000, 0xc6464b0b )
	ROM_LOAD_ODD ( "4143.bin",     0x40000, 0x20000, 0x440fc0b5 )
	ROM_LOAD_WIDE_SWAP( "41_32.rom",    0x80000, 0x80000, 0x4e9648ca )

	ROM_REGION_DISPOSE(0x200000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "41_gfx1.rom",  0x000000, 0x80000, 0xff77985a )
	ROM_LOAD( "41_gfx5.rom",  0x080000, 0x80000, 0x01d1cb11 )
	ROM_LOAD( "41_gfx3.rom",  0x100000, 0x80000, 0x983be58f )
	ROM_LOAD( "41_gfx7.rom",  0x180000, 0x80000, 0xaeaa3509 )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "41_09.rom",    0x000000, 0x08000, 0x0f9d8527 )
	ROM_CONTINUE(             0x010000, 0x08000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD( "41_18.rom",    0x00000, 0x20000, 0xd1f15aeb )
	ROM_LOAD( "41_19.rom",    0x20000, 0x20000, 0x15aec3a6 )
ROM_END

ROM_START( mercs_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "so2_30e.rom",  0x00000, 0x20000, 0xe17f9bf7 )
	ROM_LOAD_ODD ( "so2_35e.rom",  0x00000, 0x20000, 0x78e63575 )
	ROM_LOAD_EVEN( "so2_31e.rom",  0x40000, 0x20000, 0x51204d36 )
	ROM_LOAD_ODD ( "so2_36e.rom",  0x40000, 0x20000, 0x9cfba8b4 )
	ROM_LOAD_WIDE_SWAP( "so2_32.rom",   0x80000, 0x80000, 0x2eb5cf0c )

	ROM_REGION_DISPOSE(0x400000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD         ( "so2_gfx2.rom", 0x000000, 0x80000, 0x597c2875 )
	ROM_LOAD_GFX_EVEN( "so2_20.rom",   0x080000, 0x20000, 0x8ca751a3 )
	ROM_LOAD_GFX_ODD ( "so2_10.rom",   0x080000, 0x20000, 0xe9f569fd )
	ROM_LOAD         ( "so2_gfx6.rom", 0x100000, 0x80000, 0xaa6102af )
	ROM_LOAD_GFX_EVEN( "so2_24.rom",   0x180000, 0x20000, 0x3f254efe )
	ROM_LOAD_GFX_ODD ( "so2_14.rom",   0x180000, 0x20000, 0xf5a8905e )
	ROM_LOAD         ( "so2_gfx4.rom", 0x200000, 0x80000, 0x912a9ca0 )
	ROM_LOAD_GFX_EVEN( "so2_22.rom",    0x280000, 0x20000, 0xfce9a377 )
	ROM_LOAD_GFX_ODD ( "so2_12.rom",    0x280000, 0x20000, 0xb7df8a06 )
	ROM_LOAD         ( "so2_gfx8.rom", 0x300000, 0x80000, 0x839e6869 )
	ROM_LOAD_GFX_EVEN( "so2_26.rom",    0x380000, 0x20000, 0xf3aa5a4a )
	ROM_LOAD_GFX_ODD ( "so2_16.rom",    0x380000, 0x20000, 0xb43cd1a8 )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "so2_09.rom",   0x00000, 0x10000, 0xd09d7c7a )
	ROM_RELOAD(               0x08000, 0x10000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD( "so2_18.rom",   0x00000, 0x20000, 0xbbea1643 )
	ROM_LOAD( "so2_19.rom",   0x20000, 0x20000, 0xac58aa71 )
ROM_END

ROM_START( mercsu_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "so2_30e.rom",  0x00000, 0x20000, 0xe17f9bf7 )
	ROM_LOAD_ODD ( "s02-35",       0x00000, 0x20000, 0x4477df61 )
	ROM_LOAD_EVEN( "so2_31e.rom",  0x40000, 0x20000, 0x51204d36 )
	ROM_LOAD_ODD ( "so2_36e.rom",  0x40000, 0x20000, 0x9cfba8b4 )
	ROM_LOAD_WIDE_SWAP( "so2_32.rom",   0x80000, 0x80000, 0x2eb5cf0c )

	ROM_REGION(0x400000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD         ( "so2_gfx2.rom", 0x000000, 0x80000, 0x597c2875 )
	ROM_LOAD_GFX_EVEN( "so2_20.rom",   0x080000, 0x20000, 0x8ca751a3 )
	ROM_LOAD_GFX_ODD ( "so2_10.rom",   0x080000, 0x20000, 0xe9f569fd )
	ROM_LOAD         ( "so2_gfx6.rom", 0x100000, 0x80000, 0xaa6102af )
	ROM_LOAD_GFX_EVEN( "so2_24.rom",   0x180000, 0x20000, 0x3f254efe )
	ROM_LOAD_GFX_ODD ( "so2_14.rom",   0x180000, 0x20000, 0xf5a8905e )
	ROM_LOAD         ( "so2_gfx4.rom", 0x200000, 0x80000, 0x912a9ca0 )
	ROM_LOAD_GFX_EVEN( "so2_22.rom",    0x280000, 0x20000, 0xfce9a377 )
	ROM_LOAD_GFX_ODD ( "so2_12.rom",    0x280000, 0x20000, 0xb7df8a06 )
	ROM_LOAD         ( "so2_gfx8.rom", 0x300000, 0x80000, 0x839e6869 )
	ROM_LOAD_GFX_EVEN( "so2_26.rom",    0x380000, 0x20000, 0xf3aa5a4a )
	ROM_LOAD_GFX_ODD ( "so2_16.rom",    0x380000, 0x20000, 0xb43cd1a8 )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "so2_09.rom",   0x00000, 0x10000, 0xd09d7c7a )
	ROM_RELOAD(               0x08000, 0x10000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD( "so2_18.rom",   0x00000, 0x20000, 0xbbea1643 )
	ROM_LOAD( "so2_19.rom",   0x20000, 0x20000, 0xac58aa71 )
ROM_END

ROM_START( mercsj_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "so2_30e.rom",  0x00000, 0x20000, 0xe17f9bf7 )
	ROM_LOAD_ODD ( "so2_42.bin",   0x00000, 0x20000, 0x2c3884c6 )
	ROM_LOAD_EVEN( "so2_31e.rom",  0x40000, 0x20000, 0x51204d36 )
	ROM_LOAD_ODD ( "so2_36e.rom",  0x40000, 0x20000, 0x9cfba8b4 )
	ROM_LOAD_WIDE_SWAP( "so2_32.rom",   0x80000, 0x80000, 0x2eb5cf0c )

	ROM_REGION_DISPOSE(0x400000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD         ( "so2_gfx2.rom", 0x000000, 0x80000, 0x597c2875 )
	ROM_LOAD_GFX_EVEN( "so2_20.rom",   0x080000, 0x20000, 0x8ca751a3 )
	ROM_LOAD_GFX_ODD ( "so2_10.rom",   0x080000, 0x20000, 0xe9f569fd )
	ROM_LOAD         ( "so2_gfx6.rom", 0x100000, 0x80000, 0xaa6102af )
	ROM_LOAD_GFX_EVEN( "so2_24.rom",   0x180000, 0x20000, 0x3f254efe )
	ROM_LOAD_GFX_ODD ( "so2_14.rom",   0x180000, 0x20000, 0xf5a8905e )
	ROM_LOAD         ( "so2_gfx4.rom", 0x200000, 0x80000, 0x912a9ca0 )
	ROM_LOAD_GFX_EVEN( "so2_22.rom",   0x280000, 0x20000, 0xfce9a377 )
	ROM_LOAD_GFX_ODD ( "so2_12.rom",   0x280000, 0x20000, 0xb7df8a06 )
	ROM_LOAD         ( "so2_gfx8.rom", 0x300000, 0x80000, 0x839e6869 )
	ROM_LOAD_GFX_EVEN( "so2_26.rom",   0x380000, 0x20000, 0xf3aa5a4a )
	ROM_LOAD_GFX_ODD ( "so2_16.rom",   0x380000, 0x20000, 0xb43cd1a8 )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "so2_09.rom",   0x00000, 0x10000, 0xd09d7c7a )
	ROM_RELOAD(               0x08000, 0x10000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD( "so2_18.rom",   0x00000, 0x20000, 0xbbea1643 )
	ROM_LOAD( "so2_19.rom",   0x20000, 0x20000, 0xac58aa71 )
ROM_END

ROM_START( mtwins_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "che_30.rom",   0x00000, 0x20000, 0x9a2a2db1 )
	ROM_LOAD_ODD ( "che_35.rom",   0x00000, 0x20000, 0xa7f96b02 )
	ROM_LOAD_EVEN( "che_31.rom",   0x40000, 0x20000, 0xbbff8a99 )
	ROM_LOAD_ODD ( "che_36.rom",   0x40000, 0x20000, 0x0fa00c39 )
	ROM_LOAD_WIDE_SWAP( "ch_32.rom",    0x80000, 0x80000, 0x9b70bd41 )

	ROM_REGION_DISPOSE(0x200000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "ch_gfx1.rom",  0x000000, 0x80000, 0xf33ca9d4 )
	ROM_LOAD( "ch_gfx5.rom",  0x080000, 0x80000, 0x4ec75f15 )
	ROM_LOAD( "ch_gfx3.rom",  0x100000, 0x80000, 0x0ba2047f )
	ROM_LOAD( "ch_gfx7.rom",  0x180000, 0x80000, 0xd85d00d6 )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ch_09.rom",    0x00000, 0x10000, 0x4d4255b7 )
	ROM_RELOAD(               0x08000, 0x10000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD( "ch_18.rom",    0x00000, 0x20000, 0xf909e8de )
	ROM_LOAD( "ch_19.rom",    0x20000, 0x20000, 0xfc158cf7 )
ROM_END

ROM_START( chikij_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "chj36a.bin",   0x00000, 0x20000, 0xec1328d8 )
	ROM_LOAD_ODD ( "chj42a.bin",   0x00000, 0x20000, 0x4ae13503 )
	ROM_LOAD_EVEN( "chj37a.bin",   0x40000, 0x20000, 0x46d2cf7b )
	ROM_LOAD_ODD ( "chj43a.bin",   0x40000, 0x20000, 0x8d387fe8 )
	ROM_LOAD_WIDE_SWAP( "ch_32.rom",    0x80000, 0x80000, 0x9b70bd41 )

	ROM_REGION(0x200000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "ch_gfx1.rom",  0x000000, 0x80000, 0xf33ca9d4 )
	ROM_LOAD( "ch_gfx5.rom",  0x080000, 0x80000, 0x4ec75f15 )
	ROM_LOAD( "ch_gfx3.rom",  0x100000, 0x80000, 0x0ba2047f )
	ROM_LOAD( "ch_gfx7.rom",  0x180000, 0x80000, 0xd85d00d6 )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ch_09.rom",    0x00000, 0x10000, 0x4d4255b7 )
	ROM_RELOAD(            0x08000, 0x10000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD( "ch_18.rom",    0x00000, 0x20000, 0xf909e8de )
	ROM_LOAD( "ch_19.rom",    0x20000, 0x20000, 0xfc158cf7 )
ROM_END

ROM_START( msword_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "mse_30.rom",   0x00000, 0x20000, 0x03fc8dbc )
	ROM_LOAD_ODD ( "mse_35.rom",   0x00000, 0x20000, 0xd5bf66cd )
	ROM_LOAD_EVEN( "mse_31.rom",   0x40000, 0x20000, 0x30332bcf )
	ROM_LOAD_ODD ( "mse_36.rom",   0x40000, 0x20000, 0x8f7d6ce9 )
	ROM_LOAD_WIDE_SWAP( "ms_32.rom",    0x80000, 0x80000, 0x2475ddfc )

	ROM_REGION_DISPOSE(0x200000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "ms_gfx1.rom",  0x000000, 0x80000, 0x0d2bbe00 )
	ROM_LOAD( "ms_gfx5.rom",  0x080000, 0x80000, 0xc00fe7e2 )
	ROM_LOAD( "ms_gfx3.rom",  0x100000, 0x80000, 0x3a1a5bf4 )
	ROM_LOAD( "ms_gfx7.rom",  0x180000, 0x80000, 0x4ccacac5 )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ms_9.rom",     0x00000, 0x10000, 0x57b29519 )
	ROM_RELOAD(               0x08000, 0x10000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD( "ms_18.rom",    0x00000, 0x20000, 0xfb64e90d )
	ROM_LOAD( "ms_19.rom",    0x20000, 0x20000, 0x74f892b9 )
ROM_END

ROM_START( mswordu_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "msu30",   0x00000, 0x20000, 0xd963c816 )
	ROM_LOAD_ODD ( "msu35",   0x00000, 0x20000, 0x72f179b3 )
	ROM_LOAD_EVEN( "msu31",   0x40000, 0x20000, 0x20cd7904 )
	ROM_LOAD_ODD ( "msu36",   0x40000, 0x20000, 0xbf88c080 )
	ROM_LOAD_WIDE_SWAP( "ms_32.rom",    0x80000, 0x80000, 0x2475ddfc )

	ROM_REGION_DISPOSE(0x200000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "ms_gfx1.rom",  0x000000, 0x80000, 0x0d2bbe00 )
	ROM_LOAD( "ms_gfx5.rom",  0x080000, 0x80000, 0xc00fe7e2 )
	ROM_LOAD( "ms_gfx3.rom",  0x100000, 0x80000, 0x3a1a5bf4 )
	ROM_LOAD( "ms_gfx7.rom",  0x180000, 0x80000, 0x4ccacac5 )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ms_9.rom",     0x00000, 0x10000, 0x57b29519 )
	ROM_RELOAD(               0x08000, 0x10000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD( "ms_18.rom",    0x00000, 0x20000, 0xfb64e90d )
	ROM_LOAD( "ms_19.rom",    0x20000, 0x20000, 0x74f892b9 )
ROM_END

ROM_START( mswordj_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "msj_30.rom",   0x00000, 0x20000, 0x04f0ef50 )
	ROM_LOAD_ODD ( "msj_35.rom",   0x00000, 0x20000, 0x9fcbb9cd )
	ROM_LOAD_EVEN( "msj_31.rom",   0x40000, 0x20000, 0x6c060d70 )
	ROM_LOAD_ODD ( "msj_36.rom",   0x40000, 0x20000, 0xaec77787 )
	ROM_LOAD_WIDE_SWAP( "ms_32.rom",    0x80000, 0x80000, 0x2475ddfc )

	ROM_REGION_DISPOSE(0x200000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "ms_gfx1.rom",  0x000000, 0x80000, 0x0d2bbe00 )
	ROM_LOAD( "ms_gfx5.rom",  0x080000, 0x80000, 0xc00fe7e2 )
	ROM_LOAD( "ms_gfx3.rom",  0x100000, 0x80000, 0x3a1a5bf4 )
	ROM_LOAD( "ms_gfx7.rom",  0x180000, 0x80000, 0x4ccacac5 )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ms_9.rom",     0x00000, 0x10000, 0x57b29519 )
	ROM_RELOAD(               0x08000, 0x10000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD( "ms_18.rom",    0x00000, 0x20000, 0xfb64e90d )
	ROM_LOAD( "ms_19.rom",    0x20000, 0x20000, 0x74f892b9 )
ROM_END

ROM_START( cawing_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "cae_30a.rom",  0x00000, 0x20000, 0x91fceacd )
	ROM_LOAD_ODD ( "cae_35a.rom",  0x00000, 0x20000, 0x3ef03083 )
	ROM_LOAD_EVEN( "cae_31a.rom",  0x40000, 0x20000, 0xe5b75caf )
	ROM_LOAD_ODD ( "cae_36a.rom",  0x40000, 0x20000, 0xc73fd713 )
	ROM_LOAD_WIDE_SWAP( "ca_32.rom", 0x80000, 0x80000, 0x0c4837d4 )

	ROM_REGION_DISPOSE(0x200000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "ca_gfx1.rom",  0x000000, 0x80000, 0x4d0620fd )
	ROM_LOAD( "ca_gfx5.rom",  0x080000, 0x80000, 0x66d4cc37 )
	ROM_LOAD( "ca_gfx3.rom",  0x100000, 0x80000, 0x0b0341c3 )
	ROM_LOAD( "ca_gfx7.rom",  0x180000, 0x80000, 0xb6f896f2 )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ca_9.rom",     0x00000, 0x08000, 0x96fe7485 )
	ROM_CONTINUE(             0x10000, 0x08000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD ( "ca_18.rom",    0x00000, 0x20000, 0x4a613a2c )
	ROM_LOAD ( "ca_19.rom",    0x20000, 0x20000, 0x74584493 )
ROM_END

ROM_START( cawingj_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "cae_30a.rom",  0x00000, 0x20000, 0x91fceacd )
	ROM_LOAD_ODD ( "caj42a.bin",   0x00000, 0x20000, 0x039f8362 )
	ROM_LOAD_EVEN( "cae_31a.rom",  0x40000, 0x20000, 0xe5b75caf )
	ROM_LOAD_ODD ( "cae_36a.rom",  0x40000, 0x20000, 0xc73fd713 )
	ROM_LOAD_EVEN( "caj34.bin",    0x80000, 0x20000, 0x51ea57f4 )
	ROM_LOAD_ODD ( "caj40.bin",    0x80000, 0x20000, 0x2ab71ae1 )
	ROM_LOAD_EVEN( "caj35.bin",    0xc0000, 0x20000, 0x01d71973 )
	ROM_LOAD_ODD ( "caj41.bin",    0xc0000, 0x20000, 0x3a43b538 )

	ROM_REGION(0x200000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD_GFX_EVEN( "caj24.bin",    0x000000, 0x20000, 0xe356aad7 )
	ROM_LOAD_GFX_ODD ( "caj17.bin",    0x000000, 0x20000, 0x540f2fd8 )
	ROM_LOAD_GFX_EVEN( "caj25.bin",    0x040000, 0x20000, 0xcdd0204d )
	ROM_LOAD_GFX_ODD ( "caj18.bin",    0x040000, 0x20000, 0x29c1d4b1 )
	ROM_LOAD_GFX_EVEN( "caj09.bin",    0x080000, 0x20000, 0x41b0f9a6 )
	ROM_LOAD_GFX_ODD ( "caj01.bin",    0x080000, 0x20000, 0x1002d0b8 )
	ROM_LOAD_GFX_EVEN( "caj10.bin",    0x0c0000, 0x20000, 0xbf8a5f52 )
	ROM_LOAD_GFX_ODD ( "caj02.bin",    0x0c0000, 0x20000, 0x125b018d )
	ROM_LOAD_GFX_EVEN( "caj38.bin",    0x100000, 0x20000, 0x2464d4ab )
	ROM_LOAD_GFX_ODD ( "caj32.bin",    0x100000, 0x20000, 0x9b5836b3 )
	ROM_LOAD_GFX_EVEN( "caj39.bin",    0x140000, 0x20000, 0xeea23b67 )
	ROM_LOAD_GFX_ODD ( "caj33.bin",    0x140000, 0x20000, 0xdde3891f )
	ROM_LOAD_GFX_EVEN( "caj13.bin",    0x180000, 0x20000, 0x6f3948b2 )
	ROM_LOAD_GFX_ODD ( "caj05.bin",    0x180000, 0x20000, 0x207373d7 )
	ROM_LOAD_GFX_EVEN( "caj14.bin",    0x1c0000, 0x20000, 0x8458e7d7 )
	ROM_LOAD_GFX_ODD ( "caj06.bin",    0x1c0000, 0x20000, 0xcf80e164 )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ca_9.rom",     0x00000, 0x08000, 0x96fe7485 )
	ROM_CONTINUE(             0x10000, 0x08000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD ( "ca_18.rom",    0x00000, 0x20000, 0x4a613a2c )
	ROM_LOAD ( "ca_19.rom",    0x20000, 0x20000, 0x74584493 )
ROM_END

ROM_START( nemo_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "nme_30a.rom",  0x00000, 0x20000, 0xd2c03e56 )
	ROM_LOAD_ODD ( "nme_35a.rom",  0x00000, 0x20000, 0x5fd31661 )
	ROM_LOAD_EVEN( "nme_31a.rom",  0x40000, 0x20000, 0xb2bd4f6f )
	ROM_LOAD_ODD ( "nme_36a.rom",  0x40000, 0x20000, 0xee9450e3 )
	ROM_LOAD_WIDE_SWAP( "nm_32.rom",    0x80000, 0x80000, 0xd6d1add3 )

	ROM_REGION_DISPOSE(0x200000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "nm_gfx1.rom",  0x000000, 0x80000, 0x9e878024 )
	ROM_LOAD( "nm_gfx5.rom",  0x080000, 0x80000, 0x487b8747 )
	ROM_LOAD( "nm_gfx3.rom",  0x100000, 0x80000, 0xbb01e6b6 )
	ROM_LOAD( "nm_gfx7.rom",  0x180000, 0x80000, 0x203dc8c6 )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "nm_09.rom",    0x000000, 0x08000, 0x0f4b0581 )
	ROM_CONTINUE(             0x010000, 0x08000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD( "nm_18.rom",    0x00000, 0x20000, 0xbab333d4 )
	ROM_LOAD( "nm_19.rom",    0x20000, 0x20000, 0x2650a0a8 )
ROM_END

ROM_START( nemoj_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "nm36.bin",     0x00000, 0x20000, 0xdaeceabb )
	ROM_LOAD_ODD ( "nm42.bin",     0x00000, 0x20000, 0x55024740 )
	ROM_LOAD_EVEN( "nm37.bin",     0x40000, 0x20000, 0x619068b6 )
	ROM_LOAD_ODD ( "nm43.bin",     0x40000, 0x20000, 0xa948a53b )
	ROM_LOAD_WIDE_SWAP( "nm_32.rom",    0x80000, 0x80000, 0xd6d1add3 )

	ROM_REGION_DISPOSE(0x200000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "nm_gfx1.rom",  0x000000, 0x80000, 0x9e878024 )
	ROM_LOAD( "nm_gfx5.rom",  0x080000, 0x80000, 0x487b8747 )
	ROM_LOAD( "nm_gfx3.rom",  0x100000, 0x80000, 0xbb01e6b6 )
	ROM_LOAD( "nm_gfx7.rom",  0x180000, 0x80000, 0x203dc8c6 )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "nm_09.rom",    0x000000, 0x08000, 0x0f4b0581 )
	ROM_CONTINUE(             0x010000, 0x08000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD( "nm_18.rom",    0x00000, 0x20000, 0xbab333d4 )
	ROM_LOAD( "nm_19.rom",    0x20000, 0x20000, 0x2650a0a8 )
ROM_END

ROM_START( sf2_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "sf2e_30b.rom",    0x00000, 0x20000, 0x57bd7051 )
	ROM_LOAD_ODD ( "sf2e_37b.rom",    0x00000, 0x20000, 0x62691cdd )
	ROM_LOAD_EVEN( "sf2e_31b.rom",    0x40000, 0x20000, 0xa673143d )
	ROM_LOAD_ODD ( "sf2e_38b.rom",    0x40000, 0x20000, 0x4c2ccef7 )
	ROM_LOAD_EVEN( "sf2_28b.rom",     0x80000, 0x20000, 0x4009955e )
	ROM_LOAD_ODD ( "sf2_35b.rom",     0x80000, 0x20000, 0x8c1f3994 )
	ROM_LOAD_EVEN( "sf2_29b.rom",     0xc0000, 0x20000, 0xbb4af315 )
	ROM_LOAD_ODD ( "sf2_36b.rom",     0xc0000, 0x20000, 0xc02a13eb )

	ROM_REGION_DISPOSE(0x600000)
	ROM_LOAD( "sf2gfx01.rom",       0x000000, 0x80000, 0xba529b4f )
	ROM_LOAD( "sf2gfx10.rom",       0x080000, 0x80000, 0x14b84312 )
	ROM_LOAD( "sf2gfx20.rom",       0x100000, 0x80000, 0xc1befaa8 )
	ROM_LOAD( "sf2gfx02.rom",       0x180000, 0x80000, 0x22c9cc8e )
	ROM_LOAD( "sf2gfx11.rom",       0x200000, 0x80000, 0x2c7e2229 )
	ROM_LOAD( "sf2gfx21.rom",       0x280000, 0x80000, 0x994bfa58 )
	ROM_LOAD( "sf2gfx03.rom",       0x300000, 0x80000, 0x4b1b33a8 )
	ROM_LOAD( "sf2gfx12.rom",       0x380000, 0x80000, 0x5e9cd89a )
	ROM_LOAD( "sf2gfx22.rom",       0x400000, 0x80000, 0x0627c831 )
	ROM_LOAD( "sf2gfx04.rom",       0x480000, 0x80000, 0x57213be8 )
	ROM_LOAD( "sf2gfx13.rom",       0x500000, 0x80000, 0xb5548f17 )
	ROM_LOAD( "sf2gfx23.rom",       0x580000, 0x80000, 0x3e66ad9d )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sf2_09.rom",   0x00000, 0x08000, 0xa4823a1b )
	ROM_CONTINUE(             0x10000, 0x08000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD ( "sf2_18.rom",       0x00000, 0x20000, 0x7f162009 )
	ROM_LOAD ( "sf2_19.rom",       0x20000, 0x20000, 0xbeade53f )
ROM_END

ROM_START( sf2a_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "sf2u.30a",    0x00000, 0x20000, 0x08beb861 )
	ROM_LOAD_ODD ( "sf2u.37a",    0x00000, 0x20000, 0xb7638d69 )
	ROM_LOAD_EVEN( "sf2u.31a",    0x40000, 0x20000, 0x0d5394e0 )
	ROM_LOAD_ODD ( "sf2u.38a",    0x40000, 0x20000, 0x42d6a79e )
	ROM_LOAD_EVEN( "sf2u.28a",    0x80000, 0x20000, 0x387a175c )
	ROM_LOAD_ODD ( "sf2u.35a",    0x80000, 0x20000, 0xa1a5adcc )
	ROM_LOAD_EVEN( "sf2_29b.rom", 0xc0000, 0x20000, 0xbb4af315 )
	ROM_LOAD_ODD ( "sf2_36b.rom", 0xc0000, 0x20000, 0xc02a13eb )

	ROM_REGION_DISPOSE(0x600000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "sf2gfx01.rom",       0x000000, 0x80000, 0xba529b4f )
	ROM_LOAD( "sf2gfx10.rom",       0x080000, 0x80000, 0x14b84312 )
	ROM_LOAD( "sf2gfx20.rom",       0x100000, 0x80000, 0xc1befaa8 )
	ROM_LOAD( "sf2gfx02.rom",       0x180000, 0x80000, 0x22c9cc8e )
	ROM_LOAD( "sf2gfx11.rom",       0x200000, 0x80000, 0x2c7e2229 )
	ROM_LOAD( "sf2gfx21.rom",       0x280000, 0x80000, 0x994bfa58 )
	ROM_LOAD( "sf2gfx03.rom",       0x300000, 0x80000, 0x4b1b33a8 )
	ROM_LOAD( "sf2gfx12.rom",       0x380000, 0x80000, 0x5e9cd89a )
	ROM_LOAD( "sf2gfx22.rom",       0x400000, 0x80000, 0x0627c831 )
	ROM_LOAD( "sf2gfx04.rom",       0x480000, 0x80000, 0x57213be8 )
	ROM_LOAD( "sf2gfx13.rom",       0x500000, 0x80000, 0xb5548f17 )
	ROM_LOAD( "sf2gfx23.rom",       0x580000, 0x80000, 0x3e66ad9d )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sf2_09.rom",   0x00000, 0x08000, 0xa4823a1b )
	ROM_CONTINUE(             0x10000, 0x08000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD ( "sf2_18.rom",       0x00000, 0x20000, 0x7f162009 )
	ROM_LOAD ( "sf2_19.rom",       0x20000, 0x20000, 0xbeade53f )
ROM_END

ROM_START( sf2b_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "sf2e_30b.rom",   0x00000, 0x20000, 0x57bd7051 )
	ROM_LOAD_ODD ( "sf2u.37b",       0x00000, 0x20000, 0x4a54d479 )
	ROM_LOAD_EVEN( "sf2e_31b.rom",   0x40000, 0x20000, 0xa673143d )
	ROM_LOAD_ODD ( "sf2e_38b.rom",   0x40000, 0x20000, 0x4c2ccef7 )
	ROM_LOAD_EVEN( "sf2_28b.rom",    0x80000, 0x20000, 0x4009955e )
	ROM_LOAD_ODD ( "sf2_35b.rom",    0x80000, 0x20000, 0x8c1f3994 )
	ROM_LOAD_EVEN( "sf2_29b.rom",    0xc0000, 0x20000, 0xbb4af315 )
	ROM_LOAD_ODD ( "sf2_36b.rom",    0xc0000, 0x20000, 0xc02a13eb )

	ROM_REGION_DISPOSE(0x600000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "sf2gfx01.rom",       0x000000, 0x80000, 0xba529b4f )
	ROM_LOAD( "sf2gfx10.rom",       0x080000, 0x80000, 0x14b84312 )
	ROM_LOAD( "sf2gfx20.rom",       0x100000, 0x80000, 0xc1befaa8 )
	ROM_LOAD( "sf2gfx02.rom",       0x180000, 0x80000, 0x22c9cc8e )
	ROM_LOAD( "sf2gfx11.rom",       0x200000, 0x80000, 0x2c7e2229 )
	ROM_LOAD( "sf2gfx21.rom",       0x280000, 0x80000, 0x994bfa58 )
	ROM_LOAD( "sf2gfx03.rom",       0x300000, 0x80000, 0x4b1b33a8 )
	ROM_LOAD( "sf2gfx12.rom",       0x380000, 0x80000, 0x5e9cd89a )
	ROM_LOAD( "sf2gfx22.rom",       0x400000, 0x80000, 0x0627c831 )
	ROM_LOAD( "sf2gfx04.rom",       0x480000, 0x80000, 0x57213be8 )
	ROM_LOAD( "sf2gfx13.rom",       0x500000, 0x80000, 0xb5548f17 )
	ROM_LOAD( "sf2gfx23.rom",       0x580000, 0x80000, 0x3e66ad9d )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sf2_09.rom",   0x00000, 0x08000, 0xa4823a1b )
	ROM_CONTINUE(             0x10000, 0x08000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD ( "sf2_18.rom",       0x00000, 0x20000, 0x7f162009 )
	ROM_LOAD ( "sf2_19.rom",       0x20000, 0x20000, 0xbeade53f )
ROM_END

ROM_START( sf2e_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "sf2u.30e",    0x00000, 0x20000, 0xf37cd088 )
	ROM_LOAD_ODD ( "sf2u.37e",    0x00000, 0x20000, 0x6c61a513 )
	ROM_LOAD_EVEN( "sf2u.31e",    0x40000, 0x20000, 0x7c4771b4 )
	ROM_LOAD_ODD ( "sf2u.38e",    0x40000, 0x20000, 0xa4bd0cd9 )
	ROM_LOAD_EVEN( "sf2u.28e",    0x80000, 0x20000, 0xe3b95625 )
	ROM_LOAD_ODD ( "sf2u.35e",    0x80000, 0x20000, 0x3648769a )
	ROM_LOAD_EVEN( "sf2_29b.rom", 0xc0000, 0x20000, 0xbb4af315 )
	ROM_LOAD_ODD ( "sf2_36b.rom", 0xc0000, 0x20000, 0xc02a13eb )

	ROM_REGION_DISPOSE(0x600000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "sf2gfx01.rom",       0x000000, 0x80000, 0xba529b4f )
	ROM_LOAD( "sf2gfx10.rom",       0x080000, 0x80000, 0x14b84312 )
	ROM_LOAD( "sf2gfx20.rom",       0x100000, 0x80000, 0xc1befaa8 )
	ROM_LOAD( "sf2gfx02.rom",       0x180000, 0x80000, 0x22c9cc8e )
	ROM_LOAD( "sf2gfx11.rom",       0x200000, 0x80000, 0x2c7e2229 )
	ROM_LOAD( "sf2gfx21.rom",       0x280000, 0x80000, 0x994bfa58 )
	ROM_LOAD( "sf2gfx03.rom",       0x300000, 0x80000, 0x4b1b33a8 )
	ROM_LOAD( "sf2gfx12.rom",       0x380000, 0x80000, 0x5e9cd89a )
	ROM_LOAD( "sf2gfx22.rom",       0x400000, 0x80000, 0x0627c831 )
	ROM_LOAD( "sf2gfx04.rom",       0x480000, 0x80000, 0x57213be8 )
	ROM_LOAD( "sf2gfx13.rom",       0x500000, 0x80000, 0xb5548f17 )
	ROM_LOAD( "sf2gfx23.rom",       0x580000, 0x80000, 0x3e66ad9d )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sf2_09.rom",   0x00000, 0x08000, 0xa4823a1b )
	ROM_CONTINUE(             0x10000, 0x08000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD ( "sf2_18.rom",       0x00000, 0x20000, 0x7f162009 )
	ROM_LOAD ( "sf2_19.rom",       0x20000, 0x20000, 0xbeade53f )
ROM_END

ROM_START( sf2j_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "sf2j30.bin",    0x00000, 0x20000, 0x79022b31 )
	ROM_LOAD_ODD ( "sf2j37.bin",    0x00000, 0x20000, 0x516776ec )
	ROM_LOAD_EVEN( "sf2j31.bin",    0x40000, 0x20000, 0xfe15cb39 )
	ROM_LOAD_ODD ( "sf2j38.bin",    0x40000, 0x20000, 0x38614d70 )
	ROM_LOAD_EVEN( "sf2j28.bin",    0x80000, 0x20000, 0xd283187a )
	ROM_LOAD_ODD ( "sf2j35.bin",    0x80000, 0x20000, 0xd28158e4 )
	ROM_LOAD_EVEN( "sf2_29b.rom",   0xc0000, 0x20000, 0xbb4af315 )
	ROM_LOAD_ODD ( "sf2_36b.rom",   0xc0000, 0x20000, 0xc02a13eb )

	ROM_REGION_DISPOSE(0x600000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "sf2gfx01.rom",       0x000000, 0x80000, 0xba529b4f )
	ROM_LOAD( "sf2gfx10.rom",       0x080000, 0x80000, 0x14b84312 )
	ROM_LOAD( "sf2gfx20.rom",       0x100000, 0x80000, 0xc1befaa8 )
	ROM_LOAD( "sf2gfx02.rom",       0x180000, 0x80000, 0x22c9cc8e )
	ROM_LOAD( "sf2gfx11.rom",       0x200000, 0x80000, 0x2c7e2229 )
	ROM_LOAD( "sf2gfx21.rom",       0x280000, 0x80000, 0x994bfa58 )
	ROM_LOAD( "sf2gfx03.rom",       0x300000, 0x80000, 0x4b1b33a8 )
	ROM_LOAD( "sf2gfx12.rom",       0x380000, 0x80000, 0x5e9cd89a )
	ROM_LOAD( "sf2gfx22.rom",       0x400000, 0x80000, 0x0627c831 )
	ROM_LOAD( "sf2gfx04.rom",       0x480000, 0x80000, 0x57213be8 )
	ROM_LOAD( "sf2gfx13.rom",       0x500000, 0x80000, 0xb5548f17 )
	ROM_LOAD( "sf2gfx23.rom",       0x580000, 0x80000, 0x3e66ad9d )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sf2_09.rom",   0x00000, 0x08000, 0xa4823a1b )
	ROM_CONTINUE(             0x10000, 0x08000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD ( "sf2_18.rom",       0x00000, 0x20000, 0x7f162009 )
	ROM_LOAD ( "sf2_19.rom",       0x20000, 0x20000, 0xbeade53f )
ROM_END

ROM_START( c3wonders_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "3wonders.30",  0x00000, 0x20000, 0x0b156fd8 )
	ROM_LOAD_ODD ( "3wonders.35",  0x00000, 0x20000, 0x57350bf4 )
	ROM_LOAD_EVEN( "3wonders.31",  0x40000, 0x20000, 0x0e723fcc )
	ROM_LOAD_ODD ( "3wonders.36",  0x40000, 0x20000, 0x523a45dc )
	ROM_LOAD_EVEN( "3wonders.28",  0x80000, 0x20000, 0x054137c8 )
	ROM_LOAD_ODD ( "3wonders.33",  0x80000, 0x20000, 0x7264cb1b )
	ROM_LOAD_EVEN( "3wonders.29",  0xc0000, 0x20000, 0x37ba3e20 )
	ROM_LOAD_ODD ( "3wonders.34",  0xc0000, 0x20000, 0xf99f46c0 )

	ROM_REGION_DISPOSE(0x400000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "3wonders.01",  0x000000, 0x80000, 0x902489d0 )
	ROM_LOAD( "3wonders.02",  0x080000, 0x80000, 0xe9a034f4 )
	ROM_LOAD( "3wonders.05",  0x100000, 0x80000, 0x86aef804 )
	ROM_LOAD( "3wonders.06",  0x180000, 0x80000, 0x13cb0e7c )
	ROM_LOAD( "3wonders.03",  0x200000, 0x80000, 0xe35ce720 )
	ROM_LOAD( "3wonders.04",  0x280000, 0x80000, 0xdf0eea8b )
	ROM_LOAD( "3wonders.07",  0x300000, 0x80000, 0x4f057110 )
	ROM_LOAD( "3wonders.08",  0x380000, 0x80000, 0x1f055014 )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "3wonders.09",  0x00000, 0x08000, 0xabfca165 )    /* could have one bad byte */
	ROM_CONTINUE(             0x10000, 0x08000 )                /* (compare with Japan version) */

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD ( "3wonders.18",  0x00000, 0x20000, 0x26b211ab )
	ROM_LOAD ( "3wonders.19",  0x20000, 0x20000, 0xdbe64ad0 )
ROM_END

ROM_START( c3wonderj_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "rtj36.bin",    0x00000, 0x20000, 0xe3741247 )
	ROM_LOAD_ODD ( "rtj42.bin",    0x00000, 0x20000, 0xb4baa117 )
	ROM_LOAD_EVEN( "rtj37.bin",    0x40000, 0x20000, 0xa1f677b0 )
	ROM_LOAD_ODD ( "rtj43.bin",    0x40000, 0x20000, 0x85337a47 )

	ROM_LOAD_EVEN( "3wonders.28",  0x80000, 0x20000, 0x054137c8 )
	ROM_LOAD_ODD ( "3wonders.33",  0x80000, 0x20000, 0x7264cb1b )
	ROM_LOAD_EVEN( "rtj35.bin",    0xc0000, 0x20000, 0xe72f9ea3 )
	ROM_LOAD_ODD ( "rtj41.bin",    0xc0000, 0x20000, 0xa11ee998 )

	ROM_REGION_DISPOSE(0x400000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "3wonders.01",  0x000000, 0x80000, 0x902489d0 )
	ROM_LOAD( "3wonders.02",  0x080000, 0x80000, 0xe9a034f4 )
	ROM_LOAD( "3wonders.05",  0x100000, 0x80000, 0x86aef804 )
	ROM_LOAD( "3wonders.06",  0x180000, 0x80000, 0x13cb0e7c )
	ROM_LOAD( "3wonders.03",  0x200000, 0x80000, 0xe35ce720 )
	ROM_LOAD( "3wonders.04",  0x280000, 0x80000, 0xdf0eea8b )
	ROM_LOAD( "3wonders.07",  0x300000, 0x80000, 0x4f057110 )
	ROM_LOAD( "3wonders.08",  0x380000, 0x80000, 0x1f055014 )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "rt23.bin",     0x00000, 0x08000, 0x7d5a77a7 )    /* could have one bad byte */
	ROM_CONTINUE(             0x10000, 0x08000 )                /* (compare with US version) */

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD ( "3wonders.18",  0x00000, 0x20000, 0x26b211ab )
	ROM_LOAD ( "3wonders.19",  0x20000, 0x20000, 0xdbe64ad0 )
ROM_END

ROM_START( kod_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "kod30.rom",    0x00000, 0x20000, 0xc7414fd4 )
	ROM_LOAD_ODD ( "kod37.rom",    0x00000, 0x20000, 0xa5bf40d2 )
	ROM_LOAD_EVEN( "kod31.rom",    0x40000, 0x20000, 0x1fffc7bd )
	ROM_LOAD_ODD ( "kod38.rom",    0x40000, 0x20000, 0x89e57a82 )
	ROM_LOAD_EVEN( "kod28.rom",    0x80000, 0x20000, 0x9367bcd9 )
	ROM_LOAD_ODD ( "kod35.rom",    0x80000, 0x20000, 0x4ca6a48a )
	ROM_LOAD_EVEN( "kod29.rom",    0xc0000, 0x20000, 0x6a0ba878 )
	ROM_LOAD_ODD ( "kod36.rom",    0xc0000, 0x20000, 0xb509b39d )

	ROM_REGION_DISPOSE(0x400000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "kod01.rom",    0x000000, 0x80000, 0x5f74bf78 )
	ROM_LOAD( "kod10.rom",    0x080000, 0x80000, 0x9ef36604 )
	ROM_LOAD( "kod02.rom",    0x100000, 0x80000, 0xe45b8701 )
	ROM_LOAD( "kod11.rom",    0x180000, 0x80000, 0x113358f3 )
	ROM_LOAD( "kod03.rom",    0x200000, 0x80000, 0x5e5303bf )
	ROM_LOAD( "kod12.rom",    0x280000, 0x80000, 0x402b9b4f )
	ROM_LOAD( "kod04.rom",    0x300000, 0x80000, 0xa7750322 )
	ROM_LOAD( "kod13.rom",    0x380000, 0x80000, 0x38853c44 )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "kod09.rom",    0x00000, 0x08000, 0xf5514510 )
	ROM_CONTINUE(             0x10000, 0x08000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD ( "kod18.rom",    0x00000, 0x20000, 0x69ecb2c8 )
	ROM_LOAD ( "kod19.rom",    0x20000, 0x20000, 0x02d851c1 )
ROM_END

ROM_START( kodj_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "kd30.bin",    0x00000, 0x20000, 0xebc788ad )
	ROM_LOAD_ODD ( "kd37.bin",    0x00000, 0x20000, 0xe55c3529 )
	ROM_LOAD_EVEN( "kd31.bin",    0x40000, 0x20000, 0xc710d722 )
	ROM_LOAD_ODD ( "kd38.bin",    0x40000, 0x20000, 0x57d6ed3a )
	ROM_LOAD_WIDE_SWAP("kd33.bin",0x80000,  0x80000, 0x9bd7ad4b)

	ROM_REGION_DISPOSE(0x400000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "kod01.rom",    0x000000, 0x80000, 0x5f74bf78 )
	ROM_LOAD( "kod10.rom",    0x080000, 0x80000, 0x9ef36604 )
	ROM_LOAD( "kod02.rom",    0x100000, 0x80000, 0xe45b8701 )
	ROM_LOAD( "kod11.rom",    0x180000, 0x80000, 0x113358f3 )
	ROM_LOAD( "kod03.rom",    0x200000, 0x80000, 0x5e5303bf )
	ROM_LOAD( "kod12.rom",    0x280000, 0x80000, 0x402b9b4f )
	ROM_LOAD( "kod04.rom",    0x300000, 0x80000, 0xa7750322 )
	ROM_LOAD( "kod13.rom",    0x380000, 0x80000, 0x38853c44 )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "kd09.bin",    0x00000, 0x08000, 0xbac6ec26 )
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD ( "kd18.bin",    0x00000, 0x20000, 0x4c63181d )
	ROM_LOAD ( "kd19.bin",    0x20000, 0x20000, 0x92941b80 )
ROM_END

ROM_START( kodb_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "kod.17",    0x00000, 0x80000, 0x036dd74c )
	ROM_LOAD_ODD ( "kod.18",    0x00000, 0x80000, 0x3e4b7295 )

	ROM_REGION_DISPOSE(0x400000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD_GFX_EVEN( "kod.di",   0x000000, 0x80000, 0xab031763 )
	ROM_LOAD_GFX_ODD ( "kod.ci",   0x000000, 0x80000, 0x22228bc5 )
	ROM_LOAD_GFX_EVEN( "kod.dp",   0x100000, 0x80000, 0x3eec9580 )
	ROM_LOAD_GFX_ODD ( "kod.cp",   0x100000, 0x80000, 0xe3b8589e )
	ROM_LOAD_GFX_EVEN( "kod.bi",   0x200000, 0x80000, 0x4a1b43fe )
	ROM_LOAD_GFX_ODD ( "kod.ai",   0x200000, 0x80000, 0xcffbf4be )
	ROM_LOAD_GFX_EVEN( "kod.bp",   0x300000, 0x80000, 0x4e1c52b7 )
	ROM_LOAD_GFX_ODD ( "kod.ap",   0x300000, 0x80000, 0xfdf5f163 )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "kod.15",        0x00000, 0x08000, 0x01cae60c )
	ROM_CONTINUE(              0x10000, 0x08000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD ( "kod.16",       0x00000, 0x40000, 0xa2db1575 )
ROM_END

ROM_START( captcomm_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_WIDE_SWAP( "cce_23d.rom",  0x000000, 0x80000, 0x19c58ece )
	ROM_LOAD_WIDE_SWAP( "cc_22d.rom",   0x080000, 0x80000, 0xa91949b7 )
	ROM_LOAD_EVEN( "cc_24d.rom",        0x100000, 0x20000, 0x680e543f )
	ROM_LOAD_ODD ( "cc_28d.rom",        0x100000, 0x20000, 0x8820039f )

	ROM_REGION_DISPOSE(0x400000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "gfx_02.rom",   0x000000, 0x80000, 0x00637302 )
	ROM_LOAD( "gfx_06.rom",   0x080000, 0x80000, 0x0c69f151 )
	ROM_LOAD( "gfx_01.rom",   0x100000, 0x80000, 0x7261d8ba )
	ROM_LOAD( "gfx_05.rom",   0x180000, 0x80000, 0x28718bed )
	ROM_LOAD( "gfx_04.rom",   0x200000, 0x80000, 0xcc87cf61 )
	ROM_LOAD( "gfx_08.rom",   0x280000, 0x80000, 0x1f9ebb97 )
	ROM_LOAD( "gfx_03.rom",   0x300000, 0x80000, 0x6a60f949 )
	ROM_LOAD( "gfx_07.rom",   0x380000, 0x80000, 0xd4acc53a )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "cc_09.rom",    0x00000, 0x08000, 0x698e8b58 )
	ROM_CONTINUE(             0x10000, 0x08000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD ( "cc_18.rom",    0x00000, 0x20000, 0x6de2c2db )
	ROM_LOAD ( "cc_19.rom",    0x20000, 0x20000, 0xb99091ae )
ROM_END

ROM_START( captcomu_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_WIDE_SWAP( "23b",   0x000000, 0x80000, 0x03da44fd )
	ROM_LOAD_WIDE_SWAP( "22c",   0x080000, 0x80000, 0x9b82a052 )
	ROM_LOAD_EVEN( "24b",        0x100000, 0x20000, 0x84ff99b2 )
	ROM_LOAD_ODD ( "28b",        0x100000, 0x20000, 0xfbcec223 )

	ROM_REGION_DISPOSE(0x400000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "gfx_02.rom",   0x000000, 0x80000, 0x00637302 )
	ROM_LOAD( "gfx_06.rom",   0x080000, 0x80000, 0x0c69f151 )
	ROM_LOAD( "gfx_01.rom",   0x100000, 0x80000, 0x7261d8ba )
	ROM_LOAD( "gfx_05.rom",   0x180000, 0x80000, 0x28718bed )
	ROM_LOAD( "gfx_04.rom",   0x200000, 0x80000, 0xcc87cf61 )
	ROM_LOAD( "gfx_08.rom",   0x280000, 0x80000, 0x1f9ebb97 )
	ROM_LOAD( "gfx_03.rom",   0x300000, 0x80000, 0x6a60f949 )
	ROM_LOAD( "gfx_07.rom",   0x380000, 0x80000, 0xd4acc53a )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "cc_09.rom",    0x00000, 0x08000, 0x698e8b58 )
	ROM_CONTINUE(             0x10000, 0x08000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD ( "cc_18.rom",    0x00000, 0x20000, 0x6de2c2db )
	ROM_LOAD ( "cc_19.rom",    0x20000, 0x20000, 0xb99091ae )
ROM_END

ROM_START( captcomj_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_WIDE_SWAP( "cc23.bin",   0x000000, 0x80000, 0x5b482b62 )
	ROM_LOAD_WIDE_SWAP( "cc22.bin",   0x080000, 0x80000, 0x0fd34195 )
	ROM_LOAD_EVEN( "cc24.bin",        0x100000, 0x20000, 0x3a794f25 )
	ROM_LOAD_ODD ( "cc28.bin",        0x100000, 0x20000, 0xfc3c2906 )

	ROM_REGION_DISPOSE(0x400000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "gfx_02.rom",   0x000000, 0x80000, 0x00637302 )
	ROM_LOAD( "gfx_06.rom",   0x080000, 0x80000, 0x0c69f151 )
	ROM_LOAD( "gfx_01.rom",   0x100000, 0x80000, 0x7261d8ba )
	ROM_LOAD( "gfx_05.rom",   0x180000, 0x80000, 0x28718bed )
	ROM_LOAD( "gfx_04.rom",   0x200000, 0x80000, 0xcc87cf61 )
	ROM_LOAD( "gfx_08.rom",   0x280000, 0x80000, 0x1f9ebb97 )
	ROM_LOAD( "gfx_03.rom",   0x300000, 0x80000, 0x6a60f949 )
	ROM_LOAD( "gfx_07.rom",   0x380000, 0x80000, 0xd4acc53a )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "cc_09.rom",    0x00000, 0x08000, 0x698e8b58 )
	ROM_CONTINUE(             0x10000, 0x08000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD ( "cc_18.rom",    0x00000, 0x20000, 0x6de2c2db )
	ROM_LOAD ( "cc_19.rom",    0x20000, 0x20000, 0xb99091ae )
ROM_END

ROM_START( knights_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_WIDE_SWAP( "kr_23e.rom",   0x00000, 0x80000, 0x1b3997eb )
	ROM_LOAD_WIDE_SWAP( "kr_22.rom",    0x80000, 0x80000, 0xd0b671a9 )

	ROM_REGION_DISPOSE(0x400000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "kr_gfx2.rom",  0x000000, 0x80000, 0xf095be2d )
	ROM_LOAD( "kr_gfx6.rom",  0x080000, 0x80000, 0x0200bc3d )
	ROM_LOAD( "kr_gfx1.rom",  0x100000, 0x80000, 0x9e36c1a4 )
	ROM_LOAD( "kr_gfx5.rom",  0x180000, 0x80000, 0x1f4298d2 )
	ROM_LOAD( "kr_gfx4.rom",  0x200000, 0x80000, 0x179dfd96 )
	ROM_LOAD( "kr_gfx8.rom",  0x280000, 0x80000, 0x0bb2b4e7 )
	ROM_LOAD( "kr_gfx3.rom",  0x300000, 0x80000, 0xc5832cae )
	ROM_LOAD( "kr_gfx7.rom",  0x380000, 0x80000, 0x37fa8751 )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "kr_09.rom",    0x00000, 0x10000, 0x5e44d9ee )
	ROM_RELOAD(               0x08000, 0x10000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD ( "kr_18.rom",    0x00000, 0x20000, 0xda69d15f )
	ROM_LOAD ( "kr_19.rom",    0x20000, 0x20000, 0xbfc654e9 )
ROM_END

ROM_START( knightsj_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "krj30.bin",   0x00000, 0x20000, 0xad3d1a8e )
	ROM_LOAD_ODD ( "krj37.bin",   0x00000, 0x20000, 0xe694a491 )
	ROM_LOAD_EVEN( "krj31.bin",   0x40000, 0x20000, 0x85596094 )
	ROM_LOAD_ODD ( "krj38.bin",   0x40000, 0x20000, 0x9198bf8f )
	ROM_LOAD_EVEN( "krj28.bin",   0x80000, 0x20000, 0xfe6eb08d )
	ROM_LOAD_ODD ( "krj35.bin",   0x80000, 0x20000, 0x1172806d )
	ROM_LOAD_EVEN( "krj29.bin",   0xc0000, 0x20000, 0xf854b020 )
	ROM_LOAD_ODD ( "krj36.bin",   0xc0000, 0x20000, 0xeb52e78d )

	ROM_REGION(0x400000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "kr_gfx2.rom",  0x000000, 0x80000, 0xf095be2d )
	ROM_LOAD( "kr_gfx6.rom",  0x080000, 0x80000, 0x0200bc3d )
	ROM_LOAD( "kr_gfx1.rom",  0x100000, 0x80000, 0x9e36c1a4 )
	ROM_LOAD( "kr_gfx5.rom",  0x180000, 0x80000, 0x1f4298d2 )
	ROM_LOAD( "kr_gfx4.rom",  0x200000, 0x80000, 0x179dfd96 )
	ROM_LOAD( "kr_gfx8.rom",  0x280000, 0x80000, 0x0bb2b4e7 )
	ROM_LOAD( "kr_gfx3.rom",  0x300000, 0x80000, 0xc5832cae )
	ROM_LOAD( "kr_gfx7.rom",  0x380000, 0x80000, 0x37fa8751 )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "kr_09.rom",    0x00000, 0x10000, 0x5e44d9ee )
	ROM_RELOAD(               0x08000, 0x10000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD ( "kr_18.rom",    0x00000, 0x20000, 0xda69d15f )
	ROM_LOAD ( "kr_19.rom",    0x20000, 0x20000, 0xbfc654e9 )
ROM_END

ROM_START( sf2ce_rom )
	ROM_REGION(0x180000)      /* 68000 code */
	ROM_LOAD_WIDE_SWAP( "sf2ce.23",     0x000000, 0x80000, 0x3f846b74 )
	ROM_LOAD_WIDE_SWAP( "sf2ce.22",     0x080000, 0x80000, 0x99f1cca4 )
	ROM_LOAD_WIDE_SWAP( "sf2ce.21",     0x100000, 0x80000, 0x925a7877 )

	ROM_REGION_DISPOSE(0x600000)     /* temporary space for graphics (disposed after conversion) */
		/* Plane 1+2 */
	ROM_LOAD( "sf2.02",       0x000000, 0x80000, 0xcdb5f027 ) /* sprites (left half)*/
	ROM_LOAD( "sf2.06",       0x080000, 0x80000, 0x21e3f87d ) /* sprites */
	ROM_LOAD( "sf2.11",       0x100000, 0x80000, 0xd6ec9a0a ) /* sprites */

	ROM_LOAD( "sf2.01",       0x180000, 0x80000, 0x03b0d852 ) /* sprites (right half) */
	ROM_LOAD( "sf2.05",       0x200000, 0x80000, 0xba8a2761 ) /* sprites */
	ROM_LOAD( "sf2.10",       0x280000, 0x80000, 0x960687d5 ) /* sprites */

	/* Plane 3+4 */
	ROM_LOAD( "sf2.04",       0x300000, 0x80000, 0xe2799472 ) /* sprites */
	ROM_LOAD( "sf2.08",       0x380000, 0x80000, 0xbefc47df ) /* sprites */
	ROM_LOAD( "sf2.13",       0x400000, 0x80000, 0xed2c67f6 ) /* sprites */

	ROM_LOAD( "sf2.03",       0x480000, 0x80000, 0x840289ec ) /* sprites */
	ROM_LOAD( "sf2.07",       0x500000, 0x80000, 0xe584bfb5 ) /* sprites */
	ROM_LOAD( "sf2.12",       0x580000, 0x80000, 0x978ecd18 ) /* sprites */

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sf2.09",       0x00000, 0x08000, 0x08f6b60e )
	ROM_CONTINUE(          0x10000, 0x08000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD ( "sf2.18",       0x00000, 0x20000, 0x7f162009 )
	ROM_LOAD ( "sf2.19",       0x20000, 0x20000, 0xbeade53f )
ROM_END

ROM_START( sf2cej_rom )
	ROM_REGION(0x180000)      /* 68000 code */
	ROM_LOAD_WIDE( "sf2cej.23",    0x000000, 0x80000, 0x7c463f94 )
	ROM_LOAD_WIDE( "sf2cej.22",    0x080000, 0x80000, 0x6628f6a6 )
	ROM_LOAD_WIDE( "sf2cej.21",    0x100000, 0x80000, 0xfcb8fe8f )

	ROM_REGION_DISPOSE(0x600000)     /* temporary space for graphics (disposed after conversion) */
		/* Plane 1+2 */
	ROM_LOAD( "sf2.02",       0x000000, 0x80000, 0xcdb5f027 ) /* sprites (left half)*/
	ROM_LOAD( "sf2.06",       0x080000, 0x80000, 0x21e3f87d ) /* sprites */
	ROM_LOAD( "sf2.11",       0x100000, 0x80000, 0xd6ec9a0a ) /* sprites */

	ROM_LOAD( "sf2.01",       0x180000, 0x80000, 0x03b0d852 ) /* sprites (right half) */
	ROM_LOAD( "sf2.05",       0x200000, 0x80000, 0xba8a2761 ) /* sprites */
	ROM_LOAD( "sf2.10",       0x280000, 0x80000, 0x960687d5 ) /* sprites */

	/* Plane 3+4 */
	ROM_LOAD( "sf2.04",       0x300000, 0x80000, 0xe2799472 ) /* sprites */
	ROM_LOAD( "sf2.08",       0x380000, 0x80000, 0xbefc47df ) /* sprites */
	ROM_LOAD( "sf2.13",       0x400000, 0x80000, 0xed2c67f6 ) /* sprites */

	ROM_LOAD( "sf2.03",       0x480000, 0x80000, 0x840289ec ) /* sprites */
	ROM_LOAD( "sf2.07",       0x500000, 0x80000, 0xe584bfb5 ) /* sprites */
	ROM_LOAD( "sf2.12",       0x580000, 0x80000, 0x978ecd18 ) /* sprites */

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sf2.09",       0x00000, 0x08000, 0x08f6b60e )
	ROM_CONTINUE(          0x10000, 0x08000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD ( "sf2.18",       0x00000, 0x20000, 0x7f162009 )
	ROM_LOAD ( "sf2.19",       0x20000, 0x20000, 0xbeade53f )
ROM_END

ROM_START( sf2rb_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_WIDE( "sf2d__23.rom", 0x000000, 0x80000, 0x450532b0 )
	ROM_LOAD_WIDE( "sf2d__22.rom", 0x080000, 0x80000, 0xfe9d9cf5 )
	ROM_LOAD_WIDE( "sf2cej.21",    0x100000, 0x80000, 0xfcb8fe8f )

	ROM_REGION_DISPOSE(0x600000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "sf2.02",       0x000000, 0x80000, 0xcdb5f027 )
	ROM_LOAD( "sf2.06",       0x080000, 0x80000, 0x21e3f87d )
	ROM_LOAD( "sf2.11",       0x100000, 0x80000, 0xd6ec9a0a )
	ROM_LOAD( "sf2.01",       0x180000, 0x80000, 0x03b0d852 )
	ROM_LOAD( "sf2.05",       0x200000, 0x80000, 0xba8a2761 )
	ROM_LOAD( "sf2.10",       0x280000, 0x80000, 0x960687d5 )
	ROM_LOAD( "sf2.04",       0x300000, 0x80000, 0xe2799472 )
	ROM_LOAD( "sf2.08",       0x380000, 0x80000, 0xbefc47df )
	ROM_LOAD( "sf2.13",       0x400000, 0x80000, 0xed2c67f6 )
	ROM_LOAD( "sf2.03",       0x480000, 0x80000, 0x840289ec )
	ROM_LOAD( "sf2.07",       0x500000, 0x80000, 0xe584bfb5 )
	ROM_LOAD( "sf2.12",       0x580000, 0x80000, 0x978ecd18 )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sf2.09",       0x00000, 0x08000, 0x08f6b60e )
	ROM_CONTINUE(             0x10000, 0x08000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD ( "sf2.18",       0x00000, 0x20000, 0x7f162009 )
	ROM_LOAD ( "sf2.19",       0x20000, 0x20000, 0xbeade53f )
ROM_END

ROM_START( varth_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "vae_30a.rom",  0x00000, 0x20000, 0x7fcd0091 )
	ROM_LOAD_ODD ( "vae_35a.rom",  0x00000, 0x20000, 0x35cf9509 )
	ROM_LOAD_EVEN( "vae_31a.rom",  0x40000, 0x20000, 0x15e5ee81 )
	ROM_LOAD_ODD ( "vae_36a.rom",  0x40000, 0x20000, 0x153a201e )
	ROM_LOAD_EVEN( "vae_28a.rom",  0x80000, 0x20000, 0x7a0e0d25 )
	ROM_LOAD_ODD ( "vae_33a.rom",  0x80000, 0x20000, 0xf2365922 )
	ROM_LOAD_EVEN( "vae_29a.rom",  0xc0000, 0x20000, 0x5e2cd2c3 )
	ROM_LOAD_ODD ( "vae_34a.rom",  0xc0000, 0x20000, 0x3d9bdf83 )

	ROM_REGION_DISPOSE(0x200000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "va_gfx1.rom",  0x000000, 0x80000, 0x0b1ace37 )
	ROM_LOAD( "va_gfx5.rom",  0x080000, 0x80000, 0xb1fb726e )
	ROM_LOAD( "va_gfx3.rom",  0x100000, 0x80000, 0x44dfe706 )
	ROM_LOAD( "va_gfx7.rom",  0x180000, 0x80000, 0x4c6588cd )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "va_09.rom",    0x000000, 0x08000, 0x7a99446e )
	ROM_CONTINUE(             0x010000, 0x08000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD( "va_18.rom",    0x00000, 0x20000, 0xde30510e )
	ROM_LOAD( "va_19.rom",    0x20000, 0x20000, 0x0610a4ac )
ROM_END

ROM_START( varthj_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "vaj36b.bin",   0x00000, 0x20000, 0x1d798d6a )
	ROM_LOAD_ODD ( "vaj42b.bin",   0x00000, 0x20000, 0x0f720233 )
	ROM_LOAD_EVEN( "vaj37b.bin",   0x40000, 0x20000, 0x24414b17 )
	ROM_LOAD_ODD ( "vaj43b.bin",   0x40000, 0x20000, 0x34b4b06c )
	ROM_LOAD_EVEN( "vaj34b.bin",   0x80000, 0x20000, 0x87c79aed )
	ROM_LOAD_ODD ( "vaj40b.bin",   0x80000, 0x20000, 0x210b4bd0 )
	ROM_LOAD_EVEN( "vaj35b.bin",   0xc0000, 0x20000, 0x6b0da69f )
	ROM_LOAD_ODD ( "vaj41b.bin",   0xc0000, 0x20000, 0x6542c8a4 )

	ROM_REGION_DISPOSE(0x200000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "va_gfx1.rom",  0x000000, 0x80000, 0x0b1ace37 )
	ROM_LOAD( "va_gfx5.rom",  0x080000, 0x80000, 0xb1fb726e )
	ROM_LOAD( "va_gfx3.rom",  0x100000, 0x80000, 0x44dfe706 )
	ROM_LOAD( "va_gfx7.rom",  0x180000, 0x80000, 0x4c6588cd )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "va_09.rom",    0x000000, 0x08000, 0x7a99446e )
	ROM_CONTINUE(             0x010000, 0x08000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD( "va_18.rom",    0x00000, 0x20000, 0xde30510e )
	ROM_LOAD( "va_19.rom",    0x20000, 0x20000, 0x0610a4ac )
ROM_END

ROM_START( cworld2j_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "q536.bin",       0x00000, 0x20000, 0x38a08099 )
	ROM_LOAD_ODD ( "q542.bin",       0x00000, 0x20000, 0x4d29b3a4 )
	ROM_LOAD_EVEN( "q537.bin",       0x40000, 0x20000, 0xeb547ebc )
	ROM_LOAD_ODD ( "q543.bin",       0x40000, 0x20000, 0x3ef65ea8 )
	ROM_LOAD_EVEN( "q534.bin",       0x80000, 0x20000, 0x7fcc1317 )
	ROM_LOAD_ODD ( "q540.bin",       0x80000, 0x20000, 0x7f14b7b4 )
	ROM_LOAD_EVEN( "q535.bin",       0xc0000, 0x20000, 0xabacee26 )
	ROM_LOAD_ODD ( "q541.bin",       0xc0000, 0x20000, 0xd3654067 )

	ROM_REGION_DISPOSE(0x200000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD_GFX_EVEN( "q524.bin",   0x000000, 0x20000, 0xb419d139 )
	ROM_LOAD_GFX_ODD ( "q517.bin",   0x000000, 0x20000, 0xbd3b4d11 )
	ROM_LOAD_GFX_EVEN( "q525.bin",   0x040000, 0x20000, 0x979237cb )
	ROM_LOAD_GFX_ODD ( "q518.bin",   0x040000, 0x20000, 0xc57da03c )
	ROM_LOAD_GFX_EVEN( "q509.bin",   0x080000, 0x20000, 0x48496d80 )
	ROM_LOAD_GFX_ODD ( "q501.bin",   0x080000, 0x20000, 0xc5453f56 )
	ROM_LOAD_GFX_EVEN( "q510.bin",   0x0c0000, 0x20000, 0x119e5e93 )
	ROM_LOAD_GFX_ODD ( "q502.bin",   0x0c0000, 0x20000, 0xa2cadcbe )
	ROM_LOAD_GFX_EVEN( "q538.bin",   0x100000, 0x20000, 0x9c24670c )
	ROM_LOAD_GFX_ODD ( "q532.bin",   0x100000, 0x20000, 0x3ef9c7c2 )
	ROM_LOAD_GFX_EVEN( "q539.bin",   0x140000, 0x20000, 0xa5839b25 )
	ROM_LOAD_GFX_ODD ( "q533.bin",   0x140000, 0x20000, 0x04d03930 )
	ROM_LOAD_GFX_EVEN( "q513.bin",   0x180000, 0x20000, 0xc741ac52 )
	ROM_LOAD_GFX_ODD ( "q505.bin",   0x180000, 0x20000, 0x143e068f )
	ROM_LOAD_GFX_EVEN( "q514.bin",   0x1c0000, 0x20000, 0xa8755f82 )
	ROM_LOAD_GFX_ODD ( "q506.bin",   0x1c0000, 0x20000, 0xc92a91fc )

	ROM_REGION(0x20000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "q523.bin",       0x000000, 0x10000, 0xe14dc524 )
	ROM_RELOAD(                 0x010000, 0x10000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD ( "q530.bin",       0x00000, 0x20000, 0xd10c1b68 )
	ROM_LOAD ( "q531.bin",       0x20000, 0x20000, 0x7d17e496 )
ROM_END

ROM_START( wof_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_WIDE_SWAP( "tk2e_23b.rom",  0x000000, 0x80000, 0x11fb2ed1 )
	ROM_LOAD_WIDE_SWAP( "tk2e_22b.rom",  0x080000, 0x80000, 0x479b3f24 )

	ROM_REGION_DISPOSE(0x400000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "tk2_gfx2.rom",   0x000000, 0x80000, 0xc5ca2460 )
	ROM_LOAD( "tk2_gfx6.rom",   0x080000, 0x80000, 0x1abd14d6 )
	ROM_LOAD( "tk2_gfx1.rom",   0x100000, 0x80000, 0x0d9cb9bf )
	ROM_LOAD( "tk2_gfx5.rom",   0x180000, 0x80000, 0x291f0f0b )
	ROM_LOAD( "tk2_gfx4.rom",   0x200000, 0x80000, 0xe349551c )
	ROM_LOAD( "tk2_gfx8.rom",   0x280000, 0x80000, 0xb27948e3 )
	ROM_LOAD( "tk2_gfx3.rom",   0x300000, 0x80000, 0x45227027 )
	ROM_LOAD( "tk2_gfx7.rom",   0x380000, 0x80000, 0x3edeb949 )

	ROM_REGION(0x28000) /* QSound Z80 code */
	ROM_LOAD( "tk2_qa.rom",     0x00000, 0x08000, 0xc9183a0d )
	ROM_CONTINUE(               0x10000, 0x18000 )

	ROM_REGION(0x200000) /* QSound samples */
	ROM_LOAD( "tk2_q1.rom",     0x000000, 0x80000, 0x611268cf )
	ROM_LOAD( "tk2_q2.rom",     0x080000, 0x80000, 0x20f55ca9 )
	ROM_LOAD( "tk2_q3.rom",     0x100000, 0x80000, 0xbfcf6f52 )
	ROM_LOAD( "tk2_q4.rom",     0x180000, 0x80000, 0x36642e88 )
ROM_END

ROM_START( wofj_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_WIDE_SWAP( "tk2j23c.bin",  0x000000, 0x80000, 0x9b215a68 )
	ROM_LOAD_WIDE_SWAP( "tk2j22c.bin",  0x080000, 0x80000, 0xb74b09ac )

	ROM_REGION_DISPOSE(0x400000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "tk2_gfx2.rom",   0x000000, 0x80000, 0xc5ca2460 )
	ROM_LOAD( "tk207.bin",      0x080000, 0x80000, 0xd706568e )
	ROM_LOAD( "tk2_gfx1.rom",   0x100000, 0x80000, 0x0d9cb9bf )
	ROM_LOAD( "tk205.bin",      0x180000, 0x80000, 0xe4a44d53 )
	ROM_LOAD( "tk2_gfx4.rom",   0x200000, 0x80000, 0xe349551c )
	ROM_LOAD( "tk208.bin",      0x280000, 0x80000, 0xd4a19a02 )
	ROM_LOAD( "tk2_gfx3.rom",   0x300000, 0x80000, 0x45227027 )
	ROM_LOAD( "tk206.bin",      0x380000, 0x80000, 0x58066ba8 )

	ROM_REGION(0x28000) /* QSound Z80 code */
	ROM_LOAD( "tk2_qa.rom",     0x00000, 0x08000, 0xc9183a0d )
	ROM_CONTINUE(               0x10000, 0x18000 )

	ROM_REGION(0x200000) /* QSound samples */
	ROM_LOAD( "tk2_q1.rom",     0x000000, 0x80000, 0x611268cf )
	ROM_LOAD( "tk2_q2.rom",     0x080000, 0x80000, 0x20f55ca9 )
	ROM_LOAD( "tk2_q3.rom",     0x100000, 0x80000, 0xbfcf6f52 )
	ROM_LOAD( "tk2_q4.rom",     0x180000, 0x80000, 0x36642e88 )
ROM_END

ROM_START( sf2t_rom )
	ROM_REGION(0x180000)      /* 68000 code */
	ROM_LOAD_WIDE_SWAP( "sf2.23",       0x000000, 0x80000, 0x89a1fc38 )
	ROM_LOAD_WIDE_SWAP( "sf2.22",       0x080000, 0x80000, 0xaea6e035 )
	ROM_LOAD_WIDE_SWAP( "sf2.21",       0x100000, 0x80000, 0xfd200288 )

	ROM_REGION_DISPOSE(0x600000)     /* temporary space for graphics (disposed after conversion) */
		/* Plane 1+2 */
	ROM_LOAD( "sf2.02",       0x000000, 0x80000, 0xcdb5f027 ) /* sprites (left half)*/
	ROM_LOAD( "sf2.06",       0x080000, 0x80000, 0x21e3f87d ) /* sprites */
	ROM_LOAD( "sf2.11",       0x100000, 0x80000, 0xd6ec9a0a ) /* sprites */

	ROM_LOAD( "sf2.01",       0x180000, 0x80000, 0x03b0d852 ) /* sprites (right half) */
	ROM_LOAD( "sf2.05",       0x200000, 0x80000, 0xba8a2761 ) /* sprites */
	ROM_LOAD( "sf2.10",       0x280000, 0x80000, 0x960687d5 ) /* sprites */

	/* Plane 3+4 */
	ROM_LOAD( "sf2.04",       0x300000, 0x80000, 0xe2799472 ) /* sprites */
	ROM_LOAD( "sf2.08",       0x380000, 0x80000, 0xbefc47df ) /* sprites */
	ROM_LOAD( "sf2.13",       0x400000, 0x80000, 0xed2c67f6 ) /* sprites */

	ROM_LOAD( "sf2.03",       0x480000, 0x80000, 0x840289ec ) /* sprites */
	ROM_LOAD( "sf2.07",       0x500000, 0x80000, 0xe584bfb5 ) /* sprites */
	ROM_LOAD( "sf2.12",       0x580000, 0x80000, 0x978ecd18 ) /* sprites */

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sf2.09",       0x00000, 0x08000, 0x08f6b60e )
	ROM_CONTINUE(          0x10000, 0x08000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD ( "sf2.18",       0x00000, 0x20000, 0x7f162009 )
	ROM_LOAD ( "sf2.19",       0x20000, 0x20000, 0xbeade53f )
ROM_END

ROM_START( sf2tj_rom )
	ROM_REGION(0x180000)      /* 68000 code */
	ROM_LOAD_WIDE_SWAP( "sf2tj.23",   0x000000, 0x80000, 0xea73b4dc )
	ROM_LOAD_WIDE_SWAP( "sf2tj.22",   0x080000, 0x80000, 0xaea6e035 )
	ROM_LOAD_WIDE_SWAP( "sf2tj.21",   0x100000, 0x80000, 0xfd200288 )

	ROM_REGION_DISPOSE(0x600000)     /* temporary space for graphics (disposed after conversion) */
		/* Plane 1+2 */
	ROM_LOAD( "sf2.02",       0x000000, 0x80000, 0xcdb5f027 ) /* sprites (left half)*/
	ROM_LOAD( "sf2.06",       0x080000, 0x80000, 0x21e3f87d ) /* sprites */
	ROM_LOAD( "sf2.11",       0x100000, 0x80000, 0xd6ec9a0a ) /* sprites */

	ROM_LOAD( "sf2.01",       0x180000, 0x80000, 0x03b0d852 ) /* sprites (right half) */
	ROM_LOAD( "sf2.05",       0x200000, 0x80000, 0xba8a2761 ) /* sprites */
	ROM_LOAD( "sf2.10",       0x280000, 0x80000, 0x960687d5 ) /* sprites */

	/* Plane 3+4 */
	ROM_LOAD( "sf2.04",       0x300000, 0x80000, 0xe2799472 ) /* sprites */
	ROM_LOAD( "sf2.08",       0x380000, 0x80000, 0xbefc47df ) /* sprites */
	ROM_LOAD( "sf2.13",       0x400000, 0x80000, 0xed2c67f6 ) /* sprites */

	ROM_LOAD( "sf2.03",       0x480000, 0x80000, 0x840289ec ) /* sprites */
	ROM_LOAD( "sf2.07",       0x500000, 0x80000, 0xe584bfb5 ) /* sprites */
	ROM_LOAD( "sf2.12",       0x580000, 0x80000, 0x978ecd18 ) /* sprites */

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sf2.09",       0x00000, 0x08000, 0x08f6b60e )
	ROM_CONTINUE(          0x10000, 0x08000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD ( "sf2.18",       0x00000, 0x20000, 0x7f162009 )
	ROM_LOAD ( "sf2.19",       0x20000, 0x20000, 0xbeade53f )
ROM_END

ROM_START( dino_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_WIDE_SWAP( "cde_23a.rom",  0x000000, 0x80000, 0x8f4e585e )
	ROM_LOAD_WIDE_SWAP( "cde_22a.rom",  0x080000, 0x80000, 0x9278aa12 )
	ROM_LOAD_WIDE_SWAP( "cde_21a.rom",  0x100000, 0x80000, 0x66d23de2 )

	ROM_REGION_DISPOSE(0x400000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "cd_gfx02.rom",   0x000000, 0x80000, 0x09c8fc2d )
	ROM_LOAD( "cd_gfx06.rom",   0x080000, 0x80000, 0xe7599ac4 )
	ROM_LOAD( "cd_gfx01.rom",   0x100000, 0x80000, 0x8da4f917 )
	ROM_LOAD( "cd_gfx05.rom",   0x180000, 0x80000, 0x470befee )
	ROM_LOAD( "cd_gfx04.rom",   0x200000, 0x80000, 0x637ff38f )
	ROM_LOAD( "cd_gfx08.rom",   0x280000, 0x80000, 0x211b4b15 )
	ROM_LOAD( "cd_gfx03.rom",   0x300000, 0x80000, 0x6c40f603 )
	ROM_LOAD( "cd_gfx07.rom",   0x380000, 0x80000, 0x22bfb7a3 )

	ROM_REGION(0x28000) /* QSound Z80 code */
	ROM_LOAD( "cd_q.rom",       0x00000, 0x08000, 0x605fdb0b )
	ROM_CONTINUE(               0x10000, 0x18000 )

	ROM_REGION(0x200000)  /* QSound samples */
	ROM_LOAD( "cd_q1.rom",      0x000000, 0x80000, 0x60927775 )
	ROM_LOAD( "cd_q2.rom",      0x080000, 0x80000, 0x770f4c47 )
	ROM_LOAD( "cd_q3.rom",      0x100000, 0x80000, 0x2f273ffc )
	ROM_LOAD( "cd_q4.rom",      0x180000, 0x80000, 0x2c67821d )
ROM_END

ROM_START( punisher_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "pse_26.rom",       0x000000, 0x20000, 0x389a99d2 )
	ROM_LOAD_ODD ( "pse_30.rom",       0x000000, 0x20000, 0x68fb06ac )
	ROM_LOAD_EVEN( "pse_27.rom",       0x040000, 0x20000, 0x3eb181c3 )
	ROM_LOAD_ODD ( "pse_31.rom",       0x040000, 0x20000, 0x37108e7b )
	ROM_LOAD_EVEN( "pse_24.rom",       0x080000, 0x20000, 0x0f434414 )
	ROM_LOAD_ODD ( "pse_28.rom",       0x080000, 0x20000, 0xb732345d )
	ROM_LOAD_EVEN( "pse_25.rom",       0x0c0000, 0x20000, 0xb77102e2 )
	ROM_LOAD_ODD ( "pse_29.rom",       0x0c0000, 0x20000, 0xec037bce )
	ROM_LOAD_WIDE_SWAP( "ps_21.rom",   0x100000, 0x80000, 0x8affa5a9 )

	ROM_REGION_DISPOSE(0x400000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "ps_gfx2.rom",   0x000000, 0x80000, 0x64fa58d4 )
	ROM_LOAD( "ps_gfx6.rom",   0x080000, 0x80000, 0xa544f4cc )
	ROM_LOAD( "ps_gfx1.rom",   0x100000, 0x80000, 0x77b7ccab )
	ROM_LOAD( "ps_gfx5.rom",   0x180000, 0x80000, 0xc54ea839 )
	ROM_LOAD( "ps_gfx4.rom",   0x200000, 0x80000, 0x60da42c8 )
	ROM_LOAD( "ps_gfx8.rom",   0x280000, 0x80000, 0x8f02f436 )
	ROM_LOAD( "ps_gfx3.rom",   0x300000, 0x80000, 0x0122720b )
	ROM_LOAD( "ps_gfx7.rom",   0x380000, 0x80000, 0x04c5acbd )

	ROM_REGION(0x28000) /* QSound Z80 code */
	ROM_LOAD( "ps_q.rom",       0x00000, 0x08000, 0x49ff4446 )
	ROM_CONTINUE(               0x10000, 0x18000 )

	ROM_REGION(0x200000) /* QSound samples */
	ROM_LOAD( "ps_q1.rom",      0x000000, 0x80000, 0x31fd8726 )
	ROM_LOAD( "ps_q2.rom",      0x080000, 0x80000, 0x980a9eef )
	ROM_LOAD( "ps_q3.rom",      0x100000, 0x80000, 0x0dd44491 )
	ROM_LOAD( "ps_q4.rom",      0x180000, 0x80000, 0xbed42f03 )
ROM_END

ROM_START( punishrj_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_WIDE_SWAP( "psj23.bin",   0x000000, 0x80000, 0x6b2fda52 )
	ROM_LOAD_WIDE_SWAP( "psj22.bin",   0x080000, 0x80000, 0xe01036bc )
	ROM_LOAD_WIDE_SWAP( "ps_21.rom",   0x100000, 0x80000, 0x8affa5a9 )

	ROM_REGION_DISPOSE(0x400000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "ps_gfx2.rom",   0x000000, 0x80000, 0x64fa58d4 )
	ROM_LOAD( "ps_gfx6.rom",   0x080000, 0x80000, 0xa544f4cc )
	ROM_LOAD( "ps_gfx1.rom",   0x100000, 0x80000, 0x77b7ccab )
	ROM_LOAD( "ps_gfx5.rom",   0x180000, 0x80000, 0xc54ea839 )
	ROM_LOAD( "ps_gfx4.rom",   0x200000, 0x80000, 0x60da42c8 )
	ROM_LOAD( "ps_gfx8.rom",   0x280000, 0x80000, 0x8f02f436 )
	ROM_LOAD( "ps_gfx3.rom",   0x300000, 0x80000, 0x0122720b )
	ROM_LOAD( "ps_gfx7.rom",   0x380000, 0x80000, 0x04c5acbd )

	ROM_REGION(0x28000) /* QSound Z80 code */
	ROM_LOAD( "ps_q.rom",       0x00000, 0x08000, 0x49ff4446 )
	ROM_CONTINUE(               0x10000, 0x18000 )

	ROM_REGION(0x200000) /* QSound samples */
	ROM_LOAD( "ps_q1.rom",      0x000000, 0x80000, 0x31fd8726 )
	ROM_LOAD( "ps_q2.rom",      0x080000, 0x80000, 0x980a9eef )
	ROM_LOAD( "ps_q3.rom",      0x100000, 0x80000, 0x0dd44491 )
	ROM_LOAD( "ps_q4.rom",      0x180000, 0x80000, 0xbed42f03 )
ROM_END

ROM_START( slammast_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_WIDE_SWAP( "mbe_23e.rom",  0x000000, 0x80000, 0x5394057a )
	ROM_LOAD_EVEN( "mbe_24b.rom",       0x080000, 0x20000, 0x95d5e729 )
	ROM_LOAD_ODD ( "mbe_28b.rom",       0x080000, 0x20000, 0xb1c7cbcb )
	ROM_LOAD_EVEN( "mbe_25b.rom",       0x0c0000, 0x20000, 0xa50d3fd4 )
	ROM_LOAD_ODD ( "mbe_29b.rom",       0x0c0000, 0x20000, 0x08e32e56 )
	ROM_LOAD_WIDE_SWAP( "mbe_20a.rom",  0x100000, 0x80000, 0xaeb557b0 )
	ROM_LOAD_WIDE_SWAP( "mbe_21a.rom",  0x180000, 0x80000, 0xd5007b05 )

	ROM_REGION_DISPOSE(0x600000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "mb_gfx02.rom",   0x000000, 0x80000, 0x2ffbfea8 )
	ROM_LOAD( "mb_gfx06.rom",   0x080000, 0x80000, 0xb76c70e9 )
	ROM_LOAD( "mb_gfx11.rom",   0x100000, 0x80000, 0x8fb94743 )
	ROM_LOAD( "mb_gfx01.rom",   0x180000, 0x80000, 0x41468e06 )
	ROM_LOAD( "mb_gfx05.rom",   0x200000, 0x80000, 0x506b9dc9 )
	ROM_LOAD( "mb_gfx10.rom",   0x280000, 0x80000, 0x97976ff5 )
	ROM_LOAD( "mb_gfx04.rom",   0x300000, 0x80000, 0x1eb9841d )
	ROM_LOAD( "mb_gfx08.rom",   0x380000, 0x80000, 0xe60c9556 )
	ROM_LOAD( "mb_gfx13.rom",   0x400000, 0x80000, 0xda810d5f )
	ROM_LOAD( "mb_gfx03.rom",   0x480000, 0x80000, 0xf453aa9e )
	ROM_LOAD( "mb_gfx07.rom",   0x500000, 0x80000, 0xaff8c2fb )
	ROM_LOAD( "mb_gfx12.rom",   0x580000, 0x80000, 0xb350a840 )

	ROM_REGION(0x28000) /* QSound Z80 code */
	ROM_LOAD( "mb_qa.rom",      0x00000, 0x08000, 0xe21a03c4 )
	ROM_CONTINUE(               0x10000, 0x18000 )

	ROM_REGION(0x400000) /* QSound samples */
	ROM_LOAD( "mb_q1.rom",      0x000000, 0x80000, 0x0630c3ce )
	ROM_LOAD( "mb_q2.rom",      0x080000, 0x80000, 0x354f9c21 )
	ROM_LOAD( "mb_q3.rom",      0x100000, 0x80000, 0x7838487c )
	ROM_LOAD( "mb_q4.rom",      0x180000, 0x80000, 0xab66e087 )
	ROM_LOAD( "mb_q5.rom",      0x200000, 0x80000, 0xc789fef2 )
        ROM_LOAD( "mb_q6.rom",      0x280000, 0x80000, 0xecb81b61 )
	ROM_LOAD( "mb_q7.rom",      0x300000, 0x80000, 0x041e49ba )
	ROM_LOAD( "mb_q8.rom",      0x380000, 0x80000, 0x59fe702a )
ROM_END

ROM_START( mbomber_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "mbde_26.rom",       0x000000, 0x20000, 0x72b7451c )
	ROM_LOAD_ODD ( "mbde_30.rom",       0x000000, 0x20000, 0xa036dc16 )
	ROM_LOAD_EVEN( "mbde_27.rom",       0x040000, 0x20000, 0x4086f534 )
	ROM_LOAD_ODD ( "mbde_31.rom",       0x040000, 0x20000, 0x085f47f0 )
	ROM_LOAD_EVEN( "mbde_24.rom",       0x080000, 0x20000, 0xc20895a5 )
	ROM_LOAD_ODD ( "mbde_28.rom",       0x080000, 0x20000, 0x2618d5e1 )
	ROM_LOAD_EVEN( "mbde_25.rom",       0x0c0000, 0x20000, 0x9bdb6b11 )
	ROM_LOAD_ODD ( "mbde_29.rom",       0x0c0000, 0x20000, 0x3f52d5e5 )
	ROM_LOAD_WIDE_SWAP( "mbde_20.rom",  0x100000, 0x80000, 0xb8b2139b )
	ROM_LOAD_WIDE_SWAP( "mbde_21.rom",  0x180000, 0x80000, 0x690c026a )

	ROM_REGION_DISPOSE(0x600000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "mb_gfx02.rom",   0x000000, 0x80000, 0x2ffbfea8 )
	ROM_LOAD( "mb_gfx06.rom",   0x080000, 0x80000, 0xb76c70e9 )
	ROM_LOAD( "mb_gfx11.rom",   0x100000, 0x80000, 0x8fb94743 )
	ROM_LOAD( "mb_gfx01.rom",   0x180000, 0x80000, 0x41468e06 )
	ROM_LOAD( "mb_gfx05.rom",   0x200000, 0x80000, 0x506b9dc9 )
	ROM_LOAD( "mb_gfx10.rom",   0x280000, 0x80000, 0x97976ff5 )
	ROM_LOAD( "mb_gfx04.rom",   0x300000, 0x80000, 0x1eb9841d )
	ROM_LOAD( "mb_gfx08.rom",   0x380000, 0x80000, 0xe60c9556 )
	ROM_LOAD( "mb_gfx13.rom",   0x400000, 0x80000, 0xda810d5f )
	ROM_LOAD( "mb_gfx03.rom",   0x480000, 0x80000, 0xf453aa9e )
	ROM_LOAD( "mb_gfx07.rom",   0x500000, 0x80000, 0xaff8c2fb )
	ROM_LOAD( "mb_gfx12.rom",   0x580000, 0x80000, 0xb350a840 )

	ROM_REGION(0x28000) /* QSound Z80 code */
	ROM_LOAD( "mb_q.rom",       0x00000, 0x08000, 0xd6fa76d1 )
	ROM_CONTINUE(               0x10000, 0x18000 )

	ROM_REGION(0x400000) /* QSound samples */
	ROM_LOAD( "mb_q1.rom",      0x000000, 0x80000, 0x0630c3ce )
	ROM_LOAD( "mb_q2.rom",      0x080000, 0x80000, 0x354f9c21 )
	ROM_LOAD( "mb_q3.rom",      0x100000, 0x80000, 0x7838487c )
	ROM_LOAD( "mb_q4.rom",      0x180000, 0x80000, 0xab66e087 )
	ROM_LOAD( "mb_q5.rom",      0x200000, 0x80000, 0xc789fef2 )
	ROM_LOAD( "mb_q6.rom",      0x280000, 0x80000, 0xecb81b61 )
	ROM_LOAD( "mb_q7.rom",      0x300000, 0x80000, 0x041e49ba )
	ROM_LOAD( "mb_q8.rom",      0x380000, 0x80000, 0x59fe702a )
ROM_END

ROM_START( mbomberj_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "mbde_26.rom",       0x000000, 0x20000, 0x72b7451c )
	ROM_LOAD_ODD ( "mbde30.rom",        0x000000, 0x20000, 0xbeff31cf )
	ROM_LOAD_EVEN( "mbde_27.rom",       0x040000, 0x20000, 0x4086f534 )
	ROM_LOAD_ODD ( "mbde_31.rom",       0x040000, 0x20000, 0x085f47f0 )
	ROM_LOAD_EVEN( "mbde_24.rom",       0x080000, 0x20000, 0xc20895a5 )
	ROM_LOAD_ODD ( "mbde_28.rom",       0x080000, 0x20000, 0x2618d5e1 )
	ROM_LOAD_EVEN( "mbde_25.rom",       0x0c0000, 0x20000, 0x9bdb6b11 )
	ROM_LOAD_ODD ( "mbde_29.rom",       0x0c0000, 0x20000, 0x3f52d5e5 )
	ROM_LOAD_WIDE_SWAP( "mbde_20.rom",  0x100000, 0x80000, 0xb8b2139b )
	ROM_LOAD_WIDE_SWAP( "mbde_21.rom",  0x180000, 0x80000, 0x690c026a )

	ROM_REGION_DISPOSE(0x600000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "mb_gfx02.rom",   0x000000, 0x80000, 0x2ffbfea8 )
	ROM_LOAD( "mb_gfx06.rom",   0x080000, 0x80000, 0xb76c70e9 )
	ROM_LOAD( "mb_gfx11.rom",   0x100000, 0x80000, 0x8fb94743 )
	ROM_LOAD( "mbgfx01.rom",    0x180000, 0x80000, 0xa53b1c81 )
	ROM_LOAD( "mb_gfx05.rom",   0x200000, 0x80000, 0x506b9dc9 )
	ROM_LOAD( "mb_gfx10.rom",   0x280000, 0x80000, 0x97976ff5 )
	ROM_LOAD( "mb_gfx04.rom",   0x300000, 0x80000, 0x1eb9841d )
	ROM_LOAD( "mb_gfx08.rom",   0x380000, 0x80000, 0xe60c9556 )
	ROM_LOAD( "mb_gfx13.rom",   0x400000, 0x80000, 0xda810d5f )
	ROM_LOAD( "mbgfx03.rom",    0x480000, 0x80000, 0x23fe10f6 )
	ROM_LOAD( "mb_gfx07.rom",   0x500000, 0x80000, 0xaff8c2fb )
	ROM_LOAD( "mb_gfx12.rom",   0x580000, 0x80000, 0xb350a840 )

	ROM_REGION(0x28000) /* QSound Z80 code */
	ROM_LOAD( "mb_q.rom",       0x00000, 0x08000, 0xd6fa76d1 )
	ROM_CONTINUE(               0x10000, 0x18000 )

	ROM_REGION(0x400000) /* QSound samples */
	ROM_LOAD( "mb_q1.rom",      0x000000, 0x80000, 0x0630c3ce )
	ROM_LOAD( "mb_q2.rom",      0x080000, 0x80000, 0x354f9c21 )
	ROM_LOAD( "mb_q3.rom",      0x100000, 0x80000, 0x7838487c )
	ROM_LOAD( "mb_q4.rom",      0x180000, 0x80000, 0xab66e087 )
	ROM_LOAD( "mb_q5.rom",      0x200000, 0x80000, 0xc789fef2 )
	ROM_LOAD( "mb_q6.rom",      0x280000, 0x80000, 0xecb81b61 )
	ROM_LOAD( "mb_q7.rom",      0x300000, 0x80000, 0x041e49ba )
	ROM_LOAD( "mb_q8.rom",      0x380000, 0x80000, 0x59fe702a )
ROM_END

ROM_START( pnickj_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "pnij36.bin",   0x00000, 0x20000, 0x2d4ffb2b )
	ROM_LOAD_ODD ( "pnij42.bin",   0x00000, 0x20000, 0xc085dfaf )

	ROM_REGION_DISPOSE(0x200000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD_GFX_EVEN( "pnij26.bin",   0x000000, 0x20000, 0xe2af981e )
	ROM_LOAD_GFX_ODD ( "pnij18.bin",   0x000000, 0x20000, 0xf17a0e56 )
	ROM_LOAD_GFX_EVEN( "pnij27.bin",   0x040000, 0x20000, 0x83d5cb0e )
	ROM_LOAD_GFX_ODD ( "pnij19.bin",   0x040000, 0x20000, 0xaf08b230 )
	ROM_LOAD_GFX_EVEN( "pnij09.bin",   0x080000, 0x20000, 0x48177b0a )
	ROM_LOAD_GFX_ODD ( "pnij01.bin",   0x080000, 0x20000, 0x01a0f311 )
	ROM_LOAD_GFX_EVEN( "pnij10.bin",   0x0c0000, 0x20000, 0xc2acc171 )
	ROM_LOAD_GFX_ODD ( "pnij02.bin",   0x0c0000, 0x20000, 0x0e21fc33 )
	ROM_LOAD_GFX_EVEN( "pnij38.bin",   0x100000, 0x20000, 0xeb75bd8c )
	ROM_LOAD_GFX_ODD ( "pnij32.bin",   0x100000, 0x20000, 0x84560bef )
	ROM_LOAD_GFX_EVEN( "pnij39.bin",   0x140000, 0x20000, 0x70fbe579 )
	ROM_LOAD_GFX_ODD ( "pnij33.bin",   0x140000, 0x20000, 0x3ed2c680 )
	ROM_LOAD_GFX_EVEN( "pnij13.bin",   0x180000, 0x20000, 0x406451b0 )
	ROM_LOAD_GFX_ODD ( "pnij05.bin",   0x180000, 0x20000, 0x8c515dc0 )
	ROM_LOAD_GFX_EVEN( "pnij14.bin",   0x1c0000, 0x20000, 0x7fe59b19 )
	ROM_LOAD_GFX_ODD ( "pnij06.bin",   0x1c0000, 0x20000, 0x79f4bfe3 )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "pnij17.bin",   0x00000, 0x10000, 0xe86f787a )
	ROM_RELOAD(               0x08000, 0x10000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD ( "pnij24.bin",   0x00000, 0x20000, 0x5092257d )
	ROM_LOAD ( "pnij25.bin",   0x20000, 0x20000, 0x22109aaa )
ROM_END

ROM_START( qad_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_EVEN( "qdu_36a.rom",  0x00000, 0x20000, 0xde9c24a0 )
	ROM_LOAD_ODD ( "qdu_42a.rom",  0x00000, 0x20000, 0xcfe36f0c )
	ROM_LOAD_EVEN( "qdu_37a.rom",  0x40000, 0x20000, 0x10d22320 )
	ROM_LOAD_ODD ( "qdu_43a.rom",  0x40000, 0x20000, 0x15e6beb9 )

	ROM_REGION_DISPOSE(0x200000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD_GFX_EVEN( "qdu_24.rom", 0x000000, 0x20000, 0x2f1bd0ec )
	ROM_LOAD_GFX_ODD ( "qdu_17.rom", 0x000000, 0x20000, 0xa812f9e2 )
	ROM_LOAD_GFX_EVEN( "qdu_09.rom", 0x080000, 0x20000, 0x8c3f9f44 )
	ROM_LOAD_GFX_ODD ( "qdu_01.rom", 0x080000, 0x20000, 0xf688cf8f )
	ROM_LOAD_GFX_EVEN( "qdu_38.rom", 0x100000, 0x20000, 0xccdddd1f )
	ROM_LOAD_GFX_ODD ( "qdu_32.rom", 0x100000, 0x20000, 0xa8d295d3 )
	ROM_LOAD_GFX_EVEN( "qdu_13.rom", 0x180000, 0x20000, 0xafbd551b )
	ROM_LOAD_GFX_ODD ( "qdu_05.rom", 0x180000, 0x20000, 0xc3db0910 )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "qdu_23.rom",  0x00000, 0x10000, 0xcfb5264b )
	ROM_RELOAD(              0x08000, 0x10000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD ( "qdu_30.rom",  0x00000, 0x20000, 0xf190da84 )
	ROM_LOAD ( "qdu_31.rom",  0x20000, 0x20000, 0xb7583f73 )
ROM_END

ROM_START( qadj_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_WIDE_SWAP( "qad23a.bin",   0x00000, 0x80000, 0x4d3553de )
	ROM_LOAD_WIDE_SWAP( "qad22a.bin",   0x80000, 0x80000, 0x3191ddd0 )

	ROM_REGION_DISPOSE(0x200000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "qad03.bin",   0x000000, 0x80000, 0xcea4ca8c )
	ROM_LOAD( "qad01.bin",   0x080000, 0x80000, 0x9d853b57 )
	ROM_LOAD( "qad04.bin",   0x100000, 0x80000, 0x41b74d1b )
	ROM_LOAD( "qad02.bin",   0x180000, 0x80000, 0xb35976c4 )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "qad09.bin",   0x00000, 0x10000, 0x733161cc )
	ROM_RELOAD(              0x08000, 0x10000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD ( "qad18.bin",   0x00000, 0x20000, 0x2bfe6f6a )
	ROM_LOAD ( "qad19.bin",   0x20000, 0x20000, 0x13d3236b )
ROM_END

ROM_START( megaman_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_WIDE_SWAP( "rcma_23b.rom",   0x00000, 0x80000, 0x61e4a397 )
	ROM_LOAD_WIDE_SWAP( "rcma_22b.rom",   0x80000, 0x80000, 0x708268c4 )
	ROM_LOAD_WIDE_SWAP( "rcma_21a.rom",   0x100000, 0x80000, 0x4376ea95 )

	ROM_REGION_DISPOSE(0x800000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "rcm_03.rom",    0x000000, 0x80000, 0x36f3073c )
	ROM_LOAD( "rcm_07.rom",    0x080000, 0x80000, 0x826de013 )
	ROM_LOAD( "rcm_12.rom",    0x100000, 0x80000, 0xfed5f203 )
	ROM_LOAD( "rcm_16.rom",    0x180000, 0x80000, 0x93d97fde )
	ROM_LOAD( "rcm_01.rom",    0x200000, 0x80000, 0x6ecdf13f )
	ROM_LOAD( "rcm_05.rom",    0x280000, 0x80000, 0x5dd131fd )
	ROM_LOAD( "rcm_10.rom",    0x300000, 0x80000, 0x4dc8ada9 )
	ROM_LOAD( "rcm_14.rom",    0x380000, 0x80000, 0x303be3bd )
	ROM_LOAD( "rcm_04.rom",    0x400000, 0x80000, 0x54e622ff )
	ROM_LOAD( "rcm_08.rom",    0x480000, 0x80000, 0xfbff64cf )
	ROM_LOAD( "rcm_13.rom",    0x500000, 0x80000, 0x5069d4a9 )
	ROM_LOAD( "rcm_17.rom",    0x580000, 0x80000, 0x92371042 )
	ROM_LOAD( "rcm_02.rom",    0x600000, 0x80000, 0x944d4f0f )
	ROM_LOAD( "rcm_06.rom",    0x680000, 0x80000, 0xf0faf813 )
	ROM_LOAD( "rcm_11.rom",    0x700000, 0x80000, 0xf2b9ee06 )
	ROM_LOAD( "rcm_15.rom",    0x780000, 0x80000, 0x4f2d372f )

	ROM_REGION(0x28000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "rcm_09.rom",    0x000000, 0x08000, 0x9632d6ef )
	ROM_CONTINUE(              0x010000, 0x18000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD( "rcm_18.rom",    0x00000, 0x20000, 0x80f1f8aa )
	ROM_LOAD( "rcm_19.rom",    0x20000, 0x20000, 0xf257dbe1 )
ROM_END

ROM_START( rockmanj_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_WIDE_SWAP( "rcm23a.bin",   0x00000, 0x80000, 0xefd96cb2 )
	ROM_LOAD_WIDE_SWAP( "rcm22a.bin",   0x80000, 0x80000, 0x8729a689 )
	ROM_LOAD_WIDE_SWAP( "rcm21a.bin",   0x100000, 0x80000, 0x517ccde2 )

	ROM_REGION_DISPOSE(0x800000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "rcm_03.rom",    0x000000, 0x80000, 0x36f3073c )
	ROM_LOAD( "rcm_07.rom",    0x080000, 0x80000, 0x826de013 )
	ROM_LOAD( "rcm_12.rom",    0x100000, 0x80000, 0xfed5f203 )
	ROM_LOAD( "rcm_16.rom",    0x180000, 0x80000, 0x93d97fde )
	ROM_LOAD( "rcm_01.rom",    0x200000, 0x80000, 0x6ecdf13f )
	ROM_LOAD( "rcm_05.rom",    0x280000, 0x80000, 0x5dd131fd )
	ROM_LOAD( "rcm_10.rom",    0x300000, 0x80000, 0x4dc8ada9 )
	ROM_LOAD( "rcm_14.rom",    0x380000, 0x80000, 0x303be3bd )
	ROM_LOAD( "rcm_04.rom",    0x400000, 0x80000, 0x54e622ff )
	ROM_LOAD( "rcm_08.rom",    0x480000, 0x80000, 0xfbff64cf )
	ROM_LOAD( "rcm_13.rom",    0x500000, 0x80000, 0x5069d4a9 )
	ROM_LOAD( "rcm_17.rom",    0x580000, 0x80000, 0x92371042 )
	ROM_LOAD( "rcm_02.rom",    0x600000, 0x80000, 0x944d4f0f )
	ROM_LOAD( "rcm_06.rom",    0x680000, 0x80000, 0xf0faf813 )
	ROM_LOAD( "rcm_11.rom",    0x700000, 0x80000, 0xf2b9ee06 )
	ROM_LOAD( "rcm_15.rom",    0x780000, 0x80000, 0x4f2d372f )

	ROM_REGION(0x28000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "rcm_09.rom",    0x000000, 0x08000, 0x9632d6ef )
	ROM_CONTINUE(              0x010000, 0x18000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD( "rcm_18.rom",    0x00000, 0x20000, 0x80f1f8aa )
	ROM_LOAD( "rcm_19.rom",    0x20000, 0x20000, 0xf257dbe1 )
ROM_END

ROM_START( pang3_rom )
	ROM_REGION(CODE_SIZE)      /* 68000 code */
	ROM_LOAD_WIDE_SWAP( "pa3j-17.11l",  0x00000, 0x80000, 0x21f6e51f )
	ROM_LOAD_WIDE_SWAP( "pa3j-16.10l",  0x80000, 0x80000, 0xca1d7897 )

	ROM_REGION_DISPOSE(0x400000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "pa3-01m.2c",    0x100000, 0x100000, 0x068a152c )
	ROM_CONTINUE(              0x000000, 0x100000 )
	ROM_LOAD( "pa3-07m.2f",    0x300000, 0x100000, 0x3a4a619d )
	ROM_CONTINUE(              0x200000, 0x100000 )

	ROM_REGION(0x18000) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "pa3-11.11f",    0x000000, 0x08000, 0x90a08c46 )
	ROM_RELOAD(                0x010000, 0x08000 )

	ROM_REGION(0x40000) /* Samples */
	ROM_LOAD( "pa3-05.10d",    0x00000, 0x20000, 0x73a10d5d )
	ROM_LOAD( "pa3-06.11d",    0x20000, 0x20000, 0xaffa4f82 )
ROM_END



/* Please include this credit for the CPS1 driver */
#define CPS1_CREDITS "Paul Leaman"


struct GameDriver forgottn_driver =
{
	__FILE__,
	0,
	"forgottn",
	"Forgotten Worlds",
	"1988",
	"Capcom",
	CPS1_CREDITS,
	0,
	&forgottn_machine_driver,
	cps1_init_machine,

	forgottn_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	forgottn_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver ghouls_driver =
{
	__FILE__,
	0,
	"ghouls",
	"Ghouls'n Ghosts",
	"1988",
	"Capcom",
	CPS1_CREDITS,
	0,
	&ghouls_machine_driver,
	cps1_init_machine,

	ghouls_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	ghouls_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver ghoulsj_driver =
{
	__FILE__,
	&ghouls_driver,
	"ghoulsj",
	"Dai Makai-Mura",
	"1988",
	"Capcom",
	CPS1_CREDITS,
	0,
	&ghouls_machine_driver,
	cps1_init_machine,

	ghoulsj_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	ghouls_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver strider_driver =
{
	__FILE__,
	0,
	"strider",
	"Strider (US)",
	"1989",
	"Capcom",
	CPS1_CREDITS,
	0,
	&strider_machine_driver,
	cps1_init_machine,

	strider_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	strider_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver striderj_driver =
{
	__FILE__,
	&strider_driver,
	"striderj",
	"Strider (Japan)",
	"1989",
	"Capcom",
	CPS1_CREDITS,
	0,
	&strider_machine_driver,
	cps1_init_machine,

	striderj_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	strider_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver dwj_driver =
{
	__FILE__,
	0,
	"dwj",
	"Tenchi o Kurau (Japan)",
	"1989",
	"Capcom",
	CPS1_CREDITS,
	0,
	&dwj_machine_driver,
	cps1_init_machine,

	dwj_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	dwj_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver willow_driver =
{
	__FILE__,
	0,
	"willow",
	"Willow (Japan, English)",
	"1989",
	"Capcom",
	CPS1_CREDITS,
	0,
	&willow_machine_driver,
	cps1_init_machine,

	willow_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	willow_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver willowj_driver =
{
	__FILE__,
	&willow_driver,
	"willowj",
	"Willow (Japan, Japanese)",
	"1989",
	"Capcom",
	CPS1_CREDITS,
	0,
	&willow_machine_driver,
	cps1_init_machine,

	willowj_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	willow_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver unsquad_driver =
{
	__FILE__,
	0,
	"unsquad",
	"UN Squadron",
	"1989",
	"Capcom",
	CPS1_CREDITS,
	0,
	&unsquad_machine_driver,
	cps1_init_machine,

	unsquad_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	unsquad_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver area88_driver =
{
	__FILE__,
	&unsquad_driver,
	"area88",
	"Area 88",
	"1989",
	"Capcom",
	CPS1_CREDITS,
	0,
	&unsquad_machine_driver,
	cps1_init_machine,

	area88_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	unsquad_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver ffight_driver =
{
	__FILE__,
	0,
	"ffight",
	"Final Fight (World)",
	"1989",
	"Capcom",
	CPS1_CREDITS,
	0,
	&ffight_machine_driver,
	cps1_init_machine,

	ffight_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	ffight_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver ffightu_driver =
{
	__FILE__,
	&ffight_driver,
	"ffightu",
	"Final Fight (US)",
	"1989",
	"Capcom",
	CPS1_CREDITS,
	0,
	&ffight_machine_driver,
	cps1_init_machine,

	ffightu_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	ffight_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver ffightj_driver =
{
	__FILE__,
	&ffight_driver,
	"ffightj",
	"Final Fight (Japan)",
	"1989",
	"Capcom",
	CPS1_CREDITS,
	0,
	&ffight_machine_driver,
	cps1_init_machine,

	ffightj_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	ffight_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver c1941_driver =
{
	__FILE__,
	0,
	"1941",
	"1941 (World)",
	"1990",
	"Capcom",
	CPS1_CREDITS,
	0,
	&c1941_machine_driver,
	cps1_init_machine,

	c1941_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	c1941_input_ports,
	0, 0, 0,

	ORIENTATION_ROTATE_270,
	0, 0
};

struct GameDriver c1941j_driver =
{
	__FILE__,
	&c1941_driver,
	"1941j",
	"1941 (Japan)",
	"1990",
	"Capcom",
	CPS1_CREDITS,
	0,
	&c1941_machine_driver,
	cps1_init_machine,

	c1941j_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	c1941_input_ports,
	0, 0, 0,

	ORIENTATION_ROTATE_270,
	0, 0
};

struct GameDriver mercs_driver =
{
	__FILE__,
	0,
	"mercs",
	"Mercs (World)",
	"1990",
	"Capcom",
	CPS1_CREDITS,
	0,
	&mercs_machine_driver,
	cps1_init_machine,

	mercs_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	mercs_input_ports,
	0, 0, 0,

	ORIENTATION_ROTATE_270,
	0, 0
};

struct GameDriver mercsu_driver =
{
	__FILE__,
	&mercs_driver,
	"mercsu",
	"Mercs (US)",
	"1990",
	"Capcom",
	CPS1_CREDITS,
	0,
	&mercs_machine_driver,
	cps1_init_machine,

	mercsu_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	mercs_input_ports,
	0, 0, 0,

	ORIENTATION_ROTATE_270,
	0, 0
};

struct GameDriver mercsj_driver =
{
	__FILE__,
	&mercs_driver,
	"mercsj",
	"Senjo no Ookami II (Japan)",
	"1990",
	"Capcom",
	CPS1_CREDITS,
	0,
	&mercs_machine_driver,
	cps1_init_machine,

	mercsj_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	mercs_input_ports,
	0, 0, 0,

	ORIENTATION_ROTATE_270,
	0, 0
};

struct GameDriver mtwins_driver =
{
	__FILE__,
	0,
	"mtwins",
	"Mega Twins / Chiki Chiki Boys (World)",
	"1990",
	"Capcom",
	CPS1_CREDITS,
	0,
	&mtwins_machine_driver,
	cps1_init_machine,

	mtwins_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	mtwins_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver chikij_driver =
{
	__FILE__,
	&mtwins_driver,
	"chikij",
	"Mega Twins / Chiki Chiki Boys (Japan)",
	"1990",
	"Capcom",
	CPS1_CREDITS,
	0,
	&mtwins_machine_driver,
	cps1_init_machine,

	chikij_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	mtwins_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver msword_driver =
{
	__FILE__,
	0,
	"msword",
	"Magic Sword (World)",
	"1990",
	"Capcom",
	CPS1_CREDITS,
	0,
	&msword_machine_driver,
	cps1_init_machine,

	msword_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	msword_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver mswordu_driver =
{
	__FILE__,
	&msword_driver,
	"mswordu",
	"Magic Sword (US)",
	"1990",
	"Capcom",
	CPS1_CREDITS,
	0,
	&msword_machine_driver,
	cps1_init_machine,

	mswordu_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	msword_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver mswordj_driver =
{
	__FILE__,
	&msword_driver,
	"mswordj",
	"Magic Sword (Japan)",
	"1990",
	"Capcom",
	CPS1_CREDITS,
	0,
	&msword_machine_driver,
	cps1_init_machine,

	mswordj_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	msword_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver cawing_driver =
{
	__FILE__,
	0,
	"cawing",
	"Carrier Air Wing / U.S. Navy (World)",
	"1990",
	"Capcom",
	CPS1_CREDITS,
	0,
	&cawing_machine_driver,
	cps1_init_machine,

	cawing_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	cawing_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver cawingj_driver =
{
	__FILE__,
	&cawing_driver,
	"cawingj",
	"Carrier Air Wing / U.S. Navy (Japan)",
	"1990",
	"Capcom",
	CPS1_CREDITS,
	0,
	&cawing_machine_driver,
	cps1_init_machine,

	cawingj_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	cawing_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver nemo_driver =
{
	__FILE__,
	0,
	"nemo",
	"Nemo (World)",
	"1990",
	"Capcom",
	CPS1_CREDITS,
	0,
	&nemo_machine_driver,
	cps1_init_machine,

	nemo_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	nemo_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver nemoj_driver =
{
	__FILE__,
	&nemo_driver,
	"nemoj",
	"Nemo (Japan)",
	"1990",
	"Capcom",
	CPS1_CREDITS,
	0,
	&nemo_machine_driver,
	cps1_init_machine,

	nemoj_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	nemo_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver sf2_driver =
{
	__FILE__,
	0,
	"sf2",
	"Street Fighter II (World rev B)",
	"1991",
	"Capcom",
	CPS1_CREDITS,
	0,
	&sf2_machine_driver,
	cps1_init_machine,

	sf2_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	sf2_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver sf2a_driver =
{
	__FILE__,
	&sf2_driver,
	"sf2a",
	"Street Fighter II (US rev A)",
	"1991",
	"Capcom",
	CPS1_CREDITS,
	0,
	&sf2_machine_driver,
	cps1_init_machine,

	sf2a_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	sf2_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver sf2b_driver =
{
	__FILE__,
	&sf2_driver,
	"sf2b",
	"Street Fighter II (US rev B)",
	"1991",
	"Capcom",
	CPS1_CREDITS,
	0,
	&sf2_machine_driver,
	cps1_init_machine,

	sf2b_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	sf2_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver sf2e_driver =
{
	__FILE__,
	&sf2_driver,
	"sf2e",
	"Street Fighter II (US rev E)",
	"1991",
	"Capcom",
	CPS1_CREDITS,
	0,
	&sf2_machine_driver,
	cps1_init_machine,

	sf2e_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	sf2_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver sf2j_driver =
{
	__FILE__,
	&sf2_driver,
	"sf2j",
	"Street Fighter II (Japan)",
	"1991",
	"Capcom",
	CPS1_CREDITS,
	0,
	&sf2_machine_driver,
	cps1_init_machine,

	sf2j_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	sf2_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver c3wonders_driver =
{
	__FILE__,
	0,
	"3wonders",
	"Three Wonders (US)",
	"1991",
	"Capcom",
	CPS1_CREDITS,
	0,
	&c3wonders_machine_driver,
	cps1_init_machine,

	c3wonders_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	c3wonders_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver c3wonderj_driver =
{
	__FILE__,
	&c3wonders_driver,
	"3wonderj",
	"Wonder 3 (Japan)",
	"1991",
	"Capcom",
	CPS1_CREDITS,
	0,
	&c3wonders_machine_driver,
	cps1_init_machine,

	c3wonderj_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	c3wonders_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver kod_driver =
{
	__FILE__,
	0,
	"kod",
	"King of Dragons (World)",
	"1991",
	"Capcom",
	CPS1_CREDITS,
	0,
	&kod_machine_driver,
	cps1_init_machine,

	kod_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	kod_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver kodj_driver =
{
	__FILE__,
	&kod_driver,
	"kodj",
	"King of Dragons (Japan)",
	"1991",
	"Capcom",
	CPS1_CREDITS,
	0,
	&kod_machine_driver,
	cps1_init_machine,

	kodj_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	kod_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver kodb_driver =
{
	__FILE__,
	&kod_driver,
	"kodb",
	"King of Dragons (bootleg)",
	"1991",
	"Capcom",
	CPS1_CREDITS,
	GAME_NOT_WORKING,
	&kod_machine_driver,
	cps1_init_machine,

	kodb_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	kod_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver captcomm_driver =
{
	__FILE__,
	0,
	"captcomm",
	"Captain Commando (World)",
	"1991",
	"Capcom",
	CPS1_CREDITS,
	0,
	&captcomm_machine_driver,
	cps1_init_machine,

	captcomm_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	captcomm_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver captcomu_driver =
{
	__FILE__,
	&captcomm_driver,
	"captcomu",
	"Captain Commando (US)",
	"1991",
	"Capcom",
	CPS1_CREDITS,
	0,
	&captcomm_machine_driver,
	cps1_init_machine,

	captcomu_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	captcomm_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver captcomj_driver =
{
	__FILE__,
	&captcomm_driver,
	"captcomj",
	"Captain Commando (Japan)",
	"1991",
	"Capcom",
	CPS1_CREDITS,
	0,
	&captcomm_machine_driver,
	cps1_init_machine,

	captcomj_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	captcomm_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver knights_driver =
{
	__FILE__,
	0,
	"knights",
	"Knights of the Round (World)",
	"1991",
	"Capcom",
	CPS1_CREDITS,
	0,
	&knights_machine_driver,
	cps1_init_machine,

	knights_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	knights_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver knightsj_driver =
{
	__FILE__,
	&knights_driver,
	"knightsj",
	"Knights of the Round (Japan)",
	"1991",
	"Capcom",
	CPS1_CREDITS,
	0,
	&knights_machine_driver,
	cps1_init_machine,

	knightsj_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	knights_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver sf2ce_driver =
{
	__FILE__,
	0,
	"sf2ce",
	"Street Fighter II' - Champion Edition (World)",
	"1992",
	"Capcom",
	CPS1_CREDITS,
	0,
	&sf2_machine_driver,
	cps1_init_machine,

	sf2ce_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	sf2_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver sf2cej_driver =
{
	__FILE__,
	&sf2ce_driver,
	"sf2cej",
	"Street Fighter II - Champion Edition (Japan)",
	"1992",
	"Capcom",
	CPS1_CREDITS,
	0,
	&sf2_machine_driver,
	cps1_init_machine,

	sf2cej_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	sf2_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver sf2rb_driver =
{
	__FILE__,
	&sf2ce_driver,
	"sf2rb",
	"Street Fighter II (Rainbow Edition)",
	"1991",
	"hack",
	CPS1_CREDITS,
	GAME_NOT_WORKING,
	&sf2_machine_driver,
	cps1_init_machine,

	sf2rb_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	sf2_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver varth_driver =
{
	__FILE__,
	0,
	"varth",
	"Varth (World)",
	"1992",
	"Capcom",
	CPS1_CREDITS,
	0,
	&varth_machine_driver,
	cps1_init_machine,

	varth_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	varth_input_ports,
	0, 0, 0,

	ORIENTATION_ROTATE_270,
	0, 0
};

struct GameDriver varthj_driver =
{
	__FILE__,
	&varth_driver,
	"varthj",
	"Varth (Japan)",
	"1992",
	"Capcom",
	CPS1_CREDITS,
	0,
	&varth_machine_driver,
	cps1_init_machine,

	varthj_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	varth_input_ports,
	0, 0, 0,

	ORIENTATION_ROTATE_270,
	0, 0
};

struct GameDriver cworld2j_driver =
{
	__FILE__,
	0,
	"cworld2j",
	"Capcom World 2 (Japan)",
	"1992",
	"Capcom",
	CPS1_CREDITS,
	0,
	&cworld2j_machine_driver,
	cps1_init_machine,

	cworld2j_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	cworld2j_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver wof_driver =
{
	__FILE__,
	0,
	"wof",
	"Warriors of Fate (World)",
	"1992",
	"Capcom",
	CPS1_CREDITS,
	0,
	&wof_machine_driver,
	cps1_init_machine,

	wof_rom,
	0, wof_decode,
	0,
	0,      /* sound_prom */

	wof_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	cps1_eeprom_load, cps1_eeprom_save
};

struct GameDriver wofj_driver =
{
	__FILE__,
	&wof_driver,
	"wofj",
	"Tenchi o Kurau 2 (Japan)",
	"1992",
	"Capcom",
	CPS1_CREDITS,
	0,
	&wof_machine_driver,
	cps1_init_machine,

	wofj_rom,
	0, wof_decode,
	0,
	0,      /* sound_prom */

	wof_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	cps1_eeprom_load, cps1_eeprom_save
};

struct GameDriver sf2t_driver =
{
	__FILE__,
	&sf2ce_driver,
	"sf2t",
	"Street Fighter II' - Hyper Fighting (US)",
	"1992",
	"Capcom",
	CPS1_CREDITS,
	0,
	&sf2_machine_driver,
	cps1_init_machine,

	sf2t_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	sf2_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver sf2tj_driver =
{
	__FILE__,
	&sf2ce_driver,
	"sf2tj",
	"Street Fighter II Turbo (Japan)",
	"1991",
	"Capcom",
	CPS1_CREDITS,
	0,
	&sf2_machine_driver,
	cps1_init_machine,

	sf2tj_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	sf2_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};


struct GameDriver dino_driver =
{
	__FILE__,
	0,
	"dino",
	"Cadillacs and Dinosaurs (World)",
	"1993",
	"Capcom",
	CPS1_CREDITS,
	0,
	&dino_machine_driver,
	cps1_init_machine,

	dino_rom,
	0, dino_decode,
	0,
	0,      /* sound_prom */

	dino_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	cps1_eeprom_load, cps1_eeprom_save
};


struct GameDriver punisher_driver =
{
	__FILE__,
	0,
	"punisher",
	"Punisher (World)",
	"1993",
	"Capcom",
	CPS1_CREDITS,
	0,
	&punisher_machine_driver,
	cps1_init_machine,

	punisher_rom,
	0, punisher_decode,
	0,
	0,      /* sound_prom */

	punisher_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	cps1_eeprom_load, cps1_eeprom_save
};

struct GameDriver punishrj_driver =
{
	__FILE__,
	&punisher_driver,
	"punishrj",
	"Punisher (Japan)",
	"1993",
	"Capcom",
	CPS1_CREDITS,
	0,
	&punisher_machine_driver,
	cps1_init_machine,

	punishrj_rom,
	0, punisher_decode,
	0,
	0,      /* sound_prom */

	punisher_input_ports,
	0,0,0,
	ORIENTATION_DEFAULT,
	cps1_eeprom_load, cps1_eeprom_save
};

struct GameDriver slammast_driver =
{
	__FILE__,
	0,
	"slammast",
	"Slam Masters (World)",
	"1993",
	"Capcom",
	CPS1_CREDITS,
	GAME_NOT_WORKING,
	&slammast_machine_driver,
	cps1_init_machine,

	slammast_rom,
	0, slammast_decode,
	0,
	0,      /* sound_prom */

	slammast_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	cps1_eeprom_load, cps1_eeprom_save
};

struct GameDriver mbomber_driver =
{
	__FILE__,
	&slammast_driver,
	"mbomber",
	"Muscle Bomber Duo (World)",
	"1993",
	"Capcom",
	CPS1_CREDITS,
	GAME_NOT_WORKING,
	&mbomber_machine_driver,
	cps1_init_machine,

	mbomber_rom,
	0, slammast_decode,
	0,
	0,      /* sound_prom */

	mbomber_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	cps1_eeprom_load, cps1_eeprom_save
};

struct GameDriver mbomberj_driver =
{
	__FILE__,
	&slammast_driver,
	"mbomberj",
	"Muscle Bomber Duo (Japan)",
	"1993",
	"Capcom",
	CPS1_CREDITS,
	GAME_NOT_WORKING,
	&mbomber_machine_driver,
	cps1_init_machine,

	mbomberj_rom,
	0, slammast_decode,
	0,
	0,      /* sound_prom */

	mbomber_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	cps1_eeprom_load, cps1_eeprom_save
};

struct GameDriver pnickj_driver =
{
	__FILE__,
	0,
	"pnickj",
	"Pnickies (Japan)",
	"1994",
	"Capcom (Compile license)",
	CPS1_CREDITS,
	0,
	&pnickj_machine_driver,
	cps1_init_machine,

	pnickj_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	pnickj_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver qad_driver =
{
	__FILE__,
	0,
	"qad",
	"Quiz & Dragons (US)",
	"1992",
	"Capcom",
	CPS1_CREDITS,
	0,
	&qad_machine_driver,
	cps1_init_machine,

	qad_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	qad_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver qadj_driver =
{
	__FILE__,
	&qad_driver,
	"qadj",
	"Quiz & Dragons (Japan)",
	"1994",
	"Capcom",
	CPS1_CREDITS,
	0,
	&qad_machine_driver,
	cps1_init_machine,

	qadj_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	qadj_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver megaman_driver =
{
	__FILE__,
	0,
	"megaman",
	"Mega Man - The Power Battle (Asia)",
	"1995",
	"Capcom",
	CPS1_CREDITS,
	0,
	&megaman_machine_driver,
	cps1_init_machine,

	megaman_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	megaman_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

struct GameDriver rockmanj_driver =
{
	__FILE__,
	&megaman_driver,
	"rockmanj",
	"Rockman - The Power Battle (Japan)",
	"1995",
	"Capcom",
	CPS1_CREDITS,
	0,
	&megaman_machine_driver,
	cps1_init_machine,

	rockmanj_rom,
	0, 0,
	0,
	0,      /* sound_prom */

	megaman_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	0, 0
};

static void pang3_decode(void)
{
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];
	int A,src,dst;

	for (A = 0x80000;A < 0x100000;A += 2)
	{
		/* only the low 8 bits of each word are encrypted */
		src = READ_WORD(&RAM[A]);
		dst = src & 0xff00;
		if ( src & 0x01) dst ^= 0x04;
		if ( src & 0x02) dst ^= 0x21;
		if ( src & 0x04) dst ^= 0x01;
		if (~src & 0x08) dst ^= 0x50;
		if ( src & 0x10) dst ^= 0x40;
		if ( src & 0x20) dst ^= 0x06;
		if ( src & 0x40) dst ^= 0x08;
		if (~src & 0x80) dst ^= 0x88;
		WRITE_WORD(&RAM[A],dst);
	}

	/* patch out EEPROM test */
	WRITE_WORD(&RAM[0x20de2], 0x4e71);
	WRITE_WORD(&RAM[0x20de4], 0x4e71);
	WRITE_WORD(&RAM[0x20de6], 0x4e71);
}

struct GameDriver pang3_driver =
{
	__FILE__,
	0,
	"pang3",
	"Pang! 3 (Japan)",
	"1995",
	"Mitchell",
	CPS1_CREDITS,
	0,
	&pang3_machine_driver,
	cps1_init_machine,

	pang3_rom,
	pang3_decode, 0,
	0,
	0,      /* sound_prom */

	pang3_input_ports,
	0, 0, 0,

	ORIENTATION_DEFAULT,
	cps1_eeprom_load, cps1_eeprom_save
};


int is_cps1_driver(const struct GameDriver *drv)
{
	return (drv->driver_init==cps1_init_machine);
}

static int cps1_bitswap1(int src,int key,int select)
{
	if (select & (1 << ((key >> 0) & 7)))
		src = (src & 0xfc) | ((src & 0x01) << 1) | ((src & 0x02) >> 1);
	if (select & (1 << ((key >> 4) & 7)))
		src = (src & 0xf3) | ((src & 0x04) << 1) | ((src & 0x08) >> 1);
	if (select & (1 << ((key >> 8) & 7)))
		src = (src & 0xcf) | ((src & 0x10) << 1) | ((src & 0x20) >> 1);
	if (select & (1 << ((key >>12) & 7)))
		src = (src & 0x3f) | ((src & 0x40) << 1) | ((src & 0x80) >> 1);
	return src;
}


static int cps1_bitswap2(int src,int key,int select)
{
	if (select & (1 << ((key >>12) & 7)))
		src = (src & 0xfc) | ((src & 0x01) << 1) | ((src & 0x02) >> 1);
	if (select & (1 << ((key >> 8) & 7)))
		src = (src & 0xf3) | ((src & 0x04) << 1) | ((src & 0x08) >> 1);
	if (select & (1 << ((key >> 4) & 7)))
		src = (src & 0xcf) | ((src & 0x10) << 1) | ((src & 0x20) >> 1);
	if (select & (1 << ((key >> 0) & 7)))
		src = (src & 0x3f) | ((src & 0x40) << 1) | ((src & 0x80) >> 1);
	return src;
}

static int cps1_bytedecode(int src,int swap_key1,int swap_key2,int xor_key,int select)
{
	src = cps1_bitswap1(src,swap_key1 & 0xffff,select & 0xff);
	src = ((src & 0x7f) << 1) | ((src & 0x80) >> 7);
	src = cps1_bitswap2(src,swap_key1 >> 16,select & 0xff);
	src ^= xor_key;
	src = ((src & 0x7f) << 1) | ((src & 0x80) >> 7);
	src = cps1_bitswap2(src,swap_key2 & 0xffff,select >> 8);
	src = ((src & 0x7f) << 1) | ((src & 0x80) >> 7);
	src = cps1_bitswap1(src,swap_key2 >> 16,select >> 8);
	return src;
}

static void _cps1_decode(unsigned char *src,unsigned char *dest_op,unsigned char *dest_data,
		int base_addr,int length,int swap_key1,int swap_key2,int addr_key,int xor_key)
{
	int A;
	int select;

	for (A = 0;A < length;A++)
	{
		/* decode opcodes */
		select = (A + base_addr) + addr_key;
		dest_op[A] = cps1_bytedecode(src[A],swap_key1,swap_key2,xor_key,select);

		/* decode data */
		select = ((A + base_addr) ^ 0x1fc0) + addr_key + 1;
		dest_data[A] = cps1_bytedecode(src[A],swap_key1,swap_key2,xor_key,select);
	}
}

static void cps1_decode(int swap_key1,int swap_key2,int addr_key,int xor_key)
{
	extern int encrypted_cpu;
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[1].memory_region];
	encrypted_cpu = 1;
	_cps1_decode(RAM,ROM,RAM,0x0000,0x8000, swap_key1,swap_key2,addr_key,xor_key);
}

void wof_decode(void)      { cps1_decode(0x01234567,0x54163072,0x5151,0x51); }
void dino_decode(void)     { cps1_decode(0x76543210,0x24601357,0x4343,0x43); }
void punisher_decode(void) { cps1_decode(0x67452103,0x75316024,0x2222,0x22); }
void slammast_decode(void) { cps1_decode(0x54321076,0x65432107,0x3131,0x19); }
