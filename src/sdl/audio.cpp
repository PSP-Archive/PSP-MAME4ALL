#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<SDL.h>
#include<SDL_mixer.h>
#ifdef SOUND_THREADED
#include<SDL_thread.h>
#endif
#ifdef DREAMCAST
#include<SDL_dreamcast.h>
#endif

#include "driver.h"
#include "minimal.h"

#ifdef DREAMCAST
#ifdef MAME4ALL_CLASSIC
#define MUSIC_FILENAME "classic.mod"
#else
#ifdef MAME4ALL_GOLD
#define MUSIC_FILENAME "gold.mod"
#else
#define MUSIC_FILENAME "aged.mod"
#endif
#endif
#else
#ifdef PSP
#define MUSIC_FILENAME "music.psp"
#else
#define MUSIC_FILENAME "music.mod"
#endif
#endif

extern int soundcard;
extern int gp2x_sound_enable;

#define AUDIO_BUFFER_SIZE_2 (AUDIO_BUFFER_SIZE/2)
#define REAL_AUDIO_BUFFER_ALL (AUDIO_BUFFER_SIZE_2*4)+2048

static unsigned short sndbuffer_all[REAL_AUDIO_BUFFER_ALL] __attribute__ ((__aligned__ (32)));
static unsigned short *sndbuffer[4] __attribute__ ((__aligned__ (32))) = {
	&sndbuffer_all[AUDIO_BUFFER_SIZE_2*0], &sndbuffer_all[AUDIO_BUFFER_SIZE_2*1],
	&sndbuffer_all[AUDIO_BUFFER_SIZE_2*2], &sndbuffer_all[AUDIO_BUFFER_SIZE_2*3],
};

#ifdef DEBUG_SOUND
unsigned sound_cuantos[4]={ 0,0,0,0 };
unsigned sound_ajustes=0;
unsigned sound_alcanza_callback=0;
unsigned sound_recupera_callback=0;
unsigned sound_alcanza_render=0;
#endif

#ifndef NOSOUND
unsigned n_callback_sndbuff=0, n_render_sndbuff=2;
unsigned short *callback_sndbuff, *render_sndbuff, *mixedAudio;
#ifdef SOUND_THREADED
static SDL_sem *data_available_sem, *callback_sem;
#endif
int closing_sound=1;

extern int sound_slice;
extern int sound_real_sample_rate;
extern int sound_real_sample_rate_min;
extern int sound_real_sample_rate_max;
static int sound_slice_min=1024;
static int sound_slice_max=2048;

static void adjust_sound_slice(int offs)
{
	sound_slice+=offs;
	sound_real_sample_rate = (sound_slice * Machine->drv->frames_per_second );
	sound_real_sample_rate_min=sound_real_sample_rate-16;
	sound_real_sample_rate_max=sound_real_sample_rate+16;
}

void mame4all_init_sound_slice(void)
{
#ifndef SOUND_THREADED
	if (sound_slice&31)
		sound_slice+=32-(sound_slice&31);
//	sound_slice+=32;
#endif
	sound_slice_min=sound_slice-32; //(sound_slice>>2);
	sound_slice_max=sound_slice+32; //(sound_slice>>2);
	adjust_sound_slice(0);
}

void sound_callback (void *userdata, Uint8 *stream, int len)
{
	if (!closing_sound)
	{
#ifdef SOUND_THREADED
		while ((!closing_sound) && (((unsigned)render_sndbuff)==((unsigned)callback_sndbuff)))
			SDL_SemWait(data_available_sem);
#else
		unsigned new_n_callback_sndbuff=(n_callback_sndbuff+1)&3;

		while(new_n_callback_sndbuff==n_render_sndbuff)
		{
#ifdef DEBUG_SOUND
			sound_recupera_callback++;
#endif
/*
			if (sound_slice<=sound_slice_max)
				adjust_sound_slice(16);
*/
			osd_update_audio();
		}
		n_callback_sndbuff=new_n_callback_sndbuff;

		callback_sndbuff=sndbuffer[n_callback_sndbuff];
#if !defined(PROFILER_MAME4ALL) && defined (DEBUG_SOUND)
		if (n_callback_sndbuff==n_render_sndbuff)
		{
			n_render_sndbuff=2;
			mixedAudio=render_sndbuff=sndbuffer[3];
			n_callback_sndbuff=0;
			callback_sndbuff=sndbuffer[0];
			sound_alcanza_callback++;
		}
#endif
#endif
#ifdef SOUND_THREADED
		if (((unsigned)render_sndbuff)!=((unsigned)callback_sndbuff))
#endif
		{
#ifndef DREAMCAST
			fast_memcpy(stream,callback_sndbuff,len);
#else
			SDL_DC_SetSoundBuffer(callback_sndbuff);
#endif
#ifdef SOUND_THREADED
			callback_sndbuff=render_sndbuff;
			SDL_SemPost(callback_sem);
#endif
		}
	}
}
#endif


void init_sound(unsigned rate, int stereo, int bits)
{
#ifdef NOSOUND
	soundcard=gp2x_sound_enable=0;
#else
	int retries=0;
#ifdef SOUND_THREADED
	data_available_sem=SDL_CreateSemaphore(0);
	callback_sem=SDL_CreateSemaphore(0);
#endif

#ifndef DREAMCAST
	for(retries=0;retries<100;retries++)
	{
		if (!
#endif
			Mix_OpenAudio(rate,(bits==16)?AUDIO_S16:AUDIO_U8,(stereo)?2:1,AUDIO_BUFFER_SIZE>>(((bits>>3)-1)+(stereo?1:0)))
#ifndef DREAMCAST
		)
			break;
		SDL_Delay(100);
	}
#else
	;
#endif
	Mix_PlayMusic(Mix_LoadMUS(DATA_PREFIX MUSIC_FILENAME),-1);
	Mix_VolumeMusic(80);
	n_render_sndbuff=2;
	mixedAudio=render_sndbuff=sndbuffer[2];
	n_callback_sndbuff=0;
	callback_sndbuff=sndbuffer[0];
	soundcard=gp2x_sound_enable=1;
#endif
}

void quit_sound(void)
{
#ifndef NOSOUND
	closing_sound=1;
#ifdef SOUND_THREADED
	SDL_SemPost(data_available_sem);
#endif
	SDL_PauseAudio(1);
	SDL_Delay(100);
#ifdef SOUND_THREADED
	SDL_SemPost(data_available_sem);
#endif
	SDL_CloseAudio();
#ifdef SOUND_THREADED
	SDL_SemPost(callback_sem);
#endif
	SDL_Delay(10);
#endif
	Mix_CloseAudio();
}

void osd_stop_music(void)
{
#ifndef NOSOUND
	Mix_VolumeMusic(0);
	SDL_Delay(333);
//	closing_sound=0;
	Mix_HookMusic(&sound_callback,NULL);
#endif
}

void osd_start_music(void)
{
#ifndef NOSOUND
	closing_sound=1;
#ifdef SOUND_THREADED
	SDL_SemPost(data_available_sem);
#endif
	SDL_Delay(100);
	Mix_HookMusic(NULL,NULL);
	Mix_VolumeMusic(80);
#endif
}

void osd_mute_sound(void)
{
#ifndef NOSOUND
	closing_sound=1;
	fast_memset(sndbuffer_all,0,REAL_AUDIO_BUFFER_ALL);
#endif
}

void osd_unmute_sound(void)
{
#ifndef NOSOUND
	closing_sound=0;
#endif
}

void gp2x_sound_play(void *buff, int len)
{
#ifndef NOSOUND
	mixedAudio+=len>>1;
	unsigned diff=(unsigned)mixedAudio-(unsigned)render_sndbuff;
	if (diff<AUDIO_BUFFER_SIZE)
		return;

	diff-=AUDIO_BUFFER_SIZE;
#ifndef SOUND_THREADED
	if (n_render_sndbuff==3)
	{
		short *offset_mixedAudio=(short *)(((unsigned)render_sndbuff)+AUDIO_BUFFER_SIZE);
#ifndef DREAMCAST
		fast_memcpy(sndbuffer[0],offset_mixedAudio,diff);
#else
		{
			register unsigned *s=(unsigned *)offset_mixedAudio;
			register unsigned *d=(unsigned *)sndbuffer[0];
			register int n=diff>>5;
			while(n--)
			{
				asm("pref @%0" : : "r" (s + 8));
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
			}
		}
#endif
	}
#endif


#ifndef SOUND_THREADED
	n_render_sndbuff=(n_render_sndbuff+1)&3;
	render_sndbuff=sndbuffer[n_render_sndbuff];
#if !defined(PROFILER_MAME4ALL) && defined(DEBUG_SOUND)
	if (n_callback_sndbuff==n_render_sndbuff)
	{
		n_render_sndbuff=0;
		render_sndbuff=sndbuffer[0];
		n_callback_sndbuff=2;
		callback_sndbuff=sndbuffer[2];
		sound_alcanza_render++;
/*
		if (sound_slice>=sound_slice_min)
			adjust_sound_slice(-16);
*/
	}
#endif

#ifdef DEBUG_SOUND
	sound_cuantos[(n_callback_sndbuff-n_render_sndbuff)&3]++;
	sound_ajustes++;
#endif

#endif

	closing_sound=0;
#ifdef SOUND_THREADED
	SDL_SemPost(data_available_sem);
	while ((!closing_sound) && (callback_sndbuff!=render_sndbuff))
		SDL_SemWait(callback_sem);
	if (render_sndbuff==sndbuffer[0])
	{
		fast_memcpy(sndbuffer[2],sndbuffer[1],diff);
		render_sndbuff=sndbuffer[2];
	}
	else
	{
		fast_memcpy(sndbuffer[0],sndbuffer[3],diff);
		render_sndbuff=sndbuffer[0];
	}
#endif
	mixedAudio=(unsigned short *)(((unsigned)render_sndbuff)+diff);
#endif
}

