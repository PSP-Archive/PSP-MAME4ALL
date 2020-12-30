#include "driver.h"

static int firstchannel,numchannels;

#define MAX_CHANNELS 16
#define MAX_SAMPLES_PER_FRAME 1664

struct PlayingSAM {
        int     bytes_played;           /* Bytes already played */
        int     max_frame_len;          /* Max Frame length*/
        int     current_frame_len;      /* Current Frame length*/
        int     loop;                   /* loop 1 yes, 0 no */
        int     active;                 /* 1 active, 0 inactive */
	int	sample_length;		/* Sample: length */
	int	sample_freq;		/* Sample: frequency */
	int	sample_res;		/* Sample: resolution (8 or 16) */
	int	sample_volume;		/* Sample: volume */
	signed char	*sample_data;	/* Sample: data (pointer to) */
};

static struct PlayingSAM disksample[MAX_CHANNELS];

/* Start one of the samples loaded from disk. Note: channel must be in the range */
/* 0 .. Samplesinterface->channels-1. It is NOT the discrete channel to pass to */
/* osd_play_sample() */
void sample_start(int channel,int samplenum,int loop)
{
	int iSampleResAdj;

	if (Machine->sample_rate == 0) return;
	if (Machine->samples == 0) return;
	if (Machine->samples->sample[samplenum] == 0) return;
	if (samplenum >= Machine->samples->total) return;

	/*
	 * 	Attempt to play all samples
	 * 	Wrap around the channel number if it exceeds MAX_CHANNELS
	 *	This may cause overlap with some channels trying to play on top
	 *	of each other but also means that more samples should play.
	 */
	channel %= MAX_CHANNELS;

	/*
	 * Setup sample structure
	 */
	disksample[channel].sample_length = Machine->samples->sample[samplenum]->length;
	disksample[channel].sample_freq = Machine->samples->sample[samplenum]->smpfreq;
	disksample[channel].sample_res = Machine->samples->sample[samplenum]->resolution;
	disksample[channel].sample_volume = Machine->samples->sample[samplenum]->volume;
	disksample[channel].sample_data = Machine->samples->sample[samplenum]->data;
	iSampleResAdj = disksample[channel].sample_res == 8 ? 0:1;
	disksample[channel].max_frame_len = (disksample[channel].sample_freq << iSampleResAdj) / Machine->drv->frames_per_second;
	if (disksample[channel].max_frame_len > MAX_SAMPLES_PER_FRAME) disksample[channel].max_frame_len = MAX_SAMPLES_PER_FRAME;
	disksample[channel].active = 1;
	disksample[channel].loop = loop;
	disksample[channel].bytes_played = 0;

}

/* 
 * New function: used to setup a sample to play which was otherwise called just using osd_play_sample()
 */
void sample_setup(int channel, signed char *data, int length, int freq, int volume, int loop)
{
	int iSampleResAdj;

        if (Machine->sample_rate == 0) return;
        if (Machine->samples == 0) return;
	channel %= MAX_CHANNELS;

	disksample[channel].sample_length = length;
	disksample[channel].sample_freq = freq;
	disksample[channel].sample_res = 8;
	disksample[channel].sample_volume = volume;
	disksample[channel].sample_data = data;

	iSampleResAdj = disksample[channel].sample_res == 8 ? 0:1;
	disksample[channel].max_frame_len = (disksample[channel].sample_freq << iSampleResAdj) / Machine->drv->frames_per_second;
	if (disksample[channel].max_frame_len > MAX_SAMPLES_PER_FRAME) disksample[channel].max_frame_len = MAX_SAMPLES_PER_FRAME;
	disksample[channel].active = 1;
	disksample[channel].loop = loop;
	disksample[channel].bytes_played = 0;
}

void sample_adjust(int channel,int freq,int volume)
{
	int iSampleResAdj;

	if (Machine->sample_rate == 0) return;
	if (Machine->samples == 0) return;
	channel %= MAX_CHANNELS;


	/*
	 * Update sample structure
	 */
	if (disksample[channel].active == 1)
	{
		if (volume != -1)
			disksample[channel].sample_volume = volume;
		if (freq != -1)
		{
			disksample[channel].sample_freq = freq;
			iSampleResAdj = disksample[channel].sample_res == 8 ? 0:1;
			disksample[channel].max_frame_len = (disksample[channel].sample_freq << iSampleResAdj) / Machine->drv->frames_per_second;
			if (disksample[channel].max_frame_len > MAX_SAMPLES_PER_FRAME) disksample[channel].max_frame_len = MAX_SAMPLES_PER_FRAME;
		}
	}

	osd_adjust_sample(channel + firstchannel,freq,volume);
}

void sample_stop(int channel)
{
	if (Machine->sample_rate == 0) return;

	channel %= MAX_CHANNELS;

	osd_stop_sample(channel + firstchannel);

	/*
	 * Update sample structure
	 */
	disksample[channel].active = 0;
}

int sample_playing(int channel)
{
	if (Machine->sample_rate == 0) return 0;
	channel %= MAX_CHANNELS;

	return disksample[channel].active;
	//return !osd_get_sample_status(channel + firstchannel);
}


int samples_sh_start(struct Samplesinterface *interface)
{
	int i;
	if (Machine->sample_rate == 0) return 0;
	for (i=0;i<MAX_CHANNELS;i++)
		fast_memset((void*)&disksample[i],0,sizeof(struct PlayingSAM));	
	numchannels = interface->channels;
	firstchannel = get_play_channels(numchannels);
	return 0;
}

void samples_sh_stop(void)
{
	int i;

	for (i=0; i<MAX_CHANNELS; i++)
	{
		sample_stop(i);
		disksample[i].active = 0;
	}
}


void samples_sh_update(void)
{
	int i;
	for (i=0; i<MAX_CHANNELS; i++)
	{
		if (disksample[i].active)
		{
			/* Assume frame length is the remaining data. */
			disksample[i].current_frame_len = disksample[i].sample_length - disksample[i].bytes_played;
			/* Adjust length if too large/small */
			if (disksample[i].current_frame_len > disksample[i].max_frame_len)
			{
				disksample[i].current_frame_len = disksample[i].max_frame_len;
			}
			if (disksample[i].current_frame_len < 0)
			{
				disksample[i].current_frame_len = 0;
			}
			
			//printf("Sample %d Frame %d Vol %d Loop %d Bits %d\n", i, disksample[i].current_frame_len, disksample[i].sample_volume, disksample[i].loop, disksample[i].sample_res);

			playsample(
				i + firstchannel,
				(signed char *)disksample[i].sample_data + disksample[i].bytes_played,
				disksample[i].current_frame_len,
				disksample[i].sample_freq,
				disksample[i].sample_volume,
				disksample[i].loop,
				OSD_PAN_CENTER,
				disksample[i].sample_res);

			disksample[i].bytes_played += disksample[i].current_frame_len;

			if (disksample[i].bytes_played >= disksample[i].sample_length)
			{
				if (disksample[i].loop)
				{
					/* start again if played through */
					disksample[i].bytes_played = 0;
				}
				else
				{
					/* stop the sample */
					sample_stop(i);
				}
			}
		}
	}
}
