#include "driver.h"
#include "gp2x_menu.h"

char mameversion[] = "0.34 ("__DATE__")";

static struct RunningMachine machine;
struct RunningMachine *Machine = &machine;
static const struct GameDriver *gamedrv;
static const struct MachineDriver *drv;

int nocheat;    /* 0 when the -cheat option was specified */

int frameskip;
int VolumePTR = 0;
static int settingsloaded;

unsigned char *ROM;


int init_machine(void);
void shutdown_machine(void);
int run_machine(void);
#if !defined(USE_CZ80) && !defined(USE_DRZ80) && !defined(USE_RAZE)
void fake_z80_freemem(void);
#else
#define fake_z80_freemem()
#endif


int run_game(int game, struct GameOptions *options)
{
	int err;

	Machine->gamedrv = gamedrv = drivers[game];
	Machine->drv = drv = gamedrv->drv;

	/* copy configuration */
	Machine->sample_rate = options->samplerate;
	Machine->sample_bits = options->samplebits;
	frameskip = options->frameskip;
	nocheat = 1;

	/* get orientation right */
	Machine->orientation = gamedrv->orientation;
	if (options->norotate)
		Machine->orientation = ORIENTATION_DEFAULT;
	if (options->ror)
	{
		/* if only one of the components is inverted, switch them */
		if ((Machine->orientation & ORIENTATION_ROTATE_180) == ORIENTATION_FLIP_X ||
				(Machine->orientation & ORIENTATION_ROTATE_180) == ORIENTATION_FLIP_Y)
			Machine->orientation ^= ORIENTATION_ROTATE_180;

		Machine->orientation ^= ORIENTATION_ROTATE_90;
	}
	if (options->rol)
	{
		/* if only one of the components is inverted, switch them */
		if ((Machine->orientation & ORIENTATION_ROTATE_180) == ORIENTATION_FLIP_X ||
				(Machine->orientation & ORIENTATION_ROTATE_180) == ORIENTATION_FLIP_Y)
			Machine->orientation ^= ORIENTATION_ROTATE_180;

		Machine->orientation ^= ORIENTATION_ROTATE_270;
	}
	if (options->flipx)
		Machine->orientation ^= ORIENTATION_FLIP_X;
	if (options->flipy)
		Machine->orientation ^= ORIENTATION_FLIP_Y;

#ifdef GP2X
	malloc_debug_init();
	malloc_debug_status(0);
#endif

	/* Do the work*/
	err = 1;

	if (init_machine() == 0)
	{
		if (osd_init() == 0)
		{
			if (run_machine() == 0)
				err = 0;
			else gp2x_text_log("Unable to start machine emulation\n");

			osd_exit();
		}
		else gp2x_text_log("Unable to initialize system\n");

		shutdown_machine();
	}
	else gp2x_text_log("Unable to initialize machine emulation\n");

#ifdef GP2X
	malloc_debug_status(1);
#endif
	return err;
}



/***************************************************************************

  Initialize the emulated machine (load the roms, initialize the various
  subsystems...). Returns 0 if successful.

***************************************************************************/
int init_machine(void)
{
	if (gamedrv->input_ports)
	{
		int total;
		const struct InputPort *from;
		struct InputPort *to;

		from = gamedrv->input_ports;

		total = 0;
		do
		{
			total++;
		} while ((from++)->type != IPT_END);

		if ((Machine->input_ports = (InputPort*)gp2x_malloc(total * sizeof(struct InputPort))) == 0)
			return 1;

		from = gamedrv->input_ports;
		to = Machine->input_ports;

		do
		{
			fast_memcpy(to,from,sizeof(struct InputPort));

			to++;
		} while ((from++)->type != IPT_END);
	}

	printf("Reading ROMs\n");	
	if (readroms() != 0)
	{
		gp2x_text_log("Error reading ROMs");
		printf("Error reading ROMs\n");
		gp2x_free(Machine->input_ports);
		return 1;
	}
	printf("ROMs Read\n");	

	{
		extern unsigned char *RAM;
		extern int encrypted_cpu;
		RAM = Machine->memory_region[drv->cpu[0].memory_region];
		ROM = RAM;
		encrypted_cpu = 0;
	}

	/* decrypt the ROMs if necessary */
	if (gamedrv->rom_decode) {
		gp2x_text_log("Decoding ROMs...");
		printf("Decoding Roms\n");
		(*gamedrv->rom_decode)();
	}

	if (gamedrv->opcode_decode)
	{
		int j;
		gp2x_text_log("Decoding Opcodes...");
		printf("Decoding Opcodes\n");

		/* find the first available memory region pointer */
		j = 0;
		while (Machine->memory_region[j]) j++;

		/* allocate a ROM array of the same length of memory region #0 */
		if ((ROM = (unsigned char*)gp2x_malloc(Machine->memory_region_length[0])) == 0)
		{
			gp2x_free(Machine->input_ports);
			/* TODO: should also gp2x_free the allocated memory regions */
			return 1;
		}

		Machine->memory_region[j] = ROM;
		Machine->memory_region_length[j] = Machine->memory_region_length[0];

		(*gamedrv->opcode_decode)();
	}


	/* read audio samples if available */
	printf("Readings Samples\n");
	Machine->samples = readsamples(gamedrv->samplenames,gamedrv->name);


	/* first of all initialize the memory handlers, which could be used by the */
	/* other initialization routines */
	gp2x_text_log("Initializing CPU...");
	printf("Initializing CPU\n");
	cpu_init();

	/* load input ports settings (keys, dip switches, and so on) */
	gp2x_text_log("Loading Input Ports Settings...");
	printf("Input Ports\n");
	settingsloaded = load_input_port_settings();

	/* ASG 971007 move from mame.c */
	gp2x_text_log("Initializing Memory Handlers...");
	if( !initmemoryhandlers() )
	{
		gp2x_free(Machine->input_ports);
		return 1;
	}

	gp2x_text_log("Driver Init...");
	if (gamedrv->driver_init) (*gamedrv->driver_init)();

	return 0;
}



void shutdown_machine(void)
{
	int i;

	/* free audio samples */
	freesamples(Machine->samples);
	Machine->samples = 0;

	/* ASG 971007 free memory element map */
	shutdownmemoryhandler();

	/* free the memory allocated for ROM and RAM */
	for (i = 0;i < MAX_MEMORY_REGIONS;i++)
	{
		if (Machine->memory_region[i])
		{
			gp2x_free(Machine->memory_region[i]);
			Machine->memory_region[i] = 0;
			Machine->memory_region_length[i] = 0;
		}
	}

	/* free the memory allocated for input ports definition */
	gp2x_free(Machine->input_ports);
	Machine->input_ports = 0;

	fake_z80_freemem();
}



static void vh_close(void)
{
	int i;


	for (i = 0;i < MAX_GFX_ELEMENTS;i++)
	{
		freegfx(Machine->gfx[i]);
		Machine->gfx[i] = 0;
	}
	osd_close_display();
	palette_stop();
}


#ifdef DREAMCAST
static unsigned extra_punt=0x04096000;
#endif

static int vh_open(void)
{
	int i;
	int visible_width;
	int visible_height;

	for (i = 0;i < MAX_GFX_ELEMENTS;i++) Machine->gfx[i] = 0;
	Machine->uifont = 0;

	if (palette_start() != 0)
	{
		vh_close();
		return 1;
	}

	/* convert the gfx ROMs into character sets. This is done BEFORE calling the driver's */
	/* convert_color_prom() routine (in palette_init()) because it might need to check the */
	/* Machine->gfx[] data */
	if (drv->gfxdecodeinfo)
	{
#if defined(DREAMCAST) && !defined(NO_DRIVER_TMNT)
		if (is_tmnt_driver(Machine->gamedrv))
		for (i = 0;i < MAX_GFX_ELEMENTS && drv->gfxdecodeinfo[i].memory_region != -1;i++)
		{
			int find_region=drv->gfxdecodeinfo[i].memory_region;
			const struct RomModule *romp = gamedrv->rom;
			int region;
			for (region = 0; romp->name || romp->offset || romp->length; region++)
			{
				if ((region==find_region)&&( ((unsigned)Machine->memory_region[region])>0x8c000000))
				{
					fast_memcpy((void *)extra_punt,Machine->memory_region[region],romp->offset & ~ROMFLAG_MASK);
					gp2x_free(Machine->memory_region[region]);
					Machine->memory_region[region]=(unsigned char*)extra_punt;
					extra_punt+=romp->offset & ~ROMFLAG_MASK;
				}
				do { romp++; } while (romp->length);
			}
		}
#endif
		for (i = 0;i < MAX_GFX_ELEMENTS && drv->gfxdecodeinfo[i].memory_region != -1;i++)
		{



                        int reglen = 8*Machine->memory_region_length[drv->gfxdecodeinfo[i].memory_region];
                        struct GfxLayout glcopy;
                        int j;


                        fast_memcpy(&glcopy,drv->gfxdecodeinfo[i].gfxlayout,sizeof(glcopy));

                        if (IS_FRAC(glcopy.total))
                                glcopy.total = reglen / glcopy.charincrement * FRAC_NUM(glcopy.total) / FRAC_DEN(glcopy.total);
                        for (j = 0;j < MAX_GFX_PLANES;j++)
                        {
                                if (IS_FRAC(glcopy.planeoffset[j]))
                                {
                                        glcopy.planeoffset[j] = FRAC_OFFSET(glcopy.planeoffset[j]) +
                                                        reglen * FRAC_NUM(glcopy.planeoffset[j]) / FRAC_DEN(glcopy.planeoffset[j]);
                                }
                        }
                        for (j = 0;j < MAX_GFX_SIZE;j++)
                        {
                                if (IS_FRAC(glcopy.xoffset[j]))
                                {
                                        glcopy.xoffset[j] = FRAC_OFFSET(glcopy.xoffset[j]) +
                                                        reglen * FRAC_NUM(glcopy.xoffset[j]) / FRAC_DEN(glcopy.xoffset[j]);
                                }
                                if (IS_FRAC(glcopy.yoffset[j]))
                                {
                                        glcopy.yoffset[j] = FRAC_OFFSET(glcopy.yoffset[j]) +
                                                        reglen * FRAC_NUM(glcopy.yoffset[j]) / FRAC_DEN(glcopy.yoffset[j]);
                                }
                        }


			if ((Machine->gfx[i] = decodegfx(Machine->memory_region[drv->gfxdecodeinfo[i].memory_region]
					+ drv->gfxdecodeinfo[i].start,
					&glcopy)) == 0)
			{
				vh_close();
				gp2x_text_log("Out of Memory: decodegfx()");
				return 1;
			}
			Machine->gfx[i]->colortable = &Machine->colortable[drv->gfxdecodeinfo[i].color_codes_start];
			Machine->gfx[i]->total_colors = drv->gfxdecodeinfo[i].total_color_codes;
		}
	}

	/* create the display bitmap, and allocate the palette */
	/* ppf 16-9-05 */
	visible_width=(drv->visible_area.max_x-drv->visible_area.min_x)+1;
	visible_height=(drv->visible_area.max_y-drv->visible_area.min_y)+1;
	if ((Machine->scrbitmap = osd_create_display(
			drv->screen_width,drv->screen_height,
			drv->video_attributes,
			visible_width,visible_height, /* Visible zone size */
			drv->visible_area.min_x, drv->visible_area.min_y)) == 0) /* Visible zone offset */
	{
		vh_close();
		return 1;
	}
	/* fin ppf 16-9-05 */

	/* initialize the palette - must be done after osd_create_display() */
	palette_init();

	return 0;
}


/***************************************************************************

  This function takes care of refreshing the screen, processing user input,
  and throttling the emulation speed to obtain the required frames per second.

***************************************************************************/

int updatescreen(void)
{
	int skipme = 1;

#ifndef GP2X
	static int framecount = 0;
	if (frameskip>=0)
	{
		/* see if we recomend skipping this frame */
		if (++framecount > frameskip)
		{
			framecount = 0;
			skipme = 0;
		}
	}
	else
		skipme=osd_update_frameskip();
#else
	skipme=osd_update_frameskip();
#endif

	/* if not, go for it */
	if (!skipme)
	{
		mame4all_prof_start(7);
		(*drv->vh_update)(Machine->scrbitmap,0);  /* update screen */
		mame4all_prof_end(7);
	}

	/* if the user pressed F3, reset the emulation */
	if (osd_key_pressed(OSD_KEY_RESET_MACHINE)) {
		while(osd_key_pressed(OSD_KEY_RESET_MACHINE)) { osd_poll_joystick(); }
		machine_reset();
	}

	if (osd_key_pressed(OSD_KEY_PAUSE)) /* pause the game */
	{
		int i;
#ifdef GP2X
		/* Mute Sound */
		gp2x_sound_thread_mute();
#else
		osd_sound_enable(0);
#endif

		for(i=0;i<10;i++) {
			if (osd_key_pressed(OSD_KEY_CANCEL)) return 1;
			if (!osd_key_pressed(OSD_KEY_PAUSE)) {osd_sound_enable(1); break;}
			osd_poll_joystick();
			gp2x_timer_delay(100);
		}
		
#if defined(GP2X) || defined(PSP)
		/* Screen change mode/orientation */
		if(i<10) {
			gp2x_rotate++;
			if (gp2x_rotate>3) gp2x_rotate=0;
			gp2x_adjust_display();
			gp2x_clear_screen();
			if (!skipme)
				osd_update_display();
			return 0;
		}
#endif

		osd_sound_enable(0);
#ifdef DREAMCAST		
		dc_change_to_normal();
#endif
		gp2x_text_pause();
#ifdef GP2X
		{ SetGP2XClock(66); }
#endif
#ifdef PSP
		{ SetGP2XClock(133); }
#endif
		while(osd_key_pressed(OSD_KEY_PAUSE)) {
			gp2x_timer_delay(100);
			osd_poll_joystick();
		}
#if defined(GP2X) || defined(PSP)
		{ extern int gp2x_freq; SetGP2XClock(gp2x_freq); }
#endif		
		/* Wait Unpause */
		while(!osd_key_pressed(OSD_KEY_PAUSE)) {
			if (osd_key_pressed(OSD_KEY_CANCEL)) return 1;
			gp2x_timer_delay(100);
			osd_poll_joystick();
		}
		while(osd_key_pressed(OSD_KEY_PAUSE)) {
			gp2x_timer_delay(100);
			osd_poll_joystick();
		}
#ifdef DREAMCAST		
		dc_change_to_hw_scaled();
#endif
		gp2x_clear_screen();	
		osd_sound_enable(1);
	}

	if (!skipme)
		osd_update_display();
	
	return 0;
}


/***************************************************************************

  Run the emulation. Start the various subsystems and the CPU emulation.
  Returns non zero in case of error.

***************************************************************************/
int run_machine(void)
{
	int res = 1;

	gp2x_text_log("Running Machine...");
#ifdef DREAMCAST
	extra_punt=0x04096000; //0x04100000;
#ifndef NO_DRIVER_CPS1
	if (!is_cps1_driver(Machine->gamedrv))
#endif
	{
		const struct RomModule *romp = gamedrv->rom;
		int     region;
		for (region = 0; romp->name || romp->offset || romp->length; region++)
		{
			if (romp->offset & ROMFLAG_DISPOSE)
			{
				fast_memcpy((void *)extra_punt,Machine->memory_region[region],romp->offset & ~ROMFLAG_MASK);
				gp2x_free(Machine->memory_region[region]);
				Machine->memory_region[region]=(unsigned char*)extra_punt;
				extra_punt+=romp->offset & ~ROMFLAG_MASK;
			}
			do { romp++; } while (romp->length);
		}
	}
#endif

	gp2x_text_log("Video Hardware...");

	if (vh_open() == 0)
	{
		tilemap_init();
		sprite_init();
		if (drv->vh_start == 0 || (*drv->vh_start)() == 0)      /* start the video hardware */
		{
#ifndef GP2X
			osd_stop_music();
			osd_reset_frameskip();
#endif

			gp2x_text_log("Audio Hardware...");
		
			if (sound_start() == 0) /* start the audio hardware */
			{
				const struct RomModule *romp = gamedrv->rom;
				int	region;

				/* free memory regions allocated with ROM_REGION_DISPOSE (typically gfx roms) */
				for (region = 0; romp->name || romp->offset || romp->length; region++)
				{
					if (romp->offset & ROMFLAG_DISPOSE)
					{
						gp2x_free (Machine->memory_region[region]);
						Machine->memory_region[region] = 0;
					}
					do { romp++; } while (romp->length);
				}

				gp2x_clear_screen(); /* Clear GP2X Screen */
				memalloc_report("RUN");
				cpu_run();      /* run the emulation! */

				/* the following MUST be done after hiscore_save() otherwise */
				/* some 68000 games will not work */
				sound_stop();
				if (drv->vh_stop) (*drv->vh_stop)();

				res = 0;
			}
			else gp2x_text_log("Unable to start audio emulation");
#ifndef GP2X
			osd_start_music();
#endif
		}
		else gp2x_text_log("Unable to start video emulation");

		sprite_close();
		tilemap_close();
		vh_close();
	}
	else gp2x_text_log("Unable to initialize display");

	return res;
}
