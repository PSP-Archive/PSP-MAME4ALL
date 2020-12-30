/***************************************************************************

Knuckle Joe - (c) 1985 Seibu Kaihatsu ( Taito License )

driver by Ernesto Corvi

Notes:
This board seems to be an Irem design.
The sound hardware is definitely the 6803-based one used by the classic Irem
games, and the video hardware is pretty much like Irem games too. The only
strange thing is that the screen is flipped vertically.

TODO:
- lots of unknown dipswitches

***************************************************************************/

#include "driver.h"
#include "m6808/m6808.h"
#include "vidhrdw/generic.h"
#include "sndhrdw/msm5205i.h"
#define M6803_PORT1 0x100
#define M6803_PORT2 0x101


/* from vidhrdw */
int kncljoe_vh_start(void);
void kncljoe_vh_convert_color_prom(unsigned char *palette,unsigned short *colortable,const unsigned char *color_prom);
void kncljoe_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh);
void kncljoe_videoram_w (int offset, int data);
void kncljoe_control_w (int offset, int data);
void kncljoe_scroll_w (int offset, int data);
extern UINT8 *kncljoe_scrollregs;	/* 0.72u2 */

static int port1;
static int port2;
static int ddr1;
static int ddr2;

void kncljoe_io_w(int offset,int data)
{
        switch (offset)
        {
                /* port 1 DDR */
                case 0:
                        ddr1 = data;
                        break;

                /* port 2 DDR */
                case 1:
                        ddr2 = data;
                        break;

                /* port 1 */
                case 2:
                        data = (port1 & ~ddr1) | (data & ddr1);
                        port1 = data;
                        break;

                /* port 2 */
                case 3:
                        data = (port2 & ~ddr2) | (data & ddr2);

                        /* write latch */
                        if ((port2 & 0x01) && !(data & 0x01))
                        {
                                /* control or data port? */
                                if (port2 & 0x04)
                                {
                                        /* PSG 0 or 1? */
                                        if (port2 & 0x10)
                                                AY8910_control_port_1_w(0,port1);
                                        else if (port2 & 0x08)
                                                AY8910_control_port_0_w(0,port1);
                                }
                                else
                                {
                                        /* PSG 0 or 1? */
                                        if (port2 & 0x10)
                                                AY8910_write_port_1_w(0,port1);
                                        else if (port2 & 0x08)
                                                AY8910_write_port_0_w(0,port1);
                                }
                        }
                        port2 = data;
                        break;
        }
}


int kncljoe_io_r(int offset)
{
        switch (offset)
        {
                /* port 1 DDR */
                case 0:
                        return ddr1;
                        break;

                /* port 2 DDR */
                case 1:
                        return ddr2;
                        break;

                /* port 1 */
                case 2:

                        /* input 0 or 1? */
                        if (port2 & 0x10)
                                return (AY8910_read_port_1_r(0) & ~ddr1) | (port1 & ddr1);
                        else if (port2 & 0x08)
                                return (AY8910_read_port_0_r(0) & ~ddr1) | (port1 & ddr1);
                        break;

                /* port 2 */
                case 3:
                        break;
        }

        return 0;
}


void kncljoe_sound_cmd_w (int offset, int data)
{
        if ((data & 0x80) == 0)
                soundlatch_w(0,data & 0x7f);
        else
                cpu_cause_interrupt(1,M6808_INT_IRQ);
                /*cpu_set_irq_line(1,0,HOLD_LINE);*/
}
static void kncljoe_port1_w (int offset, int data)
{
        port1 = data;
}

static void kncljoe_port2_w (int offset, int data)
{
        /* write latch */
        if ((port2 & 0x01) && !(data & 0x01))
        {
                /* control or data port? */
                if (port2 & 0x04)
                {
                        /* PSG 0 or 1? */
                        if (port2 & 0x08)
                                AY8910_control_port_0_w(0,port1);
                        if (port2 & 0x10)
                                AY8910_control_port_1_w(0,port1);
                }
                else
                {
                        /* PSG 0 or 1? */
                        if (port2 & 0x08)
                                AY8910_write_port_0_w(0,port1);
                        if (port2 & 0x10)
                                AY8910_write_port_1_w(0,port1);
                }
        }
        port2 = data;
}


static int kncljoe_port1_r (int offset)
{
        /* PSG 0 or 1? */
        if (port2 & 0x08)
                return AY8910_read_port_0_r(0);
        if (port2 & 0x10)
                return AY8910_read_port_1_r(0);
        return 0xff;
}

static int kncljoe_port2_r (int offset)
{
        return 0;
}

static void kncljoe_msm5205_w (int offset, int data)
{
        /* bits 2-4 select MSM5205I clock & 3b/4b playback mode */
        MSM5205I_selector_w(0,(data >> 2) & 7);
        MSM5205I_selector_w(1,((data >> 2) & 4) | 3); /* always in slave mode */

        /* bits 0 and 1 reset the two chips */
        MSM5205I_reset_w(0,data & 1);
        MSM5205I_reset_w(1,data & 2);
}

static void kncljoe_adpcm_w (int offset, int data)
{
        MSM5205I_data_w(offset,data);
}

static void kncljoe_adpcm_int(int data)
{
        //cpu_set_nmi_line(1,PULSE_LINE);
        cpu_cause_interrupt(1,M6808_INT_NMI);

        /* the first MSM5205I clocks the second */
        MSM5205I_vclk_w(1,1);
        MSM5205I_vclk_w(1,0);
}

static void irem_analog_w (int offset, int data)
{
	// No analog sound!
}


struct AY8910interface kncljoe_ay8910_interface =
{
        2,      /* 2 chips */
        3579545/4,
        { 20, 20 },
        { soundlatch_r, 0 },
        { 0 },
        { 0, irem_analog_w },
        { kncljoe_msm5205_w, 0 }
};

struct MSM5205Iinterface kncljoe_msm5205_interface =
{
        2,                    /* 2 chips            */
        384000,               /* 384KHz             */
        { kncljoe_adpcm_int, 0 },/* interrupt function */
        { MSM5205I_S96_4B,MSM5205I_SEX_4B },    /* default to 4KHz, but can be changed at run time */
        { 100, 100 }
};

/******************************************************************************/
static struct MemoryReadAddress kncljoe_sound_readmem[] = {
        { 0x0000, 0x001f, kncljoe_io_r },
        { 0x0080, 0x00ff, MRA_RAM },
        { 0x4000, 0xffff, MRA_ROM },
        { -1 } /* end of table */
};

static struct MemoryWriteAddress kncljoe_sound_writemem[] = {
        { 0x0000, 0x001f, kncljoe_io_w },
        { 0x0080, 0x00ff, MWA_RAM },
        { 0x0800, 0x0800, MWA_NOP },    /* IACK */
        { 0x0801, 0x0802, kncljoe_adpcm_w },
        { 0x9000, 0x9000, MWA_NOP },    /* IACK */
        { 0x4000, 0xffff, MWA_ROM },
        { -1 } /* end of table */
};

static struct IOReadPort kncljoe_sound_readport[] =
{
        { M6803_PORT1, M6803_PORT1, kncljoe_port1_r },
        { M6803_PORT2, M6803_PORT2, kncljoe_port2_r },
        { -1 }
};

static struct IOWritePort kncljoe_sound_writeport[] =
{
        { M6803_PORT1, M6803_PORT1, kncljoe_port1_w },
        { M6803_PORT2, M6803_PORT2, kncljoe_port2_w },
        { -1 }
};

/******************************************************************************/


static struct MemoryReadAddress readmem[] = {
	{ 0x0000, 0xbfff, MRA_ROM },
	{ 0xc000, 0xcfff, videoram_r },		/* videoram */
	{ 0xd800, 0xd800, input_port_0_r }, /* IN 0 */
	{ 0xd801, 0xd801, input_port_1_r }, /* IN 1 */
	{ 0xd802, 0xd802, input_port_2_r }, /* IN 2 */
	{ 0xd803, 0xd803, input_port_3_r },	/* DSW A */
	{ 0xd804, 0xd804, input_port_4_r },	/* DSW B */
	{ 0xe800, 0xefff, MRA_RAM },		/* spriteram */
	{ 0xf000, 0xffff, MRA_RAM },
        { -1 } /* end of table */ };

static struct MemoryWriteAddress writemem[] = {
	{ 0x0000, 0xbfff, MWA_ROM },
	{ 0xc000, 0xcfff, kncljoe_videoram_w, &videoram },
	{ 0xd000, 0xd001, kncljoe_scroll_w, &kncljoe_scrollregs },
	{ 0xd800, 0xd800, kncljoe_sound_cmd_w },
	{ 0xd801, 0xd803, kncljoe_control_w },
	{ 0xe800, 0xefff, MWA_RAM, &spriteram, &spriteram_size },
	{ 0xf000, 0xffff, MWA_RAM },
        { -1 } /* end of table */ };

/******************************************************************************/


INPUT_PORTS_START( kncljoe_input_ports )
	PORT_START	/* IN 0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	/*
	PORT_BIT_IMPULSE( 0x04, IP_ACTIVE_LOW, IPT_COIN1, 4 )
	PORT_BIT_IMPULSE( 0x08, IP_ACTIVE_LOW, IPT_COIN2, 4 )
	*/
	PORT_BITX(0x04, IP_ACTIVE_LOW, IPT_COIN1 | IPF_IMPULSE, "Coin[1]", IP_KEY_DEFAULT, IP_JOY_DEFAULT, 4)
	PORT_BITX(0x08, IP_ACTIVE_LOW, IPT_COIN2 | IPF_IMPULSE, "Coin[2]", IP_KEY_DEFAULT, IP_JOY_DEFAULT, 4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* IN 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* IN 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* DSW A */
	PORT_DIPNAME( 0x07, 0x07, "Coin A" ,IP_KEY_NONE )
	PORT_DIPSETTING( 0x00, "5 Cn/1 Crd" )
	PORT_DIPSETTING( 0x04, "4 Cn/1 Crd" )
	PORT_DIPSETTING( 0x02, "3 Cn/1 Crd" )
	PORT_DIPSETTING( 0x06, "2 Cn/1 Crd" )
	PORT_DIPSETTING( 0x07, "1 Cn/1 Crd" )
	PORT_DIPSETTING( 0x03, "1 Cn/2 Crd" )
	PORT_DIPSETTING( 0x05, "1 Cn/3 Crd" )
	PORT_DIPSETTING( 0x01, "1 Cn/5 Crd" )
	PORT_DIPNAME( 0x18, 0x18, "Coin B" ,IP_KEY_NONE )
	PORT_DIPSETTING( 0x00, "3 Cn/1 Crd" )
	PORT_DIPSETTING( 0x10, "2 Cn/1 Crd" )
	PORT_DIPSETTING( 0x18, "1 Cn/1 Crd" )
	PORT_DIPSETTING( 0x08, "1 Cn/2 Crd" )
	PORT_BITX(    0x20, 0x20, IPT_DIPSWITCH_NAME | IPF_CHEAT, "Invulnerability", IP_KEY_NONE, IP_JOY_NONE ,0 )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x40, "RAM Test?" ,IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	//PORT_SERVICE( 0x80, IP_ACTIVE_LOW )
	PORT_BITX(    0x80, 0x80, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START /* DSW B */
	PORT_DIPNAME( 0x01, 0x01, "Unknown" ,IP_KEY_NONE )
	PORT_DIPSETTING(    0x01, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x02, 0x02, "Cabinet" ,IP_KEY_NONE )
	PORT_DIPSETTING(	0x02, "Upright" )
	PORT_DIPSETTING(	0x00, "Cocktail" )
	PORT_DIPNAME( 0x04, 0x04, "Lives" ,IP_KEY_NONE )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x18, 0x18, "Bonus Lives" ,IP_KEY_NONE )
	PORT_DIPSETTING(    0x18, "10k and every 20k" )
	PORT_DIPSETTING(    0x10, "20k and every 40k" )
	PORT_DIPSETTING(    0x08, "30k and every 60k" )
	PORT_DIPSETTING(    0x00, "40k and every 80k" )
	PORT_DIPNAME( 0x60, 0x60, "Difficulty?" ,IP_KEY_NONE )
	PORT_DIPSETTING(    0x60, "Easy" )
	PORT_DIPSETTING(    0x40, "Medium" )
	PORT_DIPSETTING(    0x20, "Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0x80, 0x00, "Demo Sounds" ,IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
INPUT_PORTS_END


static struct GfxLayout charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static struct GfxLayout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8
};

static struct GfxDecodeInfo gfxdecodeinfo[] =
{
	{ 1, 0, &charlayout,   0x00, 16 },	/* colors 0x00-0x7f direct mapped */
	{ 2, 0, &spritelayout, 0x80, 16 },	/* colors 0x80-0x8f with lookup table */
	{ 3, 0, &spritelayout, 0x80, 16 },
	{ -1 } /* end of array */
};


static int kncljoe_hiload(void)
{
        unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];


        /* check if the hi score table has already been initialized */
        if (memcmp(&RAM[0xf01a],"\x00",3) == 0 &&
                        memcmp(&RAM[0xf046],"\x30",3) == 0)
        {
                void *f;


                if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
                {
                        osd_fread(f,&RAM[0xf01a],0x43);
                        osd_fclose(f);
                }

                return 1;
        }
        else return 0;  /* we can't load the hi scores yet */
}



static void kncljoe_hisave(void)
{
        void *f;
        unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];


        if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
        {
                osd_fwrite(f,&RAM[0xf01a],0x43);
                osd_fclose(f);
        }
}


static const struct MachineDriver machine_driver_kncljoe =
{
	/* basic machine hardware */
	{
		{
			CPU_Z80,
			5500000, /* ? MHz */
			0,
			readmem,writemem,0,0,
			interrupt,1
		},
		{
			CPU_M6803 | CPU_AUDIO_CPU,
			3579545/4,
			4,
			kncljoe_sound_readmem,kncljoe_sound_writemem,
			kncljoe_sound_readport,kncljoe_sound_writeport,
			ignore_interrupt,0
		}
	},
	60, DEFAULT_60HZ_VBLANK_DURATION,  /* frames per second, vblank duration */
	1,	/* 1 CPU slice per frame - interleaving is forced when a sound command is written */
	0,

	/* video hardware */
	32*8, 32*8, { 1*8, 31*8-1, 0*8, 32*8-1 },
	gfxdecodeinfo,
	128+16, 16*8+16*8,
	kncljoe_vh_convert_color_prom,

	VIDEO_TYPE_RASTER,
	0,
	kncljoe_vh_start,
	0,
	kncljoe_vh_screenrefresh,

	/* sound hardware */
	0,0,0,0,
	{
		{
			SOUND_AY8910,
			&kncljoe_ay8910_interface
		},
		{
			SOUND_MSM5205I,
			&kncljoe_msm5205_interface
		}
	}
};



ROM_START( kncljoe_rom )
    ROM_REGION( 0x10000 )  /* 64k for code */
	ROM_LOAD( "kj-1.bin", 0x0000, 0x4000, 0x4e4f5ff2 )
	ROM_LOAD( "kj-2.bin", 0x4000, 0x4000, 0xcb11514b )
	ROM_LOAD( "kj-3.bin", 0x8000, 0x4000, 0x0f50697b )

    ROM_REGION_DISPOSE( 0xc000 )	/* tiles */
	ROM_LOAD( "kj-10.bin", 0x0000,  0x4000, 0x74d3ba33 )
	ROM_LOAD( "kj-11.bin", 0x4000,  0x4000, 0x8ea01455 )
	ROM_LOAD( "kj-12.bin", 0x8000,  0x4000, 0x33367c41 )

    ROM_REGION_DISPOSE( 0x18000 )	/* sprites */
	ROM_LOAD( "kj-4.bin", 0x00000,  0x8000, 0xa499ea10 )
	ROM_LOAD( "kj-6.bin", 0x08000,  0x8000, 0x815f5c0a )
	ROM_LOAD( "kj-5.bin", 0x10000,  0x8000, 0x11111759 )

    ROM_REGION_DISPOSE( 0xc000 )	/* sprites */
	ROM_LOAD( "kj-7.bin", 0x0000,   0x4000, 0x121fcccb )
	ROM_LOAD( "kj-9.bin", 0x4000,   0x4000, 0xaffbe3eb )
	ROM_LOAD( "kj-8.bin", 0x8000,   0x4000, 0xe057e72a )

    ROM_REGION( 0x10000 )  /* 64k for audio code */
	ROM_LOAD( "kj-13.bin",0xe000, 0x2000, 0x0a0be3f5 )

    ROM_REGION( 0x420 )
	ROM_LOAD( "kjclr1.bin",  0x000, 0x100, 0xc3378ac2 ) /* tile red */
	ROM_LOAD( "kjclr2.bin",  0x100, 0x100, 0x2126da97 ) /* tile green */
	ROM_LOAD( "kjclr3.bin",  0x200, 0x100, 0xfde62164 ) /* tile blue */
	ROM_LOAD( "kjprom5.bin", 0x300, 0x020, 0x5a81dd9f ) /* sprite palette */
	ROM_LOAD( "kjprom4.bin", 0x320, 0x100, 0x48dc2066 ) /* sprite clut */
ROM_END

struct GameDriver kncljoe_driver =
{
        __FILE__,
        0,
        "kncljoe",
        "Knuckle Joe (set 1)",
        "1985",
        "[Seibu Kaihatsu] (Taito license)",
        "Credits",
        0,
        &machine_driver_kncljoe,
        0,

        kncljoe_rom,
        0, 0,
        0,
        0,      /* sound_prom */

        kncljoe_input_ports,

        PROM_MEMORY_REGION(5), 0, 0,
        ORIENTATION_DEFAULT,

        0,0
};

#if 0
GAME( 1985, kncljoe,  0,       kncljoe, kncljoe, 0, ROT0, "[Seibu Kaihatsu] (Taito license)", "Knuckle Joe (set 1)" )
#endif

