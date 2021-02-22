#ifdef DREAMCAST
#include <kos.h>
#endif

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<SDL.h>
#include<SDL_thread.h>
#include<SDL_mixer.h>

/*
 * #ifdef SDL_ROTOZOOM
 * #include<SDL_rotozoom.h>
 * #include<SDL_image.h>
 * #endif
 */

#ifdef DREAMCAST
#include<SDL_dreamcast.h>
#endif

#include "driver.h"
#include "gp2x_mame.h"
#include "minimal.h"
#include "cpuctrl.h"

#include "prof.h"

#ifdef DREAMCAST
extern void *gp2x_sdlwrapper_screens[2];
extern int gp2x_sdlwrapper_actual_screen;
#endif

#ifdef AUTOEVENTS
extern int 	mame4all_emulating;
#endif

extern int gp2x_sdlwrapper_bpp;
extern int closing_sound;
extern struct osd_bitmap *scrbitmap;
extern int gp2x_xscreen;
extern int gp2x_yscreen;
extern int gp2x_xoffset;
extern int gp2x_yoffset;
extern int gp2x_xmax;
extern int gp2x_ymax;
extern int gp2x_width;
extern int gp2x_height;
extern int gp2x_vsync;
extern int visible_game_width;
extern int visible_game_height;
extern int visible_game_width_offset;
extern int visible_game_height_offset;


#ifdef DEBUG_FRAMERATE
float mame4all_real_framerate=0.0;
unsigned mame4all_real_totalframes=0;
static __inline__ void calculate_real_framerate(void)
{
	static unsigned start_time=0;
	static unsigned start_numframes=0;

	mame4all_real_totalframes++;
	if (!start_time)
	{
		start_time=SDL_GetTicks();
		start_numframes=mame4all_real_totalframes;
	}
	else
	{
		unsigned now=SDL_GetTicks();
		if (now-start_time>=1000)
		{
			if (mame4all_real_framerate!=0.0)
				mame4all_real_framerate=(mame4all_real_framerate+((float)(mame4all_real_totalframes-start_numframes)))/2.0;
			else
				mame4all_real_framerate=(float)(mame4all_real_totalframes-start_numframes);
			start_time=now;
			start_numframes=mame4all_real_totalframes;
		}
	}
}
void print_frameskip(void);
#else
#define calculate_real_framerate()
#define print_frameskip()
#endif

#ifdef DREAMCAST
int dc_needs_video_reinit=0;
static void *change_vid_start=NULL;
static uint32 mame4all_vsync_handler_id=0;
static void mame4all_vsync_handler(uint32 code)
{
	if (change_vid_start)
	{
#ifdef GP2X_SDLWRAPPER_NODOUBLEBUFFER
		if (change_vid_start!=(void*)1)
			vid_set_start((uint32)change_vid_start);
		else
#endif
			SDL_Flip(gp2x_sdlwrapper_screen);
		change_vid_start=NULL;
	}
}

void dc_change_to_normal(void)
{
	SDL_Surface *f=gp2x_sdlwrapper_screen;
	SDL_DC_VerticalWait(SDL_FALSE);
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	pvr_shutdown();  vid_shutdown();
#ifdef GP2X_SDLWRAPPER_NODOUBLEBUFFER
	SDL_DC_SetVideoDriver(SDL_DC_DIRECT_VIDEO);
	SDL_InitSubSystem(SDL_INIT_VIDEO);
	gp2x_sdlwrapper_screen=SDL_SetVideoMode(320,240,16,SDL_HWSURFACE|SDL_FULLSCREEN);
#else
	SDL_DC_SetVideoDriver(SDL_DC_DMA_VIDEO);
	SDL_InitSubSystem(SDL_INIT_VIDEO);
	gp2x_sdlwrapper_screen=SDL_SetVideoMode(320,240,16,SDL_HWSURFACE|SDL_FULLSCREEN|SDL_DOUBLEBUF);
#endif
	SDL_DC_SetWindow(320,240);
	SDL_JoystickEventState(SDL_ENABLE);
	SDL_JoystickOpen(0);
	SDL_JoystickOpen(1);
	SDL_JoystickOpen(2);
	SDL_JoystickOpen(3);
	SDL_DC_EmulateMouse(SDL_FALSE);
	SDL_DC_EmulateKeyboard(SDL_TRUE);
	SDL_EventState(SDL_ACTIVEEVENT,SDL_IGNORE);
	SDL_EventState(SDL_MOUSEMOTION,SDL_IGNORE);
	SDL_EventState(SDL_MOUSEBUTTONDOWN,SDL_IGNORE);
	SDL_EventState(SDL_MOUSEBUTTONUP,SDL_IGNORE);
	SDL_EventState(SDL_SYSWMEVENT,SDL_IGNORE);
	SDL_EventState(SDL_VIDEORESIZE,SDL_IGNORE);
	SDL_EventState(SDL_USEREVENT,SDL_IGNORE);
	SDL_ShowCursor(SDL_DISABLE);
	SDL_FillRect(gp2x_sdlwrapper_screen,NULL,0);
	SDL_Flip(gp2x_sdlwrapper_screen);
	SDL_DC_VerticalWait(SDL_FALSE);
//	SDL_FreeSurface(f);
	dc_needs_video_reinit=0;
}

void dc_change_to_hw_scaled(void)
{
	int i;
	pvr_init_params_t dummy_params={{0,0,0,0,0 },4*1024,1,1};
	SDL_DC_VerticalWait(gp2x_vsync?SDL_TRUE:SDL_FALSE);
	SDL_Surface *f=gp2x_sdlwrapper_screen;
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	vid_shutdown(); pvr_shutdown();
	pvr_init(&dummy_params);
	for(i=0;i<4;i++)
	{
		pvr_wait_ready();
		pvr_scene_begin();
		pvr_list_begin(PVR_LIST_OP_POLY);
		pvr_list_finish();
		pvr_scene_finish();
	}
	pvr_shutdown();
	SDL_DC_VerticalWait(gp2x_vsync?SDL_TRUE:SDL_FALSE);
	SDL_DC_SetVideoDriver(SDL_DC_TEXTURED_VIDEO);
	SDL_InitSubSystem(SDL_INIT_VIDEO);
	gp2x_sdlwrapper_screen=SDL_SetVideoMode(512,512,16,SDL_HWSURFACE);
	SDL_DC_SetWindow(gp2x_xmax,gp2x_ymax);
	SDL_JoystickEventState(SDL_ENABLE);
	SDL_JoystickOpen(0);
	SDL_JoystickOpen(1);
	SDL_JoystickOpen(2);
	SDL_JoystickOpen(3);
	SDL_DC_EmulateMouse(SDL_FALSE);
	SDL_DC_EmulateKeyboard(SDL_TRUE);
	SDL_EventState(SDL_ACTIVEEVENT,SDL_IGNORE);
	SDL_EventState(SDL_MOUSEMOTION,SDL_IGNORE);
	SDL_EventState(SDL_MOUSEBUTTONDOWN,SDL_IGNORE);
	SDL_EventState(SDL_MOUSEBUTTONUP,SDL_IGNORE);
	SDL_EventState(SDL_SYSWMEVENT,SDL_IGNORE);
	SDL_EventState(SDL_VIDEORESIZE,SDL_IGNORE);
	SDL_EventState(SDL_USEREVENT,SDL_IGNORE);
	SDL_ShowCursor(SDL_DISABLE);
	SDL_FillRect(gp2x_sdlwrapper_screen,NULL,0);
	SDL_DC_VerticalWait(gp2x_vsync?SDL_TRUE:SDL_FALSE);
	SDL_Flip(gp2x_sdlwrapper_screen);
//	SDL_FreeSurface(f);
	dc_needs_video_reinit=1;
}

static void update_display_8to16_hw_scaled(void)
{
#ifdef AUTOEVENTS
	mame4all_emulating=1;
#endif
	calculate_real_framerate();
	mame4all_prof_start(1);

	if (!dc_needs_video_reinit)
		dc_change_to_hw_scaled();
	change_vid_start=NULL;

	Uint32 tmp=(Uint32)(((unsigned)gp2x_sdlwrapper_screen->pixels)+((gp2x_xscreen+(256*gp2x_yscreen))<<1));
	register Uint8 *s = (Uint8 *)(scrbitmap->line[gp2x_yoffset]+gp2x_xoffset);
	register Uint32 *d = (Uint32 *)(void *)
		(0xe0000000 | ( tmp & 0x03ffffe0));

	QACR0 = (((tmp)>>26)<<2)&0x1c;
	QACR1 = (((tmp)>>26)<<2)&0x1c;

	register int max_x=gp2x_width, max_y=gp2x_height;
	register int offset_src = (((int)(scrbitmap->line[1] - scrbitmap->line[0])-max_x));
	register int offset_dst = (512 - max_x)>>1;
	register int i, j;
	for(j=0;j<max_y;j++)
	{
		for(i=0;i<max_x;i+=16)
		{
			asm("pref @%0" : : "r" (s + 32));

			register Uint32 o;

			o=((Uint32)gp2x_sdlwrapper_palette[*s++])&0x0000ffff;
			o|=(((Uint32)gp2x_sdlwrapper_palette[*s++])<<16);
			d[0]=o;

			o=((Uint32)gp2x_sdlwrapper_palette[*s++])&0x0000ffff;
			o|=(((Uint32)gp2x_sdlwrapper_palette[*s++])<<16);
			d[1]=o;

			o=((Uint32)gp2x_sdlwrapper_palette[*s++])&0x0000ffff;
			o|=(((Uint32)gp2x_sdlwrapper_palette[*s++])<<16);
			d[2]=o;

			o=((Uint32)gp2x_sdlwrapper_palette[*s++])&0x0000ffff;
			o|=(((Uint32)gp2x_sdlwrapper_palette[*s++])<<16);
			d[3]=o;

			o=((Uint32)gp2x_sdlwrapper_palette[*s++])&0x0000ffff;
			o|=(((Uint32)gp2x_sdlwrapper_palette[*s++])<<16);
			d[4]=o;

			o=((Uint32)gp2x_sdlwrapper_palette[*s++])&0x0000ffff;
			o|=(((Uint32)gp2x_sdlwrapper_palette[*s++])<<16);
			d[5]=o;

			o=((Uint32)gp2x_sdlwrapper_palette[*s++])&0x0000ffff;
			o|=(((Uint32)gp2x_sdlwrapper_palette[*s++])<<16);
			d[6]=o;

			o=((Uint32)gp2x_sdlwrapper_palette[*s++])&0x0000ffff;
			o|=(((Uint32)gp2x_sdlwrapper_palette[*s++])<<16);
			d[7]=o;

			asm("pref @%0" : : "r" (d));
			d += 8;
		}
		s+=offset_src;
		d+=offset_dst;
	}
	d = (Uint32 *)0xe0000000;
	d[0] = d[8] = 0;

	SDL_Flip(gp2x_sdlwrapper_screen);
	gp2x_sdlwrapper_actual_screen=(gp2x_sdlwrapper_actual_screen+1)&1;
	mame4all_prof_end(1);
}
#endif

static void update_display_8to16(void)
{
#ifdef AUTOEVENTS
	mame4all_emulating=1;
#endif
	calculate_real_framerate();
	mame4all_prof_start(1);
#ifndef DREAMCAST
	SDL_LockSurface(gp2x_sdlwrapper_screen);
#ifdef PSP_RES	
	register Uint16 *dst= (Uint16 *)(((unsigned)gp2x_sdlwrapper_screen->pixels)+((gp2x_xscreen+(480*gp2x_yscreen))<<1));
#else
	register Uint16 *dst= (Uint16 *)(((unsigned)gp2x_sdlwrapper_screen->pixels)+((gp2x_xscreen+(320*gp2x_yscreen))<<1));
#endif
	register Uint8 *src= (Uint8 *)(scrbitmap->line[gp2x_yoffset]+gp2x_xoffset);
	register int offset_src = ((int)(scrbitmap->line[1] - scrbitmap->line[0])-gp2x_width);
	register int offset_dst = (gp2x_sdlwrapper_screen->pitch>>1) - gp2x_width;
	register int max_x=gp2x_width, max_y=gp2x_height;
	register int i, j;
	for(j=0;j<max_y;j++)
	{
		for(i=0;i<max_x;i++)
			*dst++ = gp2x_sdlwrapper_palette[*src++];
		src+=offset_src;
		dst+=offset_dst;
	}
	SDL_UnlockSurface(gp2x_sdlwrapper_screen);
	SDL_Flip(gp2x_sdlwrapper_screen);
	gp2x_joystick_read();
#else
	change_vid_start=NULL;
	Uint32 tmp=(Uint32)(((unsigned)gp2x_sdlwrapper_screen->pixels)+((gp2x_xscreen+(320*gp2x_yscreen))<<1));
	register Uint8 *s = (Uint8 *)(scrbitmap->line[gp2x_yoffset]+gp2x_xoffset);
	register Uint32 *d = (Uint32 *)(void *)
		(0xe0000000 | ( tmp & 0x03ffffe0));

	QACR0 = (((tmp)>>26)<<2)&0x1c;
	QACR1 = (((tmp)>>26)<<2)&0x1c;

	register int max_x=gp2x_width, max_y=gp2x_height;
	register int offset_src = (((int)(scrbitmap->line[1] - scrbitmap->line[0])-max_x));
	register int offset_dst = (gp2x_sdlwrapper_screen->w - max_x)>>1;
	register int i, j;
	for(j=0;j<max_y;j++)
	{
		for(i=0;i<max_x;i+=16)
		{
			asm("pref @%0" : : "r" (s + 32));

			register Uint32 o;

			o=((Uint32)gp2x_sdlwrapper_palette[*s++])&0x0000ffff;
			o|=(((Uint32)gp2x_sdlwrapper_palette[*s++])<<16);
			d[0]=o;

			o=((Uint32)gp2x_sdlwrapper_palette[*s++])&0x0000ffff;
			o|=(((Uint32)gp2x_sdlwrapper_palette[*s++])<<16);
			d[1]=o;

			o=((Uint32)gp2x_sdlwrapper_palette[*s++])&0x0000ffff;
			o|=(((Uint32)gp2x_sdlwrapper_palette[*s++])<<16);
			d[2]=o;

			o=((Uint32)gp2x_sdlwrapper_palette[*s++])&0x0000ffff;
			o|=(((Uint32)gp2x_sdlwrapper_palette[*s++])<<16);
			d[3]=o;

			o=((Uint32)gp2x_sdlwrapper_palette[*s++])&0x0000ffff;
			o|=(((Uint32)gp2x_sdlwrapper_palette[*s++])<<16);
			d[4]=o;

			o=((Uint32)gp2x_sdlwrapper_palette[*s++])&0x0000ffff;
			o|=(((Uint32)gp2x_sdlwrapper_palette[*s++])<<16);
			d[5]=o;

			o=((Uint32)gp2x_sdlwrapper_palette[*s++])&0x0000ffff;
			o|=(((Uint32)gp2x_sdlwrapper_palette[*s++])<<16);
			d[6]=o;

			o=((Uint32)gp2x_sdlwrapper_palette[*s++])&0x0000ffff;
			o|=(((Uint32)gp2x_sdlwrapper_palette[*s++])<<16);
			d[7]=o;

			asm("pref @%0" : : "r" (d));
			d += 8;
		}
		s+=offset_src;
		d+=offset_dst;
	}
	d = (Uint32 *)0xe0000000;
	d[0] = d[8] = 0;

	if (gp2x_vsync)
#ifdef GP2X_SDLWRAPPER_NODOUBLEBUFFER
		change_vid_start=gp2x_sdlwrapper_screen->pixels;
#else
		change_vid_start=(void *)1;
#endif
	else
#ifdef GP2X_SDLWRAPPER_NODOUBLEBUFFER
		vid_set_start((uint32)gp2x_sdlwrapper_screen->pixels);
#else
		SDL_Flip(gp2x_sdlwrapper_screen);
#endif
	gp2x_sdlwrapper_actual_screen=(gp2x_sdlwrapper_actual_screen+1)&1;
#ifdef GP2X_SDLWRAPPER_NODOUBLEBUFFER
	gp2x_sdlwrapper_screen->pixels=gp2x_sdlwrapper_screens[gp2x_sdlwrapper_actual_screen];
#endif
#endif
	mame4all_prof_end(1);
}

static void update_display_8to16_sw_scaled(void)
{
#ifdef AUTOEVENTS
	mame4all_emulating=1;
#endif
	calculate_real_framerate();
	mame4all_prof_start(1);
	int max_x=gp2x_xmax;
	int max_y=gp2x_ymax;
	unsigned dx=visible_game_width << 8;
	unsigned dy=visible_game_height << 8;
	dx/=max_x;
	dy/=max_y;
#ifndef DREAMCAST
	SDL_LockSurface(gp2x_sdlwrapper_screen);
#ifdef PSP_RES
	register Uint16 *dst= (Uint16 *)(((unsigned)gp2x_sdlwrapper_screen->pixels)+((gp2x_xscreen+(480*gp2x_yscreen))<<1));
#else
	register Uint16 *dst= (Uint16 *)(((unsigned)gp2x_sdlwrapper_screen->pixels)+((gp2x_xscreen+(320*gp2x_yscreen))<<1));
#endif
	register int offset_dst = (gp2x_sdlwrapper_screen->pitch>>1) - (max_x);
	register int i,j;
	unsigned oy=0;
	for(j=0;j<max_y;j++,oy+=dy)
	{
		unsigned ox=0;
		register Uint8 *src= (Uint8 *)(scrbitmap->line[gp2x_yoffset+(oy>>8)]+gp2x_xoffset);
		for(i=0;i<max_x;i++,ox+=dx)
		{
			*dst++ = gp2x_sdlwrapper_palette[*src];
			src+=ox>>8;
			ox&=0xFF;
		}
		dst+=offset_dst;
	}
	SDL_UnlockSurface(gp2x_sdlwrapper_screen);
	SDL_Flip(gp2x_sdlwrapper_screen);
	gp2x_joystick_read();
#else
	change_vid_start=NULL;
	Uint32 tmp=(Uint32)(((unsigned)gp2x_sdlwrapper_screen->pixels)+((gp2x_xscreen+(320*gp2x_yscreen))<<1));
	register Uint32 *d = (Uint32 *)(void *)
		(0xe0000000 | ( tmp & 0x03ffffe0));

	QACR0 = (((tmp)>>26)<<2)&0x1c;
	QACR1 = (((tmp)>>26)<<2)&0x1c;

	register int offset_dst = (gp2x_sdlwrapper_screen->w - max_x)>>1;
	register int i, j;
	unsigned oy=0;
	for(j=0;j<max_y;j++,oy+=dy)
	{
		unsigned ox=0;
		register Uint8 *s = (Uint8 *)(scrbitmap->line[gp2x_yoffset+(oy>>8)]+gp2x_xoffset);
		for(i=0;i<max_x;i+=16)
		{
			asm("pref @%0" : : "r" (s + 32));

			register Uint32 o;

			o=((Uint32)gp2x_sdlwrapper_palette[*s])&0x0000ffff;
			s+=ox>>8; ox&=0xFF; ox+=dx;
			o|=(((Uint32)gp2x_sdlwrapper_palette[*s])<<16);
			s+=ox>>8; ox&=0xFF; ox+=dx;
			d[0]=o; 

			o=((Uint32)gp2x_sdlwrapper_palette[*s])&0x0000ffff;
			s+=ox>>8; ox&=0xFF; ox+=dx;
			o|=(((Uint32)gp2x_sdlwrapper_palette[*s])<<16);
			s+=ox>>8; ox&=0xFF; ox+=dx;
			d[1]=o;

			o=((Uint32)gp2x_sdlwrapper_palette[*s])&0x0000ffff;
			s+=ox>>8; ox&=0xFF; ox+=dx;
			o|=(((Uint32)gp2x_sdlwrapper_palette[*s])<<16);
			s+=ox>>8; ox&=0xFF; ox+=dx;
			d[2]=o;

			o=((Uint32)gp2x_sdlwrapper_palette[*s])&0x0000ffff;
			s+=ox>>8; ox&=0xFF; ox+=dx;
			o|=(((Uint32)gp2x_sdlwrapper_palette[*s])<<16);
			s+=ox>>8; ox&=0xFF; ox+=dx;
			d[3]=o;

			o=((Uint32)gp2x_sdlwrapper_palette[*s])&0x0000ffff;
			s+=ox>>8; ox&=0xFF; ox+=dx;
			o|=(((Uint32)gp2x_sdlwrapper_palette[*s])<<16);
			s+=ox>>8; ox&=0xFF; ox+=dx;
			d[4]=o;

			o=((Uint32)gp2x_sdlwrapper_palette[*s])&0x0000ffff;
			s+=ox>>8; ox&=0xFF; ox+=dx;
			o|=(((Uint32)gp2x_sdlwrapper_palette[*s])<<16);
			s+=ox>>8; ox&=0xFF; ox+=dx;
			d[5]=o;

			o=((Uint32)gp2x_sdlwrapper_palette[*s])&0x0000ffff;
			s+=ox>>8; ox&=0xFF; ox+=dx;
			o|=(((Uint32)gp2x_sdlwrapper_palette[*s])<<16);
			s+=ox>>8; ox&=0xFF; ox+=dx;
			d[6]=o;

			o=((Uint32)gp2x_sdlwrapper_palette[*s])&0x0000ffff;
			s+=ox>>8; ox&=0xFF; ox+=dx;
			o|=(((Uint32)gp2x_sdlwrapper_palette[*s])<<16);
			s+=ox>>8; ox&=0xFF; ox+=dx;
			d[7]=o;

			asm("pref @%0" : : "r" (d));
			d += 8;
		}
		d+=offset_dst;
	}
	d = (Uint32 *)0xe0000000;
	d[0] = d[8] = 0;

	if (gp2x_vsync)
#ifdef GP2X_SDLWRAPPER_NODOUBLEBUFFER
		change_vid_start=gp2x_sdlwrapper_screen->pixels;
#else
		change_vid_start=(void *)1;
#endif
	else
#ifdef GP2X_SDLWRAPPER_NODOUBLEBUFFER
		vid_set_start((uint32)gp2x_sdlwrapper_screen->pixels);
#else
		SDL_Flip(gp2x_sdlwrapper_screen);
#endif
	gp2x_sdlwrapper_actual_screen=(gp2x_sdlwrapper_actual_screen+1)&1;
#ifdef GP2X_SDLWRAPPER_NODOUBLEBUFFER
	gp2x_sdlwrapper_screen->pixels=gp2x_sdlwrapper_screens[gp2x_sdlwrapper_actual_screen];
#endif
#endif
}

static void update_display_8to16_div2(void)
{
#ifdef AUTOEVENTS
	mame4all_emulating=1;
#endif
	calculate_real_framerate();
	mame4all_prof_start(1);
#ifndef DREAMCAST
	SDL_LockSurface(gp2x_sdlwrapper_screen);
#ifdef PSP_RES
	register Uint16 *dst= (Uint16 *)(((unsigned)gp2x_sdlwrapper_screen->pixels)+(gp2x_xscreen+(480*gp2x_yscreen)));
#else
	register Uint16 *dst= (Uint16 *)(((unsigned)gp2x_sdlwrapper_screen->pixels)+(gp2x_xscreen+(320*gp2x_yscreen)));
#endif
	register Uint8 *src= (Uint8 *)(scrbitmap->line[gp2x_yoffset]+gp2x_xoffset);
	register int offset_src = ((int)(scrbitmap->line[2] - scrbitmap->line[0])-gp2x_width);
	register int offset_dst = (gp2x_sdlwrapper_screen->pitch>>1) - (gp2x_width>>1);
	register int max_x=gp2x_width, max_y=gp2x_height;
	register int i, j;
	for(j=0;j<max_y;j+=2)
	{
		for(i=0;i<max_x;i+=2)
		{
			*dst++ = gp2x_sdlwrapper_palette[*src];
			src+=2;
		}
		src+=offset_src;
		dst+=offset_dst;
	}
	SDL_UnlockSurface(gp2x_sdlwrapper_screen);
	SDL_Flip(gp2x_sdlwrapper_screen);
	gp2x_joystick_read();
#else
	change_vid_start=NULL;
	Uint32 tmp=(Uint32)(((unsigned)gp2x_sdlwrapper_screen->pixels)+(gp2x_xscreen+(320*gp2x_yscreen)));
	register Uint8 *s = (Uint8 *)(scrbitmap->line[gp2x_yoffset]+gp2x_xoffset);
	register Uint32 *d = (Uint32 *)(void *)
		(0xe0000000 | ( tmp & 0x03ffffe0));

	QACR0 = (((tmp)>>26)<<2)&0x1c;
	QACR1 = (((tmp)>>26)<<2)&0x1c;

	register int max_x=gp2x_width, max_y=gp2x_height;
	register int offset_src = (((int)(scrbitmap->line[2] - scrbitmap->line[0])-max_x));
	register int offset_dst = (gp2x_sdlwrapper_screen->w - (max_x>>1))>>1;
	register int i, j;
	for(j=0;j<max_y;j+=2)
	{
		for(i=0;i<max_x;i+=32)
		{
			asm("pref @%0" : : "r" (s + 32));

			register Uint32 o;

			o=((Uint32)gp2x_sdlwrapper_palette[*s++])&0x0000ffff; s++;
			o|=(((Uint32)gp2x_sdlwrapper_palette[*s++])<<16); s++;
			d[0]=o;

			o=((Uint32)gp2x_sdlwrapper_palette[*s++])&0x0000ffff; s++;
			o|=(((Uint32)gp2x_sdlwrapper_palette[*s++])<<16); s++;
			d[1]=o;

			o=((Uint32)gp2x_sdlwrapper_palette[*s++])&0x0000ffff; s++;
			o|=(((Uint32)gp2x_sdlwrapper_palette[*s++])<<16); s++;
			d[2]=o;

			o=((Uint32)gp2x_sdlwrapper_palette[*s++])&0x0000ffff; s++;
			o|=(((Uint32)gp2x_sdlwrapper_palette[*s++])<<16); s++;
			d[3]=o;

			o=((Uint32)gp2x_sdlwrapper_palette[*s++])&0x0000ffff; s++;
			o|=(((Uint32)gp2x_sdlwrapper_palette[*s++])<<16); s++;
			d[4]=o;

			o=((Uint32)gp2x_sdlwrapper_palette[*s++])&0x0000ffff; s++;
			o|=(((Uint32)gp2x_sdlwrapper_palette[*s++])<<16); s++;
			d[5]=o;

			o=((Uint32)gp2x_sdlwrapper_palette[*s++])&0x0000ffff; s++;
			o|=(((Uint32)gp2x_sdlwrapper_palette[*s++])<<16); s++;
			d[6]=o;

			o=((Uint32)gp2x_sdlwrapper_palette[*s++])&0x0000ffff; s++;
			o|=(((Uint32)gp2x_sdlwrapper_palette[*s++])<<16); s++;
			d[7]=o;

			asm("pref @%0" : : "r" (d));
			d += 8;
		}
		s+=offset_src;
		d+=offset_dst;
	}
	d = (Uint32 *)0xe0000000;
	d[0] = d[8] = 0;

	if (gp2x_vsync)
#ifdef GP2X_SDLWRAPPER_NODOUBLEBUFFER
		change_vid_start=gp2x_sdlwrapper_screen->pixels;
#else
		change_vid_start=(void *)1;
#endif
	else
#ifdef GP2X_SDLWRAPPER_NODOUBLEBUFFER
		vid_set_start((uint32)gp2x_sdlwrapper_screen->pixels);
#else
		SDL_Flip(gp2x_sdlwrapper_screen);
#endif
	gp2x_sdlwrapper_actual_screen=(gp2x_sdlwrapper_actual_screen+1)&1;
#ifdef GP2X_SDLWRAPPER_NODOUBLEBUFFER
	gp2x_sdlwrapper_screen->pixels=gp2x_sdlwrapper_screens[gp2x_sdlwrapper_actual_screen];
#endif
#endif
	mame4all_prof_end(1);
}

static void update_display_16to16(void)
{
	SDL_Flip(gp2x_sdlwrapper_screen);
}

update_display_func p_update_display=update_display_8to16;

void update_p_update_display_func(unsigned n)
{
	if (gp2x_sdlwrapper_bpp!=8)
		p_update_display=update_display_16to16;
	else
	{
		switch(n)
		{
			case 0:
				p_update_display=update_display_8to16; break;
			case 1:
				p_update_display=update_display_8to16_div2; break;
#ifdef DREAMCAST
			case 3:
				p_update_display=update_display_8to16_hw_scaled;break;
#endif
			default:
				p_update_display=update_display_8to16_sw_scaled;
		}
	}
#ifdef DREAMCAST
	if (gp2x_vsync && n!=3)
	{
		change_vid_start=NULL;
		if (!mame4all_vsync_handler_id)
			mame4all_vsync_handler_id=vblank_handler_add(mame4all_vsync_handler);
	}
	else
	{
		if (mame4all_vsync_handler_id)
			vblank_handler_remove(mame4all_vsync_handler_id);
		mame4all_vsync_handler_id=0;
	}
#endif
}


static void fade16(unsigned short n)
{
	if (gp2x_sdlwrapper_bpp!=8)
		SDL_Flip(gp2x_sdlwrapper_screen);
	else
	{
#ifndef DREAMCAST
		SDL_LockSurface(gp2x_sdlwrapper_screen);
#endif
		unsigned short rs=gp2x_sdlwrapper_screen->format->Rshift;
		unsigned short gs=gp2x_sdlwrapper_screen->format->Gshift;
		unsigned short bs=gp2x_sdlwrapper_screen->format->Bshift;
		unsigned short rm=gp2x_sdlwrapper_screen->format->Rmask;
		unsigned short gm=gp2x_sdlwrapper_screen->format->Gmask;
		unsigned short bm=gp2x_sdlwrapper_screen->format->Bmask;
#ifdef PSP_RES
		register Uint16 *dst= (Uint16 *)(((unsigned)gp2x_sdlwrapper_screen->pixels)+((gp2x_xscreen+(480*gp2x_yscreen))<<1));
#else
		register Uint16 *dst= (Uint16 *)(((unsigned)gp2x_sdlwrapper_screen->pixels)+((gp2x_xscreen+(320*gp2x_yscreen))<<1));
#endif
		register Uint8 *src= (Uint8 *)(scrbitmap->line[gp2x_yoffset]+gp2x_xoffset);
		register int offset_src = ((int)(scrbitmap->line[1] - scrbitmap->line[0])-gp2x_width);
		register int offset_dst = (gp2x_sdlwrapper_screen->pitch>>1) - gp2x_width;
		register int max_x=gp2x_width, max_y=gp2x_height;
		register int i, j;
		for(j=0;j<max_y;j++)
		{
			for(i=0;i<max_x;i++)
			{
				unsigned short d=gp2x_sdlwrapper_palette[*src++];
				register unsigned short r=(d&rm)>>rs;
				register unsigned short g=(d&gm)>>gs;
				register unsigned short b=(d&bm)>>bs;
				if (r>n)
					r-=n;
				else
					r=0;
				if (g>n)
					g-=n;
				else
					g=0;
				if (b>n)
					b-=n;
				else
					b=0;
				*dst++=( (r<<rs) | (g<<gs) | (b<<bs) ); 
			}
			src+=offset_src;
			dst+=offset_dst;
		}
#ifndef DREAMCAST
		SDL_UnlockSurface(gp2x_sdlwrapper_screen);
		SDL_Flip(gp2x_sdlwrapper_screen);
		gp2x_joystick_read();
#else
		vid_set_start((uint32)gp2x_sdlwrapper_screen->pixels);
		gp2x_sdlwrapper_actual_screen=(gp2x_sdlwrapper_actual_screen+1)&1;
		gp2x_sdlwrapper_screen->pixels=gp2x_sdlwrapper_screens[gp2x_sdlwrapper_actual_screen];
#endif
	}
}

#if defined(DEBUG_SOUND) &&  !defined(NOSOUND)
void print_debug_sound(void)
{
	extern unsigned sound_cuantos[4];
	extern unsigned sound_ajustes;
	extern unsigned sound_alcanza_callback;
	extern unsigned sound_recupera_callback;
	extern unsigned sound_alcanza_render;
	int i;
	for(i=0;i<4;i++)
		printf("distancia %i = %i -> %.2f%%\n",i,sound_cuantos[i],(((float)sound_cuantos[i])*100.0)/((float)sound_ajustes));
	printf("CALLBACK RECUPERA %i VECES\n",sound_recupera_callback);
	printf("CALLBACK ALCANZA %i VECES A RENDER\n",sound_alcanza_callback);
	printf("RENDER ALCANZA %i VECES A CALLBACK\n",sound_alcanza_render);
}
#else
#define print_debug_sound()
#endif


void fade_me(void)
{
	int i;
	print_frameskip();
#ifdef PROFILER_MAME4ALL
	mame4all_prof_show();
#endif
	memalloc_report("EXIT");
#ifndef NOSOUND
	osd_start_music();

	print_debug_sound();
#endif
#ifdef DREAMCAST
	if (dc_needs_video_reinit)
		dc_change_to_normal();
#endif
	if (!gp2x_rotate)
	for(i=0;i<192;i+=8)
	{
		Mix_VolumeMusic((i>>2)+32);
		fade16(i);
#ifndef DREAMCAST
		SDL_Delay(30);
#endif
	}
}
