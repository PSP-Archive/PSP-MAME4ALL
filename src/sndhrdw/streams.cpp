#include "driver.h"
#include <math.h>

static int stream_joined_channels[MAX_STREAM_CHANNELS];
static char stream_name[MAX_STREAM_CHANNELS][40];
static void *stream_buffer[MAX_STREAM_CHANNELS];
static int stream_buffer_len[MAX_STREAM_CHANNELS];
static int stream_sample_rate[MAX_STREAM_CHANNELS];
static int stream_sample_bits[MAX_STREAM_CHANNELS];
static int stream_volume[MAX_STREAM_CHANNELS];
static int stream_pan[MAX_STREAM_CHANNELS];
static int stream_buffer_pos[MAX_STREAM_CHANNELS];
static int stream_sample_length[MAX_STREAM_CHANNELS];	/* in usec */
static int stream_param[MAX_STREAM_CHANNELS];
static void (*stream_callback[MAX_STREAM_CHANNELS])(int param,void *buffer,int length);
static void (*stream_callback_multi[MAX_STREAM_CHANNELS])(int param,void **buffer,int length);

static int memory[MAX_STREAM_CHANNELS];
static int r1[MAX_STREAM_CHANNELS];
static int r2[MAX_STREAM_CHANNELS];
static int r3[MAX_STREAM_CHANNELS];
static int c[MAX_STREAM_CHANNELS];

/*
signal >--R1--+--R2--+
              |      |
              C      R3---> amp
              |      |
             GND    GND
*/

/* R1, R2, R3 in Ohm; C in pF */
/* set C = 0 to disable the filter */
void set_RC_filter(int channel,int R1,int R2,int R3,int C)
{
	r1[channel] = R1;
	r2[channel] = R2;
	r3[channel] = R3;
	c[channel] = C;
}

void apply_RC_filter_8(int channel,signed char *buf,int len,int sample_rate)
{
	float R1,R2,R3,C;
	float Req;
	int K;
	int i;

	if (c[channel] == 0) return;	/* filter disabled */

	R1 = r1[channel];
	R2 = r2[channel];
	R3 = r3[channel];
	C = (float)c[channel] * 1E-12;
	Req = (R1*(R2+R3))/(R1+R2+R3);
	K = 0x10000 * exp(-1 / (Req * C) / sample_rate);
	buf[0] = buf[0] + (memory[channel] - buf[0]) * K / 0x10000;
	for (i = 1;i < len;i++)
		buf[i] = buf[i] + (buf[i-1] - buf[i]) * K / 0x10000;

	memory[channel] = buf[len-1];
}

void apply_RC_filter_16(int channel,signed short *buf,int len,int sample_rate)
{
	float R1,R2,R3,C;
	float Req;
	int K;
	int i;

	if (c[channel] == 0) return;	/* filter disabled */

	R1 = r1[channel];
	R2 = r2[channel];
	R3 = r3[channel];
	C = (float)c[channel] * 1E-12;
	Req = (R1*(R2+R3))/(R1+R2+R3);
	K = 0x10000 * exp(-1 / (Req * C) / sample_rate);
	buf[0] = buf[0] + (memory[channel] - buf[0]) * K / 0x10000;
	for (i = 1;i < len;i++)
		buf[i] = buf[i] + (buf[i-1] - buf[i]) * K / 0x10000;

	memory[channel] = buf[len-1];
}



int streams_sh_start(void)
{
	int i;


	for (i = 0;i < MAX_STREAM_CHANNELS;i++)
	{
		stream_joined_channels[i] = 1;
		stream_buffer[i] = 0;
		c[i] = 0;	/* Set RC Filters Off */
	}

	return 0;
}


void streams_sh_stop(void)
{
	int i;


	for (i = 0;i < MAX_STREAM_CHANNELS;i++)
	{
		if (stream_buffer[i]) {
			gp2x_free(stream_buffer[i]);
			stream_buffer[i] = 0;
		}
	}
}


void streams_sh_update(void)
{
	int channel,i;

	if (Machine->sample_rate == 0) return;

	/* update all the output buffers */
	for (channel = 0;channel < MAX_STREAM_CHANNELS;channel += stream_joined_channels[channel])
	{
		if (stream_buffer[channel])
		{
			int newpos;
			int buflen;


			newpos = stream_buffer_len[channel];

			buflen = newpos - stream_buffer_pos[channel];

			if (stream_joined_channels[channel] > 1)
			{
				void *buf[MAX_STREAM_CHANNELS];

				if (buflen > 0)
				{
					if (stream_sample_bits[channel] == 16)
					{
						for (i = 0;i < stream_joined_channels[channel];i++)
							buf[i] = &((short *)stream_buffer[channel+i])[stream_buffer_pos[channel+i]];
					}
					else
					{
						for (i = 0;i < stream_joined_channels[channel];i++)
							buf[i] = &((char *)stream_buffer[channel+i])[stream_buffer_pos[channel+i]];
					}

					(*stream_callback_multi[channel])(stream_param[channel],buf,buflen);
				}

				for (i = 0;i < stream_joined_channels[channel];i++)
					stream_buffer_pos[channel+i] = 0;

				if (stream_sample_bits[channel] == 16)
				{
					for (i = 0;i < stream_joined_channels[channel];i++)
						apply_RC_filter_16(channel+i,(short int*)stream_buffer[channel+i],stream_buffer_len[channel+i],stream_sample_rate[channel+i]);
				}
				else
				{
					for (i = 0;i < stream_joined_channels[channel];i++)
						apply_RC_filter_8(channel+i,(signed char *)stream_buffer[channel+i],stream_buffer_len[channel+i],stream_sample_rate[channel+i]);
				}
			}
			else

			{
				if (buflen > 0)
				{
					void *buf;


					if (stream_sample_bits[channel] == 16)
						buf = &((short *)stream_buffer[channel])[stream_buffer_pos[channel]];
					else
						buf = &((char *)stream_buffer[channel])[stream_buffer_pos[channel]];

					(*stream_callback[channel])(stream_param[channel],buf,buflen);
				}

				stream_buffer_pos[channel] = 0;

				if (stream_sample_bits[channel] == 16)
					apply_RC_filter_16(channel,(short int*)stream_buffer[channel],stream_buffer_len[channel],stream_sample_rate[channel]);
				else
					apply_RC_filter_8(channel,(signed char*)stream_buffer[channel],stream_buffer_len[channel],stream_sample_rate[channel]);
			}
		}
	}

	for (channel = 0;channel < MAX_STREAM_CHANNELS;channel += stream_joined_channels[channel])
	{
		if (stream_buffer[channel])
		{
			if (stream_sample_bits[channel] == 16)
			{
				for (i = 0;i < stream_joined_channels[channel];i++)
					osd_play_streamed_sample_16(channel+i,
							stream_buffer[channel+i],stream_buffer_len[channel+i]<<1,
							stream_sample_rate[channel+i],stream_volume[channel+i],stream_pan[channel+i]);
			}
			else
			{
				for (i = 0;i < stream_joined_channels[channel];i++)
					osd_play_streamed_sample(channel+i,
							(signed char*)stream_buffer[channel+i],stream_buffer_len[channel+i],
							stream_sample_rate[channel+i],stream_volume[channel+i],stream_pan[channel+i]);
			}
		}
	}
}


int stream_init(const char *name,int sample_rate,int sample_bits,
		int param,void (*callback)(int param,void *buffer,int length))
{
	int channel;


	channel = get_play_channels(1);

	stream_joined_channels[channel] = 1;

	strcpy(stream_name[channel],name);

	stream_buffer_len[channel] = sample_rate / Machine->drv->frames_per_second;
	/* adjust sample rate to make it a multiple of buffer_len */
	sample_rate = stream_buffer_len[channel] * Machine->drv->frames_per_second;

	/* Values aren't initialised if sound is disabled */
	if( sample_bits != 0 && stream_buffer_len[channel] != 0 )
	{
		if (stream_buffer[channel])
			gp2x_free(stream_buffer[channel]);
		if ((stream_buffer[channel] = gp2x_malloc((sample_bits/8)*stream_buffer_len[channel])) == 0)
			return -1;
	}

	stream_sample_rate[channel] = sample_rate;
	stream_sample_bits[channel] = sample_bits;
	stream_volume[channel] = 100;
	stream_pan[channel] = OSD_PAN_CENTER;
	stream_buffer_pos[channel] = 0;
	if (sample_rate)
		stream_sample_length[channel] = 1000000 / sample_rate;
	else
		stream_sample_length[channel] = 0;
	stream_param[channel] = param;
	stream_callback[channel] = callback;

	return channel;
}


int stream_init_multi(int channels,const char **name,int sample_rate,int sample_bits,
		int param,void (*callback)(int param,void **buffer,int length))
{
	int channel,i;


	channel = get_play_channels(channels);

	stream_joined_channels[channel] = channels;
	sample_rate=(int)(((100.0-((snd_str_reduction-1.0)*5.0))*((double)sample_rate))/100.0);

	for (i = 0;i < channels;i++)
	{
		strcpy(stream_name[channel+i],name[i]);

		stream_buffer_len[channel+i] = sample_rate / Machine->drv->frames_per_second;
		/* adjust sample rate to make it a multiple of buffer_len */
		sample_rate = stream_buffer_len[channel+i] * Machine->drv->frames_per_second;

		/* Values aren't initialised if sound is disabled */
		if( sample_bits != 0 && stream_buffer_len[channel] != 0 )
		{
			if (stream_buffer[channel+i])
				gp2x_free(stream_buffer[channel+i]);
			if ((stream_buffer[channel+i] = gp2x_malloc((sample_bits/8)*stream_buffer_len[channel+i])) == 0)
				return -1;
		}

		stream_sample_rate[channel+i] = sample_rate;
		stream_sample_bits[channel+i] = sample_bits;
		stream_volume[channel+i] = 100;
		stream_pan[channel+i] = 0;
		stream_buffer_pos[channel+i] = 0;
		if (sample_rate)
			stream_sample_length[channel+i] = 1000000 / sample_rate;
		else
			stream_sample_length[channel+i] = 0;
	}

	stream_param[channel] = param;
	stream_callback_multi[channel] = callback;

	return channel;
}


/* min_interval is in usec */
void stream_update(int channel,int min_interval)
{
	int newpos;
	int buflen;


	if (stream_buffer[channel] == 0)
		return;

	/* get current position based on the timer */
	newpos = cpu_scalebyfcount(stream_buffer_len[channel]);

	buflen = newpos - stream_buffer_pos[channel];

	if (buflen * stream_sample_length[channel] > min_interval)
	{
		if (stream_joined_channels[channel] > 1)
		{
			void *buf[MAX_STREAM_CHANNELS];
			int i;


			if (stream_sample_bits[channel] == 16)
			{
				for (i = 0;i < stream_joined_channels[channel];i++)
					buf[i] = &((short *)stream_buffer[channel+i])[stream_buffer_pos[channel+i]];
			}
			else
			{
				for (i = 0;i < stream_joined_channels[channel];i++)
					buf[i] = &((char *)stream_buffer[channel+i])[stream_buffer_pos[channel+i]];
			}

			(*stream_callback_multi[channel])(stream_param[channel],buf,buflen);

			for (i = 0;i < stream_joined_channels[channel];i++)
				stream_buffer_pos[channel+i] += buflen;
		}
		else
		{
			void *buf;


			if (stream_sample_bits[channel] == 16)
				buf = &((short *)stream_buffer[channel])[stream_buffer_pos[channel]];
			else
				buf = &((char *)stream_buffer[channel])[stream_buffer_pos[channel]];

			(*stream_callback[channel])(stream_param[channel],buf,buflen);

			stream_buffer_pos[channel] += buflen;
		}
	}
}


void stream_set_volume(int channel,int volume)
{
	/* backwards compatibility with old 0-255 volume range */
	if (volume > 100) volume = volume * 25 / 255;

	stream_volume[channel] = volume;
}


int stream_get_volume(int channel)
{
	return stream_volume[channel];
}


void stream_set_pan(int channel,int pan)
{
	stream_pan[channel] = pan;
}


int stream_get_pan(int channel)
{
	return stream_pan[channel];
}


const char *stream_get_name(int channel)
{
	if (stream_buffer[channel])
		return stream_name[channel];
	else return 0;	/* unused channel */
}
