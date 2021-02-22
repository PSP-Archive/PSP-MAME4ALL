#include "driver.h"
#include "minimal.h"

#ifdef DREAMCAST
#include<kos.h>
#endif

/* audio related stuff */
#ifndef SND_CHANNELS_REDUCTION
#define NUMVOICES 16
#else
#define NUMVOICES 8
#endif

#ifndef SND_OPM_REDUCTION
#define SND_OPM_REDUCTION 2.0
#endif
#ifndef SND_PCM_REDUCTION
#define SND_PCM_REDUCTION 2.0
#endif
#ifndef SND_STR_REDUCTION
#define SND_STR_REDUCTION 2.0
#endif
#ifndef SND_UDP_REDUCTION
#define SND_UDP_REDUCTION 2.0
#endif

#ifndef GP2X
int gp2x_sound_rate=DEFAULT_SAMPLE_RATE;
#endif

double snd_opm_reduction=SND_OPM_REDUCTION;
double snd_pcm_reduction=SND_PCM_REDUCTION;
double snd_str_reduction=SND_STR_REDUCTION;
double snd_udp_reduction=SND_UDP_REDUCTION;

struct lpWaves {
	signed char *lpData;
	unsigned int bits;
	unsigned int len; /* length of sound buffer */
	unsigned int freq; /* frequency of sound buffer */
	unsigned int active; /* 1 active, 0 inactive */
	unsigned int volume; /* volume 0 - 64 */
	unsigned int loop; /* loop 1 yes, 0 no */
	unsigned int step; /* Calculated frequency step */
}__attribute__ ((__aligned__ (32)));

struct lpWaves lpWave[NUMVOICES];
unsigned int sound_enable = 0;
unsigned int soundcard;
unsigned int chn_max=0; /* Max Number of channels */

int sound_slice=DEFAULT_SAMPLE_RATE/60;
#if defined(GP2X) || defined(NOSOUND)
#define mame4all_init_sound_slice()
#define MAX_SAMPLE_RATE 44100
static short mixedAudio[MAX_SAMPLE_RATE/30];
#else
extern short *mixedAudio;
void mame4all_init_sound_slice();
#endif

#include "sound_scale.h"

INLINE void gp2x_sound_mixer_channel_16(signed short *stream, unsigned int len, int t, const struct lpWaves *wave)
{
	signed short *sample=(signed short *)wave->lpData;
	unsigned int pos=0;
	unsigned int step = wave->step;
	const unsigned int sample_len=(wave->len)<<16;
	do {
		*(stream++)+=(sample[pos>>16])>>t;
		pos+=step;
		while (pos>=sample_len) pos-=sample_len;
		len--;
	} while (len);	
}

INLINE void gp2x_sound_mixer_channel_8(signed short *stream, unsigned int len, int t, const struct lpWaves *wave)
{
	signed char *sample=(signed char *)wave->lpData;
	unsigned int pos=0;
	unsigned int step = wave->step;
	const unsigned int sample_len=(wave->len)<<16;
	do {
		*(stream++)+=((((signed short)sample[pos>>16])<<8))>>t;
		pos+=step;
		while (pos>=sample_len) pos-=sample_len;
		len--;
	} while (len);
}

INLINE void gp2x_sound_frame ( short *stream, const int len )
{
	struct lpWaves *wave = lpWave;
	unsigned int n = NUMVOICES, t = 0;

	/* Buffer Set to Zero */
#ifndef DREAMCAST
	fast_memset(stream,0,len<<1);
#else
	{unsigned *s=(unsigned *)stream;int n=len>>4;while(n--){*s++=0;*s++=0;*s++=0;*s++=0;*s++=0;*s++=0;*s++=0;*s++=0;}}
#endif
	
	/* Sound Enable ? */	
	if (!sound_enable) return;
	
	do { if (wave->active) t++; wave++; } while (--n); n=t; /* Active Channels now */
	if (!t) return; /* No Active Channels */
	if (t>chn_max) chn_max=t; else t=chn_max; /* Total number of Active channels */

	/* For each of the channels */
	wave=lpWave;
	do {
		/* Channel active ? */
		if (wave->active) {
			if (wave->lpData && wave->volume) {
				if (wave->bits==16)
					gp2x_sound_mixer_channel_16(stream,len,sound_scale[((t<<6)/wave->volume)-1],wave);
				else
					gp2x_sound_mixer_channel_8 (stream,len,sound_scale[((t<<6)/wave->volume)-1],wave);
			}
			if (!(wave->loop))
				wave->active=0;
			n--;
		}
		/* Next channel */
		wave++;
	} while (n);
}

int msdos_init_seal (void)
{
	return 0;
}

int msdos_init_sound(void)
{
	int i;
	sound_enable=0;

	sound_slice=(gp2x_sound_rate/(Machine->drv->frames_per_second));
	mame4all_init_sound_slice();

	/* Initialize Sound Buffer Structure */
	for (i = 0; i < NUMVOICES; i++)
	{
		lpWave[i].lpData = 0;
		lpWave[i].bits = 0;
		lpWave[i].len = 0;
		lpWave[i].freq = 0;
		lpWave[i].active = 0;
		lpWave[i].volume = 0;
		lpWave[i].loop = 0;
		lpWave[i].step = 0;
	}
	chn_max=0;

#ifndef NOSOUND
	if (!soundcard)     /* Sound Deactivated and not emulated */
	{
#endif
#ifdef GP2X
		gp2x_text_log("GP2X No Sound Init...");
#endif
		/* update the Machine structure to show that sound is disabled */
		Machine->sample_rate = 0;
		return 0;
#ifndef NOSOUND
	}
	else
	{
		double r=1.0;
#ifdef GP2X
		gp2x_text_log("GP2X Sound Init...");
#endif
#ifndef NO_DRIVER_TMNT
		if (is_tmnt_driver(Machine->gamedrv))
			r=1.0;
		else
#endif
#ifndef NO_DRIVER_CPS1
		if (is_cps1_driver(Machine->gamedrv))
			r=1.0;
		else
#endif
#ifndef NO_DRIVER_NEOGEO
		if (is_neogeo_driver(Machine->gamedrv))
			r=1.0;
		else
#endif
#ifndef NO_DRIVER_SYSTEM16
		if (is_system16_driver(Machine->gamedrv))
			r=1.0;
		else
#endif
			r=0.7;
		snd_opm_reduction=SND_OPM_REDUCTION*r;
		snd_pcm_reduction=SND_PCM_REDUCTION*r;
		snd_str_reduction=SND_STR_REDUCTION*r;
		snd_udp_reduction=SND_UDP_REDUCTION*r;
	}
#ifdef PSP
	switch(soundcard){
		case 1:
		case 2:
		case 3: /* 22K 16v */
			default_sample_rate=22050;
			snd_opm_reduction=1.0;
			snd_pcm_reduction=1.0;
			snd_str_reduction=1.0;
			snd_udp_reduction=1.0;
			num_voices=16;
			break;
		case 4: /* 44K 16v */
			default_sample_rate=44100;
			snd_opm_reduction=1.0;
			snd_pcm_reduction=1.0;
			snd_str_reduction=1.0;
			snd_udp_reduction=1.0;
			num_voices=16;
			break;
		case 5: /* 44K 8v */
			default_sample_rate=44100;
			snd_opm_reduction=1.0;
			snd_pcm_reduction=1.0;
			snd_str_reduction=1.0;
			snd_udp_reduction=1.0;
			num_voices=8;
			break;
		case 6: /* 44K 4v */
			default_sample_rate=44100;
			snd_opm_reduction=1.0;
			snd_pcm_reduction=1.0;
			snd_str_reduction=1.0;
			snd_udp_reduction=1.0;
			num_voices=4;
			break;
		case 7: /* 33K 16v */
			default_sample_rate=33075;
			snd_opm_reduction=1.0;
			snd_pcm_reduction=1.0;
			snd_str_reduction=1.0;
			snd_udp_reduction=1.0;
			num_voices=16;
			break;
		case 8: /* 33K 8v */
			default_sample_rate=33075;
			snd_opm_reduction=1.0;
			snd_pcm_reduction=1.0;
			snd_str_reduction=1.0;
			snd_udp_reduction=1.0;
			num_voices=8;
			break;
		case 9: /* 33k 4v */
			default_sample_rate=33075;
			snd_opm_reduction=1.0;
			snd_pcm_reduction=1.0;
			snd_str_reduction=1.0;
			snd_udp_reduction=1.0;
			num_voices=4;
			break;
		default:
			break;
	}
#endif
	sound_enable = 1;

	/* update the Machine structure to reflect the actual sample rate */
	Machine->sample_rate = gp2x_sound_rate;

#ifdef GP2X
	/* Start the Sound Thread */
	gp2x_sound_thread_start();
#endif
	return 0;
#endif
}

void msdos_shutdown_sound(void)
{
	int i;

#ifdef GP2X
	/* Stop the Sound Thread */
	gp2x_sound_thread_stop();
#endif

	/* stop and release voices */
	sound_enable = 0;
	
	for (i = 0; i < NUMVOICES; i++)
	{
		lpWave[i].lpData = 0;
		lpWave[i].bits = 0;
		lpWave[i].len = 0;
		lpWave[i].freq = 0;
		lpWave[i].active = 0;
		lpWave[i].volume = 0;
		lpWave[i].loop = 0;
		lpWave[i].step = 0;
	}
	chn_max=0;
}


void osd_update_audio(void)
{
#ifndef NOSOUND
	/* Sound Enable ? */
	if (!soundcard) return;

	/* Sound Update ! */
	gp2x_sound_frame(mixedAudio, sound_slice);
	
	/* Sound Play ! */
	gp2x_sound_play(mixedAudio, sound_slice<<1);
#endif
}


void playsample(int channel,signed char *data,int len,int freq,int volume,int loop, int pan, int bits)
{
#ifndef NOSOUND
	if (!soundcard || channel >= NUMVOICES) return;

	lpWave[channel].active=0;

	if (volume < 0) {
		volume = 0;
	} else if (volume > 0) {
		if (volume > 100) volume = volume * 25 / 255;
		volume = (volume<<6) / 100;
		if (pan != OSD_PAN_CENTER) volume>>=1;
		volume+=20;
		if (volume>64) volume=64;
	}
	
	if (len < 0) len=0;
	else if (bits==16) len>>=1;
	lpWave[channel].lpData = data;
	if (freq < 0) freq = 0;
	lpWave[channel].bits = bits;	
	lpWave[channel].volume=volume;
	lpWave[channel].len=len;
	if (lpWave[channel].freq!=freq) {
		lpWave[channel].freq=freq;
		lpWave[channel].step=(unsigned int)((((double)(freq)) / ((double)gp2x_sound_rate)) * ((double)(1<<16)));
	}
	lpWave[channel].loop=loop;
	lpWave[channel].active=((freq>0) && (len>0));
#endif
}


void osd_adjust_sample(int channel,int freq,int volume)
{
#ifndef NOSOUND
	
	if (!soundcard || channel >= NUMVOICES) return;

	if (freq >= 0 && lpWave[channel].freq!=freq) {
		lpWave[channel].freq=freq;
		lpWave[channel].step=(unsigned int)((((double)(freq)) / ((double)gp2x_sound_rate)) * ((double)(1<<16)));
	}
	if (volume >= 0) {
		if (volume > 0) {
			if (volume > 100) volume = volume * 25 / 255;
			volume = (volume<<6) / 100;
			volume+=20;
			if (volume>64) volume=64;
		}
		lpWave[channel].volume = volume;
	}
#endif
}


void osd_stop_sample(int channel)
{
#ifndef NOSOUND
	if (!soundcard || channel >= NUMVOICES) return;
	lpWave[channel].active = 0;
#endif
}


void osd_restart_sample(int channel)
{
#ifndef NOSOUND
	if (!soundcard || channel >= NUMVOICES) return;
	lpWave[channel].active = 1;
#endif
}


int osd_get_sample_status(int channel)
{
#ifndef NOSOUND
	if (!soundcard || channel >= NUMVOICES) return -1;
	return !(lpWave[channel].active);
#else
	return -1;
#endif
}


void osd_sound_enable(int enable_it)
{ 	
	sound_enable = enable_it;
}
