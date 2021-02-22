#ifdef DREAMCAST
#include <kos.h>
// extern uint8 romdisk[];
KOS_INIT_FLAGS(INIT_DEFAULT);
// KOS_INIT_ROMDISK(romdisk);
#endif

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<SDL.h>
#include<SDL_mixer.h>

#ifdef PSP
#include "pspincludes.h"
extern void gp2x_gamelist_text_out_fmt(int , int , char* , ...);
SDL_Joystick *sdl_joy;
#endif

#ifdef PSP_2K
#include "gp2x_mame.h" /* for debug */

#define PSP2K_MEM_TOP		0xa000000	/* extended memory on SLIM */
#define PSP2K_MEM_BOTTOM	0xbffffff
#define PSP2K_MEM_SIZE		0x2000000	/* 32M */
#define	PSP2K_CHUNK			0x400000	/* 4M */
#define MEMORY_IS_PSP2K(mem)	((unsigned int)mem >= PSP2K_MEM_TOP)
#define PSP2K_MEM_STRUCT	4			// size information

unsigned int psp2k_mem_offset;
unsigned int psp2k_mem_left;

void psp2k_mem_init(void)
{
	psp2k_mem_offset = PSP2K_MEM_TOP + PSP2K_MEM_SIZE;
	psp2k_mem_left   = PSP2K_MEM_SIZE;

}

static void *psp2k_mem_alloc(unsigned int size)
{

	void *mem = NULL;

	if ((size+PSP2K_MEM_STRUCT) <= psp2k_mem_left) {
		psp2k_mem_offset -= size;
		psp2k_mem_offset -= PSP2K_MEM_STRUCT;					// size header
		*(unsigned int*)psp2k_mem_offset = size;
		psp2k_mem_offset += PSP2K_MEM_STRUCT;					// size headder
		mem = (unsigned char *)psp2k_mem_offset;
		psp2k_mem_left -= (size+PSP2K_MEM_STRUCT);
	} else {
		mem = malloc(size);
	}
	return mem;
}

static void psp2k_mem_free(void *mem)
{
	unsigned int size;

	if (mem == (void *)psp2k_mem_offset) {
		// if mem is top of extend memory, it can free.
		psp2k_mem_offset -= PSP2K_MEM_STRUCT;
		size = *(unsigned int*)psp2k_mem_offset;
		psp2k_mem_offset += PSP2K_MEM_STRUCT;
		psp2k_mem_offset += size;
		psp2k_mem_left += (size+PSP2K_MEM_STRUCT);
	}
	if (!mem || MEMORY_IS_PSP2K(mem))
		return;	// don't free extended memory
	free(mem);
}
#endif

#ifdef DREAMCAST
#include<SDL_dreamcast.h>
#define chdir(A) (fs_chdir(A))
extern void *gp2x_sdlwrapper_screens[2];
extern int gp2x_sdlwrapper_actual_screen;
extern int dc_needs_video_reinit;
#else
#include <unistd.h>
#include "icon.h"
#endif

#include "minimal.h"
#include "prof.h"

#ifdef PROFILER_MAME4ALL
#include "timeval.h"
#endif

int gp2x_sdlwrapper_bpp=8;

#ifdef AUTOEVENTS
int 	mame4all_emulating=0;
#endif

#ifndef GP2X_SDLWRAPPER_NODOUBLEBUFFER
SDL_Color	gp2x_sdlwrapper_palette[256];
#else
short		gp2x_sdlwrapper_palette[256];
int		gp2x_sdlwrapper_is_single=0;
#endif
void		*gp2x_sdlwrapper_screen_pixels=NULL;
SDL_Surface	*gp2x_sdlwrapper_screen=NULL;
float		gp2x_sdlwrapper_ticksdivisor=1.0;

#ifdef DREAMCAST
void *gp2x_sdlwrapper_screens[2];
int gp2x_sdlwrapper_actual_screen=1;
#endif


extern int soundcard;

extern int closing_sound;
extern void init_sound(unsigned rate, int stereo, int bits);
extern void quit_sound(void);

#ifdef DREAMCAST
#define DC_VIDEO_HZ_MEM_POSICION 0xcfbfa65
static unsigned char dc_video_hz_copy=0;
static void get_dc_video_hz(void)
{
	unsigned char *mihz=(unsigned char *)DC_VIDEO_HZ_MEM_POSICION;
	dcache_inval_range(DC_VIDEO_HZ_MEM_POSICION&0xFFFF0000,0x10000);
	dc_video_hz_copy=*mihz;
	switch (dc_video_hz_copy)
	{
		case 2:
			SDL_DC_Default60Hz(SDL_TRUE);
		case 1:
			SDL_DC_ShowAskHz(SDL_FALSE);
			break;
	}
}

static void set_dc_video_hz(void)
{
	unsigned int *p=(unsigned int *)0xcff00ff;
	unsigned char *mihz=(unsigned char *)DC_VIDEO_HZ_MEM_POSICION;
	FILE *f=fopen(DATA_SD_PREFIX "menu.bin","rb");
	if (!f) f=fopen(DATA_PREFIX "menu.bin","rb");
	if (f!=NULL)
	{
		void *mem;
		long len;
		fseek(f,0,SEEK_END);
		len=ftell(f);
		fseek(f,0,SEEK_SET);
		mem=malloc(len);
		fread(mem,1,len,f);
		fclose(f);
		*mihz=dc_video_hz_copy;
		dcache_flush_range(DC_VIDEO_HZ_MEM_POSICION&0xFFFF0000,0x10000);
		arch_exec(mem,len);
		free(mem); // THIS NEVER MUST EXECUTE IT
	}
	*p=0x12345678; // FORCE EXCEPTION
}
#else
#define get_dc_video_hz()
#define set_dc_video_hz()
#endif

void SetGP2XClock(int mhz)
{
#ifdef PSP
	scePowerSetClockFrequency(mhz,mhz,mhz/2);
#endif
}

#ifdef DREAMCAST
#include<dirent.h>
extern "C" { void fs_sdcard_shutdown(void); void fs_sdcard_init(void); int fs_sdcard_unmount(void); int fs_sdcard_mount(void); void sci_init(void); }

int sdcard_exists=0;
void reinit_sdcard(void)
{
	static uint32 last=(uint32)-5000;
	uint32 now=(((unsigned long long)timer_us_gettime64())>>10);
	if (now-last>5000) {
		char *dir="/sd/mame4all";
		DIR *d=NULL;
		fs_sdcard_shutdown();
		timer_spin_sleep(111);
		fs_sdcard_init();
		timer_spin_sleep(111);
		d=opendir(dir);
		sdcard_exists=(d!=NULL);
		if (d) {
			closedir(d);
			fs_mkdir("/sd/mame4all/cfg");
		}
		last=now;
	}
}
#endif

#ifdef DREAMCAST
static uint32 mame4all_dc_args[4]={ 0, 0, 0, 0};
static void mame4all_dreamcast_handler(irq_t source, irq_context_t *context)
{
	irq_create_context(context,context->r[15], (uint32)&gp2x_deinit, (uint32 *)&mame4all_dc_args[0],0);
}
#endif

void gp2x_init(int tickspersecond, int bpp, int rate, int bits, int stereo, int hz)
{
	extern int gp2x_double_buffer;
#ifdef DREAMCAST
	reinit_sdcard();
	if (sdcard_exists) chdir(ROM_SD_PREFIX);
	else
#endif
	chdir(ROM_PREFIX);

	get_dc_video_hz();

	SDL_Event e;
	Uint32 flags=SDL_HWSURFACE;

#ifdef DREAMCAST
	flags|=SDL_FULLSCREEN;
#ifdef GP2X_SDLWRAPPER_NODOUBLEBUFFER
	SDL_DC_SetVideoDriver(SDL_DC_DIRECT_VIDEO);
#else
	SDL_DC_SetVideoDriver(SDL_DC_DMA_VIDEO);
#endif
#endif
	puts("MAIN!!!!");
#if defined(DREAMCAST) && !defined(DEBUG_UAE4ALL)
    irq_set_handler(EXC_USER_BREAK_PRE,&mame4all_dreamcast_handler);
    irq_set_handler(EXC_INSTR_ADDRESS,&mame4all_dreamcast_handler);
    irq_set_handler(EXC_ILLEGAL_INSTR,&mame4all_dreamcast_handler);
    irq_set_handler(EXC_SLOT_ILLEGAL_INSTR,&mame4all_dreamcast_handler);
    irq_set_handler(EXC_GENERAL_FPU,&mame4all_dreamcast_handler);
    irq_set_handler(EXC_SLOT_FPU,&mame4all_dreamcast_handler);
    irq_set_handler(EXC_DATA_ADDRESS_WRITE,&mame4all_dreamcast_handler);
    irq_set_handler(EXC_DTLB_MISS_WRITE,&mame4all_dreamcast_handler);
    irq_set_handler(EXC_OFFSET_000,&mame4all_dreamcast_handler);
    irq_set_handler(EXC_OFFSET_100,&mame4all_dreamcast_handler);
    irq_set_handler(EXC_OFFSET_400,&mame4all_dreamcast_handler);
    irq_set_handler(EXC_OFFSET_600,&mame4all_dreamcast_handler);
    irq_set_handler(EXC_FPU,&mame4all_dreamcast_handler);
    irq_set_handler(EXC_TRAPA,&mame4all_dreamcast_handler);
    irq_set_handler(EXC_RESET_UDI,&mame4all_dreamcast_handler);
    irq_set_handler(EXC_UNHANDLED_EXC,&mame4all_dreamcast_handler);
    extern void *arch_abort_handler;
    arch_abort_handler=(void *)&mame4all_dreamcast_handler;
#endif
	SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_JOYSTICK);

#ifndef GP2X_SDLWRAPPER_NODOUBLEBUFFER
	flags|=SDL_DOUBLEBUF;
	gp2x_double_buffer=1;
	gp2x_sdlwrapper_screen=SDL_SetVideoMode(320,240,bpp,flags);
	gp2x_sdlwrapper_screen_pixels=gp2x_sdlwrapper_screen->pixels;
#else
	gp2x_double_buffer=0;
#ifdef PSP_RES
	gp2x_sdlwrapper_screen=SDL_SetVideoMode(480,272,16,flags);
#else
	gp2x_sdlwrapper_screen=SDL_SetVideoMode(320,240,16,flags);
#endif
	gp2x_sdlwrapper_bpp=bpp;
	if (bpp==16)
		gp2x_sdlwrapper_screen_pixels=gp2x_sdlwrapper_screen->pixels;
	else
	{
		if (!gp2x_sdlwrapper_screen_pixels)
#ifdef PSP_RES
			gp2x_sdlwrapper_screen_pixels=malloc(480*272*bpp>>3);
#else
			gp2x_sdlwrapper_screen_pixels=malloc(320*240*bpp>>3);
#endif
#ifdef DREAMCAST
		gp2x_sdlwrapper_screens[0]=(void *)0xA5000000; //gp2x_sdlwrapper_screen->pixels;
		gp2x_sdlwrapper_screens[1]=(void *)0xA5025800; //gp2x_sdlwrapper_screen->pixels+(320*256*2);
		fast_memset((void *)0xA5000000,0,0x4B000);
		gp2x_sdlwrapper_actual_screen=1;
		gp2x_sdlwrapper_screen->pixels=gp2x_sdlwrapper_screens[1];
#endif
	}
#endif
#ifdef GP2X_SDLWRAPPER_NOMSEC
	gp2x_sdlwrapper_ticksdivisor=((float)tickspersecond)*1000.0;
#endif

#ifdef DREAMCAST
	SDL_JoystickEventState(SDL_ENABLE);
	SDL_JoystickOpen(0);
	SDL_JoystickOpen(1);
	SDL_JoystickOpen(2);
	SDL_JoystickOpen(3);
	SDL_DC_EmulateMouse(SDL_FALSE);
	SDL_DC_EmulateKeyboard(SDL_TRUE);
#else
#ifdef PSP
	SDL_JoystickEventState(SDL_ENABLE);
	sdl_joy = SDL_JoystickOpen(0);
#endif
	SDL_WM_SetCaption("MAME4ALL","MAME4ALL");
	SDL_WM_SetIcon(SDL_LoadBMP_RW(SDL_RWFromMem((void*)rawicon, sizeof(rawicon)),0),NULL);
#endif
	SDL_EventState(SDL_ACTIVEEVENT,SDL_IGNORE);
	SDL_EventState(SDL_MOUSEMOTION,SDL_IGNORE);
	SDL_EventState(SDL_MOUSEBUTTONDOWN,SDL_IGNORE);
	SDL_EventState(SDL_MOUSEBUTTONUP,SDL_IGNORE);
	SDL_EventState(SDL_SYSWMEVENT,SDL_IGNORE);
	SDL_EventState(SDL_VIDEORESIZE,SDL_IGNORE);
	SDL_EventState(SDL_USEREVENT,SDL_IGNORE);
	SDL_ShowCursor(SDL_DISABLE);

	init_sound(rate,stereo,bits);

#ifndef DREAMCAST
	atexit(gp2x_deinit);
#endif

	SDL_Delay(100);
	while (SDL_PollEvent(&e) > 0)
		SDL_Delay(10);

#ifdef PROFILER_MAME4ALL
	mame4all_prof_init();
	mame4all_prof_add("Main");		// 0
	mame4all_prof_add("Blit");		// 1
	mame4all_prof_add("CPU0");		// 2
	mame4all_prof_add("CPU1");		// 3
	mame4all_prof_add("CPU2");		// 4
	mame4all_prof_add("CPU3");		// 5
	mame4all_prof_add("sound_update");	// 6
	mame4all_prof_add("screen_update");	// 7
#endif
}

void gp2x_deinit(void)
{
	quit_sound();
	SDL_Delay(333);
	set_dc_video_hz();
	SDL_Quit();
#ifdef PSP
	sceKernelExitGame(); 
#else
	exit(0);
#endif

}

#ifndef GP2X_SDLWRAPPER_NODOUBLEBUFFER
void gp2x_video_setpalette(void)
{
	SDL_SetColors(gp2x_sdlwrapper_screen,gp2x_sdlwrapper_palette,0,256);
}

#else

void gp2x_video_flip(void)
{
#ifdef AUTOEVENTS
	mame4all_emulating=0;
#endif
#ifdef GP2X_SDLWRAPPER_NODOUBLEBUFFER
	gp2x_sdlwrapper_is_single=0;
#endif
	if (gp2x_sdlwrapper_bpp!=8)
		SDL_Flip(gp2x_sdlwrapper_screen);
	else
	{
#ifndef DREAMCAST
		SDL_LockSurface(gp2x_sdlwrapper_screen);
		register Uint8 *src=(Uint8 *)gp2x_sdlwrapper_screen_pixels;
		register Uint16 *dst=(Uint16 *)gp2x_sdlwrapper_screen->pixels;
		unsigned offs=(gp2x_sdlwrapper_screen->pitch>>1)-gp2x_sdlwrapper_screen->w;
		int i,j;
#ifdef PSP_RES
		for(j=0;j<272;j++)
		{
			for(i=0;i<480;i++)
#else
		for(j=0;j<240;j++)
		{
			for(i=0;i<320;i++)
#endif
				*dst++ = gp2x_sdlwrapper_palette[*src++];
			dst+=offs;
		}
		SDL_UnlockSurface(gp2x_sdlwrapper_screen);
		SDL_Flip(gp2x_sdlwrapper_screen);
		gp2x_joystick_read();
#else
		if (dc_needs_video_reinit)
			return;
		register Uint8 *s=(Uint8 *)gp2x_sdlwrapper_screen_pixels;
		register Uint32 *d=(Uint32 *)(void *)
			(0xe0000000 | (((unsigned long)gp2x_sdlwrapper_screen->pixels) & 0x03ffffe0));
		QACR0 = ((((unsigned int)gp2x_sdlwrapper_screen->pixels)>>26)<<2)&0x1c;
		QACR1 = ((((unsigned int)gp2x_sdlwrapper_screen->pixels)>>26)<<2)&0x1c;

		register int n=(320*240)>>4;
		while(n--) {
			asm("pref @%0" : : "r" (s + 8));

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
		d = (Uint32 *)0xe0000000;
		d[0] = d[8] = 0;

		vid_set_start((uint32)gp2x_sdlwrapper_screen->pixels);
		gp2x_sdlwrapper_actual_screen=(gp2x_sdlwrapper_actual_screen+1)&1;
		gp2x_sdlwrapper_screen->pixels=gp2x_sdlwrapper_screens[gp2x_sdlwrapper_actual_screen];
#endif
	}
}

#endif

#ifdef GP2X_SDLWRAPPER_NODOUBLEBUFFER
void gp2x_video_flip_single(void)
{
	gp2x_video_flip();
	gp2x_sdlwrapper_is_single=1;
}
#endif

static void keyprocess(Uint32 *st, SDLKey key, SDL_bool pressed)
{
	Uint32 val=0, njoy=0;
	switch(key)
	{
		case SDLK_RETURN:
			val=GP2X_START; njoy=0; break;
		case SDLK_LCTRL:
			val=GP2X_A; njoy=0; break;
		case SDLK_LALT:
			val=GP2X_X; njoy=0; break;
		case SDLK_SPACE:
			val=GP2X_Y; njoy=0; break;
		case SDLK_LSHIFT:
			val=GP2X_B; njoy=0; break;
		case SDLK_TAB:
			val=GP2X_R; njoy=0; break;
		case SDLK_BACKSPACE:
			val=GP2X_L; njoy=0; break;
		case SDLK_LEFT:
			val=GP2X_LEFT; njoy=0; break;
		case SDLK_RIGHT:
			val=GP2X_RIGHT; njoy=0; break;
		case SDLK_UP:
			val=GP2X_UP; njoy=0; break;
		case SDLK_DOWN:
			val=GP2X_DOWN; njoy=0; break;
		case SDLK_F1:
			val=GP2X_SELECT; njoy=0; break;
		case SDLK_F11:
			val=GP2X_VOL_UP; njoy=0; break;
		case SDLK_F12:
			val=GP2X_VOL_DOWN; njoy=0; break;
		case SDLK_ESCAPE:
			val=GP2X_PUSH; njoy=0; break;
#ifndef DREAMCAST
		case SDLK_F10:
			if (pressed)
				SDL_WM_ToggleFullScreen(gp2x_sdlwrapper_screen);
			return;
#endif
		case SDLK_F9:
			val=GP2X_PUSH|GP2X_L; njoy=0; break;
		case SDLK_z:
			val=GP2X_START; njoy=1; break;
		case SDLK_e:
			val=GP2X_A; njoy=1; break;
		case SDLK_q:
			val=GP2X_X; njoy=1; break;
		case SDLK_x:
			val=GP2X_Y; njoy=1; break;
		case SDLK_c:
			val=GP2X_B; njoy=1; break;
		case SDLK_1:
			val=GP2X_R; njoy=1; break;
		case SDLK_2:
			val=GP2X_L; njoy=1; break;
		case SDLK_a:
			val=GP2X_LEFT; njoy=1; break;
		case SDLK_d:
			val=GP2X_RIGHT; njoy=1; break;
		case SDLK_w:
			val=GP2X_UP; njoy=1; break;
		case SDLK_s:
			val=GP2X_DOWN; njoy=1; break;
		case SDLK_F2:
			val=GP2X_SELECT; njoy=1; break;
		case SDLK_v:
			val=GP2X_START; njoy=2; break;
		case SDLK_y:
			val=GP2X_A; njoy=2; break;
		case SDLK_r:
			val=GP2X_X; njoy=2; break;
		case SDLK_b:
			val=GP2X_Y; njoy=2; break;
		case SDLK_n:
			val=GP2X_B; njoy=2; break;
		case SDLK_4:
			val=GP2X_R; njoy=2; break;
		case SDLK_5:
			val=GP2X_L; njoy=2; break;
		case SDLK_f:
			val=GP2X_LEFT; njoy=2; break;
		case SDLK_h:
			val=GP2X_RIGHT; njoy=2; break;
		case SDLK_t:
			val=GP2X_UP; njoy=2; break;
		case SDLK_g:
			val=GP2X_DOWN; njoy=2; break;
		case SDLK_F3:
			val=GP2X_SELECT; njoy=2; break;
		case SDLK_m:
			val=GP2X_START; njoy=3; break;
		case SDLK_o:
			val=GP2X_A; njoy=3; break;
		case SDLK_u:
			val=GP2X_X; njoy=3; break;
		case SDLK_COMMA:
			val=GP2X_Y; njoy=3; break;
		case SDLK_PERIOD:
			val=GP2X_B; njoy=3; break;
		case SDLK_8:
			val=GP2X_R; njoy=3; break;
		case SDLK_9:
			val=GP2X_L; njoy=3; break;
		case SDLK_j:
			val=GP2X_LEFT; njoy=3; break;
		case SDLK_l:
			val=GP2X_RIGHT; njoy=3; break;
		case SDLK_i:
			val=GP2X_UP; njoy=3; break;
		case SDLK_k:
			val=GP2X_DOWN; njoy=3; break;
		case SDLK_F4:
			val=GP2X_SELECT; njoy=3; break;
		default:
			return;
	}
	if (pressed)
		(st[njoy]) |= val;
	else
		(st[njoy]) ^= val;
}
#ifdef PSP
static void joyprocess(Uint32 *st, Uint8 button, SDL_bool pressed)
{
	Uint32 val=0;
	switch(button)
	{
		/*case PSP_CTRL_START://11
			val=GP2X_START; break;
		case PSP_CTRL_SQUARE://3
			val=GP2X_A; break;
		case PSP_CTRL_CROSS://2
			val=GP2X_X; break;
		case PSP_CTRL_TRIANGLE://0
			val=GP2X_Y; break;
		case PSP_CTRL_CIRCLE://1
			val=GP2X_B; break;
		case PSP_CTRL_RTRIGGER://5
			val=GP2X_R; break;
		case PSP_CTRL_LTRIGGER://4
			val=GP2X_L; break;
		case PSP_CTRL_LEFT://7
			val=GP2X_LEFT; break;
		case PSP_CTRL_RIGHT://9
			val=GP2X_RIGHT; break;
		case PSP_CTRL_UP://8
			val=GP2X_UP; break;
		case PSP_CTRL_DOWN://6
			val=GP2X_DOWN; break;
		case PSP_CTRL_SELECT://10
			val=GP2X_SELECT; break;
		case PSP_CTRL_HOME://12
			val=GP2X_PUSH; break;
		default:
			return;*/
		case 11://11
			val=GP2X_START; break;
		case 3://3
			val=GP2X_A; break;
		case 2://2
			val=GP2X_X; break;
		case 0://0
			val=GP2X_Y; break;
		case 1://1
			val=GP2X_B; break;
		case 5://5
			val=GP2X_R; break;
		case 4://4
			val=GP2X_L; break;
		case 7://7
			val=GP2X_LEFT; break;
		case 9://9
			val=GP2X_RIGHT; break;
		case 8://8
			val=GP2X_UP; break;
		case 6://6
			val=GP2X_DOWN; break;
		case 10://10
			val=GP2X_SELECT; break;
		case 12://12
			val=GP2X_PUSH; break;
	//	case 12://12
			//if (pressed)
			//	SDL_WM_ToggleFullScreen(gp2x_sdlwrapper_screen);
			//break;
		default:
			return;
	}
	if (pressed)
		(*st) |= val;
	else
		(*st) ^= val;
}
#endif

Uint32 _st_[4]={0,0,0,0};

#ifdef AUTOEVENTS
static unsigned long real_gp2x_joystick_read(int njoy)
#else
unsigned long gp2x_joystick_read_n(int njoy)
#endif
{
	SDL_Event event;
	if (njoy==1)
	while(SDL_PollEvent(&event))
		switch(event.type)
		{
#ifndef DREAMCAST
			case SDL_QUIT:
				exit(0);
				break;
#endif
			case SDL_KEYDOWN:
				keyprocess((Uint32 *)&_st_,event.key.keysym.sym,SDL_TRUE);
				break;
			case SDL_KEYUP:
				keyprocess((Uint32 *)&_st_,event.key.keysym.sym,SDL_FALSE);
				break;
#ifdef PSP
			case SDL_JOYBUTTONDOWN:
				joyprocess((Uint32 *)&_st_,event.jbutton.button,SDL_TRUE);
				break;
			case SDL_JOYBUTTONUP:
				joyprocess((Uint32 *)&_st_,event.jbutton.button,SDL_FALSE);
				break;		
			case SDL_JOYAXISMOTION:
				if (event.jaxis.axis==0)
				{
					static int reset_xl=0;
					static int reset_xr=0;
					if (event.jaxis.value<-18000)
					{
						if (!reset_xl){
							joyprocess((Uint32 *)&_st_,7,SDL_TRUE);}
						reset_xl=1;
					}
					else if (event.jaxis.value>18000)
					{
						if (!reset_xr){
							joyprocess((Uint32 *)&_st_,9,SDL_TRUE);}
						reset_xr=1;
					}
					else
					{
						if (reset_xr)
							joyprocess((Uint32 *)&_st_,9,SDL_FALSE);
						reset_xr=0;
//					}
//					else if (event.jaxis.value<0)
//					{
						if (reset_xl)
							joyprocess((Uint32 *)&_st_,7,SDL_FALSE);
						reset_xl=0;
					}
				}
				else if (event.jaxis.axis==1)
				{
					static int reset_yu=0;
					static int reset_yd=0;
					if (event.jaxis.value<-18000)
					{
						if (!reset_yu){
							joyprocess((Uint32 *)&_st_,8,SDL_TRUE);}
						reset_yu=1;
					}
					else if (event.jaxis.value>18000)
					{
						if (!reset_yd) {
							joyprocess((Uint32 *)&_st_,6,SDL_TRUE);}
						reset_yd=1;
					}
					else
					{
						if (reset_yd)
							joyprocess((Uint32 *)&_st_,6,SDL_FALSE);
						reset_yd=0;
//					}
//					else if (event.jaxis.value<0)
//					{
						if (reset_yu)
							joyprocess((Uint32 *)&_st_,8,SDL_FALSE);
						reset_yu=0;
					}
				}
				break;
#endif
#ifdef DREAMCAST
			case SDL_JOYAXISMOTION:
				if (event.jaxis.axis==0)
				{
					static int escape_yapulsado=0;
					static int f14_yapulsado[4]= {0,0,0,0};
					int wi=event.jaxis.which&3;
					if (event.jaxis.value<-100)
					{
						if (!escape_yapulsado)
							keyprocess((Uint32 *)&_st_,SDLK_ESCAPE,SDL_TRUE);
						escape_yapulsado=1;
						if (f14_yapulsado[wi])
							keyprocess((Uint32 *)&_st_,(SDLKey)(((unsigned)SDLK_F1)+wi),SDL_FALSE);
						f14_yapulsado[wi]=0;
					}
					else
					if (event.jaxis.value>100)
					{
						if (escape_yapulsado)
							keyprocess((Uint32 *)&_st_,SDLK_ESCAPE,SDL_FALSE);
						escape_yapulsado=0;
						if (!f14_yapulsado[wi])
							keyprocess((Uint32 *)&_st_,(SDLKey)(((unsigned)SDLK_F1)+wi),SDL_TRUE);
						f14_yapulsado[wi]=1;
					}
					else
					{
						if (escape_yapulsado)
							keyprocess((Uint32 *)&_st_,SDLK_ESCAPE,SDL_FALSE);
						escape_yapulsado=0;
						if (f14_yapulsado[wi])
							keyprocess((Uint32 *)&_st_,(SDLKey)(((unsigned)SDLK_F1)+wi),SDL_FALSE);
						f14_yapulsado[wi]=0;
					}
				}
				else
				if (event.jaxis.axis==1)
				{
					static int reset_yapulsado=0;
					if (event.jaxis.value<-100)
					{
						if (!reset_yapulsado)
							keyprocess((Uint32 *)&_st_,SDLK_F9,SDL_TRUE);
						reset_yapulsado=1;
					}
					else
					{
						if (reset_yapulsado)
							keyprocess((Uint32 *)&_st_,SDLK_F9,SDL_FALSE);
						reset_yapulsado=0;
					}
				}
				break;
#endif
		}
	return _st_[njoy-1];
}

#ifdef AUTOEVENTS
unsigned long gp2x_joystick_read_n(int njoy)
{
	static Uint32 first_time=0;
	static Uint32 tiempo=0;
	static Uint32 st[4]={0,0,0,0};

	if (njoy!=1)
		return 0;
	if (!mame4all_emulating)
		return real_gp2x_joystick_read(1);
	tiempo++;
//printf("%i\n",tiempo);
	if (tiempo>AUTOEVENTS) {
		printf("Ticks=%u\n",SDL_GetTicks()-first_time);
#ifdef DREAMCAST
		arch_reboot();
#endif
		st[1]=GP2X_PUSH;
	} else
		switch (tiempo)
		{
			case 1: first_time=SDL_GetTicks(); break;
			case 1500: keyprocess((Uint32 *)&st,SDLK_F1,SDL_TRUE); break;
			case 1505: keyprocess((Uint32 *)&st,SDLK_F1,SDL_FALSE); break;
			case 1650: keyprocess((Uint32 *)&st,SDLK_RETURN,SDL_TRUE); break;
			case 1655: keyprocess((Uint32 *)&st,SDLK_RETURN,SDL_FALSE); break;
			default:
				   if (tiempo>1700)
				   	switch (tiempo&63)
					{
						case 0: keyprocess((Uint32 *)&st,SDLK_RIGHT,SDL_TRUE); break;
						case 9: keyprocess((Uint32 *)&st,SDLK_RIGHT,SDL_FALSE); break;
						case 10: keyprocess((Uint32 *)&st,SDLK_LCTRL,SDL_TRUE); break;
						case 19: keyprocess((Uint32 *)&st,SDLK_LCTRL,SDL_FALSE); break;
						case 20: keyprocess((Uint32 *)&st,SDLK_UP,SDL_TRUE); break;
						case 29: keyprocess((Uint32 *)&st,SDLK_UP,SDL_FALSE); break;
			 	   	}
		}
	return st[1];
}
#endif

#if defined (_WINDOWS) || defined(DREAMCAST)
// long int random(void) { return rand(); }
#endif

// void sync(void) { }

#ifndef DREAMCAST
#if defined(DEBUG_MEMALLOC_REPORT)
static unsigned punt=0, parciales=0;
static void *allocs[128*1024];
static unsigned sizes[128*1024];
static unsigned totalsize=0;
static unsigned maximo=0;

static unsigned u_punt=0, u_parciales=0;
static void *u_allocs[128*1024];
static unsigned u_sizes[128*1024];
static unsigned u_totalsize=0;
static unsigned u_maximo=0;


void *unzip_malloc(size_t size)
{
	void *ret=malloc(size);
	u_allocs[u_punt]=ret;
	u_sizes[u_punt]=size;
	u_totalsize+=size;
	u_punt++; u_parciales++;
	if (u_totalsize>u_maximo)
		u_maximo=u_totalsize;
#ifdef DEBUG_MEMALLOC_FULL_REPORT_UNZIP
		printf("UNZIP MALLOC(%i) -> %i total, %i maximo -> %p\n",size,u_totalsize,u_maximo,ret); fflush(stdout);
#endif
	return ret;
}

void unzip_free(void *ptr)
{
	unsigned i;

	if (!ptr)
		return;
	
	for(i=0;i<u_punt;i++)
	  if (u_allocs[i])
		if (ptr==u_allocs[i])
		{
			u_allocs[i]=NULL;
			u_totalsize-=u_sizes[i];
			u_parciales--;
			free(ptr);
#ifdef DEBUG_MEMALLOC_FULL_REPORT_UNZIP
			printf("UNZIP FREE(%i) -> %i total, %i maximo -> %p\n",sizes[i],u_totalsize,u_maximo,ptr); fflush(stdout);
#endif
			return;
		}
	printf("DIOS !!! %p no reservado antes !!! \n",ptr); fflush(stdout);
	exit(0);
}

void *gp2x_malloc(size_t size)
{
	void *ret=malloc(size);
	allocs[punt]=ret;
	sizes[punt]=size;
	totalsize+=size;
	punt++; parciales++;
	if (totalsize>maximo)
		maximo=totalsize;
#ifdef DEBUG_MEMALLOC_FULL_REPORT
		printf("MALLOC(%i) -> %i total, %i maximo -> %p\n",size,totalsize,maximo,ret); fflush(stdout);
#endif
	return ret;
}

void gp2x_free(void *ptr)
{
	unsigned i;
	for(i=0;i<punt;i++)
	  if (allocs[i])
		if (ptr==allocs[i])
		{
			allocs[i]=NULL;
			totalsize-=sizes[i];
			parciales--;
			free(ptr);
#ifdef DEBUG_MEMALLOC_FULL_REPORT
			printf("FREE(%i) -> %i total, %i maximo -> %p\n",sizes[i],totalsize,maximo, ptr); fflush(stdout);
#endif
			return;
		}
	unzip_free(ptr);
}

void _memalloc_report_(char *msg)
{
	printf("MEMALLOC REPORT %s -> %i total, %i maximo\n",msg,totalsize,maximo);
	printf("             UNZIP -> %i total, %i maximo\n",u_totalsize,u_maximo);
}

void _memalloc_report_full(char *msg)
{
	int i;
	puts("------------");
	printf("MEMALLOC REPORT %s -> %i total, %i maximo\n",msg,totalsize,maximo);
	printf("             UNZIP -> %i total, %i maximo\n",u_totalsize,u_maximo);
	for(i=0;i<punt;i++)
		if (allocs[i])
			printf(" - %p (%i)\n", allocs[i], sizes[i]);
	puts("------------");
	fflush(stdout);
}
#else
#ifdef PSP_2K

void *gp2x_malloc(size_t size)
{
	void *ret;
	char text[64];
	if (size >= PSP2K_CHUNK) { ret = psp2k_mem_alloc(size);}
	else { ret = malloc(size);}
	return ret;
}
void gp2x_free(void *ptr) {	psp2k_mem_free(ptr); }

#else
#ifdef PSP
static unsigned punt=0, parciales=0;
static void *allocs[16*1024];
static unsigned sizes[16*1024];
static unsigned totalsize=0;
static unsigned maximo=0;

static unsigned u_punt=0, u_parciales=0;
static void *u_allocs[16*1024];
static unsigned u_sizes[161024];
static unsigned u_totalsize=0;
static unsigned u_maximo=0;

void unzip_free(void *ptr)
{
	unsigned i;

	if (!ptr)
		return;
	
	for(i=0;i<u_punt;i++)
	  if (u_allocs[i])
		if (ptr==u_allocs[i])
		{
			u_allocs[i]=NULL;
			u_totalsize-=u_sizes[i];
			u_parciales--;
			free(ptr);
#ifdef DEBUG_MEMALLOC_FULL_REPORT_UNZIP
			printf("UNZIP FREE(%i) -> %i total, %i maximo -> %p\n",sizes[i],u_totalsize,u_maximo,ptr); fflush(stdout);
#endif
			return;
		}
	printf("DIOS !!! %p no reservado antes !!! \n",ptr); fflush(stdout);
	exit(0);
}

void *gp2x_malloc(size_t size)
{
	void *ret=malloc(size);
	allocs[punt]=ret;
	sizes[punt]=size;
	totalsize+=size;
	punt++; parciales++;
	if (totalsize>maximo)
		maximo=totalsize;
#ifdef DEBUG_MEMALLOC_FULL_REPORT
		printf("MALLOC(%i) -> %i total, %i maximo -> %p\n",size,totalsize,maximo,ret); fflush(stdout);
#endif
	return ret;
}

void gp2x_free(void *ptr)
{
	unsigned i;
	for(i=0;i<punt;i++)
	  if (allocs[i])
		if (ptr==allocs[i])
		{
			allocs[i]=NULL;
			totalsize-=sizes[i];
			parciales--;
			free(ptr);
#ifdef DEBUG_MEMALLOC_FULL_REPORT
			printf("FREE(%i) -> %i total, %i maximo -> %p\n",sizes[i],totalsize,maximo, ptr); fflush(stdout);
#endif
			return;
		}
	unzip_free(ptr);
}
#else
void *gp2x_malloc(size_t size) { return malloc(size); }
void gp2x_free(void *ptr) { free(ptr); }
#endif
#endif
#endif
#endif

#ifdef PROFILER_MAME4ALL
static unsigned mame4all_prof_total_initial=0;
unsigned mame4all_prof_total=0;
static char *mame4all_prof_msg[MAME4ALL_PROFILER_MAX];
unsigned long long mame4all_prof_initial[MAME4ALL_PROFILER_MAX];
unsigned long long mame4all_prof_sum[MAME4ALL_PROFILER_MAX];
unsigned long long mame4all_prof_executed[MAME4ALL_PROFILER_MAX];
int mame4all_prof_started[MAME4ALL_PROFILER_MAX];


void mame4all_prof_init(void)
{
	unsigned i;
	unsigned long long s=get_time_in_micros();
	for(i=0;i<MAME4ALL_PROFILER_MAX;i++)
	{
		mame4all_prof_initial[i]=s;
		mame4all_prof_sum[i]=0;

		if (!mame4all_prof_total)
		{
			mame4all_prof_msg[i]=NULL;
			mame4all_prof_started[i]=0;
			mame4all_prof_executed[i]=0;
		}
		else
		{
			if (mame4all_prof_started[i])
				mame4all_prof_executed[i]=1;
			else
				mame4all_prof_executed[i]=0;
		}
	}
	mame4all_prof_total_initial=s;
}

void mame4all_prof_add(char *msg)
{
	if (mame4all_prof_total<MAME4ALL_PROFILER_MAX)
	{
		mame4all_prof_msg[mame4all_prof_total]=msg;	
		mame4all_prof_total++;
	}
}

void mame4all_prof_show(void)
{
	int i;

	for(i=mame4all_prof_total-1;i>=0;i--)
		mame4all_prof_end(i);

	float toper=0;
	unsigned long long to=mame4all_prof_sum[0];

	puts("\n\n\n\n");
	puts("--------------------------------------------");
	for(i=0;i<mame4all_prof_total;i++)
	{
		unsigned long long t0=mame4all_prof_sum[i];
		float percent=(float)t0;
		percent*=100.0;
		percent/=(float)to;
		toper+=percent;
#ifndef PROFILER_SDL
		t0/=1000;
#endif
		printf("%s: %.2f%% -> Ticks=%i -> %iK veces\n",mame4all_prof_msg[i],percent,((unsigned)t0),(unsigned)(mame4all_prof_executed[i]>>10));
	}
//	printf("TOTAL: %.2f%% -> Ticks=%i\n",toper,to);
	puts("--------------------------------------------"); fflush(stdout);
}

void mame4all_prof_setmsg(int n, char *msg)
{
	mame4all_prof_msg[n]=msg;
}

#endif
