/*

  GP2X minimal library v0.A by rlyeh, (c) 2005. emulnation.info@rlyeh (swap it!)

  Thanks to Squidge, Robster, snaff, Reesy and NK, for the help & previous work! :-)

  License
  =======

  Free for non-commercial projects (it would be nice receiving a mail from you).
  Other cases, ask me first.

  GamePark Holdings is not allowed to use this library and/or use parts from it.

*/

#include "minimal.h"
#include "cpuctrl.h"
#include "stdio.h"

unsigned long 			gp2x_dev[3];
static volatile unsigned long 	*gp2x_memregl;
volatile unsigned short 	*gp2x_memregs;
static unsigned long 		gp2x_ticks_per_second;
unsigned char 			*gp2x_screen8;
unsigned short 			*gp2x_screen15;
unsigned short 			*gp2x_logvram15[2];
unsigned long 			gp2x_physvram[2];
unsigned int			gp2x_nflip;
volatile unsigned short 	gp2x_palette[512];
int				gp2x_sound_rate=DEFAULT_SAMPLE_RATE;
int 				gp2x_pal_50hz=0;

unsigned char  			*gp2x_dualcore_ram;
unsigned long  			 gp2x_dualcore_ram_size;

#define MAX_SAMPLE_RATE 44100

void gp2x_video_flip(void)
{
#ifdef MMUHACK
	flushcache(gp2x_screen8,gp2x_screen8+(640*480),0);
#endif
  	unsigned long address=gp2x_physvram[gp2x_nflip];
	gp2x_nflip=gp2x_nflip?0:1;
  	gp2x_screen15=gp2x_logvram15[gp2x_nflip]; 
  	gp2x_screen8=(unsigned char *)gp2x_screen15; 
   	gp2x_memregs[0x290E>>1]=(unsigned short)(address & 0xFFFF);
  	gp2x_memregs[0x2910>>1]=(unsigned short)(address >> 16);
  	gp2x_memregs[0x2912>>1]=(unsigned short)(address & 0xFFFF);
  	gp2x_memregs[0x2914>>1]=(unsigned short)(address >> 16);
}


void gp2x_video_flip_single(void)
{
#ifdef MMUHACK
	flushcache(gp2x_screen8,gp2x_screen8+(640*480),0);
#endif
  	unsigned long address=gp2x_physvram[0];
  	gp2x_nflip=0;
  	gp2x_screen15=gp2x_logvram15[gp2x_nflip]; 
  	gp2x_screen8=(unsigned char *)gp2x_screen15; 
  	gp2x_memregs[0x290E>>1]=(unsigned short)(address & 0xFFFF);
  	gp2x_memregs[0x2910>>1]=(unsigned short)(address >> 16);
  	gp2x_memregs[0x2912>>1]=(unsigned short)(address & 0xFFFF);
  	gp2x_memregs[0x2914>>1]=(unsigned short)(address >> 16);
}


void gp2x_video_setpalette(void)
{
  	unsigned short *g=(unsigned short *)gp2x_palette;
	unsigned short *memreg=(short unsigned int *)&gp2x_memregs[0x295A>>1];
  	int i=512;
  	gp2x_memregs[0x2958>>1]=0;                                                     
  	do { *memreg=*g++; } while(--i);
}


int sound_vol;

unsigned long gp2x_joystick_read(void)
{
  	extern int gp2x_rotate;
  	unsigned long value=(gp2x_memregs[0x1198>>1] & 0x00FF);
  	unsigned long res;
  	if(value==0xFD) value=0xFA;
  	if(value==0xF7) value=0xEB;
  	if(value==0xDF) value=0xAF;
  	if(value==0x7F) value=0xBE;
  	res=~((gp2x_memregs[0x1184>>1] & 0xFF00) | value | (gp2x_memregs[0x1186>>1] << 16));
  	
  	/* Check USB Joypad */
  	if (num_of_joys>0 && usbjoy_player1)
  		res |= gp2x_usbjoy_check(joys[0]);
  	
  	/* GP2X F200 Push Button */
  	if ((res & GP2X_UP) && (res & GP2X_DOWN) && (res & GP2X_LEFT) && (res & GP2X_RIGHT))
  		res |= GP2X_PUSH;
  	
  	if (!(gp2x_rotate&1)) {
  		if ( (res & GP2X_VOL_UP) &&  (res & GP2X_VOL_DOWN)) gp2x_sound_volume(100,100);
  		if ( (res & GP2X_VOL_UP) && !(res & GP2X_VOL_DOWN)) gp2x_sound_volume(sound_vol+1,sound_vol+1);
  		if (!(res & GP2X_VOL_UP) &&  (res & GP2X_VOL_DOWN)) gp2x_sound_volume(sound_vol-1,sound_vol-1);
  	}
  	return res;
}


void gp2x_sound_volume(int l, int r)
{
 	l=l<0?0:l; l=l>319?319:l; r=r<0?0:r; r=r>319?319:r;
 	sound_vol=l;
 	l=(((l*0x50)/100)<<8)|((r*0x50)/100); /*0x5A, 0x60*/
 	ioctl(gp2x_dev[2], SOUND_MIXER_WRITE_PCM, &l); /*SOUND_MIXER_WRITE_VOLUME*/
}


void gp2x_timer_delay(unsigned long ticks)
{
	unsigned long ini=gp2x_timer_read();
	while (gp2x_timer_read()-ini<ticks);
}


unsigned long gp2x_timer_read(void)
{
 	return gp2x_memregl[0x0A00>>2]/gp2x_ticks_per_second;
}

unsigned long gp2x_timer_read_real(void)
{
 	return gp2x_memregl[0x0A00>>2];
}

unsigned long gp2x_timer_read_scale(void)
{
 	return gp2x_ticks_per_second;
}

void gp2x_timer_profile(void)
{
	static unsigned long i=0;
	if (!i) i=gp2x_memregl[0x0A00>>2];
	else {
		printf("%u\n",gp2x_memregl[0x0A00>>2]-i);
		i=0;	
	}
}

static pthread_t gp2x_sound_thread=0;								// Thread for gp2x_sound_thread_play()
static volatile int gp2x_sound_thread_exit=0;							// Flag to end gp2x_sound_thread_play() thread
static volatile int gp2x_sound_buffer=0;							// Current sound buffer
static short gp2x_sound_buffers_total[(MAX_SAMPLE_RATE*16)/30];					// Sound buffer
static void *gp2x_sound_buffers[16] = {								// Sound buffers
	(void *)(gp2x_sound_buffers_total+((MAX_SAMPLE_RATE*0)/30)),
	(void *)(gp2x_sound_buffers_total+((MAX_SAMPLE_RATE*1)/30)),
	(void *)(gp2x_sound_buffers_total+((MAX_SAMPLE_RATE*2)/30)),
	(void *)(gp2x_sound_buffers_total+((MAX_SAMPLE_RATE*3)/30)),
	(void *)(gp2x_sound_buffers_total+((MAX_SAMPLE_RATE*4)/30)),
	(void *)(gp2x_sound_buffers_total+((MAX_SAMPLE_RATE*5)/30)),
	(void *)(gp2x_sound_buffers_total+((MAX_SAMPLE_RATE*6)/30)),
	(void *)(gp2x_sound_buffers_total+((MAX_SAMPLE_RATE*7)/30)),
	(void *)(gp2x_sound_buffers_total+((MAX_SAMPLE_RATE*8)/30)),
	(void *)(gp2x_sound_buffers_total+((MAX_SAMPLE_RATE*9)/30)),
	(void *)(gp2x_sound_buffers_total+((MAX_SAMPLE_RATE*10)/30)),
	(void *)(gp2x_sound_buffers_total+((MAX_SAMPLE_RATE*11)/30)),
	(void *)(gp2x_sound_buffers_total+((MAX_SAMPLE_RATE*12)/30)),
	(void *)(gp2x_sound_buffers_total+((MAX_SAMPLE_RATE*13)/30)),
	(void *)(gp2x_sound_buffers_total+((MAX_SAMPLE_RATE*14)/30)),
	(void *)(gp2x_sound_buffers_total+((MAX_SAMPLE_RATE*15)/30))
};
static volatile int sndlen=(MAX_SAMPLE_RATE*2)/60;						// Current sound buffer length

void gp2x_sound_play(void *buff, int len)
{
	int nbuff=(gp2x_sound_buffer+1)&15;							// Sound buffer to write
	fast_memcpy(gp2x_sound_buffers[nbuff],buff,len);					// Write the sound buffer
	gp2x_sound_buffer=nbuff;								// Update the current sound buffer
	sndlen=len;										// Update the sound buffer length
}

void gp2x_sound_thread_mute(void)
{
	fast_memset(gp2x_sound_buffers_total,0,(MAX_SAMPLE_RATE*16*2)/30);
	sndlen=(gp2x_sound_rate*2)/60;
}

static void *gp2x_sound_thread_play(void *none)
{
	int nbuff=gp2x_sound_buffer;								// Number of the sound buffer to play
	do {
		write(gp2x_dev[1], gp2x_sound_buffers[nbuff], sndlen); 				// Play the sound buffer
		ioctl(gp2x_dev[1], SOUND_PCM_SYNC, 0);						// Synchronize Audio
		nbuff=(nbuff+(nbuff!=gp2x_sound_buffer))&15;					// Update the sound buffer to play
	} while(!gp2x_sound_thread_exit);							// Until the end of the sound thread
	pthread_exit(0);
}

void gp2x_sound_thread_start(void)
{
	gp2x_sound_thread=0;
	gp2x_sound_thread_exit=0;
	gp2x_sound_buffer=0;
	gp2x_sound_set_rate(gp2x_sound_rate);
	sndlen=(gp2x_sound_rate*2)/60;
	gp2x_sound_thread_mute();
	pthread_create( &gp2x_sound_thread, NULL, gp2x_sound_thread_play, NULL);
}

void gp2x_sound_thread_stop(void)
{
	gp2x_sound_thread_exit=1;
	gp2x_timer_delay(500);
	gp2x_sound_thread=0;
	gp2x_sound_thread_mute();
}

void gp2x_sound_set_rate(int rate)
{
	ioctl(gp2x_dev[1], SNDCTL_DSP_SPEED,  &rate);
}

void gp2x_init(int ticks_per_second, int bpp, int rate, int bits, int stereo, int Hz)
{
  	gp2x_ticks_per_second=7372800/ticks_per_second; 

  	gp2x_dev[0] = open("/dev/mem",   O_RDWR); 
  	gp2x_dev[1] = open("/dev/dsp",   O_WRONLY);
  	gp2x_dev[2] = open("/dev/mixer", O_WRONLY);

       	gp2x_memregl=(unsigned long  *)mmap(0, 0x10000, PROT_READ|PROT_WRITE, MAP_SHARED, gp2x_dev[0], 0xc0000000);
        gp2x_memregs=(unsigned short *)gp2x_memregl;
	gp2x_video_setpalette();

	gp2x_sound_volume(100,100);
       	gp2x_memregs[0x0F16>>1] = 0x830a; usleep(100000);
       	gp2x_memregs[0x0F58>>1] = 0x100c; usleep(100000); 

	/* Upper memory access */
	gp2x_dualcore_ram_size=0x2000000;
	gp2x_dualcore_ram=(unsigned char  *)mmap(0, gp2x_dualcore_ram_size, PROT_READ|PROT_WRITE, MAP_SHARED, gp2x_dev[0], 0x02000000);
	gp2x_malloc_init(gp2x_dualcore_ram);

	/* Take video memory */
        gp2x_physvram[0]=0x4000000-640*480;
        gp2x_logvram15[0]=(unsigned short *)upper_take(gp2x_physvram[0],640*480);
       
        gp2x_physvram[1]=0x4000000-640*480*2;
        gp2x_logvram15[1]=(unsigned short *)upper_take(gp2x_physvram[1],640*480);

	gp2x_screen15=gp2x_logvram15[0];
	gp2x_screen8=(unsigned char *)gp2x_screen15;	
	gp2x_nflip=0;

  	gp2x_memregs[0x28DA>>1]=(((bpp+1)/8)<<9)|0xAB; /*8/15/16/24bpp...*/
  	gp2x_memregs[0x290C>>1]=320*((bpp+1)/8); /*line width in bytes*/

 	ioctl(gp2x_dev[1], SNDCTL_DSP_SETFMT, &bits);
  	ioctl(gp2x_dev[1], SNDCTL_DSP_STEREO, &stereo);
	ioctl(gp2x_dev[1], SNDCTL_DSP_SPEED,  &rate);
	//stereo = 0x80000|7; ioctl(gp2x_dev[1], SNDCTL_DSP_SETFRAGMENT, &stereo);
	//stereo = 0x00100009; ioctl(gp2x_dev[1], SNDCTL_DSP_SETFRAGMENT, &stereo);
	stereo = 0x00100007; ioctl(gp2x_dev[1], SNDCTL_DSP_SETFRAGMENT, &stereo);

	cpuctrl_init(); /* cpuctrl.cpp */
	Disable_940(); /* cpuctrl.cpp */

#ifdef MMUHACK
	/* MMU Tables Hack by Squidge */
	mmuhack(); /* squidgehack.cpp */
#endif
	/* RAM Tweaks */
	set_ram_tweaks();

  	fast_memset(gp2x_screen15, 0, 640*480); gp2x_video_flip();
  	fast_memset(gp2x_screen15, 0, 640*480); gp2x_video_flip();

	/* USB Joysticks Initialization */
	gp2x_usbjoy_init();
	
	atexit(gp2x_deinit);
}

#if defined(__cplusplus)
extern "C" {
#endif
extern int fcloseall(void);
#if defined(__cplusplus)
}
#endif

void gp2x_deinit(void)
{
	/* USB Joysticks Close */
	gp2x_usbjoy_close();

#ifdef MMUHACK
	flushcache(gp2x_screen8,gp2x_screen8+(640*480),0);
	mmuunhack(); /* squidgehack.cpp */
#endif
	cpuctrl_deinit(); /* cpuctrl.cpp */
	gp2x_memregs[0x28DA>>1]=0x4AB; /*set video mode*/
	gp2x_memregs[0x290C>>1]=640;   
  	//munmap((void *)gp2x_dualcore_ram, gp2x_dualcore_ram_size);
  	munmap((void *)gp2x_memregl, 0x10000);
 	close(gp2x_dev[2]);
 	close(gp2x_dev[1]);
 	close(gp2x_dev[0]);
	fcloseall(); /*close all files*/
 
  	{  /* Zaq121 07/05/2006 Alternative frontend support -> */
    		extern char *ext_menu, *ext_state;

		if(ext_menu && ext_state)
      			execl(ext_menu, ext_menu, "-state", ext_state, NULL);
    		else if(ext_menu)
      			execl(ext_menu, ext_menu, NULL);
    		else
    		{
      			chdir("/usr/gp2x"); /*go to menu*/
      			execl("gp2xmenu", "gp2xmenu", NULL);
    		}
  	} /* <- end mod */
}

void SetGP2XClock(int mhz)
{
	/* set_display_clock_div(16+8*(mhz>=220)); */ // Fixed the LCD timing issue (washed screen). Thx, god_at_hell and Aruseman.
	set_FCLK(mhz);
	set_DCLK_Div(0);
	set_920_Div(0);
}

void SetVideoScaling(int pixels,int width,int height)
{
	extern int gp2x_bordertvout;
	extern int gp2x_bordertvout_width;
	extern int gp2x_bordertvout_height;
	extern int gp2x_text_width;
	float x, y;
	float xscale,yscale;

	int bpp=(gp2x_memregs[0x28DA>>1]>>9)&0x3;  /* bytes per pixel */

	gp2x_pal_50hz=0;
	
	if(gp2x_memregs[0x2800>>1]&0x100) /* TV-Out ON? */
	{
		if (gp2x_bordertvout)
		{
			pixels+=((((pixels*gp2x_bordertvout_width)/100)+7)&~7);
			width+=((((width*gp2x_bordertvout_width)/100)+7)&~7);
			height+=((((height*gp2x_bordertvout_height)/100)+7)&~7);	
			gp2x_text_width=pixels;
		}
		xscale=489.0; /* X-Scale with TV-Out (PAL or NTSC) */
		if (gp2x_memregs[0x2818>>1]  == 287) /* PAL? */
		{
			yscale=(pixels*274.0)/320.0; /* Y-Scale with PAL */
			gp2x_pal_50hz=1;
		}
		else if (gp2x_memregs[0x2818>>1]  == 239) /* NTSC? */
			yscale=(pixels*331.0)/320.0; /* Y-Scale with NTSC */
	}
  	else /* TV-Out OFF? */
  	{
		gp2x_bordertvout=0;
		/*
		if (gp2x_bordertvout)
		{
			pixels+=((((pixels*gp2x_bordertvout_width)/100)+7)&~7);
			width+=((((width*gp2x_bordertvout_width)/100)+7)&~7);
			height+=((((height*gp2x_bordertvout_height)/100)+7)&~7);	
			gp2x_text_width=pixels;
		}
		*/
		xscale=1024.0; /* X-Scale for LCD */
		yscale=pixels; /* Y-Scale for LCD */
	}

	x=(xscale*width)/320.0;
	y=(yscale*bpp*height)/240.0;
	gp2x_memregs[0x2906>>1]=(unsigned short)x; /* scale horizontal */
	gp2x_memregl[0x2908>>2]=(unsigned  long)y; /* scale vertical */
	gp2x_memregs[0x290C>>1]=pixels*bpp; /* Set Video Width */
}

#define RAMTWEAKSTRUCTURE "--active %d --trc %d --tras %d --twr %d --tmrd %d --trfc %d --trp %d --trcd %d"
void set_ram_tweaks(void)
{
	extern char gp2x_path_mame[24];
	char name[256];
	int args[8]={0,-1,-1,-1,-1,-1,-1,-1};
	FILE *f;
	sprintf(name,"%scfg/mame_tweaks.cfg",gp2x_path_mame);
	f=fopen(name,"rb");
	if (f) {
		fscanf(f,RAMTWEAKSTRUCTURE,&args[0],&args[1],&args[2],&args[3],&args[4],&args[5],&args[6],&args[7]);
		fclose(f);
		if (args[0]) {
			printf("RAM Tweaks Activated\r\n");
			if((args[1]!=-1) && (args[1]-1 < 16)) set_tRC(args[1]-1);
			if((args[2]!=-1) && (args[2]-1 < 16)) set_tRAS(args[2]-1);
			if((args[3]!=-1) && (args[3]-1 < 16)) set_tWR(args[3]-1);
			if((args[4]!=-1) && (args[4]-1 < 16)) set_tMRD(args[4]-1);
			if((args[5]!=-1) && (args[5]-1 < 16)) set_tRFC(args[5]-1);
			if((args[6]!=-1) && (args[6] < 16)) set_tRP(args[6]-1);
			if((args[7]!=-1) && (args[7]-1 < 16)) set_tRCD(args[7]-1);
		} else {
			printf("RAM Tweaks Disabled\r\n");
		}
	}
}

