/***************************************************************************
	meadows.c
	Sound handler
	Dead Eye, Gypsy Juggler

	J. Buchmueller, June '98
****************************************************************************/

#include "driver.h"
#include "s2650/s2650.h"

byte meadows_0c00 = 0;
byte meadows_0c01 = 0;
byte meadows_0c02 = 0;
byte meadows_0c03 = 0;
byte meadows_dac  = 0;
static int dac_enable;

#define BASE_CLOCK		5000000
#define BASE_CTR1       (BASE_CLOCK / 256)
#define BASE_CTR2		(BASE_CLOCK / 32)

#define DIV2OR4_CTR2	0x01
#define ENABLE_CTR2     0x02
#define ENABLE_DAC      0x04
#define ENABLE_CTR1     0x08

static	int channel;
static	int freq1 = 1000;
static	int freq2 = 1000;
static	signed char waveform[2] = { -120, 120 };

/************************************/
/* Sound handler init				*/
/************************************/
int meadows_sh_init(const char *gamename)
{
    return 0;
}

/************************************/
/* Sound handler start				*/
/************************************/
int meadows_sh_start(void)
{
	channel = get_play_channels(2);
	osd_play_sample(channel, waveform, sizeof(waveform), freq1, 0, 1);
	osd_play_sample(channel+1, waveform, sizeof(waveform), freq2, 0, 1);
    return 0;
}

/************************************/
/* Sound handler stop				*/
/************************************/
void meadows_sh_stop(void)
{
	osd_stop_sample(channel);
    osd_stop_sample(channel+1);
}

/************************************/
/* Sound handler update 			*/
/************************************/
void meadows_sh_update(void)
{
static  byte latched_0c01 = 0;
static	byte latched_0c02 = 0;
static	byte latched_0c03 = 0;
int preset, amp;

    if (latched_0c01 != meadows_0c01 || latched_0c03 != meadows_0c03)
	{
		/* amplitude is a combination of the upper 4 bits of 0c01 */
		/* and bit 4 merged from S2650's flag output */
		amp = ((meadows_0c03 & ENABLE_CTR1) == 0) ? 0 : (meadows_0c01 & 0xf0) >> 1;
		if (S2650_get_flag())
			amp += 0x80;
		/* calculate frequency for counter #1 */
		/* bit 0..3 of 0c01 are ctr preset */
		preset = (meadows_0c01 & 15) ^ 15;
		if (preset)
			freq1 = BASE_CTR1 / (preset + 1);
		else amp = 0;
		osd_adjust_sample(channel, freq1 * sizeof(waveform), amp);
    }

	if (latched_0c02 != meadows_0c02 || latched_0c03 != meadows_0c03)
	{
		/* calculate frequency for counter #2 */
		/* 0c02 is ctr preset, 0c03 bit 0 enables division by 2 */
		amp = ((meadows_0c03 & ENABLE_CTR2) != 0) ? 0xa0 : 0;
		preset = meadows_0c02 ^ 0xff;
		if (preset)
		{
			freq2 = BASE_CTR2 / (preset + 1) / 2;
			if ((meadows_0c03 & DIV2OR4_CTR2) == 0)
				freq2 >>= 1;
		}
		else amp = 0;
		osd_adjust_sample(channel+1, freq2 * sizeof(waveform), amp);
    }

	if (latched_0c03 != meadows_0c03)
	{
		dac_enable = meadows_0c03 & ENABLE_DAC;

		if (dac_enable)
			DAC_data_w(0, meadows_dac);
		else
			DAC_data_w(0, 0);
    }

    latched_0c01 = meadows_0c01;
	latched_0c02 = meadows_0c02;
	latched_0c03 = meadows_0c03;
}

/************************************/
/* Write DAC value					*/
/************************************/
void meadows_sh_dac_w(int data)
{
	meadows_dac = data;
	if (dac_enable)
		DAC_data_w(0, meadows_dac);
	else
		DAC_data_w(0, 0);
}


