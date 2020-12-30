/***************************************************************************

	NAMCO sound driver.

	This driver handles the three known types of NAMCO wavetable sounds:

		- 3-voice mono (Pac-Man, Pengo, Dig Dug, etc)
		- 8-voice mono (Mappy, Dig Dug 2, etc)
		- 8-voice stereo (System 1)

***************************************************************************/

#include "driver.h"

/* note: we work with signed samples here, unlike other drivers */
#ifdef SIGNED_SAMPLES
	#define AUDIO_CONV(A) (A)
#else
	#define AUDIO_CONV(A) (A+0x80)
#endif

/* 8 voices max */
#define MAX_VOICES 8


/* this structure defines the parameters for a channel */
typedef struct
{
	int frequency;
	int counter;
	int volume[2];
	const unsigned char *wave;
} sound_channel;


/* globals available to everyone */
unsigned char *namco_soundregs;
unsigned char *namco_wavedata;

/* data about the sound system */
static sound_channel channel_list[MAX_VOICES];
static sound_channel *last_channel;

/* global sound parameters */
static const unsigned char *sound_prom;
static int samples_per_byte;
static int sample_bits;
static int num_voices;
static int sound_enable;
static int stream;

/* mixer tables and internal buffers */
static signed char *mixer_table;
static signed char *mixer_lookup;
static short *mixer_buffer;
static short *mixer_buffer_2;



/* build a table to divide by the number of voices; gain is specified as gain*16 */
static int make_mixer_table(int voices, int gain, int bits)
{
	int count = voices * 128;
	int i;

	/* allocate memory */
	mixer_table = (signed char*)gp2x_malloc(256 * voices * bits / 8);
	if (!mixer_table)
		return 1;

	/* find the middle of the table */
	mixer_lookup = mixer_table + (128 * voices * bits / 8);

	/* fill in the table - 8 bit case */
	if (bits == 8)
	{
		for (i = 0; i < count; i++)
		{
			int val = i * gain / (voices * 16);
			if (val > 127) val = 127;
			mixer_lookup[ i] = AUDIO_CONV(val);
			mixer_lookup[-i] = AUDIO_CONV(-val);
		}
	}

	/* fill in the table - 16 bit case */
	else
	{
		for (i = 0; i < count; i++)
		{
			int val = i * gain * 16 / voices;
			if (val > 32767) val = 32767;
			((short *)mixer_lookup)[ i] = val;
			((short *)mixer_lookup)[-i] = -val;
		}
	}

	return 0;
}


/* generate sound to the mix buffer in mono */
static void namco_update_mono(int ch, void *buffer, int length)
{
	sound_channel *voice;
	short *mix;
	int i;

	/* if no sound, we're done */
	if (sound_enable == 0)
	{
		if (sample_bits == 16)
			fast_memset(buffer, 0, length * 2);
		else
			fast_memset(buffer, AUDIO_CONV(0x00), length);
		return;
	}

	/* zap the contents of the mixer buffer */
	fast_memset(mixer_buffer, 0, length * sizeof(short));

	/* loop over each voice and add its contribution */
	for (voice = channel_list; voice < last_channel; voice++)
	{
		int f = voice->frequency;
		int v = voice->volume[0];

		/* only update if we have non-zero volume and frequency */
		if (v && f)
		{
			const unsigned char *w = voice->wave;
			int c = voice->counter;

			mix = mixer_buffer;

			/* add our contribution */
			for (i = 0; i < length; i++)
			{
				int offs;

				c += f;
				offs = (c >> 15) & 0x1f;
				if (samples_per_byte == 1)	/* use only low 4 bits */
					*mix++ += ((w[offs] & 0x0f) - 8) * v;
				else	/* use full byte, first the high 4 bits, then the low 4 bits */
				{
					if (offs & 1)
						*mix++ += ((w[offs>>1] & 0x0f) - 8) * v;
					else
						*mix++ += (((w[offs>>1]>>4) & 0x0f) - 8) * v;
				}
			}

			/* update the counter for this voice */
			voice->counter = c;
		}
	}

	/* mix it down */
	mix = mixer_buffer;
	if (sample_bits == 16)
	{
		short *dest = (short *)buffer;
		for (i = 0; i < length; i++)
			*dest++ = ((short *)mixer_lookup)[*mix++];
	}
	else
	{
		unsigned char *dest = (unsigned char *)buffer;
		for (i = 0; i < length; i++)
			*dest++ = mixer_lookup[*mix++];
	}
}


/* generate sound to the mix buffer in stereo */
static void namco_update_stereo(int ch, void **buffer, int length)
{
	sound_channel *voice;
	short *lmix, *rmix;
	int i;

	/* if no sound, we're done */
	if (sound_enable == 0)
	{
		if (sample_bits == 16)
		{
			fast_memset(buffer[0], 0, length * 2);
			fast_memset(buffer[1], 0, length * 2);
		}
		else
		{
			fast_memset(buffer[0], AUDIO_CONV(0x00), length);
			fast_memset(buffer[1], AUDIO_CONV(0x00), length);
		}
		return;
	}

	/* zap the contents of the mixer buffer */
	fast_memset(mixer_buffer, 0, length * sizeof(short));
	fast_memset(mixer_buffer_2, 0, length * sizeof(short));

	/* loop over each voice and add its contribution */
	for (voice = channel_list; voice < last_channel; voice++)
	{
		int f = voice->frequency;
		int lv = voice->volume[0];
		int rv = voice->volume[1];

		/* only update if we have non-zero volume and frequency */
		if ((lv || rv) && f)
		{
			const unsigned char *w = voice->wave;
			int c = voice->counter;

			lmix = mixer_buffer;
			rmix = mixer_buffer_2;

			/* add our contribution */
			for (i = 0; i < length; i++)
			{
				int offs;

				c += f;
				offs = (c >> 15) & 0x1f;
				if (samples_per_byte == 1)	/* use only low 4 bits */
				{
					*lmix++ += ((w[offs] & 0x0f) - 8) * lv;
					*rmix++ += ((w[offs] & 0x0f) - 8) * rv;
				}
				else	/* use full byte, first the high 4 bits, then the low 4 bits */
				{
					if (offs & 1)
					{
						*lmix++ += ((w[offs>>1] & 0x0f) - 8) * lv;
						*rmix++ += ((w[offs>>1] & 0x0f) - 8) * rv;
					}
					else
					{
						*lmix++ += (((w[offs>>1]>>4) & 0x0f) - 8) * lv;
						*rmix++ += (((w[offs>>1]>>4) & 0x0f) - 8) * rv;
					}
				}
			}

			/* update the counter for this voice */
			voice->counter = c;
		}
	}

	/* mix it down */
	lmix = mixer_buffer;
	rmix = mixer_buffer_2;
	if (sample_bits == 16)
	{
		short *dest1 = (short *)buffer[0];
		short *dest2 = (short *)buffer[1];
		for (i = 0; i < length; i++)
		{
			*dest1++ = ((short *)mixer_lookup)[*lmix++];
			*dest2++ = ((short *)mixer_lookup)[*rmix++];
		}
	}
	else
	{
		unsigned char *dest1 = (unsigned char *)buffer[0];
		unsigned char *dest2 = (unsigned char *)buffer[1];
		for (i = 0; i < length; i++)
		{
			*dest1++ = mixer_lookup[*lmix++];
			*dest2++ = mixer_lookup[*rmix++];
		}
	}
}


int namco_sh_start(struct namco_interface *intf)
{
	const char *mono_name = "NAMCO sound";
	const char *stereo_names[] =
	{
		"NAMCO sound left",
		"NAMCO sound right"
	};
	sound_channel *voice;

	/* get stream channels */
	sample_bits = Machine->sample_bits;
	if (intf->stereo)
	{
		stream = stream_init_multi(2, stereo_names, intf->samplerate, sample_bits, 0, namco_update_stereo);
		stream_set_volume(stream + 0, intf->volume);
		stream_set_volume(stream + 1, intf->volume);
		stream_set_pan(stream + 0, OSD_PAN_LEFT);
		stream_set_pan(stream + 1, OSD_PAN_RIGHT);
	}
	else
	{
		stream = stream_init(mono_name, intf->samplerate, sample_bits, 0, namco_update_mono);
		stream_set_volume(stream, intf->volume);
	}

	/* allocate a pair of buffers to mix into - 1 second's worth should be more than enough */
	if ((mixer_buffer = (signed short*) gp2x_malloc(2 * sizeof(short) * intf->samplerate)) == 0)
		return 1;
	mixer_buffer_2 = mixer_buffer + intf->samplerate;

	/* build the mixer table */
	if (make_mixer_table(intf->voices, intf->gain, Machine->sample_bits))
	{
		gp2x_free (mixer_buffer);
		return 1;
	}

	/* extract globals from the interface */
	num_voices = intf->voices;
	last_channel = channel_list + num_voices;

	if (intf->region == -1)
	{
		sound_prom = namco_wavedata;
		samples_per_byte = 2;	/* first 4 high bits, then low 4 bits */
	}
	else
	{
		sound_prom = Machine->memory_region[intf->region];
		samples_per_byte = 1;	/* use only low 4 bits */
	}

	/* start with sound enabled, many games don't have a sound enable register */
	sound_enable = 1;

	/* reset all the voices */
	for (voice = channel_list; voice < last_channel; voice++)
	{
		voice->frequency = 0;
		voice->volume[0] = voice->volume[1] = 0;
		voice->wave = &sound_prom[0];
		voice->counter = 0;
	}

	return 0;
}


void namco_sh_stop(void)
{
	gp2x_free (mixer_table);
	gp2x_free (mixer_buffer);
}


void namco_sh_update(void)
{
}


/********************************************************************************/


void pengo_sound_enable_w(int offset,int data)
{
	sound_enable = data;
}

void pengo_sound_w(int offset,int data)
{
	sound_channel *voice;
	int base;

	/* update the streams */
	stream_update(stream, 0);

	/* set the register */
	namco_soundregs[offset] = data & 0x0f;

	/* recompute all the voice parameters */
	for (base = 0, voice = channel_list; voice < last_channel; voice++, base += 5)
	{
		voice->frequency = namco_soundregs[0x14 + base];	/* always 0 */
		voice->frequency = voice->frequency * 16 + namco_soundregs[0x13 + base];
		voice->frequency = voice->frequency * 16 + namco_soundregs[0x12 + base];
		voice->frequency = voice->frequency * 16 + namco_soundregs[0x11 + base];
		if (voice == 0)
			voice->frequency = voice->frequency * 16 + namco_soundregs[0x10 + base];
		else
			voice->frequency = voice->frequency * 16;

		voice->volume[0] = namco_soundregs[0x15 + base] & 0x0f;
		voice->wave = &sound_prom[32 * (namco_soundregs[0x05 + base] & 7)];
	}
}


/********************************************************************************/

void mappy_sound_enable_w(int offset,int data)
{
	sound_enable = offset;
}

void mappy_sound_w(int offset,int data)
{
	sound_channel *voice;
	int base;

	/* update the streams */
	stream_update(stream, 0);

	/* set the register */
	namco_soundregs[offset] = data;

	/* recompute all the voice parameters */
	for (base = 0, voice = channel_list; voice < last_channel; voice++, base += 8)
	{
		voice->frequency = namco_soundregs[0x06 + base] & 15;	/* high bits are from here */
		voice->frequency = voice->frequency * 256 + namco_soundregs[0x05 + base];
		voice->frequency = voice->frequency * 256 + namco_soundregs[0x04 + base];

		voice->volume[0] = namco_soundregs[0x03 + base] & 0x0f;
		voice->wave = &sound_prom[32 * ((namco_soundregs[0x06 + base] >> 4) & 7)];
	}
}
