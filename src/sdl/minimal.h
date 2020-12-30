/* 
  SDL wrapper by Chui. Preliminar version.
*/

/*

  GP2X minimal library v0.A by rlyeh, (c) 2005. emulnation.info@rlyeh (swap it!)

  Thanks to Squidge, Robster, snaff, Reesy and NK, for the help & previous work! :-)

  License
  =======

  Free for non-commercial projects (it would be nice receiving a mail from you).
  Other cases, ask me first.

  GamePark Holdings is not allowed to use this library and/or use parts from it.

*/

#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>

#ifndef __MINIMAL_H__
#define __MINIMAL_H__

#if defined(__cplusplus) && !defined(USE_CPLUS)
extern "C" {
#endif

#define STATIC_INLINE_DREAMCAST static __inline__

#ifndef NO_FASTMEM
void* fast_memcpy(void *OUT, const void *IN, size_t N);
void* fast_memset(const void *DST, int C, size_t LENGTH);
#else
#define fast_memcpy memcpy
#define fast_memset memset
#endif

#ifdef PSP
#define random() rand()
#endif

#ifdef PSP_RES
#define GP2X_BYTE_SIZE_SCREEN (480*272)
#else
#define GP2X_BYTE_SIZE_SCREEN (320*240)
#endif

#if defined(DREAMCAST) && !defined(SOUND_THREADED)
#define AUDIO_BUFFER_SIZE (384*4)
#else
#define AUDIO_BUFFER_SIZE (1024*2)
#endif


enum  { 
	GP2X_UP=0x1,       GP2X_LEFT=0x4,       GP2X_DOWN=0x10,  GP2X_RIGHT=0x40,
        GP2X_START=1<<8,   GP2X_SELECT=1<<9,    GP2X_L=1<<10,    GP2X_R=1<<11,
        GP2X_A=1<<12,      GP2X_B=1<<13,        GP2X_X=1<<14,    GP2X_Y=1<<15,
        GP2X_VOL_UP=1<<23, GP2X_VOL_DOWN=1<<22, GP2X_PUSH=1<<27,
	GP2X_JOY1 = 1<<28,
	GP2X_JOY2 = 1<<29,
	GP2X_JOY3 = 1<<30,
	GP2X_JOY4 = 1<<31
};
                                            
                                            
#ifndef GP2X_SDLWRAPPER_NODOUBLEBUFFER
extern SDL_Color	gp2x_sdlwrapper_palette[256];
#define gp2x_video_color8(C,R,G,B) (gp2x_sdlwrapper_palette[(C)].r=(R), gp2x_sdlwrapper_palette[(C)].g=(G), gp2x_sdlwrapper_palette[(C)].b=(B))
#else
extern short		gp2x_sdlwrapper_palette[256];
extern int		gp2x_sdlwrapper_is_single;
#ifdef DREAMCAST
#define gp2x_video_color8(C,R,G,B) (gp2x_sdlwrapper_palette[(C)]= (((R)<<8)&0xf800)|(((G)<<3)&0x07e0 )|(((B)>>3)&0x001f) )
#else
#define gp2x_video_color8(C,R,G,B) (gp2x_sdlwrapper_palette[(C)]=SDL_MapRGB(gp2x_sdlwrapper_screen->format,(R),(G),(B)))
#endif
#endif
#define gp2x_video_color15(R,G,B,A) SDL_MapRGBA(gp2x_sdlwrapper_screen->format,(R),(G),(B),(A))

extern void		*gp2x_sdlwrapper_screen_pixels;
extern SDL_Surface	*gp2x_sdlwrapper_screen;
extern float		gp2x_sdlwrapper_ticksdivisor;
#define gp2x_screen15	((unsigned short *)gp2x_sdlwrapper_screen_pixels)
#define gp2x_screen8	((unsigned char *)gp2x_sdlwrapper_screen_pixels)

extern void gp2x_init(int tickspersecond, int bpp, int rate, int bits, int stereo, int hz);
extern void gp2x_deinit(void);
extern void gp2x_sound_play(void *buff, int len);
#ifndef GP2X_SDLWRAPPER_NODOUBLEBUFFER
extern void gp2x_video_setpalette(void);
#else
#define gp2x_video_setpalette()
#endif

extern unsigned long gp2x_joystick_read_n(int njoy);
#define gp2x_joystick_read() gp2x_joystick_read_n(1)

#ifndef GP2X_SDLWRAPPER_NOMSEC
#define gp2x_timer_delay(T) SDL_Delay(T)
#define gp2x_timer_read() SDL_GetTicks()
#else
#define gp2x_timer_delay(T) SDL_Delay((Uint32)(((float)(T))*gp2x_sdlwrapper_ticksdivisor))
#define gp2x_timer_read() (unsigned long) (((float)SDL_GetTicks())/gp2x_sdlwrapper_ticksdivisor)
#endif

#ifndef GP2X_SDLWRAPPER_NODOUBLEBUFFER
#define gp2x_video_flip() \
{ \
	SDL_Flip(gp2x_sdlwrapper_screen); \
	gp2x_sdlwrapper_screen_pixels=(void *)gp2x_sdlwrapper_screen->pixels; \
}
#define gp2x_video_flip_single() gp2x_video_flip()
#else
extern void gp2x_video_flip(void);
extern void gp2x_video_flip_single(void);
#endif

#define gp2x_sound_volume(LEFT,RIGHT)
//#define SetVideoScaling(PIX,W,H) puts("\nSetVideoScaling not supported yet!\n")
#define SetVideoScaling(PIX,W,H)
//#define SetGP2XClock(MHz)
void SetGP2XClock(int mhz);
#define gp2x_malloc_init()

#ifndef DREAMCAST
void *gp2x_malloc(size_t size);
void gp2x_free(void *ptr);
#ifdef DEBUG_MEMALLOC_REPORT
void _memalloc_report_(char *msg);
void _memalloc_report_full(char *msg);
#define memalloc_report(MSG) _memalloc_report_(MSG)
#define memalloc_report_full(MSG) _memalloc_report_full(MSG)
#else
#define memalloc_report(MSG) 
#endif
#else
#define memalloc_report(MSG)
#define gp2x_malloc(SZ) malloc(SZ)
#ifdef DEBUG_MEMALLOC_REPORT
#define PRINT_DEBUG_MEMALLOC_ALERT printf("PELIGRO free(%p)\n",(PTR));
#else
#define PRINT_DEBUG_MEMALLOC_ALERT 
#endif
#define gp2x_free(PTR) { \
	if (((unsigned)(PTR))>0x8c000000 && ((unsigned)(PTR))<0x8d000000) \
		free(PTR); \
	PRINT_DEBUG_MEMALLOC_ALERT \
}
#endif

#if defined (_WINDOWS) || defined(DREAMCAST)
#define random() rand()
void dc_change_to_normal(void);
void dc_change_to_hw_scaled(void);
#endif

#if defined(__cplusplus) && !defined(USE_CPLUS)
}
#endif
#endif
