#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "driver.h"
#include "gp2x_mame.h"
#ifndef NO_IMAGES
 #ifdef PSP_RES
 #include "gfx/gp2xmenu480.h"
 #ifndef NO_SPLASH
 #include "gfx/gp2xsplash480.h"
 #endif
 #else
 #include "gfx/gp2xmenu.h"
 #ifndef NO_SPLASH
 #include "gfx/gp2xsplash.h"
 #endif
 #endif
#else
 unsigned char* gp2xmenu_bmp;
 unsigned char* gp2xsplash_bmp;
#endif

#ifdef PSP
#include "pspincludes.h"
//#define PrintMessage printf
#endif

#ifndef MAME4ALL_CPU_CLOCK
#define MAME4ALL_CPU_CLOCK 70
#endif

#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

static int game_num=2048;
static int game_num_avail=0;
static char game_avail[2048];
static int last_game_selected=0;

extern int gp2x_frameskip;
extern int gp2x_frameskip_auto;
extern int gp2x_double_buffer;
extern int gp2x_vsync;
int gp2x_sound_enable = 1;
#if MAME4ALL_CPU_CLOCK != 100
int gp2x_clock_cpu=(MAME4ALL_CPU_CLOCK+10); 			// Clock option for CPU's 
int gp2x_clock_sound=(MAME4ALL_CPU_CLOCK-10); 		// Clock option for Audio CPU's 
#else
int gp2x_clock_cpu=100;
int gp2x_clock_sound=100;
#endif
#ifdef PSP
int gp2x_freq=222;			// PSP Clock Frequency 
#else
int gp2x_freq=200;			// GP2X Clock Frequency 
#endif
int gp2x_save_config=0;			// Save Configuration 

extern int x_sensitivity;
extern int y_sensitivity;
extern int x_reversed;
extern int y_reversed;
extern struct KeySettings *key_settings;

extern UINT32 chk_mem_start_size, chk_mem_end_size;

char gp2x_path_mame[24]="\0";

UINT32 check_free_memory(void)
{
	UINT8 *mem;
	int size_mb = 0;
	int size_kb = 0;
	int size_b  = 0;

	while (1)
	{
		if ((mem = (UINT8*)malloc(size_mb)) == NULL)
			break;

		free(mem);
		size_mb += 1024*1024;
	}
	while (1)
	{
		if ((mem = (UINT8*)malloc(size_mb + size_kb)) == NULL)
			break;

		free(mem);
		size_kb += 1024;
	}
	while (1)
	{
		if ((mem = (UINT8*)malloc(size_mb + size_kb + size_b)) == NULL)
			break;

		free(mem);
		size_b += 100;
	}
	while (1)
	{
		if ((mem = (UINT8*)malloc(size_mb + size_kb + size_b)) == NULL)
			break;

		free(mem);
		size_b++;
	}
	return size_mb + size_kb + size_b;
}

void load_bmp_8bpp(unsigned char *out, unsigned char *in) 
{
#ifndef NO_IMAGES
	int i,x,y;
	unsigned char r,g,b,c;

	in+=14; /* Skip HEADER */
	in+=40; /* Skip INFOHD */
	
	/* Set Palette */
	for (i=0;i<256;i++) {
		b=*in++;
		g=*in++;
		r=*in++;
		c=*in++;
		gp2x_video_color8(i,r,g,b);
	}
	gp2x_video_setpalette();
	/* Set Bitmap */	
 #ifdef PSP_RES
	for (y=271;y!=-1;y--) {
		for (x=0;x<480;x++) {
			*out++=in[x+y*480];
		}
	}
 #else
	for (y=239;y!=-1;y--) {
		for (x=0;x<320;x++) {
			*out++=in[x+y*320];
		}
	}
 #endif
#else
	int i,x,y;
	unsigned char r,g,b,c;
	/* font face color */
	gp2x_video_color8(255,255,255,255);
	/* font shadow color */
	gp2x_video_color8(0,0,0,0);
	gp2x_video_color8(1,0,0,0);

	/* remaining palette */
	for (i=2;i<255;i++) {
		b=255-i;
		g=0;
		r=0;
		c=0;
		gp2x_video_color8(i,r,g,b);
	}
	gp2x_video_setpalette();
	/* Set Bitmap */	
 #ifdef PSP_RES
	for (y=271;y!=-1;y--) {
		for (x=0;x<480;x++) {
			*out++=(unsigned char)(x/2+3)&254;
		}
	}	
	gp2x_gamelist_text_out_fmt(150,10,"-= Mame4All " VERSION " =-");
	gp2x_gamelist_text_out_fmt(118,20,"Largest memory chunk: %u",check_free_memory());
 #else
	for (y=239;y!=-1;y--) {
		for (x=0;x<320;x++) {
			*out++=(unsigned char)(x/1.25+1)&254;
		}
	}
	gp2x_gamelist_text_out_fmt(66,10,"-= Mame4All " VERSION " =-");
	gp2x_gamelist_text_out_fmt(46,20,"Largest memory chunk: %u",check_free_memory());
 #endif
#endif
}

void gp2x_intro_screen(void) {
#ifndef NO_IMAGES
	char name[256];
	FILE *f;
	gp2x_video_flip();

#ifdef GP2X
	/* Problem with relative paths in some GP2X? */
	f=fopen("/mnt/sd/mamegp2x/mame_gp2x.gpe","rb");
	if (f) {
		fclose(f);
		strcpy(gp2x_path_mame,"/mnt/sd/mamegp2x/\0");
	}
#endif

#ifdef PSP
	strcpy(gp2x_path_mame,"");
#endif

#ifndef NO_SPLASH
#ifdef PSP
#ifdef PSP_RES
	sprintf(name,"%sskins/pspsplash480.bmp",gp2x_path_mame);
#else
	sprintf(name,"%sskins/pspsplash.bmp",gp2x_path_mame);
#endif
#else
	sprintf(name,"%sskins/gp2xsplash.bmp",gp2x_path_mame);
#endif
	f=fopen(name,"rb");
	if (f) {
#ifdef PSP_RES
		fread(gp2xsplash_bmp,1,1078+480*272,f);
#else
		fread(gp2xsplash_bmp,1,77878,f);
#endif
		fclose(f);
	}
	load_bmp_8bpp(gp2x_screen8,gp2xsplash_bmp);
#endif
	gp2x_video_flip();
	
#ifdef PSP
#ifdef PSP_RES
	sprintf(name,"%sskins/pspmenu480.bmp",gp2x_path_mame);
#else
	sprintf(name,"%sskins/pspmenu.bmp",gp2x_path_mame);
#endif
#else
	sprintf(name,"%sskins/gp2xmenu.bmp",gp2x_path_mame);
#endif
	f=fopen(name,"rb");
	if (f) {
#ifdef PSP_RES
		fread(gp2xmenu_bmp,1,1078+480*272,f);
#else
		fread(gp2xmenu_bmp,1,77878,f);
#endif
		fclose(f);
	}
#if !defined(GP2X) && !defined(NO_SPLASH)
	gp2x_timer_delay(2000);
	{
		int i,j;
		unsigned char *pal=&gp2xsplash_bmp[40+14];
		unsigned char *dat=&gp2xsplash_bmp[40+14+(256*4)];

#ifdef PSP_RES
		for(i=0;i<480*272;i++)
#else
		for(i=0;i<320*240;i++)
#endif
		{
			unsigned char b=pal[(dat[i]*4)+0];
			unsigned char g=pal[(dat[i]*4)+1];
			unsigned char r=pal[(dat[i]*4)+2];
			unsigned med=r; med+=g; med+=b; med/=3;
			dat[i]=128+(med>>1)-1;
		}

		pal[0]=pal[1]=pal[2]=0;
		for(i=1;i<255;i++)
		{
			pal[(i*4)+0]=pal[(i*4)+1]=pal[(i*4)+2]=255-i;
#ifdef MAME4ALL_GOLD
			if ((((unsigned)pal[(i*4)+1])<<1)<255) pal[(i*4)+1]<<=1; 
#else
#ifdef MAME4ALL_CLASSIC
			if ((((unsigned)pal[(i*4)+2])<<1)<255) pal[(i*4)+2]<<=1; 
#else
			if ((((unsigned)pal[(i*4)+0])<<1)<255) pal[(i*4)+0]<<=1; 
#endif
#endif
		}
		pal[(255*4)+0]=pal[(255*4)+1]=pal[(255*4)+2]=255;
	}
#endif
#endif
}
static void dipswitches_config(void)
{
	struct InputPort *inp=NULL;					/* input ports */
	int i;								/* Index into the input ports */
	int x_Pos;							/* Position X in the screen to print */
	int y_Pos;							/* Position Y in the screen to print */
	int index=0;							/* Index into the dipswitches to modify */
	int num_items;							/* Total number of dipswitches */
	const char *name;						/* Name of the dipswitch */
	unsigned long ExKey=0;						/* Key status */
	unsigned short *value_change;					/* Pointer of the value to be changed */
	int value_count;						/* Number of values */
	unsigned short value_range[32];					/* Different values */
	unsigned short value;						/* Current value*/
	unsigned short value_count_m;					/* Fire values count */

	/* Get input ports of the selected game */
	index=0;
	for (i=0;i<game_num;i++) {
		if (game_avail[i]==1) {
			if(index==last_game_selected) {
				inp=drivers[i]->input_ports;
			}
			index++;
		}
	}
	if (!inp) return;

	i=0;
	index=0;
	while(1)
	{
		/* Draw background image */
		load_bmp_8bpp(gp2x_screen8,gp2xmenu_bmp);
	
		i=0;
		x_Pos=41;
		y_Pos=48;
		num_items=0;

		/* Show currently selected item */
		gp2x_gamelist_text_out(x_Pos-16,y_Pos+(index*8)," >");

		/* Show Dipswitches configuration */	
		while (inp[i].type!= IPT_END)
		{
			/* Dipswitch Name */
			if ( (inp[i].type & ~IPF_MASK) == IPT_DIPSWITCH_NAME && inp[i].name!=0 && (inp[i].type & IPF_UNUSED) == 0 ) {
				name=inp[i].name;
				value=inp[i].default_value;
				if (index==num_items) {
					value_change = &(inp[i].default_value);
					value_count=0;
					value_count_m=1;
				} else
					value_count_m=0;
			}
			/* Cheat Name */
			if ( (inp[i].type & ~IPF_MASK) == (IPT_DIPSWITCH_NAME | IPF_CHEAT) && inp[i].name!=0 && (inp[i].type & IPF_UNUSED) == 0 ) {
				name=inp[i].name;
				value=inp[i].default_value;
				if (index==num_items) {
					value_change = &(inp[i].default_value);
					value_count=0;
					value_count_m=1;
				} else
					value_count_m=0;
			}
			/* Dipswitch Setting */
			if ((inp[i].type & ~IPF_MASK) == IPT_DIPSWITCH_SETTING) {
				if (value_count_m) {
					value_range[value_count]=inp[i].default_value;
					value_count++;
				}
				if (value==inp[i].default_value) {
					/* printf("%s: %s\n",name,inp[i].name); */
					gp2x_gamelist_text_out_fmt(x_Pos,y_Pos,"%s: %s",name,inp[i].name);
					y_Pos+=8;
					num_items++;
				}
			}
			i++;			
		}	

#ifdef PSP
		gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+8,"Press [] or O to confirm");	
#else
		gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+8,"Press A or B to confirm");	
#endif
	
		gp2x_video_flip();

		while(gp2x_joystick_read()&0x8c0ff55) { gp2x_timer_delay(150); }
		while(!(ExKey=gp2x_joystick_read()&0x8c0ff55)) { }
		if(ExKey & GP2X_DOWN){
			index++;
			index = index % num_items;
		}
		else if(ExKey & GP2X_UP){
			index--;
			if(index<0)
				index = num_items - 1;
		}
#ifdef GP2X
		else if(ExKey & GP2X_R || ExKey & GP2X_L){
#else
		else if(ExKey & GP2X_RIGHT || ExKey & GP2X_LEFT){
#endif
			/* Modify dipswitch */
			for(i=0;i<value_count;i++) {
				if (value_range[i]==*value_change) break;	
			}
			if (i==value_count-1)
				*value_change=value_range[0];
			if (i<value_count-1)
				*value_change=value_range[i+1];
		}

		if ((ExKey & GP2X_A) || (ExKey & GP2X_PUSH) || (ExKey & GP2X_START)) {
			return;
		}
		else if ((ExKey & GP2X_B) || (ExKey & GP2X_SELECT)){
			return;
		}
	}
}

/* Initialize the list of available games in the SD */
#ifdef USE_OLD_LIST_INIT
void game_list_init(void) {
#ifdef GP2X
  	/* Zaq121 07/05/2006 Alternative frontend support -> */
  	extern int no_selector;
  	if(no_selector) {
  		int c;
		for (c=0;(c<game_num && drivers[c]);c++) {
	      		game_avail[c]=1;
	      		game_num_avail++;
	    	}
	    	return;
	}
	/* <- end mod */
#endif
#ifdef PSP
 strcpy(gp2x_path_mame,"");     
#ifdef WIFI 
 //DownloadFileFTP("roms.txt","roms.txt");

 long lSize;
 char * buffer;
 
 FILE * pFile = fopen ("roms.txt","rb");
 if (pFile!=NULL)
    {
    fseek (pFile , 0 , SEEK_END);
    lSize = ftell (pFile);
    rewind (pFile);
    
    buffer = (char*)gp2x_malloc(lSize);
    fread (buffer,1,lSize,pFile);
    }

#endif
#endif
	char name[256];
	struct stat stat_buffer;
	unsigned int c;
	/* Check Available Games */
	for (c=0;(c<game_num && drivers[c]);c++) {

		/*
		sprintf(name,"%sroms/%s\0", gp2x_path_mame, drivers[c]->name);
		if (stat(name,&stat_buffer)==0)	{	
			game_avail[c]=1;
			game_num_avail++;
			continue;
		}
		*/
		
#ifdef WIFI
  //PrintMessage("cargando lista %s",drivers[c]->name);
  sprintf(name,"%s.zip",drivers[c]->name);
  if(strstr(buffer, name)){
  //if(drivers[c]->name == "captcomm"){
#else
		sprintf(name,"%sroms/%s.zip\0", gp2x_path_mame, drivers[c]->name);
		if (stat(name,&stat_buffer)==0)	{
#endif		
			game_avail[c]=1;
			game_num_avail++;
			continue;
		}
	}
#ifdef WIFI	
 sceKernelDelayThread(2*1000000); // 2 sec to for error?
 fclose (pFile);
 gp2x_free (buffer);
#endif
}
#else
void game_list_init(void) {
#ifdef GP2X
  	/* Zaq121 07/05/2006 Alternative frontend support -> */
  	extern int no_selector;
  	if(no_selector) {
  		int c;
		for (c=0;(c<game_num && drivers[c]);c++) {
	      		game_avail[c]=1;
	      		game_num_avail++;
	    	}
	    	return;
	}
	/* <- end mod */
#endif
#ifdef DREAMCAST
	DIR *d=opendir(ROM_PREFIX "roms");
#else
#ifdef GP2X
	char name[256];
	sprintf(name,"%sroms\0",gp2x_path_mame);
	DIR *d=opendir(name);
#else
	DIR *d=opendir("roms");
#endif
#endif
	if (d)
	{
		struct dirent *actual=readdir(d);
		int min=0;
		int max_total, max;
		int i;
		char **games=(char **)gp2x_malloc(game_num*sizeof(char *));
		for(i=0;i<game_num && drivers[i];i++)
		{
			games[i]=(char *)gp2x_malloc(32);
			strncpy(games[i],drivers[i]->name,32-5);
			strcat(games[i],".zip\0");
		}
		max=max_total=i;
		while(actual)
		{
			int encontrado=0;
			for(i=min;i<max;i++)
				if (!strcmp(actual->d_name,games[i])){
					game_avail[i]=1;
					game_num_avail++;
					if (i==min)
						min=i+1;
					else
					if (i==max)
						max=i-1;
//					puts(drivers[i]->description);
					encontrado=1;
					break;
				}
#if !defined(GP2X) && !defined(DREMCAST)
/*
			if ((!encontrado)&&(actual->d_name[0]!='.'))
				printf("SOBRA:%s\n",actual->d_name);
*/
#endif
			actual=readdir(d);
		}
		for(i=0;i<max_total;i++)
		{
#if !defined(GP2X) && !defined(DREMCAST)
/*
			if (!game_avail[i])
				printf("FALTA:%s\n",games[i]);
*/
#endif
			gp2x_free(games[i]);
		}
//		printf("TOTAL %i roms\n",max_total);
		gp2x_free(games);
		closedir(d);
	}
}
#endif

#ifdef GP2X
/* Zaq121 07/05/2006 Alternative frontend support -> */
int set_last_game_selected(char *game)
{
  	int i;
  	last_game_selected=0;
  	for (i=0;i<game_num;i++) {
    		if (game_avail[i]==1)
    		{
      			if(strcasecmp(drivers[i]->name, game) == 0)
        			return(last_game_selected);
      			last_game_selected++;
    		}
  	}
  	return -1;
}
/* <- end mod */
#endif

static void game_list_view(int *pos) {

	int i;
	int view_pos;
	int aux_pos=0;
	int screen_y = 45;
	int screen_x = 38;

	/* Draw background image */
	load_bmp_8bpp(gp2x_screen8,gp2xmenu_bmp);

	/* Check Limits */
	if (*pos<0)
		*pos=game_num_avail-1;
	if (*pos>(game_num_avail-1))
		*pos=0;
					   
	/* Set View Pos */
	if (*pos<10) {
		view_pos=0;
	} else {
		if (*pos>game_num_avail-11) {
			view_pos=game_num_avail-21;
			view_pos=(view_pos<0?0:view_pos);
		} else {
			view_pos=*pos-10;
		}
	}

	/* Show List */
	for (i=0;i<game_num;i++) {
		if (game_avail[i]==1) {
			if (aux_pos>=view_pos && aux_pos<=view_pos+20) {
				gp2x_gamelist_text_out( screen_x, screen_y, (char *)drivers[i]->description );
				if (aux_pos==*pos) {
					gp2x_gamelist_text_out( screen_x-10, screen_y,">" );
					gp2x_gamelist_text_out( screen_x-13, screen_y-1,"-" );
				}
				screen_y+=8;
			}
			aux_pos++;
		}
	}

}

static void game_list_select (int index, char *game) {
	int i;
	int aux_pos=0;
	for (i=0;i<game_num;i++) {
		if (game_avail[i]==1) {
			if(aux_pos==index) {
				strcpy(game,drivers[i]->name);
			}
			aux_pos++;
		   }
	}
}

static char *game_list_description (int index) {
	int i;
	int aux_pos=0;
	for (i=0;i<game_num;i++) {
		if (game_avail[i]==1) {
			if(aux_pos==index) {
				return((char *)drivers[i]->description);
			}
			aux_pos++;
		   }
	}
	return ((char *)0);
}

static void button_text(unsigned long button,char *text)
{
	if (button==GP2X_A) {
#ifdef GP2X
		strcpy(text,"BUTTON A\0");
#else
#ifdef PSP
		strcpy(text,"BUTTON []\0");
#else
		strcpy(text,"BUTTON X\0");
#endif
#endif
	} else if (button==GP2X_X) {
#ifdef GP2X
		strcpy(text,"BUTTON X\0");
#else
#ifdef PSP
		strcpy(text,"BUTTON X\0");
#else
		strcpy(text,"BUTTON A\0");
#endif
#endif
	} else if (button==GP2X_B) {
#ifdef PSP
		strcpy(text,"BUTTON O\0");
#else
		strcpy(text,"BUTTON B\0");
#endif
	} else if (button==GP2X_Y) {
#ifdef PSP
		strcpy(text,"BUTTON ^\0");
#else
		strcpy(text,"BUTTON Y\0");
#endif
	} else if (button==GP2X_L) {
		strcpy(text,"BUTTON L\0");
	} else if (button==GP2X_R) {
		strcpy(text,"BUTTON R\0");
	} else if (button==GP2X_VOL_UP) {
		strcpy(text,"VOLUME UP\0");
	} else if (button==GP2X_VOL_DOWN) {
		strcpy(text,"VOLUME DOWN\0");
	} else if (button==GP2X_PUSH) {
		strcpy(text,"JOYPAD PUSH\0");
	} else if (button==GP2X_START) {
		strcpy(text,"START\0");
	} else if (button==GP2X_SELECT) {
		strcpy(text,"SELECT\0");
	}
}

static unsigned long button_next(unsigned long button,int next)
{
	if (button==GP2X_A) {
		if (next) return GP2X_B; else return GP2X_SELECT;
	} else if (button==GP2X_B) {
		if (next) return GP2X_X; else return GP2X_A;
	} else if (button==GP2X_X) {
		if (next) return GP2X_Y; else return GP2X_B;
	} else if (button==GP2X_Y) {
		if (next) return GP2X_L; else return GP2X_X;
	} else if (button==GP2X_L) {
		if (next) return GP2X_R; else return GP2X_Y;
	} else if (button==GP2X_R) {
		if (next) return GP2X_VOL_UP; else return GP2X_L;
	} else if (button==GP2X_VOL_UP) {
		if (next) return GP2X_VOL_DOWN; else return GP2X_R;
	} else if (button==GP2X_VOL_DOWN) {
		if (next) return GP2X_PUSH; else return GP2X_VOL_UP;
	} else if (button==GP2X_PUSH) {
		if (next) return GP2X_START; else return GP2X_VOL_DOWN;
	} else if (button==GP2X_START) {
		if (next) return GP2X_SELECT; else return GP2X_PUSH;
	} else if (button==GP2X_SELECT) {
		if (next) return GP2X_A; else return GP2X_START;
	}
	return 0;
}

static void controller_config(void)
{

	unsigned long ExKey=0;
	int selected_option=0;
	int x_Pos = 41;
	int y_Pos = 48;
	int options_count = 18;
	char text[64];
	int i;
	char* options[] = 		{	"FIRE01 Auto    = %s",
						"FIRE02 Auto    = %s",
						"FIRE03 Auto    = %s",
						"FIRE01         = %s",
						"FIRE02         = %s",
						"FIRE03         = %s",
						"FIRE04         = %s",
						"FIRE05         = %s",
						"FIRE06         = %s",
						"FIRE07         = %s",
						"FIRE08         = %s",
						"FIRE09         = %s",
						"FIRE10         = %s",
						"X Sensitivity  = %s",
						"X Reversed     = %s",
						"Y Sensitivity  = %s",
						"Y Reversed     = %s",
						"Back to Default Values"};
	
	while (1)
	{
		/* Draw background image */
		load_bmp_8bpp(gp2x_screen8,gp2xmenu_bmp);
		
		/* Draw the options */
		gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+0,options[0],key_settings->JOY_FIRE1_AUTO?"ON\0":"OFF\0");
		gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+8,options[1],key_settings->JOY_FIRE2_AUTO?"ON\0":"OFF\0");
		gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+16,options[2],key_settings->JOY_FIRE3_AUTO?"ON\0":"OFF\0");
		button_text(key_settings->JOY_FIRE1,text);
		gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+24,options[3],text);
		button_text(key_settings->JOY_FIRE2,text);
		gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+32,options[4],text);
		button_text(key_settings->JOY_FIRE3,text);
		gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+40,options[5],text);
		button_text(key_settings->JOY_FIRE4,text);
		gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+48,options[6],text);
		button_text(key_settings->JOY_FIRE5,text);
		gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+56,options[7],text);
		button_text(key_settings->JOY_FIRE6,text);
		gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+64,options[8],text);
		button_text(key_settings->JOY_FIRE7,text);
		gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+72,options[9],text);
		button_text(key_settings->JOY_FIRE8,text);
		gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+80,options[10],text);
		button_text(key_settings->JOY_FIRE9,text);
		gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+88,options[11],text);
		button_text(key_settings->JOY_FIRE10,text);
		gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+96,options[12],text);
		if (x_sensitivity!=-1)
			sprintf(text,"%d",x_sensitivity);
		else
			strcpy(text,"DEFAULT");
		gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+104,options[13],text);
		if (x_reversed)
			strcpy(text,"ON");
		else
			strcpy(text,"OFF");
		gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+112,options[14],text);
		if (y_sensitivity!=-1)
			sprintf(text,"%d",y_sensitivity);
		else
			strcpy(text,"DEFAULT");
		gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+120,options[15],text);
		if (y_reversed)
			strcpy(text,"ON");
		else
			strcpy(text,"OFF");
		gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+128,options[16],text);
		gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+136,options[17]);
		
#ifdef PSP
		gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+152,"Press [] or O to confirm");	
#else
		gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+152,"Press A or B to confirm");	
#endif
		/* Show currently selected item */
		gp2x_gamelist_text_out(x_Pos-16,y_Pos+(selected_option*8)," >");

		gp2x_video_flip();
		while(gp2x_joystick_read()&0x8c0ff55) { gp2x_timer_delay(150); }
		while(!(ExKey=gp2x_joystick_read()&0x8c0ff55)) { }

		if(ExKey & GP2X_DOWN){
			selected_option++;
			selected_option = selected_option % options_count;
		}
		else if(ExKey & GP2X_UP){
			selected_option--;
			if(selected_option<0)
				selected_option = options_count - 1;
		}
#ifdef GP2X
		else if(ExKey & GP2X_R || ExKey & GP2X_L){
#else
		else if(ExKey & GP2X_RIGHT || ExKey & GP2X_LEFT){
#endif
			switch(selected_option) {
				
				case 0: key_settings->JOY_FIRE1_AUTO=key_settings->JOY_FIRE1_AUTO?0:1; break;
				case 1: key_settings->JOY_FIRE2_AUTO=key_settings->JOY_FIRE2_AUTO?0:1; break;
				case 2: key_settings->JOY_FIRE3_AUTO=key_settings->JOY_FIRE3_AUTO?0:1; break;
				case 3: key_settings->JOY_FIRE1=button_next((key_settings->JOY_FIRE1),(ExKey&GP2X_R)); break;
				case 4: key_settings->JOY_FIRE2=button_next((key_settings->JOY_FIRE2),(ExKey&GP2X_R)); break;
				case 5: key_settings->JOY_FIRE3=button_next((key_settings->JOY_FIRE3),(ExKey&GP2X_R)); break;
				case 6: key_settings->JOY_FIRE4=button_next((key_settings->JOY_FIRE4),(ExKey&GP2X_R)); break;
				case 7: key_settings->JOY_FIRE5=button_next((key_settings->JOY_FIRE5),(ExKey&GP2X_R)); break;
				case 8: key_settings->JOY_FIRE6=button_next((key_settings->JOY_FIRE6),(ExKey&GP2X_R)); break;
				case 9: key_settings->JOY_FIRE7=button_next((key_settings->JOY_FIRE7),(ExKey&GP2X_R)); break;
				case 10: key_settings->JOY_FIRE8=button_next((key_settings->JOY_FIRE8),(ExKey&GP2X_R)); break;
				case 11: key_settings->JOY_FIRE9=button_next((key_settings->JOY_FIRE9),(ExKey&GP2X_R)); break;
				case 12: key_settings->JOY_FIRE10=button_next((key_settings->JOY_FIRE10),(ExKey&GP2X_R)); break;
				case 13:
					/* X Sensitivity */
#ifdef GP2X
					if (ExKey & GP2X_R) {
#else
					if (ExKey & GP2X_RIGHT) {
#endif
						if (x_sensitivity!=-1) {
							x_sensitivity+=5;
							if (x_sensitivity>255) x_sensitivity=-1;
						} else
							x_sensitivity=0;
					} else {
						if (x_sensitivity!=-1) {
							x_sensitivity-=5;
							if (x_sensitivity<0) x_sensitivity=-1;
						} else
							x_sensitivity=255;
					}
					break;
				case 14:
					/* X Reversed */
					x_reversed=!x_reversed;
					break;
				case 15:
					/* Y Sensitivity */
#ifdef GP2X
					if (ExKey & GP2X_R) {
#else
					if (ExKey & GP2X_RIGHT) {
#endif
						if (y_sensitivity!=-1) {
							y_sensitivity+=5;
							if (y_sensitivity>255) y_sensitivity=-1;
						} else
							y_sensitivity=0;
					} else {
						if (y_sensitivity!=-1) {
							y_sensitivity-=5;
							if (y_sensitivity<0) y_sensitivity=-1;
						} else
							y_sensitivity=255;
					}
					break;
				case 16:
					/* Y Reversed */
					y_reversed=!y_reversed;
					break;
				case 17:
					/* Default Values */
					key_settings->JOY_FIRE1=GP2X_A;
					key_settings->JOY_FIRE2=GP2X_X;
					key_settings->JOY_FIRE3=GP2X_B;
					key_settings->JOY_FIRE4=GP2X_Y;
					key_settings->JOY_FIRE5=GP2X_L;
					key_settings->JOY_FIRE6=GP2X_R;
					key_settings->JOY_FIRE7=GP2X_VOL_UP;
					key_settings->JOY_FIRE8=GP2X_VOL_DOWN;
					key_settings->JOY_FIRE9=GP2X_PUSH;
					key_settings->JOY_FIRE10=GP2X_START;
					key_settings->JOY_FIRE1_AUTO=0;
					key_settings->JOY_FIRE2_AUTO=0;
					key_settings->JOY_FIRE3_AUTO=0;
					x_sensitivity=-1;
					y_sensitivity=-1;
					x_reversed=0;
					y_reversed=0;
					break;
			}
		}
		
		if ((ExKey & GP2X_A) || (ExKey & GP2X_PUSH) || (ExKey & GP2X_START)) {
			return;
		}
		else if ((ExKey & GP2X_B) || (ExKey & GP2X_SELECT)){
			return;
		}
	}
}

#ifdef PSP
//fixed gp3x_frameskip 
#define SAVEDATASTRUCTURE "gp2x_freq=%d;gp2x_frameskip=%d;gp2x_frameskip_auto=%d;gp2x_clock_cpu=%d;gp2x_sound_enable=%d;gp2x_clock_sound=%d;key_setting=%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%d,%d,%d,%d;gp2x_rotate=%d;gp2x_vsync=%d;gp2x_autofire=%d,%d,%d;\0"
#else
#define SAVEDATASTRUCTURE "gp2x_freq=%d;gp3x_frameskip=%d;gp2x_frameskip_auto=%d;gp2x_clock_cpu=%d;gp2x_sound_enable=%d;gp2x_clock_sound=%d;key_setting=%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%d,%d,%d,%d;gp2x_rotate=%d;gp2x_vsync=%d;gp2x_autofire=%d,%d,%d;\0"
#endif
#define SAVEDATAFILEDEF	"%scfg/mame.cfg\0"
#define SAVEDATAFILECFG "%scfg/%s.cfg\0"
/*static*/ int show_options(char *game)
{
	unsigned long ExKey=0;
	int selected_option=0;
	int x_Pos = 41;
	int y_Pos = 58;
#ifdef PSP
	int options_count = 10;
#else
	int options_count = 9;
#endif
	char text[256];
	FILE *f;

	/* Load Configuration Files */
	gp2x_save_config = 0;
#ifdef PSP
  strcpy(gp2x_path_mame,"");
#endif	
	sprintf(text,SAVEDATAFILEDEF, gp2x_path_mame);
	f=fopen(text,"r");
	if (f) {
		fscanf(f,SAVEDATASTRUCTURE,
		&gp2x_freq,&gp2x_frameskip,&gp2x_frameskip_auto,&gp2x_clock_cpu,&gp2x_sound_enable,&gp2x_clock_sound,
		&(key_settings->JOY_FIRE1),&(key_settings->JOY_FIRE2),&(key_settings->JOY_FIRE3),&(key_settings->JOY_FIRE4),
		&(key_settings->JOY_FIRE5),&(key_settings->JOY_FIRE6),&(key_settings->JOY_FIRE7),&(key_settings->JOY_FIRE8),
		&(key_settings->JOY_FIRE9),&(key_settings->JOY_FIRE10),&x_sensitivity,&x_reversed,&y_sensitivity,&y_reversed,
		&gp2x_rotate,&gp2x_vsync,&(key_settings->JOY_FIRE1_AUTO),&(key_settings->JOY_FIRE2_AUTO),&(key_settings->JOY_FIRE3_AUTO));
		fclose(f);
	}
	sprintf(text,SAVEDATAFILECFG, gp2x_path_mame, game);
	f=fopen(text,"r");
	if (f) {
		fscanf(f,SAVEDATASTRUCTURE,
		&gp2x_freq,&gp2x_frameskip,&gp2x_frameskip_auto,&gp2x_clock_cpu,&gp2x_sound_enable,&gp2x_clock_sound,
		&(key_settings->JOY_FIRE1),&(key_settings->JOY_FIRE2),&(key_settings->JOY_FIRE3),&(key_settings->JOY_FIRE4),
		&(key_settings->JOY_FIRE5),&(key_settings->JOY_FIRE6),&(key_settings->JOY_FIRE7),&(key_settings->JOY_FIRE8),
		&(key_settings->JOY_FIRE9),&(key_settings->JOY_FIRE10),&x_sensitivity,&x_reversed,&y_sensitivity,&y_reversed,
		&gp2x_rotate,&gp2x_vsync,&(key_settings->JOY_FIRE1_AUTO),&(key_settings->JOY_FIRE2_AUTO),&(key_settings->JOY_FIRE3_AUTO));
		fclose(f);
	}


	while(1)
	{
		/* Draw background image */
		load_bmp_8bpp(gp2x_screen8,gp2xmenu_bmp);

		/* Draw the options */
		gp2x_gamelist_text_out(x_Pos,y_Pos,"Selected Game:\0");
		fast_memcpy (text,game_list_description(last_game_selected),33);
		text[32]='\0';
		gp2x_gamelist_text_out(x_Pos,y_Pos+10,text);
		/*gp2x_gamelist_text_out(x_Pos,y_Pos+10,game_list_description(last_game_selected));*/

#ifdef GP2X
		/* GP2X Clock*/
		gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30, "GP2X Clock    %d MHz", gp2x_freq);
		
		/* Frame-Skip */
		if(gp2x_frameskip_auto && gp2x_frameskip!=0) {
			gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+40,"Frame-Skip    %s %d %s","<=",gp2x_frameskip, "(Auto-skip)");
		}
		else{
			gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+40,"Frame-Skip    %s %d %s","=",gp2x_frameskip,"");
		}
#else
#ifdef PSP
		switch (gp2x_rotate)
		{
			case 0: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video         %s","Fixed"); break;
			case 1: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video         %s","Fixed DIV2"); break;
			case 2: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video         %s","SW Scaled"); break;
			default: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video         %s","SW Streched"); break;
		}
		/* Frame-Skip */
		if(gp2x_frameskip_auto && gp2x_frameskip!=0) {
			gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+40,"Frame-Skip    %s %d %s","<=",gp2x_frameskip, "(Auto-skip)");
		}
		else{
			gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+40,"Frame-Skip    %s %d %s","=",gp2x_frameskip,"");
		}
#else
		switch (gp2x_rotate)
		{
			case 0: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video         %s","Fixed"); break;
			case 1: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video         %s","Fixed DIV2"); break;
			case 2: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video         %s","SW Scaled"); break;
			default: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video         %s","HW Scaled"); break;
		}
		if (gp2x_frameskip_auto)
		{
			gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+40,"Frame-Skip    Auto");
			gp2x_frameskip=-1;
		}
		else
			gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+40,"Frame-Skip    = %d",gp2x_frameskip);
#endif
#endif

		/* Video Clock */
		gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+50,"Video Clock   %d%%",gp2x_clock_cpu);

		/* Video Sync */
#ifdef GP2X
		if (gp2x_double_buffer && gp2x_vsync==1)
			gp2x_gamelist_text_out(x_Pos,y_Pos+60,"Video Sync    VSync+DblBuf");
		else if (gp2x_double_buffer && gp2x_vsync==0)
			gp2x_gamelist_text_out(x_Pos,y_Pos+60,"Video Sync    DblBuf");
		else if (!gp2x_double_buffer && gp2x_vsync==1)
			gp2x_gamelist_text_out(x_Pos,y_Pos+60,"Video Sync    VSync");
		else if (gp2x_vsync==-1)
			gp2x_gamelist_text_out(x_Pos,y_Pos+60,"Video Sync    Off");
		else
			gp2x_gamelist_text_out(x_Pos,y_Pos+60,"Video Sync    Normal");
#else
		if (gp2x_vsync)
			gp2x_gamelist_text_out(x_Pos,y_Pos+60,"Video Sync    On");
		else
			gp2x_gamelist_text_out(x_Pos,y_Pos+60,"Video Sync    Off");

#endif
		/* Sound */
		switch(gp2x_sound_enable)
		{
			case 0: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+70,"Sound         %s","Sound OFF"); break;
			case 1: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+70,"Sound         %s","Sound ON"); break;
			case 2: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+70,"Sound         %s","Emulated but OFF"); break;
			case 3: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+70,"Sound         %s","Accurate and ON"); break;
#ifdef PSP
			case 4: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+70,"Sound         %s","44k 16v"); break;
			case 5: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+70,"Sound         %s","44k 8v"); break;
			case 6: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+70,"Sound         %s","44k 4v"); break;
			case 7: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+70,"Sound         %s","33k 16v"); break;
			case 8: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+70,"Sound         %s","33k 8v"); break;
			case 9: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+70,"Sound         %s","33k 4v"); break;
#endif
		}

		/* Audio Clock */
		gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+80,"Audio Clock   %d%%",gp2x_clock_sound);

		/* Controller Configuration */
		gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+90,"Controller Configuration");

		/* Save Configuration */
		switch(gp2x_save_config) {
			case 0: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+100,"No Save Configuration"); break;
			case 1: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+100,"Save Configuration"); break;
			case 2: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+100,"Save as Default"); break;
		}
		gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+110, "Dipswitches Configuration");

#ifdef PSP
		/* PSP Clock*/
		gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+120, "PSP Clock     %d MHz", gp2x_freq);
		gp2x_gamelist_text_out(x_Pos,y_Pos+130,"Press [] to confirm, X return\0");
#else
		gp2x_gamelist_text_out(x_Pos,y_Pos+130,"Press A to confirm, B return\0");
#endif

		/* Show currently selected item */
		gp2x_gamelist_text_out(x_Pos-16,y_Pos+(selected_option*10)+30," >");

		gp2x_video_flip();
		while(gp2x_joystick_read()&0x8c0ff55) { gp2x_timer_delay(150); }
		while(!(ExKey=gp2x_joystick_read()&0x8c0ff55)) { }
		if(ExKey & GP2X_DOWN){
			selected_option++;
			selected_option = selected_option % options_count;
		}
		else if(ExKey & GP2X_UP){
			selected_option--;
			if(selected_option<0)
				selected_option = options_count - 1;
		}
#ifdef GP2X
		else if(ExKey & GP2X_R || ExKey & GP2X_L){
#else
		else if(ExKey & GP2X_RIGHT || ExKey & GP2X_LEFT){
#endif
			switch(selected_option) {
			case 0:
#ifdef GP2X
				/* GP2X Clock */
				if(ExKey & GP2X_R){
					switch (gp2x_freq) {
						case  66: gp2x_freq=75;break;
						case  75: gp2x_freq=80;break;
						case  80: gp2x_freq=100;break;
						case 100: gp2x_freq=120;break;
						case 120: gp2x_freq=133;break;
						case 133: gp2x_freq=150;break;
						case 150: gp2x_freq=166;break;
						case 166: gp2x_freq=180;break;
						case 180: gp2x_freq=200;break;
						case 200: gp2x_freq=225;break;
						case 225: gp2x_freq=233;break;
						case 233: gp2x_freq=245;break;
						case 245: gp2x_freq=250;break;
						case 250: gp2x_freq=255;break;
						case 255: gp2x_freq=260;break;
						case 260: gp2x_freq=265;break;
						case 265: gp2x_freq=266;break;
						case 266: gp2x_freq=270;break;
						case 270: gp2x_freq=275;break;
						case 275: gp2x_freq=280;break;
						case 280: gp2x_freq=285;break;
						case 285: gp2x_freq=290;break;
						case 290: gp2x_freq=295;break;
						case 295: gp2x_freq=300;break;
						case 300: gp2x_freq=66;break;
					}
				} else {
					switch (gp2x_freq) {
						case  66: gp2x_freq=300;break;
						case  75: gp2x_freq=66;break;
						case  80: gp2x_freq=75;break;
						case 100: gp2x_freq=80;break;
						case 120: gp2x_freq=100;break;
						case 133: gp2x_freq=120;break;
						case 150: gp2x_freq=133;break;
						case 166: gp2x_freq=150;break;
						case 180: gp2x_freq=166;break;
						case 200: gp2x_freq=180;break;
						case 225: gp2x_freq=200;break;
						case 233: gp2x_freq=225;break;
						case 245: gp2x_freq=233;break;
						case 250: gp2x_freq=245;break;
						case 255: gp2x_freq=250;break;
						case 260: gp2x_freq=255;break;
						case 265: gp2x_freq=260;break;
						case 266: gp2x_freq=265;break;
						case 270: gp2x_freq=266;break;
						case 275: gp2x_freq=270;break;
						case 280: gp2x_freq=275;break;
						case 285: gp2x_freq=280;break;
						case 290: gp2x_freq=285;break;
						case 295: gp2x_freq=290;break;
						case 300: gp2x_freq=295;break;
					}
				}
#else
				if (ExKey & GP2X_RIGHT)
					gp2x_rotate++;
				else
				if (ExKey & GP2X_LEFT)
					gp2x_rotate--;
				if (gp2x_rotate<0)
					gp2x_rotate=3;
				else if (gp2x_rotate>3)
					gp2x_rotate=0;

#endif
				break;
			case 1:
				/* "Frame-Skip" */
#ifdef GP2X
				if(ExKey & GP2X_R){
#else
				if(ExKey & GP2X_RIGHT){
#endif
					gp2x_frameskip ++;
					if (!gp2x_frameskip)
						gp2x_frameskip_auto=0;
					else
					if (gp2x_frameskip > 5) {
#ifdef GP2X
						gp2x_frameskip = 0;
						gp2x_frameskip_auto=!gp2x_frameskip_auto; 
#else
#ifdef PSP
						gp2x_frameskip = 0;
						gp2x_frameskip_auto=!gp2x_frameskip_auto; 
#else
						gp2x_frameskip = -1;
						gp2x_frameskip_auto = 1;
#endif
#endif
					}
				}
				else{
					gp2x_frameskip--;
#ifdef GP2X
					if (gp2x_frameskip < 0){
						gp2x_frameskip = 5;
						gp2x_frameskip_auto=!gp2x_frameskip_auto; 
					}
#else
#ifdef PSP
					if (gp2x_frameskip < 0){
						gp2x_frameskip = 5;
						gp2x_frameskip_auto=!gp2x_frameskip_auto; 
					}
#else
					if (gp2x_frameskip==-1)
						gp2x_frameskip_auto=1;
					else if (gp2x_frameskip<-1)
					{
						gp2x_frameskip = 5;
						gp2x_frameskip_auto = 0;
					}
#endif
#endif
				}
				break;
			case 2:
				/* "Video Clock" */
#ifdef GP2X
				if(ExKey & GP2X_R){
#else
				if(ExKey & GP2X_RIGHT){
#endif
					gp2x_clock_cpu += 10; /* Add 10% */
					if (gp2x_clock_cpu > 200) /* 200% is the max */
						gp2x_clock_cpu = 200;
				}
				else{
					gp2x_clock_cpu -= 10; /* Subtract 10% */
					if (gp2x_clock_cpu < 10) /* 10% is the min */
						gp2x_clock_cpu = 10;
				}
				break;
			case 3:
				/* Video Sync */
#ifdef GP2X
				if (!gp2x_double_buffer && gp2x_vsync==0)
				{ gp2x_double_buffer=0; gp2x_vsync=-1; }
				else if (!gp2x_double_buffer && gp2x_vsync==-1)
				{ gp2x_double_buffer=1; gp2x_vsync=0; }
				else if (gp2x_double_buffer && gp2x_vsync==0)
				{ gp2x_double_buffer=0; gp2x_vsync=1; }
				else if (!gp2x_double_buffer && gp2x_vsync==1)				
				{ gp2x_double_buffer=1; gp2x_vsync=1; }	
				else 
				{ gp2x_double_buffer=0; gp2x_vsync=0; }	
#else
				gp2x_vsync=(gp2x_vsync+1)&1;
#endif
				break;

			case 4:
#ifndef NOSOUND
				/* "Sound" */
				gp2x_sound_enable--;
				if (gp2x_sound_enable<0)
#ifdef PSP
					gp2x_sound_enable=9;
#else
					gp2x_sound_enable=3;
#endif
#endif
				break;
			case 5:
				/* "Audio Clock" */
#ifdef GP2X
				if(ExKey & GP2X_R){
#else
				if(ExKey & GP2X_RIGHT){
#endif
					gp2x_clock_sound += 10; /* Add 10% */
					if (gp2x_clock_sound > 200) /* 200% is the max */
						gp2x_clock_sound = 200;
				}
				else{
					gp2x_clock_sound -= 10; /* Subtract 10% */
					if (gp2x_clock_sound < 10) /* 10% is the min */
						gp2x_clock_sound = 10;
				}
				break;
			case 6:
				/* Controller Configuration */
				controller_config();
				break;
			case 7:
				/* Save Configuration */
#ifdef GP2X
				if(ExKey & GP2X_R){
#else
				if(ExKey & GP2X_RIGHT){
#endif
					gp2x_save_config += 1;
					if (gp2x_save_config > 2)
						gp2x_save_config = 0;
				}
				else{
					gp2x_save_config -= 1;
					if (gp2x_save_config < 0)
						gp2x_save_config = 2;
				}
				break;
			case 8:
				/* Dipswitches Configuration */
				dipswitches_config();
				break;
#ifdef PSP
            case 9:
				/* PSP Clock */
				if(ExKey & GP2X_RIGHT){
					switch (gp2x_freq) {
						case 133: gp2x_freq=166;break;
						case 166: gp2x_freq=200;break;
						case 200: gp2x_freq=222;break;
						case 222: gp2x_freq=266;break;
						case 266: gp2x_freq=300;break;
						case 300: gp2x_freq=333;break;
						case 333: gp2x_freq=133;break;
					}
				} else {
					switch (gp2x_freq) {
						case 133: gp2x_freq=333;break;
						case 166: gp2x_freq=133;break;
						case 200: gp2x_freq=166;break;
						case 222: gp2x_freq=200;break;
						case 266: gp2x_freq=222;break;
						case 300: gp2x_freq=266;break;
						case 333: gp2x_freq=300;break;
					}
				}
				break;
#endif
			}
		}
#ifndef GP2X
		if ((ExKey & GP2X_A) || (ExKey & GP2X_START)) {
#else
		if ((ExKey & GP2X_A) || (ExKey & GP2X_PUSH) || (ExKey & GP2X_START)) {
#endif

			/* Save Configuration Files */
			if (gp2x_save_config>1) {
				sprintf(text,SAVEDATAFILEDEF, gp2x_path_mame);
				f=fopen(text,"w");
				if (f) {
					fprintf(f,SAVEDATASTRUCTURE,
					gp2x_freq,gp2x_frameskip,gp2x_frameskip_auto,gp2x_clock_cpu,gp2x_sound_enable,gp2x_clock_sound,
					(key_settings->JOY_FIRE1),(key_settings->JOY_FIRE2),(key_settings->JOY_FIRE3),(key_settings->JOY_FIRE4),
					(key_settings->JOY_FIRE5),(key_settings->JOY_FIRE6),(key_settings->JOY_FIRE7),(key_settings->JOY_FIRE8),
					(key_settings->JOY_FIRE9),(key_settings->JOY_FIRE10),x_sensitivity,x_reversed,y_sensitivity,y_reversed,
					gp2x_rotate,gp2x_vsync,(key_settings->JOY_FIRE1_AUTO),(key_settings->JOY_FIRE2_AUTO),(key_settings->JOY_FIRE3_AUTO));
					fclose(f);
#ifdef GP2X
					sync();
#endif
				}
			}
			if (gp2x_save_config>0) {
				sprintf(text,SAVEDATAFILECFG, gp2x_path_mame, game);
				f=fopen(text,"w");
				if (f) {
					fprintf(f,SAVEDATASTRUCTURE,
					gp2x_freq,gp2x_frameskip,gp2x_frameskip_auto,gp2x_clock_cpu,gp2x_sound_enable,gp2x_clock_sound,
					(key_settings->JOY_FIRE1),(key_settings->JOY_FIRE2),(key_settings->JOY_FIRE3),(key_settings->JOY_FIRE4),
					(key_settings->JOY_FIRE5),(key_settings->JOY_FIRE6),(key_settings->JOY_FIRE7),(key_settings->JOY_FIRE8),
					(key_settings->JOY_FIRE9),(key_settings->JOY_FIRE10),x_sensitivity,x_reversed,y_sensitivity,y_reversed,
					gp2x_rotate,gp2x_vsync,(key_settings->JOY_FIRE1_AUTO),(key_settings->JOY_FIRE2_AUTO),(key_settings->JOY_FIRE3_AUTO));
					fclose(f);
#ifdef GP2X
					sync();
#endif
				}
			}

			/* Selected game will be run */
			return 1;
		}
#ifndef GP2X
		else if ((ExKey & GP2X_X) || (ExKey & GP2X_PUSH) || (ExKey & GP2X_SELECT)){
#else
		else if ((ExKey & GP2X_B) || (ExKey & GP2X_SELECT)){
#endif
			/* Return To Menu */
			return 0;
		}
	}
}

void select_game(char *game) {

	unsigned long ExKey;
	int c;

	/* No Selected game */
	strcpy(game,"builtinn");

	/* Clean screen */
	gp2x_video_flip();

	/* enable extra PSP-2000 memory */
#ifdef PSP_2K
	psp2k_mem_init();
#endif

	/* Wait until no key pressed */
	while(gp2x_joystick_read()&0x8c0ff55) 
		gp2x_timer_delay(100);

	/* Available games? */
	if(game_num_avail==0) {
		gp2x_mame_palette();
		gp2x_text_out( 35, 110, "ERROR: NO AVAILABLE GAMES FOUND" );
		gp2x_video_flip();
		while(1) { if (gp2x_joystick_read()&GP2X_A) exit(0);}
	}

#ifdef PSP
	SetGP2XClock(133);
#else
	SetGP2XClock(66);
#endif

	/* Wait until user selects a game */
	while(1) {
		game_list_view(&last_game_selected);
		gp2x_video_flip();

#ifdef GP2X
		if( (gp2x_joystick_read()&0x8c0ff55)) 
			gp2x_timer_delay(100); 
		while(!(ExKey=gp2x_joystick_read()&0x8c0ff55)) { if ((ExKey & GP2X_L) && (ExKey & GP2X_R)) exit(0); }
#else
		if( (gp2x_joystick_read()&0x8c0ff55)) 
			gp2x_timer_delay(100); 
		while(!(ExKey=gp2x_joystick_read()&0x8c0ff55))
			gp2x_timer_delay(100); 
#endif

		if (ExKey & GP2X_UP) last_game_selected--;
		if (ExKey & GP2X_DOWN) last_game_selected++;
		if (ExKey & GP2X_L) last_game_selected-=21;
		if (ExKey & GP2X_R) last_game_selected+=21;
		if ((ExKey & GP2X_L) && (ExKey & GP2X_R)) exit(0);

		/* If Button A is pressed, or a rom name was passed to this application */
#ifndef GP2X
		{
#ifdef DREAMCAST
			static int parasalir=0;
			if (ExKey & (GP2X_PUSH|GP2X_Y))
				parasalir++;
			else
				parasalir=0;
			if (parasalir>8)
				gp2x_deinit();
#else
			if (ExKey & (GP2X_PUSH|GP2X_Y))
				gp2x_deinit();
#endif
		}
		if ((ExKey & GP2X_A) || (ExKey & GP2X_START))
#else
		if ((ExKey & GP2X_A) || (ExKey & GP2X_PUSH) || (ExKey & GP2X_START))
#endif
		{
			/* Select the game */
			game_list_select(last_game_selected, game);

			/* Emulation Options */
			if(show_options(game))
			{
				/* break out of the while(1) loop */
				break;
			}
		}
	}

	SetGP2XClock(gp2x_freq);
}

