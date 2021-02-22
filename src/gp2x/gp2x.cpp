/***************************************************************************

  osdepend.c

  OS dependant stuff (display handling, keyboard scan...)
  This is the only file which should me modified in order to port the
  emulator to a different system.

***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "driver.h"
#include "gp2x_mame.h"
#include "gp2x_menu.h"

extern int gp2x_sound_rate;

#ifdef PSP
/* extern "C" int SDL_main( int argc, char *argv[] ); */
#include "pspincludes.h"
 #ifndef PSP_USERMODE
	PSP_MODULE_INFO("Mame4All", 0x1000, 1, 0); 
	PSP_MAIN_THREAD_ATTR(0); 
	/* PSP_MAIN_THREAD_STACK_SIZE_KB(32); */
 #else
	PSP_MODULE_INFO("Mame4All", 0, 1, 0);
	PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER); 
	
	/* Exit callback */
	int exit_callback(int arg1, int arg2, void *common) {
		sceKernelExitGame();
		return 0;
	}

	/* Callback thread */
	int CallbackThread(SceSize args, void *argp) {
		int cbid;

		cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
		sceKernelRegisterExitCallback(cbid);

		sceKernelSleepThreadCB();

		return 0;
	}

	/* Sets up the callback thread and returns its thread id */
	int SetupCallbacks(void) {
		int thid = 0;

		thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
		if(thid >= 0) {
				sceKernelStartThread(thid, 0, 0);
		}

		return thid;
	}
  #endif

#define printf	pspDebugScreenPrintf

int FileExist(char * fileName)
{
   FILE * infile;
   int ret = 0;
   infile = fopen(fileName, "r");
   if (infile == NULL)
      ret = (1);
      
   sceKernelDelayThread(2*1000000); // 2 sec to read error       
   fclose (infile);
   return (ret);
}

#endif

#if !defined(GP2X) && defined(USE_FILECACHE)
void filecache_init(void);
void filecache_disable(void);
#else
#define filecache_init()
#endif

int  msdos_init_seal (void);
int  msdos_init_sound(void);
void msdos_init_input(void);
void msdos_shutdown_sound(void);
void msdos_shutdown_input(void);

/* platform independent options go here */
struct GameOptions options;

/* Command Line Arguments Emulation */
char playgame[16] = "builtinn\0";

#ifdef GP2X
	/* Zaq121 07/05/2006 Alternative frontend support -> */
	char *ext_menu = NULL;
	char *ext_state = NULL;
	int no_selector = 0;
	/* <- end mod */
#endif

/* put here anything you need to do when the program is started. Return 0 if */
/* initialization was successful, nonzero otherwise. */
int osd_init(void)
{
#ifdef GP2X
	gp2x_text_log("GP2X Port Init...");
#endif
	if (msdos_init_sound())
		return 1;
	msdos_init_input();
	return 0;
}


/* put here cleanup routines to be executed when the program is terminated. */
void osd_exit(void)
{
	msdos_shutdown_sound();
	msdos_shutdown_input();
}


/* Get Command Line Options */
static __inline void parse_cmdline (int myargc, char **myargv, struct GameOptions *options, int game_index)
{
	/* from sound.c */
	extern int soundcard;

	/* from gp2x_menu.c */
	extern int gp2x_frameskip;
	extern int gp2x_sound_enable;

	options->frameskip = gp2x_frameskip; /* 0,1,2,3... */
	options->norotate  = 0;
	options->ror       = 0;
	options->rol       = 0;
	options->flipx     = 0;
	options->flipy     = 0; 

	/* read sound configuration */
	soundcard=gp2x_sound_enable; /* 0 Sound OFF, 1-5 Sound ON */
#ifdef GP2X
	switch (soundcard)
	{
		case 0: break;
		case 1: gp2x_sound_rate=16000/*15360*/; break;
		case 2: gp2x_sound_rate=22050; break;
		case 3: gp2x_sound_rate=32000/*33075*/; break;
		case 4: gp2x_sound_rate=44100; break;
		case 5: gp2x_sound_rate=11025; break;
	}
	options->samplerate = gp2x_sound_rate;
#endif
	options->samplebits = 16;

	/* misc configuration */
	options->cheat = 0;

  	#ifdef GP2X
	/* Zaq121 07/05/2006 Alternative frontend support -> */
	{
		int x;
		for(x = 1; x < myargc; x++)
		{
			if(strcasecmp(myargv[x], "-menu") == 0) {
       				if(x+1 < myargc) { ++x; ext_menu = myargv[x]; } /* External Frontend: Program Name */
      			}
      			else if(strcasecmp(myargv[x], "-state") == 0) {
        			if(x+1 < myargc) { ++x; ext_state = myargv[x]; } /* External Frontend: Arguments */
      			}
      			else if((myargv[x][0] != '-')&&(no_selector==0)) { strcpy(playgame, myargv[x]); no_selector = 1; } /* External Frontend: ROM Name */
    		}
  	}
  	/* <- end mod */
	#endif
}

int main(int argc,char **argv)
{
#ifdef PSP
	pspDebugScreenInit();
	#ifdef PSP_USERMODE
		SetupCallbacks();
	#endif
	#ifdef WIFI 
		net_io_init();
	#endif
#endif
	char text[64];
	int res, i, game_index;

	/* GP2X Video Init */
	gp2x_video_init();

#ifdef GP2X
	{
		/* Border TV-Out */
		#define BORDERTVOUTSTRUCTURE "--border_tvout %d --border_tvout_width %d --border_tvout_height %d"
		extern char gp2x_path_mame[24];
		extern int gp2x_bordertvout;
		extern int gp2x_bordertvout_width;
		extern int gp2x_bordertvout_height;
		char name[256];
		FILE *f;
		sprintf(name,"%scfg/mame_tvout.cfg",gp2x_path_mame);
		f=fopen(name,"rb");
		if (f) {
			fscanf(f,BORDERTVOUTSTRUCTURE,&gp2x_bordertvout,&gp2x_bordertvout_width,&gp2x_bordertvout_height);
			fclose(f);
		
			if (gp2x_bordertvout) {
				printf("Border TV-Out Activated\r\n");
			} else {
				printf("Border TV-Out Disabled\r\n");
			}
		}
	}
#endif

	/* Set Video Mode */
#ifdef PSP_RES
	SetVideoScaling(480,480,272);
#else
	SetVideoScaling(320,320,240);
#endif

#ifdef GP2X
	/* Zaq121 07/05/2006 Alternative frontend support -> */
	/* Check for command line arguments */
	parse_cmdline (argc, argv, &options, game_index);
	if (no_selector==0)
	/* <- end mod */
#endif
	/* Intro screen */
	gp2x_intro_screen();

	/* Initialize Game Listings */
 	game_list_init();

	while(1) {

#ifdef GP2X
		/* Disable Scaling / Stretching */
		{ extern int gp2x_text_width; gp2x_text_width=320; }
		SetVideoScaling(320,320,240);
		gp2x_rotate=0;
		/* Mute Sound */
		gp2x_sound_thread_mute();
#endif
		
#ifdef GP2X
		/* Zaq121 07/05/2006 Alternative frontend support -> */
		if (no_selector==0)
		/* <- end mod */
#endif
		/* Select Game */
		select_game(playgame); 
#ifdef GP2X
		/* Zaq121 07/05/2006 Alternative frontend support -> */
   		else {
   			extern int gp2x_freq;   /* 9/5/06 Zaq, clock speed was never getting set */
      			int show_options(char *game);
      			int set_last_game_selected(char *game);
      			if(set_last_game_selected(playgame) == -1) break;  /* game not available */
      			if(!show_options(playgame)) break;   /* return back to external menu */
      			SetGP2XClock(gp2x_freq); /* 9/5/06 Zaq, clock speed was never getting set */
    		}
		/* <- end mod */
#endif
		/* Initialize the audio library */
		msdos_init_seal();
		
		/* Restore MAME Palette */
		gp2x_mame_palette();
		
		/* Set Log Messages start at row 0 */
		gp2x_gamelist_zero();

		/* Single Video Buffer for Log Messages */
		gp2x_clear_screen();

		game_index = -1;

		/* do we have a driver for this? */
		for (i = 0; drivers[i] && (game_index == -1); i++)
		{
			if (strcasecmp(playgame,drivers[i]->name) == 0)
			{
				game_index = i;
				break;
			}
		}
	
		if (game_index == -1)
		{
			sprintf(text,"Game \"%s\" not supported\0",playgame);
			gp2x_text_log(text);
			while(1); /* MAME never ends xD */
		}

		/* parse generic (os-independent) options */
		parse_cmdline (argc, argv, &options, game_index);
	
		{	/* Mish:  I need sample rate initialised _before_ rom loading for optional rom regions */
			extern int soundcard;

			if (soundcard == 0) {    /* silence, this would be -1 if unknown in which case all roms are loaded */
				Machine->sample_rate = 0; /* update the Machine structure to show that sound is disabled */
				options.samplerate=0;
			}
		}

		sprintf(text,"Loading \"%s\"...",drivers[game_index]->description);
		gp2x_text_log(text);
	
		/* go for it */
		filecache_init();
		res = run_game (game_index , &options);
		
#if !defined(GP2X) && defined(USE_FILECACHE)
		if ( res != 0)
		{
			gp2x_text_log("Retrying without filecache...");
			filecache_disable();
			res = run_game (game_index , &options);
		}
#endif
		/* Error? */
		if( res != 0 )
		{
#ifdef PSP
			gp2x_text_log("LOAD FAILED: Press [] button...");
#else
			gp2x_text_log("LOAD FAILED: Press A button...");
#endif
			while (!(gp2x_joystick_read()&GP2X_A));
			while ((gp2x_joystick_read()&GP2X_A));
		}

#ifdef GP2X
		/* Zaq121 07/05/2006 Alternative frontend support -> */
    		if(no_selector) break;
		/* <- end mod */
#endif

	} /* MAME never ends xD */
}
