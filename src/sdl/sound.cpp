#include "driver.h"
#include "minimal.h"

#define SOUND_USESAMPLE_PATCH

#ifdef DREAMCAST
#include<kos.h>
#endif

/* audio related stuff */
#ifdef GP2X
 #ifndef DEF_SAMPLE_RATE
  #define DEF_SAMPLE_RATE
  //#define DEFAULT_SAMPLE_RATE 22050
  int default_sample_rate = 15360;
 #endif
 #define memalloc_report(MSG)
#else
 #ifndef DEF_SAMPLE_RATE
  #define DEF_SAMPLE_RATE
  int default_sample_rate = 22050;
 #endif
#endif

#ifndef SND_CHANNELS_REDUCTION
#define NUMVOICES 16
#else
#define NUMVOICES 8
#endif

unsigned short num_voices = NUMVOICES;

#define SAMPLE_NUMBER 1664

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

double snd_opm_reduction=SND_OPM_REDUCTION;
double snd_pcm_reduction=SND_PCM_REDUCTION;
double snd_str_reduction=SND_STR_REDUCTION;
double snd_udp_reduction=SND_UDP_REDUCTION;

struct lpWaves {
#ifdef SOUND_USESAMPLE_PATCH
	signed char *lpData;
#else
	short lpData[SAMPLE_NUMBER] __attribute__ ((__aligned__ (32)));
#endif
	unsigned int bits;
	unsigned int len; /* length of sound buffer */
	unsigned int freq; /* frequency of sound buffer */
	unsigned int active; /* 1 active, 0 inactive */
	unsigned int volume; /* volume 0 - 64 */
	unsigned int loop; /* loop 1 yes, 0 no */
}__attribute__ ((__aligned__ (32)));

struct lpWaves lpWave[16]; /* def for max num of voices */
unsigned int sound_enable = 0;
unsigned int soundcard;
unsigned int chn_max=0; /* Max Number of channels */

int sound_slice=default_sample_rate/60;

#if defined(GP2X) || defined(NOSOUND)
#define sound_real_sample_rate default_sample_rate
#define sound_real_sample_rate_min (default_sample_rate-10)
#define sound_real_sample_rate_max (default_sample_rate+10)
#define mame4all_init_sound_slice()
static short mixedAudio[default_sample_rate/30];
#else
extern short *mixedAudio;
int sound_real_sample_rate=default_sample_rate;
int sound_real_sample_rate_min=(default_sample_rate-10);
int sound_real_sample_rate_max=(default_sample_rate+10);
void mame4all_init_sound_slice();
#endif


static void gp2x_sound_mixer_channel_step_16bits (signed short *stream, int len, const struct lpWaves *wave, const int t)
{
	const int vol=wave->volume;
	unsigned int pos=wave->len;
	short *sample=(signed short *)wave->lpData;
	if (sample)
	do {
		*(stream++)+=((*sample++)*vol)/(t<<6); /* Update, scale and mix sample */
		if ((--pos)==0) { pos=wave->len; sample=(signed short *)wave->lpData; }
	} while (--len);
}

#ifdef SOUND_USESAMPLE_PATCH
static void gp2x_sound_mixer_channel_step_8bits (signed short *stream, int len, const struct lpWaves *wave, const int t)
{
	const int vol=wave->volume;
	unsigned int pos=wave->len;
	signed char *sample=(signed char *)wave->lpData;
	if (sample)
	do {
		*(stream++)+=((((signed short)(*sample++))<<8)*vol)/(t<<6); /* Update, scale and mix sample */
		if ((--pos)==0) { pos=wave->len; sample=(signed char *)wave->lpData; }
	} while (--len);
}
#endif

static void gp2x_sound_mixer_channel_nostep_16bits (signed short *stream, const int len, const struct lpWaves *wave, const int t)
{
	const int vol=wave->volume;
	unsigned int pos;
	const signed short *sample=(const signed short *)wave->lpData;
	unsigned int i=0;
	unsigned int freq=wave->freq;
	const unsigned int sample_len=wave->len;
	if (sample)
	do {
		pos = (i*freq)/sound_real_sample_rate; while(pos>=sample_len) pos-=sample_len;	
		*(stream++)+=((sample[pos])*vol)/(t<<6); /* Update, scale and mix sample */
	} while ((++i)<len);		
}

#ifdef SOUND_USESAMPLE_PATCH
static void gp2x_sound_mixer_channel_nostep_8bits (signed short *stream, const int len, const struct lpWaves *wave, const int t)
{
	const int vol=wave->volume;
	unsigned int pos;
	const signed char *sample=(const signed char *)wave->lpData;
	unsigned int i=0;
	unsigned int freq=wave->freq;
	const unsigned int sample_len=wave->len;
	if (sample)
	do {
		pos = (i*freq)/sound_real_sample_rate; while(pos>=sample_len) pos-=sample_len;	
		*(stream++)+=((((signed short)(sample[pos]))<<8)*vol)/(t<<6); /* Update, scale and mix sample */
	} while ((++i)<len);		
}
#endif

#ifdef SOUND_USESAMPLE_PATCH
typedef void (*gp2x_sound_mixer_channel_func)(short *, const int, const struct lpWaves *, const int);
static gp2x_sound_mixer_channel_func gp2x_sound_mixer_channel_step_array[2]=
{
	gp2x_sound_mixer_channel_step_8bits,
	gp2x_sound_mixer_channel_step_16bits
};
static gp2x_sound_mixer_channel_func gp2x_sound_mixer_channel_nostep_array[2]=
{
	gp2x_sound_mixer_channel_nostep_8bits,
	gp2x_sound_mixer_channel_nostep_16bits
};
#define gp2x_sound_mixer_channel_step(STR,LEN,WAV,T,BITS) gp2x_sound_mixer_channel_step_array[(BITS)>>4](STR,LEN,WAV,T)
#define gp2x_sound_mixer_channel_nostep(STR,LEN,WAV,T,BITS) gp2x_sound_mixer_channel_nostep_array[(BITS)>>4](STR,LEN,WAV,T)
#else
#define gp2x_sound_mixer_channel_step(STR,LEN,WAV,T,BITS) gp2x_sound_mixer_channel_step_16bits(STR,LEN,WAV,T)
#define gp2x_sound_mixer_channel_nostep(STR,LEN,WAV,T,BITS) gp2x_sound_mixer_channel_nostep_16bits(STR,LEN,WAV,T)
#endif

INLINE void gp2x_sound_frame ( short *stream, const int len )
{
	struct lpWaves *wave = lpWave;
	unsigned int n = num_voices, t = 0;

	/* Buffer Set to Zero */
#ifndef DREAMCAST
	fast_memset(stream,0,len<<1);
#else
	{
		unsigned *s=(unsigned *)stream;
		int n=len>>4;
		while(n--)
		{
			*s++=0;
			*s++=0;
			*s++=0;
			*s++=0;
			*s++=0;
			*s++=0;
			*s++=0;
			*s++=0;
		}
	}
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
			if ( ((wave->freq)>(sound_real_sample_rate_min)) && ((wave->freq)<(sound_real_sample_rate_max)) ) 
				gp2x_sound_mixer_channel_step (stream,len,wave,t,wave->bits);
			else
				gp2x_sound_mixer_channel_nostep (stream,len,wave,t,wave->bits);
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
	num_voices = NUMVOICES;
	
	sound_slice=(default_sample_rate/(Machine->drv->frames_per_second));
	mame4all_init_sound_slice();

	/* Initialize Sound Buffer Structure */
	for (i = 0; i < num_voices; i++)
	{
#ifdef SOUND_USESAMPLE_PATCH
		lpWave[i].lpData = 0;
#endif
		lpWave[i].bits = 0;
		lpWave[i].len = 0;
		lpWave[i].freq = 0;
		lpWave[i].active = 0;
		lpWave[i].volume = 0;
		lpWave[i].loop = 0;
	}
	chn_max=0;

#ifndef NOSOUND
	if (!soundcard)     /* Sound Deactivated and not emulated */
	{
#endif
#ifdef GP2X
		gp2x_text_log("GP2X Not Emulated Sound Init...");
#endif
		/* update the Machine structure to show that sound is disabled */
		Machine->sample_rate = 0;
		return 0;
#ifndef NOSOUND
	}

	if (soundcard == 2) /* Sound Deactivated but emulated */
	{
#ifdef GP2X
		gp2x_text_log("GP2X No Sound Init...");
#endif
		soundcard = 0;
		return 0;
	}

	if (soundcard == 3) /* Accurate Sound */
	{
#ifdef GP2X
		gp2x_text_log("GP2X Accurate Sound Init...");
#endif
		snd_opm_reduction=1.0;
		snd_pcm_reduction=1.0;
		snd_str_reduction=1.0;
		snd_udp_reduction=1.0;
	}
	else
	{
		float r=1.0;
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
#ifndef NO_DRIVER_SYSTEM16
		if (is_system16_driver(Machine->gamedrv))
			r=0.85;
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
	sound_slice=44100/Machine->drv->frames_per_second;
	sound_slice&=1023; /* to limit high slices numbers, lock up of hires games with low fps */
	mame4all_init_sound_slice();
#endif
	sound_enable = 1;

	/* update the Machine structure to reflect the actual sample rate */
	Machine->sample_rate = default_sample_rate;

	return 0;
#endif
}

void msdos_shutdown_sound(void)
{
	int i;

	/* stop and release voices */
	sound_enable = 0;
	
	for (i = 0; i < num_voices; i++)
	{
#ifdef SOUND_USESAMPLE_PATCH
		lpWave[i].lpData = 0;
#endif
		lpWave[i].len = 0;
		lpWave[i].freq = 0;
		lpWave[i].active = 0;
		lpWave[i].volume = 0;
		lpWave[i].loop = 0;
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
	if (!soundcard || channel >= num_voices) return;

	lpWave[channel].active=0;

	if (volume < 0) {
		volume = 0;
	} else if (volume > 0) {
		if (volume > 100) volume = volume * 25 / 255;
		volume = (volume<<6) / 100;
		if (pan != OSD_PAN_CENTER) volume>>=1;
		if (soundcard=1) volume+=20;
				if (volume>64) volume=64;
	
	}
	
#ifdef SOUND_USESAMPLE_PATCH
	if (len < 0) len=0;
	else if (bits==16) len>>=1;
	lpWave[channel].lpData = data;
#else
	if (len <= 0) {
		len = 0;
	} else if (len > 0) {
		if (bits==16) {
			len>>=1;
			if (len>SAMPLE_NUMBER) len=SAMPLE_NUMBER; /* Maximum number of samples */
#ifndef DREAMCAST
			fast_memcpy(lpWave[channel].lpData,data,len<<1);
#else
			sq_cpy(lpWave[channel].lpData,(void *)(((unsigned)data)&0xFFFFFFFC),len<<1);
#endif
		} else {
			if (len>SAMPLE_NUMBER) len=SAMPLE_NUMBER; /* Maximum number of samples */
			int i;
			for (i=0;i<len;i++)
				lpWave[channel].lpData[i]= data[i]<<8;
		}
	}
#endif

	if (freq < 0)
		freq = 0;
	
	lpWave[channel].bits = bits;	
	lpWave[channel].volume=volume;
	lpWave[channel].len=len;
	lpWave[channel].freq=freq;
	lpWave[channel].loop=loop;
	lpWave[channel].active=((freq>0) && (len>0));
#endif
}


void osd_adjust_sample(int channel,int freq,int volume)
{
#ifndef NOSOUND
	
	if (!soundcard || channel >= num_voices) return;

	if (freq >= 0)
		lpWave[channel].freq=freq;
	if (volume >= 0) {
		if (volume > 0) {
			if (volume > 100) volume = volume * 25 / 255;
			volume = (volume<<6) / 100;
			if (soundcard==1) volume+=20;
			if (volume>64) volume=64;
		}
		lpWave[channel].volume = volume;
	}
#endif
}


void osd_stop_sample(int channel)
{
#ifndef NOSOUND
	if (!soundcard || channel >= num_voices) return;
	lpWave[channel].active = 0;
#endif
}


void osd_restart_sample(int channel)
{
#ifndef NOSOUND
	if (!soundcard || channel >= num_voices) return;
	lpWave[channel].active = 1;
#endif
}


int osd_get_sample_status(int channel)
{
#ifndef NOSOUND
	if (!soundcard || channel >= num_voices) return -1;
	return !(lpWave[channel].active);
#else
	return -1;
#endif
}


void osd_sound_enable(int enable_it)
{ 	
	sound_enable = enable_it;
}
