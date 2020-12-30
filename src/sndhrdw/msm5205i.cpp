/*
 *   streaming ADPCM driver
 *   by Aaron Giles
 *
 *   Library to transcode from an ADPCM source to raw PCM.
 *   Written by Buffoni Mirko in 08/06/97
 *   References: various sources and documents.
 *
 *	 HJB 08/31/98
 *	 modified to use an automatically selected oversampling factor
 *	 for the current Machine->sample_rate
 *
 *	 01/06/99
 *	separate MSM5205I emulator form adpcm.c and some fix
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "driver.h"
#include "msm5205i.h"

/*
 * ADPCM lockup tabe
 */

/* step size index shift table */
static int index_shift[8] = { -1, -1, -1, -1, 2, 4, 6, 8 };

/* lookup table for the precomputed difference */
static int diff_lookup[49*16];

/*
 *   Compute the difference table
 */

static void ComputeTables (void)
{
	/* nibble to bit map */
	static int nbl2bit[16][4] =
	{
		{ 1, 0, 0, 0}, { 1, 0, 0, 1}, { 1, 0, 1, 0}, { 1, 0, 1, 1},
		{ 1, 1, 0, 0}, { 1, 1, 0, 1}, { 1, 1, 1, 0}, { 1, 1, 1, 1},
		{-1, 0, 0, 0}, {-1, 0, 0, 1}, {-1, 0, 1, 0}, {-1, 0, 1, 1},
		{-1, 1, 0, 0}, {-1, 1, 0, 1}, {-1, 1, 1, 0}, {-1, 1, 1, 1}
	};

	int step, nib;

	/* loop over all possible steps */
	for (step = 0; step <= 48; step++)
	{
		/* compute the step value */
		int stepval = (int)floor (16.0 * pow (11.0 / 10.0, (double)step));

		/* loop over all nibbles and compute the difference */
		for (nib = 0; nib < 16; nib++)
		{
			diff_lookup[step*16 + nib] = nbl2bit[nib][0] *
				(stepval   * nbl2bit[nib][1] +
				 stepval/2 * nbl2bit[nib][2] +
				 stepval/4 * nbl2bit[nib][3] +
				 stepval/8);
		}
	}
}

/*
 *
 *	MSM 5205 ADPCM chip:
 *
 *	Data is streamed from a CPU by means of a clock generated on the chip.
 *
 *	A reset signal is set high or low to determine whether playback (and interrupts) are occuring
 *
 */

struct MSM5205IVoice
{
	int stream;             /* number of stream system      */
	void *timer;              /* VCLK callback timer          */
	int data;               /* next adpcm data              */
	int vclk;               /* vclk signal (external mode)  */
	int reset;              /* reset pin signal             */
	int prescaler;          /* prescaler selector S1 and S2 */
	int bitwidth;           /* bit width selector -3B/4B    */
	int signal;             /* current ADPCM signal         */
	int step;               /* current ADPCM step           */
};

static const struct MSM5205Iinterface *msm5205_intf;
static struct MSM5205IVoice msm5205[MAX_MSM5205I];

/* stream update callbacks */
static void MSM5205I_update16(int chip,void *buffer,int length)
{
	struct MSM5205IVoice *voice = &msm5205[chip];

	/* if this voice is active */
	if(voice->signal)
	{
		short val = voice->signal * 16;
		short *buf = (short *)buffer;
		/* 16-bit case */
		while (length)
		{
			*buf++ = val;
			length--;
		}
	}
	else
		fast_memset (buffer,0,length*2);
}

/* timer callback at VCLK low eddge */
static void MSM5205I_vclk_callback(int num)
{
	struct MSM5205IVoice *voice = &msm5205[num];
	int val;
	int new_signal;
	/* callback user handler and latch next data */
	if(msm5205_intf->vclk_interrupt[num]) (*msm5205_intf->vclk_interrupt[num])(num);

	/* reset check at last hieddge of VCLK */
	if(voice->reset)
	{
		new_signal = 0;
		voice->step = 0;
	}
	else
	{
		/* update signal */
		/* !! MSM5205I has internal 12bit decoding, signal width is 0 to 8191 !! */
		val = voice->data;
		new_signal = voice->signal + diff_lookup[voice->step * 16 + (val & 15)];
		if (new_signal > 2047) new_signal = 2047;
		else if (new_signal < -2048) new_signal = -2048;
		voice->step += index_shift[val & 7];
		if (voice->step > 48) voice->step = 48;
		else if (voice->step < 0) voice->step = 0;
	}
	/* update when signal changed */
	if( voice->signal != new_signal)
	{
		stream_update(voice->stream,0);
		voice->signal = new_signal;
	}
}
/*
 *    Start emulation of an MSM5205I-compatible chip
 */

int MSM5205I_sh_start (const struct MSM5205Iinterface *intf)
{
	int i;

	/* save a global pointer to our interface */
	msm5205_intf = intf;

	/* compute the difference tables */
	ComputeTables ();

	/* initialize the voices */
	fast_memset (msm5205, 0, sizeof (msm5205));

	/* stream system initialize */
	for (i = 0;i < msm5205_intf->num;i++)
	{
		struct MSM5205IVoice *voice = &msm5205[i];
		char name[20];
		sprintf(name,"MSM5205I #%d",i);
		voice->stream = stream_init(name, Machine->sample_rate,Machine->sample_bits,i, MSM5205I_update16);
	}
	/* initialize */
	MSM5205I_sh_reset();
	/* success */
	return 0;
}

/*
 *    Stop emulation of an MSM5205I-compatible chip
 */

void MSM5205I_sh_stop (void)
{
}

/*
 *    Update emulation of an MSM5205I-compatible chip
 */

void MSM5205I_sh_update (void)
{
}

static void MSM5205I_set_timer(int num,int select)
{
	struct MSM5205IVoice *voice = &msm5205[num];
	static int prescaler_table[4] = {96,48,64,0};
	int prescaler = prescaler_table[select&0x03];

	if( voice->prescaler != prescaler )
	{
		/* remove VCLK timer */
		if(voice->timer)
		{
			timer_remove(voice->timer);
			voice->timer = 0;
		}
		voice->prescaler = prescaler;
		/* timer set */
		if( prescaler )
		{
			voice->timer =
				timer_pulse (TIME_IN_HZ (msm5205_intf->baseclock / prescaler), num, MSM5205I_vclk_callback);
		}
	}
}

/*
 *    Reset emulation of an MSM5205I-compatible chip
 */
void MSM5205I_sh_reset(void)
{
	int i;

	/* bail if we're not emulating sound */
	if (Machine->sample_rate == 0)
		return;

	for (i = 0; i < msm5205_intf->num; i++)
	{
		struct MSM5205IVoice *voice = &msm5205[i];
		/* initialize work */
		voice->data    = 0;
		voice->vclk    = 0;
		voice->reset   = 0;
		voice->signal  = 0;
		voice->step    = 0;
		/* timer set */
		MSM5205I_set_timer(i,msm5205_intf->select[i] & 0x03);
		/* bitwidth reset */
		msm5205[i].bitwidth = msm5205_intf->select[i]&0x04 ? 4 : 3;
	}
}

/*
 *    Handle an update of the vclk status of a chip (1 is reset ON, 0 is reset OFF)
 *    This function can use selector = MSM5205I_SEX only
 */
void MSM5205I_vclk_w (int num, int vclk)
{
	/* range check the numbers */
	if (num >= msm5205_intf->num)
	{
		return;
	}
	if( msm5205[num].prescaler != 0 )
	{
		printf("error: MSM5205I_vclk_w() called with chip = %d, but VCLK selected master mode\n", num);
	}
	else
	{
		if( msm5205[num].vclk != vclk)
		{
			msm5205[num].vclk = vclk;
			if( !vclk ) MSM5205I_vclk_callback(num);
		}
	}
}

/*
 *    Handle an update of the reset status of a chip (1 is reset ON, 0 is reset OFF)
 */

void MSM5205I_reset_w (int num, int reset)
{
	/* range check the numbers */
	if (num >= msm5205_intf->num)
	{
		return;
	}
	msm5205[num].reset = reset;
}

/*
 *    Handle an update of the data to the chip
 */

void MSM5205I_data_w (int num, int data)
{
	if( msm5205[num].bitwidth == 4)
		msm5205[num].data = data & 0x0f;
	else
		msm5205[num].data = (data & 0x07)<<1; /* unknown */
}

/*
 *    Handle an change of the selector
 */

void MSM5205I_selector_w (int num, int select)
{
	struct MSM5205IVoice *voice = &msm5205[num];

	stream_update(voice->stream,0);
	MSM5205I_set_timer(num,select);
}
/* bitsel = -3B/4B pin : 0= 3bit , 1=4bit */
void MSM5205I_bitwidth_w (int num, int bitsel)
{
	int bitwidth = bitsel ? 4 : 3;
	if( msm5205[num].bitwidth != bitwidth )
	{
		stream_update(msm5205[num].stream,0);
		msm5205[num].bitwidth = bitwidth;
	}
}

void MSM5205I_set_volume(int num,int volume)
{
        struct MSM5205IVoice *voice = &msm5205[num];

        stream_set_volume(voice->stream,volume);
}
