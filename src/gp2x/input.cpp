
#include "driver.h"
#include "usbjoy_mame.h"

void fade_me(void);

unsigned long ExKey1=0;
unsigned long ExKey2=0;
unsigned long ExKey3=0;
unsigned long ExKey4=0;

struct KeySettings keySettings = {
#ifdef GP2X
	GP2X_B,
#else
	GP2X_A,
#endif
	GP2X_X,
#ifdef GP2X
	GP2X_A,
#else
	GP2X_B,
#endif
	GP2X_Y,
	GP2X_L,
	GP2X_R,
	GP2X_VOL_UP,
	GP2X_VOL_DOWN,
	GP2X_PUSH,
	GP2X_START,
	0,
	0,
	0
};

#define GP2X_PUSH_SUB (GP2X_R|GP2X_START)
#define GP2X_SELECT_SUB (GP2X_L|GP2X_START)

struct KeySettings *key_settings = &keySettings;

int enable_trak_read=1;
int useDiagonal=0;

void msdos_init_input (void)
{
	extern char playgame[16];
	osd_poll_joystick();
	
	/* Disable Trak Read ? */
	if ( (strcasecmp(playgame,"hbarrel")==0) || (strcasecmp(playgame,"hbarrelj")==0) || (strcasecmp(playgame,"midres")==0) ||
		(strcasecmp(playgame,"midresj")==0) || (strcasecmp(playgame,"tnk3")==0) || (strcasecmp(playgame,"ikari")==0) ||
		(strcasecmp(playgame,"ikarijp")==0) || (strcasecmp(playgame,"ikarijpb")==0) || (strcasecmp(playgame,"victroad")==0) ||
		(strcasecmp(playgame,"dogosoke")==0) || (strcasecmp(playgame,"gwar")==0) || (strcasecmp(playgame,"bermudat")==0)
#ifdef GP2X		
		|| (strcasecmp(playgame,"tron")==0) || (strcasecmp(playgame,"tron2")==0) || (strcasecmp(playgame,"dotron")==0) ||
		(strcasecmp(playgame,"dotrone")==0) || (strcasecmp(playgame,"mplanets")==0) 
#endif		
		)
		enable_trak_read=0;
	else
		enable_trak_read=1;
		
	/* Use Diagonals ? */	
	if ((strcasecmp(playgame,"qbert")== 0) || (strcasecmp(playgame,"qbertjp")==0) || (strcasecmp(playgame,"qbertqub")==0) ||
	(strcasecmp(playgame,"sqbert")==0))
		useDiagonal=1;
	else
		useDiagonal=0;
#ifndef GP2X
	ExKey1=ExKey2=ExKey3=ExKey4=0;
	{
		extern Uint32 _st_[];
		_st_[0]=_st_[1]=_st_[2]=_st_[3]=0;
	}
#endif
}

void msdos_shutdown_input(void) { }


/*
 * Check if a key is pressed. The keycode is the standard PC keyboard
 * code, as defined in osdepend.h. Return 0 if the key is not pressed,
 * nonzero otherwise. Handle pseudo keycodes.
 */
int osd_key_pressed(int keycode)
{
#ifndef GP2X
	int all_ex=ExKey1|ExKey4|ExKey2|ExKey3;
#endif
	switch (keycode)
       	{
#ifdef GP2X
		case OSD_KEY_CANCEL:
			return ((ExKey1 & GP2X_L) && (ExKey1 & GP2X_R) && ((ExKey1 & GP2X_PUSH)||(ExKey1 & GP2X_START)||(ExKey1 & GP2X_SELECT)));
		case OSD_KEY_RESET_MACHINE:
			return ((ExKey1 & GP2X_START) && (ExKey1 & GP2X_SELECT));
		case OSD_KEY_PAUSE:
		case OSD_KEY_UNPAUSE:
			if ((ExKey1 & GP2X_L) && (ExKey1 & GP2X_R) && (ExKey1 & GP2X_Y)) {extern int gp2x_showfps; gp2x_showfps=!gp2x_showfps; while (gp2x_joystick_read()&(GP2X_L|GP2X_R|GP2X_Y)); gp2x_clear_screen(); return 0;}
			return ((ExKey1 & GP2X_L) && (ExKey1 & GP2X_R));
#else
		case OSD_KEY_RESET_MACHINE:
			return ((ExKey1 & GP2X_PUSH) && (ExKey1 & GP2X_L));
		case OSD_KEY_CANCEL:
			{
				static int cuenta=0;
//				if ((ExKey1 & GP2X_PUSH)&&(!(ExKey1 & GP2X_L)))
#ifdef PSP
				if (all_ex & GP2X_PUSH)
#else
				if ((all_ex & GP2X_PUSH) || ((all_ex & GP2X_PUSH_SUB)==GP2X_PUSH_SUB))
#endif
				{
					osd_mute_sound();
#ifdef DREAMCAST
					if (cuenta>8)
					{
						cuenta=0;
						fade_me();
						return 1;
					}
					else
						cuenta++;
#else
					fade_me();
					return 1;
#endif
				}
				else
				{
					if (cuenta)
					{
						osd_unmute_sound();
						cuenta=0;
					}
				}
			}
			return 0;
		case OSD_KEY_PAUSE:
		case OSD_KEY_UNPAUSE:
//			return (ExKey1 & GP2X_PUSH);
#ifdef PSP
			return ((ExKey1 & GP2X_L) && (ExKey1 & GP2X_R));
#else
			return ((all_ex & GP2X_PUSH) || ((all_ex & GP2X_PUSH_SUB)==GP2X_PUSH_SUB));
#endif
#endif


#ifdef GP2X
		case OSD_KEY_1: /* 1 Player Start == Start Button */
			return ( (ExKey1 & GP2X_START) && !(ExKey1 & GP2X_SELECT) && !(ExKey1 & GP2X_UP) && !(ExKey1 & GP2X_RIGHT) && !(ExKey1 & GP2X_DOWN) );
		case OSD_KEY_2: /* 2 Player Start == Joystick UP + Start Button */
			return ( ( (ExKey1 & GP2X_START) && !(ExKey1 & GP2X_SELECT) && (ExKey1 & GP2X_UP) && !(ExKey1 & GP2X_RIGHT) && !(ExKey1 & GP2X_DOWN) ) || (ExKey2 & GP2X_START));
		case OSD_KEY_7: /* 3 Player Start == Joystick RIGHT + Start Button */
			return ( ( (ExKey1 & GP2X_START) && !(ExKey1 & GP2X_SELECT) && !(ExKey1 & GP2X_UP) && (ExKey1 & GP2X_RIGHT) && !(ExKey1 & GP2X_DOWN) ) || (ExKey3 & GP2X_START));
		case OSD_KEY_8: /* 4 Player Start == Joystick DOWN + Start Button */
			return ( ( (ExKey1 & GP2X_START) && !(ExKey1 & GP2X_SELECT) && !(ExKey1 & GP2X_UP) && !(ExKey1 & GP2X_RIGHT) && (ExKey1 & GP2X_DOWN) ) || (ExKey4 & GP2X_START));
		case OSD_KEY_3: /* Coin A == L Button */
			return ( !(ExKey1 & GP2X_START) && (ExKey1 & GP2X_SELECT) && !(ExKey1 & GP2X_UP) && !(ExKey1 & GP2X_RIGHT) && !(ExKey1 & GP2X_DOWN) );
		case OSD_KEY_4:	/* Coin B == L Button + Joystick UP */	
			return ( ( !(ExKey1 & GP2X_START) && (ExKey1 & GP2X_SELECT) && (ExKey1 & GP2X_UP) && !(ExKey1 & GP2X_RIGHT) && !(ExKey1 & GP2X_DOWN) ) || (ExKey2 & GP2X_SELECT));
		case OSD_KEY_5: /* Coin C == L Button + Joystick RIGHT */
			return ( ( !(ExKey1 & GP2X_START) && (ExKey1 & GP2X_SELECT) && !(ExKey1 & GP2X_UP) && (ExKey1 & GP2X_RIGHT) && !(ExKey1 & GP2X_DOWN) ) || (ExKey3 & GP2X_SELECT));
		case OSD_KEY_6: /* Coin D == L Button + Joystick DOWN */
			return ( ( !(ExKey1 & GP2X_START) && (ExKey1 & GP2X_SELECT) && !(ExKey1 & GP2X_UP) && !(ExKey1 & GP2X_RIGHT) && (ExKey1 & GP2X_DOWN) ) || (ExKey4 & GP2X_SELECT));
#else
		case OSD_KEY_1:				/* 1 Player Start */
			return (ExKey1 & GP2X_START);
		case OSD_KEY_2:				/* 2 Player Start */
			return (ExKey2 & GP2X_START);
		case OSD_KEY_7:				/* 3 Player Start */
			return (ExKey3 & GP2X_START);
		case OSD_KEY_8: 			/* 4 Player Start */
			return (ExKey4 & GP2X_START);
		case OSD_KEY_3:				/* Coin A */
			return (ExKey1 & GP2X_SELECT) || ((ExKey1 & GP2X_SELECT_SUB)==GP2X_SELECT_SUB);
		case OSD_KEY_4:				/* Coin B */
			return (ExKey2 & GP2X_SELECT) || ((ExKey2 & GP2X_SELECT_SUB)==GP2X_SELECT_SUB);
		case OSD_KEY_5:				/* Coin C */
			return (ExKey3 & GP2X_SELECT) || ((ExKey3 & GP2X_SELECT_SUB)==GP2X_SELECT_SUB);
		case OSD_KEY_6:				/* Coin D */
			return (ExKey4 & GP2X_SELECT) || ((ExKey4 & GP2X_SELECT_SUB)==GP2X_SELECT_SUB);
#endif
    		default:
			return 0;
        	}
}

void osd_poll_joystick(void)
{ 
#ifdef GP2X	
	ExKey1 = gp2x_joystick_read();
	if (num_of_joys>0) {
		if (usbjoy_player1) {
			switch (num_of_joys) {
				case 4: ExKey4 = gp2x_usbjoy_check(joys[3]);
				case 3: ExKey3 = gp2x_usbjoy_check(joys[2]);
				case 2: ExKey2 = gp2x_usbjoy_check(joys[1]);
				default: break;
			}
		} else {
			switch (num_of_joys) {
				case 3: ExKey4 = gp2x_usbjoy_check(joys[2]);
				case 2: ExKey3 = gp2x_usbjoy_check(joys[1]);
				case 1: ExKey2 = gp2x_usbjoy_check(joys[0]);
				default: break;
			}
		}
	} else {
		ExKey2=ExKey1;
		if(ExKey2&GP2X_START) ExKey2^=GP2X_START;
		if(ExKey2&GP2X_SELECT) ExKey2^=GP2X_SELECT;
		ExKey4=ExKey3=ExKey2;
	}
#else
	ExKey1=gp2x_joystick_read_n(1);
	ExKey2=gp2x_joystick_read_n(2);
	ExKey3=gp2x_joystick_read_n(3);
	ExKey4=gp2x_joystick_read_n(4);
#endif
}

#ifndef GP2X
#define GET_JOY_FIRE(NJOY,MFIRE) \
			if (key_settings->JOY_FIRE##NJOY##_AUTO) { \
				f##MFIRE[(NJOY-1)]=(ExKey##NJOY & key_settings->JOY_FIRE##MFIRE); \
				if (!f##MFIRE[(NJOY-1)]) { c##MFIRE[(NJOY-1)]=0; return 0; } c##MFIRE[(NJOY-1)]++; if (c##MFIRE[(NJOY-1)]>=8) { c##MFIRE[(NJOY-1)]=0; return 0; } if (c##MFIRE[(NJOY-1)]=4) {return 0; } return 1; \
			} \
			else \
				return (ExKey##NJOY & key_settings->JOY_FIRE##MFIRE);

#define CASE_JOY_FIRES(MFIRE) \
		case OSD_JOY_FIRE##MFIRE: \
			GET_JOY_FIRE(1,MFIRE) \
		case OSD_JOY2_FIRE##MFIRE: \
			GET_JOY_FIRE(2,MFIRE) \
		case OSD_JOY3_FIRE##MFIRE: \
			GET_JOY_FIRE(3,MFIRE)
#endif

/* check if the joystick is moved in the specified direction, defined in */
/* osdepend.h. Return 0 if it is not pressed, nonzero otherwise. */
int osd_joy_pressed(int joycode)
{
	static int f1[4]={0,0,0,0};
	static int f2[4]={0,0,0,0};
	static int f3[4]={0,0,0,0};
	static int c1[4]={0,0,0,0};
	static int c2[4]={0,0,0,0};
	static int c3[4]={0,0,0,0}; /* For Auto-fire */

	switch (joycode)
	{
#ifdef GP2X
		case OSD_JOY_LEFT:
			if (useDiagonal) {
				if ((gp2x_rotate&1) && usbjoy_tate) 
  					return (ExKey1 & GP2X_RIGHT) && (ExKey1 & GP2X_UP); 
  				else 
  					return (ExKey1 & GP2X_LEFT) && (ExKey1 & GP2X_UP); 
			} else {
				if ((gp2x_rotate&1) && usbjoy_tate) 
		  			return (ExKey1 & GP2X_UP);
		  		else
		  			return (ExKey1 & GP2X_LEFT);
		  	}
		case OSD_JOY2_LEFT:
			if (useDiagonal) {
				if ((gp2x_rotate&1) && usbjoy_tate) 
  					return (ExKey2 & GP2X_RIGHT) && (ExKey2 & GP2X_UP); 
  				else 
  					return (ExKey2 & GP2X_LEFT) && (ExKey2 & GP2X_UP); 
			} else {
				if ((gp2x_rotate&1) && usbjoy_tate) 
		  			return (ExKey2 & GP2X_UP);
		  		else
		  			return (ExKey2 & GP2X_LEFT);
		  	}
		case OSD_JOY3_LEFT:
			if (useDiagonal) {
				if ((gp2x_rotate&1) && usbjoy_tate) 
  					return (ExKey3 & GP2X_RIGHT) && (ExKey3 & GP2X_UP); 
  				else 
  					return (ExKey3 & GP2X_LEFT) && (ExKey3 & GP2X_UP); 
			} else {
				if ((gp2x_rotate&1) && usbjoy_tate) 
		  			return (ExKey3 & GP2X_UP);
		  		else
		  			return (ExKey3 & GP2X_LEFT);
		  	}
		case OSD_JOY4_LEFT:
			if (useDiagonal) {
				if ((gp2x_rotate&1) && usbjoy_tate) 
  					return (ExKey4 & GP2X_RIGHT) && (ExKey4 & GP2X_UP); 
  				else 
  					return (ExKey4 & GP2X_LEFT) && (ExKey4 & GP2X_UP); 
			} else {
				if ((gp2x_rotate&1) && usbjoy_tate) 
		  			return (ExKey4 & GP2X_UP);
		  		else
		  			return (ExKey4 & GP2X_LEFT);
		  	}
#else
		case OSD_JOY_LEFT:
			if (useDiagonal)
				return (ExKey1 & GP2X_LEFT) && (ExKey1 & GP2X_UP); 
		  	else
		  		return (ExKey1 & GP2X_LEFT);
		case OSD_JOY2_LEFT:
			if (useDiagonal)
				return (ExKey2 & GP2X_LEFT) && (ExKey2 & GP2X_UP); 
			else
		  		return (ExKey2 & GP2X_LEFT);
		case OSD_JOY3_LEFT:
			if (useDiagonal)
				return (ExKey3 & GP2X_LEFT) && (ExKey3 & GP2X_UP); 
		  	else
		  		return (ExKey3 & GP2X_LEFT);
		case OSD_JOY4_LEFT:
			if (useDiagonal)
				return (ExKey4 & GP2X_LEFT) && (ExKey4 & GP2X_UP); 
		  	else
		  		return (ExKey4 & GP2X_LEFT);
#endif
#ifdef GP2X
		case OSD_JOY_RIGHT:
			if (useDiagonal) {
				if ((gp2x_rotate&1) && usbjoy_tate) 
  					return (ExKey1 & GP2X_LEFT) && (ExKey1 & GP2X_DOWN); 
  				else 
  					return (ExKey1 & GP2X_RIGHT) && (ExKey1 & GP2X_DOWN); 
			} else {
				if ((gp2x_rotate&1) && usbjoy_tate) 
		  			return (ExKey1 & GP2X_DOWN);
		  		else
				  	return (ExKey1 & GP2X_RIGHT);
			}
		case OSD_JOY2_RIGHT:
			if (useDiagonal) {
				if ((gp2x_rotate&1) && usbjoy_tate) 
  					return (ExKey2 & GP2X_LEFT) && (ExKey2 & GP2X_DOWN); 
  				else 
  					return (ExKey2 & GP2X_RIGHT) && (ExKey2 & GP2X_DOWN); 
			} else {
				if ((gp2x_rotate&1) && usbjoy_tate) 
		  			return (ExKey2 & GP2X_DOWN);
		  		else
				  	return (ExKey2 & GP2X_RIGHT);
			}
		case OSD_JOY3_RIGHT:
			if (useDiagonal) {
				if ((gp2x_rotate&1) && usbjoy_tate) 
  					return (ExKey3 & GP2X_LEFT) && (ExKey3 & GP2X_DOWN); 
  				else 
  					return (ExKey3 & GP2X_RIGHT) && (ExKey3 & GP2X_DOWN); 
			} else {
				if ((gp2x_rotate&1) && usbjoy_tate) 
		  			return (ExKey3 & GP2X_DOWN);
		  		else
				  	return (ExKey3 & GP2X_RIGHT);
			}
		case OSD_JOY4_RIGHT:
			if (useDiagonal) {
				if ((gp2x_rotate&1) && usbjoy_tate) 
  					return (ExKey4 & GP2X_LEFT) && (ExKey4 & GP2X_DOWN); 
  				else 
  					return (ExKey4 & GP2X_RIGHT) && (ExKey4 & GP2X_DOWN); 
			} else {
				if ((gp2x_rotate&1) && usbjoy_tate) 
		  			return (ExKey4 & GP2X_DOWN);
		  		else
				  	return (ExKey4 & GP2X_RIGHT);
			}
#else
		case OSD_JOY_RIGHT:
			if (useDiagonal)
				return (ExKey1 & GP2X_RIGHT) && (ExKey1 & GP2X_DOWN); 
			else
				return (ExKey1 & GP2X_RIGHT);
		case OSD_JOY2_RIGHT:
			if (useDiagonal)
				return (ExKey2 & GP2X_RIGHT) && (ExKey2 & GP2X_DOWN); 
			else
				return (ExKey2 & GP2X_RIGHT);
		case OSD_JOY3_RIGHT:
			if (useDiagonal)
				return (ExKey3 & GP2X_RIGHT) && (ExKey3 & GP2X_DOWN); 
			else
				return (ExKey3 & GP2X_RIGHT);
		case OSD_JOY4_RIGHT:
			if (useDiagonal)
				return (ExKey4 & GP2X_RIGHT) && (ExKey4 & GP2X_DOWN); 
			else
				return (ExKey4 & GP2X_RIGHT);
#endif
#ifdef GP2X
		case OSD_JOY_UP:
			if (useDiagonal) {
				if ((gp2x_rotate&1) && usbjoy_tate) 
  					return (ExKey1 & GP2X_DOWN) && (ExKey1 & GP2X_RIGHT); 
  				else 
  					return (ExKey1 & GP2X_UP) && (ExKey1 & GP2X_RIGHT); 
			} else {
				if ((gp2x_rotate&1) && usbjoy_tate) 
				  	return (ExKey1 & GP2X_RIGHT);
		  		else
				  	return (ExKey1 & GP2X_UP);
			}
		case OSD_JOY2_UP:
			if (useDiagonal) {
				if ((gp2x_rotate&1) && usbjoy_tate) 
  					return (ExKey2 & GP2X_DOWN) && (ExKey2 & GP2X_RIGHT); 
  				else 
  					return (ExKey2 & GP2X_UP) && (ExKey2 & GP2X_RIGHT); 
			} else {
				if ((gp2x_rotate&1) && usbjoy_tate) 
				  	return (ExKey2 & GP2X_RIGHT);
		  		else
				  	return (ExKey2 & GP2X_UP);
			}
		case OSD_JOY3_UP:
			if (useDiagonal) {
				if ((gp2x_rotate&1) && usbjoy_tate) 
  					return (ExKey3 & GP2X_DOWN) && (ExKey3 & GP2X_RIGHT); 
  				else 
  					return (ExKey3 & GP2X_UP) && (ExKey3 & GP2X_RIGHT); 
			} else {
				if ((gp2x_rotate&1) && usbjoy_tate) 
				  	return (ExKey3 & GP2X_RIGHT);
		  		else
				  	return (ExKey3 & GP2X_UP);
			}
		case OSD_JOY4_UP:
			if (useDiagonal) {
				if ((gp2x_rotate&1) && usbjoy_tate) 
  					return (ExKey4 & GP2X_DOWN) && (ExKey4 & GP2X_RIGHT); 
  				else 
  					return (ExKey4 & GP2X_UP) && (ExKey4 & GP2X_RIGHT); 
			} else {
				if ((gp2x_rotate&1) && usbjoy_tate) 
				  	return (ExKey4 & GP2X_RIGHT);
		  		else
				  	return (ExKey4 & GP2X_UP);
			}
#else
		case OSD_JOY_UP:
			if (useDiagonal)
 				return (ExKey1 & GP2X_UP) && (ExKey1 & GP2X_RIGHT); 
			else
				return (ExKey1 & GP2X_UP);
		case OSD_JOY2_UP:
			if (useDiagonal)
 				return (ExKey2 & GP2X_UP) && (ExKey2 & GP2X_RIGHT); 
		  	else
		  		return (ExKey2 & GP2X_UP);
		case OSD_JOY3_UP:
			if (useDiagonal)
 				return (ExKey3 & GP2X_UP) && (ExKey3 & GP2X_RIGHT); 
		  	else
		  		return (ExKey3 & GP2X_UP);
		case OSD_JOY4_UP:
			if (useDiagonal)
 				return (ExKey4 & GP2X_UP) && (ExKey4 & GP2X_RIGHT); 
		  	else
		  		return (ExKey4 & GP2X_UP);
#endif
#ifdef GP2X
		case OSD_JOY_DOWN:
			if (useDiagonal) {
				if ((gp2x_rotate&1) && usbjoy_tate) 
  					return (ExKey1 & GP2X_UP) && (ExKey1 & GP2X_LEFT); 
  				else 
  					return (ExKey1 & GP2X_DOWN) && (ExKey1 & GP2X_LEFT); 
			} else {
				if ((gp2x_rotate&1) && usbjoy_tate) 
		  			return (ExKey1 & GP2X_LEFT);
		  		else 
		  			return (ExKey1 & GP2X_DOWN);
		  	}
		case OSD_JOY2_DOWN:
			if (useDiagonal) {
				if ((gp2x_rotate&1) && usbjoy_tate) 
  					return (ExKey2 & GP2X_UP) && (ExKey2 & GP2X_LEFT); 
  				else 
  					return (ExKey2 & GP2X_DOWN) && (ExKey2 & GP2X_LEFT); 
			} else {
				if ((gp2x_rotate&1) && usbjoy_tate) 
		  			return (ExKey2 & GP2X_LEFT);
		  		else 
		  			return (ExKey2 & GP2X_DOWN);
		  	}
		case OSD_JOY3_DOWN:
			if (useDiagonal) {
				if ((gp2x_rotate&1) && usbjoy_tate) 
  					return (ExKey3 & GP2X_UP) && (ExKey3 & GP2X_LEFT); 
  				else 
  					return (ExKey3 & GP2X_DOWN) && (ExKey3 & GP2X_LEFT); 
			} else {
				if ((gp2x_rotate&1) && usbjoy_tate) 
		  			return (ExKey3 & GP2X_LEFT);
		  		else 
		  			return (ExKey3 & GP2X_DOWN);
		  	}
		case OSD_JOY4_DOWN:
			if (useDiagonal) {
				if ((gp2x_rotate&1) && usbjoy_tate) 
  					return (ExKey4 & GP2X_UP) && (ExKey4 & GP2X_LEFT); 
  				else 
  					return (ExKey4 & GP2X_DOWN) && (ExKey4 & GP2X_LEFT); 
			} else {
				if ((gp2x_rotate&1) && usbjoy_tate) 
		  			return (ExKey4 & GP2X_LEFT);
		  		else 
		  			return (ExKey4 & GP2X_DOWN);
		  	}
#else
		case OSD_JOY_DOWN:
			if (useDiagonal)
				return (ExKey1 & GP2X_DOWN) && (ExKey1 & GP2X_LEFT);
		  	else
		  		return (ExKey1 & GP2X_DOWN);
		case OSD_JOY2_DOWN:
			if (useDiagonal)
				return (ExKey1 & GP2X_DOWN) && (ExKey1 & GP2X_LEFT);
		  	else
		  		return (ExKey2 & GP2X_DOWN);
		case OSD_JOY3_DOWN:
			if (useDiagonal)
				return (ExKey1 & GP2X_DOWN) && (ExKey1 & GP2X_LEFT);
		  	else
		  		return (ExKey3 & GP2X_DOWN);
		case OSD_JOY4_DOWN:
			if (useDiagonal)
				return (ExKey1 & GP2X_DOWN) && (ExKey1 & GP2X_LEFT);
		  	else
		  		return (ExKey4 & GP2X_DOWN);
#endif
#ifdef GP2X
		case OSD_JOY_FIRE1:
		  	if (key_settings->JOY_FIRE1_AUTO) {
		  		/* Auto-fire */
		  		if (gp2x_rotate&1)
	  				f1[0]=((ExKey1 & key_settings->JOY_FIRE1)||(ExKey1 & GP2X_VOL_UP));
	  			else
	  				f1[0]=(ExKey1 & key_settings->JOY_FIRE1);
				if (!f1[0]) { c1[0]=0; return 0; } c1[0]++; if (c1[0]>=8) { c1[0]=0; return 0; } if (c1[0]>=4) {return 0; } return 1;
		  	} else {
		  		if (gp2x_rotate&1)
	  				return ((ExKey1 & key_settings->JOY_FIRE1)||(ExKey1 & GP2X_VOL_UP));
	  			else
	  				return (ExKey1 & key_settings->JOY_FIRE1);
	  		}
		case OSD_JOY2_FIRE1:
		  	if (key_settings->JOY_FIRE1_AUTO) {
		  		/* Auto-fire */
		  		if (gp2x_rotate&1)
	  				f1[1]=((ExKey2 & key_settings->JOY_FIRE1)||(ExKey2 & GP2X_VOL_UP));
	  			else
	  				f1[1]=(ExKey2 & key_settings->JOY_FIRE1);
				if (!f1[1]) { c1[1]=0; return 0; } c1[1]++; if (c1[1]>=8) { c1[1]=0; return 0; } if (c1[1]>=4) {return 0; } return 1;
		  	} else {
		  		if (gp2x_rotate&1)
	  				return ((ExKey2 & key_settings->JOY_FIRE1)||(ExKey2 & GP2X_VOL_UP));
	  			else
	  				return (ExKey2 & key_settings->JOY_FIRE1);
	  		}
		case OSD_JOY3_FIRE1:
		  	if (key_settings->JOY_FIRE1_AUTO) {
		  		/* Auto-fire */
		  		if (gp2x_rotate&1)
	  				f1[2]=((ExKey3 & key_settings->JOY_FIRE1)||(ExKey3 & GP2X_VOL_UP));
	  			else
	  				f1[2]=(ExKey3 & key_settings->JOY_FIRE1);
				if (!f1[2]) { c1[2]=0; return 0; } c1[2]++; if (c1[2]>=8) { c1[2]=0; return 0; } if (c1[2]>=4) {return 0; } return 1;
		  	} else {
		  		if (gp2x_rotate&1)
	  				return ((ExKey3 & key_settings->JOY_FIRE1)||(ExKey3 & GP2X_VOL_UP));
	  			else
	  				return (ExKey3 & key_settings->JOY_FIRE1);
	  		}
		case OSD_JOY4_FIRE1:
		  	if (key_settings->JOY_FIRE1_AUTO) {
		  		/* Auto-fire */
		  		if (gp2x_rotate&1)
	  				f1[3]=((ExKey4 & key_settings->JOY_FIRE1)||(ExKey4 & GP2X_VOL_UP));
	  			else
	  				f1[3]=(ExKey4 & key_settings->JOY_FIRE1);
				if (!f1[3]) { c1[3]=0; return 0; } c1[3]++; if (c1[3]>=8) { c1[3]=0; return 0; } if (c1[3]>=4) {return 0; } return 1;
		  	} else {
		  		if (gp2x_rotate&1)
	  				return ((ExKey4 & key_settings->JOY_FIRE1)||(ExKey4 & GP2X_VOL_UP));
	  			else
	  				return (ExKey4 & key_settings->JOY_FIRE1);
	  		}
		case OSD_JOY_FIRE2:
		  	if (key_settings->JOY_FIRE2_AUTO) {
		  		/* Auto-fire */
	  			if (gp2x_rotate&1)
  					f2[0]=((ExKey1 & key_settings->JOY_FIRE2)||(ExKey1 & GP2X_VOL_DOWN));
  				else
				  	f2[0]=(ExKey1 & key_settings->JOY_FIRE2);
				if (!f2[0]) { c2[0]=0; return 0; } c2[0]++; if (c2[0]>=8) { c2[0]=0; return 0; } if (c2[0]>=4) {return 0; } return 1;
		  	} else {
	  			if (gp2x_rotate&1)
  					return ((ExKey1 & key_settings->JOY_FIRE2)||(ExKey1 & GP2X_VOL_DOWN));
  				else
				  	return (ExKey1 & key_settings->JOY_FIRE2);
			}
		case OSD_JOY2_FIRE2:
		  	if (key_settings->JOY_FIRE2_AUTO) {
		  		/* Auto-fire */
	  			if (gp2x_rotate&1)
  					f2[1]=((ExKey2 & key_settings->JOY_FIRE2)||(ExKey2 & GP2X_VOL_DOWN));
  				else
				  	f2[1]=(ExKey2 & key_settings->JOY_FIRE2);
				if (!f2[1]) { c2[1]=0; return 0; } c2[1]++; if (c2[1]>=8) { c2[1]=0; return 0; } if (c2[1]>=4) {return 0; } return 1;
		  	} else {
	  			if (gp2x_rotate&1)
  					return ((ExKey2 & key_settings->JOY_FIRE2)||(ExKey2 & GP2X_VOL_DOWN));
  				else
				  	return (ExKey2 & key_settings->JOY_FIRE2);
			}
		case OSD_JOY3_FIRE2:
		  	if (key_settings->JOY_FIRE2_AUTO) {
		  		/* Auto-fire */
	  			if (gp2x_rotate&1)
  					f2[2]=((ExKey3 & key_settings->JOY_FIRE2)||(ExKey3 & GP2X_VOL_DOWN));
  				else
				  	f2[2]=(ExKey3 & key_settings->JOY_FIRE2);
				if (!f2[2]) { c2[2]=0; return 0; } c2[2]++; if (c2[2]>=8) { c2[2]=0; return 0; } if (c2[2]>=4) {return 0; } return 1;
		  	} else {
	  			if (gp2x_rotate&1)
  					return ((ExKey3 & key_settings->JOY_FIRE2)||(ExKey3 & GP2X_VOL_DOWN));
  				else
				  	return (ExKey3 & key_settings->JOY_FIRE2);
			}
		case OSD_JOY4_FIRE2:
		  	if (key_settings->JOY_FIRE2_AUTO) {
		  		/* Auto-fire */
	  			if (gp2x_rotate&1)
  					f2[3]=((ExKey4 & key_settings->JOY_FIRE2)||(ExKey4 & GP2X_VOL_DOWN));
  				else
				  	f2[3]=(ExKey4 & key_settings->JOY_FIRE2);
				if (!f2[3]) { c2[3]=0; return 0; } c2[3]++; if (c2[3]>=8) { c2[3]=0; return 0; } if (c2[3]>=4) {return 0; } return 1;
		  	} else {
	  			if (gp2x_rotate&1)
  					return ((ExKey4 & key_settings->JOY_FIRE2)||(ExKey4 & GP2X_VOL_DOWN));
  				else
				  	return (ExKey4 & key_settings->JOY_FIRE2);
			}
		case OSD_JOY_FIRE3:
		  	if (key_settings->JOY_FIRE3_AUTO) {
		  		/* Auto-fire */
		  		f3[0]=(ExKey1 & key_settings->JOY_FIRE3);
				if (!f3[0]) { c3[0]=0; return 0; } c3[0]++; if (c3[0]>=8) { c3[0]=0; return 0; } if (c3[0]>=4) {return 0; } return 1;
		  	} else {
	  			return (ExKey1 & key_settings->JOY_FIRE3);
	  		}
		case OSD_JOY2_FIRE3:
		  	if (key_settings->JOY_FIRE3_AUTO) {
		  		/* Auto-fire */
		  		f3[1]=(ExKey2 & key_settings->JOY_FIRE3);
				if (!f3[1]) { c3[1]=0; return 0; } c3[1]++; if (c3[1]>=8) { c3[1]=0; return 0; } if (c3[1]>=4) {return 0; } return 1;
		  	} else {
	  			return (ExKey2 & key_settings->JOY_FIRE3);
	  		}
		case OSD_JOY3_FIRE3:
		  	if (key_settings->JOY_FIRE3_AUTO) {
		  		/* Auto-fire */
		  		f3[2]=(ExKey3 & key_settings->JOY_FIRE3);
				if (!f3[2]) { c3[2]=0; return 0; } c3[2]++; if (c3[2]>=8) { c3[2]=0; return 0; } if (c3[2]>=4) {return 0; } return 1;
		  	} else {
	  			return (ExKey3 & key_settings->JOY_FIRE3);
	  		}
		case OSD_JOY4_FIRE3:
		  	if (key_settings->JOY_FIRE3_AUTO) {
		  		/* Auto-fire */
		  		f3[3]=(ExKey4 & key_settings->JOY_FIRE3);
				if (!f3[3]) { c3[3]=0; return 0; } c3[3]++; if (c3[3]>=8) { c3[3]=0; return 0; } if (c3[3]>=4) {return 0; } return 1;
		  	} else {
	  			return (ExKey4 & key_settings->JOY_FIRE3);
	  		}
#else
		CASE_JOY_FIRES(1)
		CASE_JOY_FIRES(2)
		CASE_JOY_FIRES(3)
#endif
		case OSD_JOY_FIRE4:
		  	return (ExKey1 & key_settings->JOY_FIRE4);
		case OSD_JOY2_FIRE4:
		  	return (ExKey2 & key_settings->JOY_FIRE4);
		case OSD_JOY3_FIRE4:
		  	return (ExKey3 & key_settings->JOY_FIRE4);
		case OSD_JOY4_FIRE4:
		  	return (ExKey4 & key_settings->JOY_FIRE4);
		case OSD_JOY_FIRE5:
		  	return (ExKey1 & key_settings->JOY_FIRE5);
		case OSD_JOY2_FIRE5:
		  	return (ExKey2 & key_settings->JOY_FIRE5);
		case OSD_JOY3_FIRE5:
		  	return (ExKey3 & key_settings->JOY_FIRE5);
		case OSD_JOY4_FIRE5:
		  	return (ExKey4 & key_settings->JOY_FIRE5);
		case OSD_JOY_FIRE6:
		  	return (ExKey1 & key_settings->JOY_FIRE6);
		case OSD_JOY2_FIRE6:
		  	return (ExKey2 & key_settings->JOY_FIRE6);
		case OSD_JOY3_FIRE6:
		  	return (ExKey3 & key_settings->JOY_FIRE6);
		case OSD_JOY4_FIRE6:
		  	return (ExKey4 & key_settings->JOY_FIRE6);
		case OSD_JOY_FIRE7:
		  	return (ExKey1 & key_settings->JOY_FIRE7);
		case OSD_JOY2_FIRE7:
		  	return (ExKey2 & key_settings->JOY_FIRE7);
		case OSD_JOY3_FIRE7:
		  	return (ExKey3 & key_settings->JOY_FIRE7);
		case OSD_JOY4_FIRE7:
		  	return (ExKey4 & key_settings->JOY_FIRE7);
		case OSD_JOY_FIRE8:
		  	return (ExKey1 & key_settings->JOY_FIRE8);
		case OSD_JOY2_FIRE8:
		  	return (ExKey2 & key_settings->JOY_FIRE8);
		case OSD_JOY3_FIRE8:
		  	return (ExKey3 & key_settings->JOY_FIRE8);
		case OSD_JOY4_FIRE8:
		  	return (ExKey4 & key_settings->JOY_FIRE8);
		case OSD_JOY_FIRE9:
		  	return (ExKey1 & key_settings->JOY_FIRE9);
		case OSD_JOY2_FIRE9:
		  	return (ExKey2 & key_settings->JOY_FIRE9);
		case OSD_JOY3_FIRE9:
		  	return (ExKey3 & key_settings->JOY_FIRE9);
		case OSD_JOY4_FIRE9:
		  	return (ExKey4 & key_settings->JOY_FIRE9);
		case OSD_JOY_FIRE10:
		  	return (ExKey1 & key_settings->JOY_FIRE10);
		case OSD_JOY2_FIRE10:
		  	return (ExKey2 & key_settings->JOY_FIRE10);
		case OSD_JOY3_FIRE10:
		  	return (ExKey3 & key_settings->JOY_FIRE10);
		case OSD_JOY4_FIRE10:
		  	return (ExKey4 & key_settings->JOY_FIRE10);
		case OSD_JOY_FIRE:
			return ( (ExKey1 & key_settings->JOY_FIRE1) || (ExKey1 & key_settings->JOY_FIRE2) || (ExKey1 & key_settings->JOY_FIRE3) ||
				(ExKey1 & key_settings->JOY_FIRE4) || (ExKey1 & key_settings->JOY_FIRE5) || (ExKey1 & key_settings->JOY_FIRE6) );
		case OSD_JOY2_FIRE:
			return ( (ExKey2 & key_settings->JOY_FIRE1) || (ExKey2 & key_settings->JOY_FIRE2) || (ExKey2 & key_settings->JOY_FIRE3) ||
				(ExKey2 & key_settings->JOY_FIRE4) || (ExKey2 & key_settings->JOY_FIRE5) || (ExKey2 & key_settings->JOY_FIRE6) );
		case OSD_JOY3_FIRE:
			return ( (ExKey3 & key_settings->JOY_FIRE1) || (ExKey3 & key_settings->JOY_FIRE2) || (ExKey3 & key_settings->JOY_FIRE3) ||
				(ExKey3 & key_settings->JOY_FIRE4) || (ExKey3 & key_settings->JOY_FIRE5) || (ExKey3 & key_settings->JOY_FIRE6) );
		case OSD_JOY4_FIRE:
			return ( (ExKey4 & key_settings->JOY_FIRE1) || (ExKey4 & key_settings->JOY_FIRE2) || (ExKey4 & key_settings->JOY_FIRE3) ||
				(ExKey4 & key_settings->JOY_FIRE4) || (ExKey4 & key_settings->JOY_FIRE5) || (ExKey4 & key_settings->JOY_FIRE6) );
		default: 
			return 0;
	}
}

int pos_analog_x=0;
int pos_analog_y=0;

/* return a value in the range -128 .. 128 (yes, 128, not 127) */
void osd_analogjoy_read(int *analog_x, int *analog_y)
{
#ifdef GP2X
	if (gp2x_rotate&1) {
		if( (!(ExKey1 & GP2X_UP)) && (!(ExKey1 & GP2X_DOWN)) ) { 
			pos_analog_x=0;
		} else {
			if(ExKey1 & GP2X_UP) pos_analog_x-=5;
			if(ExKey1 & GP2X_DOWN) pos_analog_x+=5;
		}
		if( (!(ExKey1 & GP2X_LEFT)) && (!(ExKey1 & GP2X_RIGHT)) ) { 
			pos_analog_y=0;
		} else {
		  	if(ExKey1 & GP2X_RIGHT) pos_analog_y-=5; 
			if(ExKey1 & GP2X_LEFT) pos_analog_y+=5;
		}
	} else
#endif
	{
		if( (!(ExKey1 & GP2X_LEFT)) && (!(ExKey1 & GP2X_RIGHT)) ) {
			pos_analog_x=0;
		} else {
			if(ExKey1 & GP2X_LEFT) pos_analog_x-=5;
	  		if(ExKey1 & GP2X_RIGHT) pos_analog_x+=5;
		}
		if( (!(ExKey1 & GP2X_UP)) && (!(ExKey1 & GP2X_DOWN)) ) {
			pos_analog_y=0;
		} else {
	  		if(ExKey1 & GP2X_UP) pos_analog_y-=5; 
	 		if(ExKey1 & GP2X_DOWN) pos_analog_y+=5;
	 	}
	}
	
	if (pos_analog_x<-128) pos_analog_x=-128;
	if (pos_analog_x>128) pos_analog_x=128;
	if (pos_analog_y<-128) pos_analog_y=-128;
	if (pos_analog_y>128) pos_analog_y=128;
	
	*analog_x = pos_analog_x;
	*analog_y = pos_analog_y;

}

void osd_trak_read(int *deltax,int *deltay)
{
	*deltax = *deltay = 0;
	if (!enable_trak_read) return; /* Franxis 05-05-2006 */
	
#ifdef GP2X
	if (gp2x_rotate&1) {
		if(ExKey1 & GP2X_UP) *deltax=-20;
		if(ExKey1 & GP2X_DOWN) *deltax=20;
	  	if(ExKey1 & GP2X_RIGHT) *deltay=20; 
		if(ExKey1 & GP2X_LEFT) *deltay=-20;
	} else
#endif
	{
		if(ExKey1 & GP2X_LEFT) *deltax=-20;
	  	if(ExKey1 & GP2X_RIGHT) *deltax=20;
	  	if(ExKey1 & GP2X_UP) *deltay=20; 
	 	if(ExKey1 & GP2X_DOWN) *deltay=-20;
	}
}

void osd_led_w(int led,int on) { }
