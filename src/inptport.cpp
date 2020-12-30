/***************************************************************************

  inptport.c

  Input ports handling

TODO:	remove the 1 analog device per port limitation
		support more than 1 "real" analog device
		support for inputports producing interrupts
		support for extra "real" hardware (PC throttle's, spinners etc)

***************************************************************************/

#include "driver.h"
#include <math.h>

/* Use the MRU code for 4way joysticks */
#define MRU_JOYSTICK

/* header identifying the version of the game.cfg file */
#define MAMECFGSTRING "MAMECFG\2"

extern int nocheat;

extern unsigned int dispensed_tickets;
extern unsigned int coins[COIN_COUNTERS];
extern unsigned int lastcoin[COIN_COUNTERS];
extern unsigned int coinlockedout[COIN_COUNTERS];

static unsigned short input_port_value[MAX_INPUT_PORTS];
static unsigned char input_vblank[MAX_INPUT_PORTS];

/* Assuming a maxium of one analog input device per port BW 101297 */
static struct InputPort *input_analog[MAX_INPUT_PORTS];
static int input_analog_value[MAX_INPUT_PORTS];
static char input_analog_init[MAX_INPUT_PORTS];

static int mouse_last_x,mouse_last_y;
static int mouse_current_x,mouse_current_y;
static int mouse_previous_x,mouse_previous_y;
static int analog_current_x, analog_current_y;
static int analog_previous_x, analog_previous_y;

int x_sensitivity = -1;
int y_sensitivity = -1;
int x_reversed = 0;
int y_reversed = 0;

int load_input_port_settings(void)
{

	/* All analog ports need initialization */
	int i;
	for (i = 0; i < MAX_INPUT_PORTS; i++)
		input_analog_init[i] = 1;

	update_input_ports();

	return 0;
}



struct ipd
{
	int type;
	const char *name;
	int keyboard;
	int joystick;
};
struct ipd inputport_defaults[] =
{
	/* Direct UP, DOWN, LEFT and RIGHT controls for all 4 players */
	{ IPT_JOYSTICK_UP,                  "Up",      OSD_KEY_UP,      OSD_JOY_UP },
	{ IPT_JOYSTICK_DOWN,                "Down",    OSD_KEY_DOWN,    OSD_JOY_DOWN },
	{ IPT_JOYSTICK_LEFT,                "Left",    OSD_KEY_LEFT,    OSD_JOY_LEFT },
	{ IPT_JOYSTICK_RIGHT,               "Right",   OSD_KEY_RIGHT,   OSD_JOY_RIGHT },
	{ IPT_JOYSTICK_UP    | IPF_PLAYER2, "2 Up",    OSD_KEY_UP,      OSD_JOY2_UP },
	{ IPT_JOYSTICK_DOWN  | IPF_PLAYER2, "2 Down",  OSD_KEY_DOWN,    OSD_JOY2_DOWN },
	{ IPT_JOYSTICK_LEFT  | IPF_PLAYER2, "2 Left",  OSD_KEY_LEFT,    OSD_JOY2_LEFT },
	{ IPT_JOYSTICK_RIGHT | IPF_PLAYER2, "2 Right", OSD_KEY_RIGHT,   OSD_JOY2_RIGHT },
	{ IPT_JOYSTICK_UP    | IPF_PLAYER3, "3 Up",    OSD_KEY_UP,      OSD_JOY3_UP },
	{ IPT_JOYSTICK_DOWN  | IPF_PLAYER3, "3 Down",  OSD_KEY_DOWN,    OSD_JOY3_DOWN },
	{ IPT_JOYSTICK_LEFT  | IPF_PLAYER3, "3 Left",  OSD_KEY_LEFT,    OSD_JOY3_LEFT },
	{ IPT_JOYSTICK_RIGHT | IPF_PLAYER3, "3 Right", OSD_KEY_RIGHT,   OSD_JOY3_RIGHT },
	{ IPT_JOYSTICK_UP    | IPF_PLAYER4, "4 Up",    OSD_KEY_UP,      OSD_JOY4_UP },
	{ IPT_JOYSTICK_DOWN  | IPF_PLAYER4, "4 Down",  OSD_KEY_DOWN,    OSD_JOY4_DOWN },
	{ IPT_JOYSTICK_LEFT  | IPF_PLAYER4, "4 Left",  OSD_KEY_LEFT,    OSD_JOY4_LEFT },
	{ IPT_JOYSTICK_RIGHT | IPF_PLAYER4, "4 Right", OSD_KEY_RIGHT,   OSD_JOY4_RIGHT },
	/* Diagonal direction controls for all 4 players */
	{ IPT_JOYSTICKRIGHT_UP,                  "Right/Up",        OSD_KEY_I,       OSD_JOY_FIRE2 },
	{ IPT_JOYSTICKRIGHT_DOWN,                "Right/Down",      OSD_KEY_K,       OSD_JOY_FIRE3 },
	{ IPT_JOYSTICKRIGHT_LEFT,                "Right/Left",      OSD_KEY_J,       OSD_JOY_FIRE1 },
	{ IPT_JOYSTICKRIGHT_RIGHT,               "Right/Right",     OSD_KEY_L,       OSD_JOY_FIRE4 },
	{ IPT_JOYSTICKLEFT_UP,                   "Left/Up",         OSD_KEY_E,       OSD_JOY_UP },
	{ IPT_JOYSTICKLEFT_DOWN,                 "Left/Down",       OSD_KEY_D,       OSD_JOY_DOWN },
	{ IPT_JOYSTICKLEFT_LEFT,                 "Left/Left",       OSD_KEY_S,       OSD_JOY_LEFT },
	{ IPT_JOYSTICKLEFT_RIGHT,                "Left/Right",      OSD_KEY_F,       OSD_JOY_RIGHT },
	{ IPT_JOYSTICKRIGHT_UP    | IPF_PLAYER2, "2 Right/Up",      OSD_KEY_I,       OSD_JOY2_FIRE2 },
	{ IPT_JOYSTICKRIGHT_DOWN  | IPF_PLAYER2, "2 Right/Down",    OSD_KEY_K,       OSD_JOY2_FIRE3 },
	{ IPT_JOYSTICKRIGHT_LEFT  | IPF_PLAYER2, "2 Right/Left",    OSD_KEY_J,       OSD_JOY2_FIRE1 },
	{ IPT_JOYSTICKRIGHT_RIGHT | IPF_PLAYER2, "2 Right/Right",   OSD_KEY_L,       OSD_JOY2_FIRE4 },
	{ IPT_JOYSTICKLEFT_UP     | IPF_PLAYER2, "2 Left/Up",       OSD_KEY_E,       OSD_JOY2_UP },
	{ IPT_JOYSTICKLEFT_DOWN   | IPF_PLAYER2, "2 Left/Down",     OSD_KEY_D,       OSD_JOY2_DOWN },
	{ IPT_JOYSTICKLEFT_LEFT   | IPF_PLAYER2, "2 Left/Left",     OSD_KEY_S,       OSD_JOY2_LEFT },
	{ IPT_JOYSTICKLEFT_RIGHT  | IPF_PLAYER2, "2 Left/Right",    OSD_KEY_F,       OSD_JOY2_RIGHT },
	{ IPT_JOYSTICKRIGHT_UP    | IPF_PLAYER3, "3 Right/Up",      OSD_KEY_I,       OSD_JOY3_FIRE2 },
	{ IPT_JOYSTICKRIGHT_DOWN  | IPF_PLAYER3, "3 Right/Down",    OSD_KEY_K,       OSD_JOY3_FIRE3 },
	{ IPT_JOYSTICKRIGHT_LEFT  | IPF_PLAYER3, "3 Right/Left",    OSD_KEY_J,       OSD_JOY3_FIRE1 },
	{ IPT_JOYSTICKRIGHT_RIGHT | IPF_PLAYER3, "3 Right/Right",   OSD_KEY_L,       OSD_JOY3_FIRE4 },
	{ IPT_JOYSTICKLEFT_UP     | IPF_PLAYER3, "3 Left/Up",       OSD_KEY_E,       OSD_JOY3_UP },
	{ IPT_JOYSTICKLEFT_DOWN   | IPF_PLAYER3, "3 Left/Down",     OSD_KEY_D,       OSD_JOY3_DOWN },
	{ IPT_JOYSTICKLEFT_LEFT   | IPF_PLAYER3, "3 Left/Left",     OSD_KEY_S,       OSD_JOY3_LEFT },
	{ IPT_JOYSTICKLEFT_RIGHT  | IPF_PLAYER3, "3 Left/Right",    OSD_KEY_F,       OSD_JOY3_RIGHT },
	{ IPT_JOYSTICKRIGHT_UP    | IPF_PLAYER4, "4 Right/Up",      OSD_KEY_I,       OSD_JOY4_FIRE2 },
	{ IPT_JOYSTICKRIGHT_DOWN  | IPF_PLAYER4, "4 Right/Down",    OSD_KEY_K,       OSD_JOY4_FIRE3 },
	{ IPT_JOYSTICKRIGHT_LEFT  | IPF_PLAYER4, "4 Right/Left",    OSD_KEY_J,       OSD_JOY4_FIRE1 },
	{ IPT_JOYSTICKRIGHT_RIGHT | IPF_PLAYER4, "4 Right/Right",   OSD_KEY_L,       OSD_JOY4_FIRE4 },
	{ IPT_JOYSTICKLEFT_UP     | IPF_PLAYER4, "4 Left/Up",       OSD_KEY_E,       OSD_JOY4_UP },
	{ IPT_JOYSTICKLEFT_DOWN   | IPF_PLAYER4, "4 Left/Down",     OSD_KEY_D,       OSD_JOY4_DOWN },
	{ IPT_JOYSTICKLEFT_LEFT   | IPF_PLAYER4, "4 Left/Left",     OSD_KEY_S,       OSD_JOY4_LEFT },
	{ IPT_JOYSTICKLEFT_RIGHT  | IPF_PLAYER4, "4 Left/Right",    OSD_KEY_F,       OSD_JOY4_RIGHT },
	/* Fire button controls for all 4 players */
	{ IPT_BUTTON1,               "Button 1",      OSD_KEY_LCONTROL,OSD_JOY_FIRE1 },
	{ IPT_BUTTON2,               "Button 2",      OSD_KEY_ALT,     OSD_JOY_FIRE2 },
	{ IPT_BUTTON3,               "Button 3",      OSD_KEY_SPACE,   OSD_JOY_FIRE3 },
	{ IPT_BUTTON4,               "Button 4",      OSD_KEY_LSHIFT,  OSD_JOY_FIRE4 },
	{ IPT_BUTTON5,               "Button 5",      OSD_KEY_Z,       OSD_JOY_FIRE5 },
	{ IPT_BUTTON6,               "Button 6",      OSD_KEY_X,       OSD_JOY_FIRE6 },
	{ IPT_BUTTON7,               "Button 7",      OSD_KEY_C,       OSD_JOY_FIRE7 },
	{ IPT_BUTTON8,               "Button 8",      OSD_KEY_V,       OSD_JOY_FIRE8 },
	{ IPT_BUTTON1 | IPF_PLAYER2, "2 Button 1",    OSD_KEY_LCONTROL,OSD_JOY2_FIRE1 },
	{ IPT_BUTTON2 | IPF_PLAYER2, "2 Button 2",    OSD_KEY_ALT,     OSD_JOY2_FIRE2 },
	{ IPT_BUTTON3 | IPF_PLAYER2, "2 Button 3",    OSD_KEY_SPACE,   OSD_JOY2_FIRE3 },
	{ IPT_BUTTON4 | IPF_PLAYER2, "2 Button 4",    OSD_KEY_LSHIFT,  OSD_JOY2_FIRE4 },
	{ IPT_BUTTON5 | IPF_PLAYER2, "2 Button 5",    OSD_KEY_Z,       OSD_JOY2_FIRE5 },
	{ IPT_BUTTON6 | IPF_PLAYER2, "2 Button 6",    OSD_KEY_X,       OSD_JOY2_FIRE6 },
	{ IPT_BUTTON7 | IPF_PLAYER2, "2 Button 7",    OSD_KEY_C,       OSD_JOY2_FIRE7 },
	{ IPT_BUTTON8 | IPF_PLAYER2, "2 Button 8",    OSD_KEY_V,       OSD_JOY2_FIRE8 },
	{ IPT_BUTTON1 | IPF_PLAYER3, "3 Button 1",    OSD_KEY_LCONTROL,OSD_JOY3_FIRE1 },
	{ IPT_BUTTON2 | IPF_PLAYER3, "3 Button 2",    OSD_KEY_ALT,     OSD_JOY3_FIRE2 },
	{ IPT_BUTTON3 | IPF_PLAYER3, "3 Button 3",    OSD_KEY_SPACE,   OSD_JOY3_FIRE3 },
	{ IPT_BUTTON4 | IPF_PLAYER3, "3 Button 4",    OSD_KEY_LSHIFT,  OSD_JOY3_FIRE4 },
	{ IPT_BUTTON5 | IPF_PLAYER3, "3 Button 5",    OSD_KEY_Z,       OSD_JOY3_FIRE5 },
	{ IPT_BUTTON6 | IPF_PLAYER3, "3 Button 6",    OSD_KEY_X,       OSD_JOY3_FIRE6 },
	{ IPT_BUTTON7 | IPF_PLAYER3, "3 Button 7",    OSD_KEY_C,       OSD_JOY3_FIRE7 },
	{ IPT_BUTTON8 | IPF_PLAYER3, "3 Button 8",    OSD_KEY_V,       OSD_JOY3_FIRE8 },
	{ IPT_BUTTON1 | IPF_PLAYER4, "4 Button 1",    OSD_KEY_LCONTROL,OSD_JOY4_FIRE1 },
	{ IPT_BUTTON2 | IPF_PLAYER4, "4 Button 2",    OSD_KEY_ALT,     OSD_JOY4_FIRE2 },
	{ IPT_BUTTON3 | IPF_PLAYER4, "4 Button 3",    OSD_KEY_SPACE,   OSD_JOY4_FIRE3 },
	{ IPT_BUTTON4 | IPF_PLAYER4, "4 Button 4",    OSD_KEY_LSHIFT,  OSD_JOY4_FIRE4 },
	{ IPT_BUTTON5 | IPF_PLAYER4, "4 Button 5",    OSD_KEY_Z,       OSD_JOY4_FIRE5 },
	{ IPT_BUTTON6 | IPF_PLAYER4, "4 Button 6",    OSD_KEY_X,       OSD_JOY4_FIRE6 },
	{ IPT_BUTTON7 | IPF_PLAYER4, "4 Button 7",    OSD_KEY_C,       OSD_JOY4_FIRE7 },
	{ IPT_BUTTON8 | IPF_PLAYER4, "4 Button 8",    OSD_KEY_V,       OSD_JOY4_FIRE8 },
	/* Inser Coin and Player Start controls for all 4 players */
	{ IPT_COIN1,               "Coin A",          OSD_KEY_3,           0 },
	{ IPT_COIN2,               "Coin B",          OSD_KEY_4,           0 },
	{ IPT_COIN3,               "Coin C",          OSD_KEY_5,           0 },
	{ IPT_COIN4,               "Coin D",          OSD_KEY_6,           0 },
	{ IPT_TILT,                "Tilt",            OSD_KEY_T,       IP_JOY_NONE },
	{ IPT_START1,              "1 Player Start",  OSD_KEY_1,           0 },
	{ IPT_START2,              "2 Players Start", OSD_KEY_2,           0 },
	{ IPT_START3,              "3 Players Start", OSD_KEY_7,           0 },
	{ IPT_START4,              "4 Players Start", OSD_KEY_8,           0 },
	/* Various analogue control types for all 4 players */
	{ IPT_PADDLE,              "Paddle",          IPF_DEC(OSD_KEY_LEFT) | IPF_INC(OSD_KEY_RIGHT) | IPF_DELTA(4), \
	                                              IPF_DEC(OSD_JOY_LEFT) | IPF_INC(OSD_JOY_RIGHT) | IPF_DELTA(4) },
	{ IPT_PADDLE | IPF_PLAYER2,  "Paddle 2",          IPF_DELTA(4), IPF_DELTA(4) },
	{ IPT_PADDLE | IPF_PLAYER3,  "Paddle 3",          IPF_DELTA(4), IPF_DELTA(4) },
	{ IPT_PADDLE | IPF_PLAYER4,  "Paddle 4",          IPF_DELTA(4), IPF_DELTA(4) },
	{ IPT_DIAL,                "Dial",            IPF_DEC(OSD_KEY_LEFT) | IPF_INC(OSD_KEY_RIGHT) | IPF_DELTA(4), \
	                                              IPF_DEC(OSD_JOY_LEFT) | IPF_INC(OSD_JOY_RIGHT) | IPF_DELTA(4) },
	{ IPT_DIAL | IPF_PLAYER2,  "Dial 2",          IPF_DELTA(4), IPF_DELTA(4) },
	{ IPT_DIAL | IPF_PLAYER3,  "Dial 3",          IPF_DELTA(4), IPF_DELTA(4) },
	{ IPT_DIAL | IPF_PLAYER4,  "Dial 4",          IPF_DELTA(4), IPF_DELTA(4) },
	{ IPT_TRACKBALL_X,         "Trak X",          IPF_DEC(OSD_KEY_LEFT) | IPF_INC(OSD_KEY_RIGHT) | IPF_DELTA(4), \
	                                              IPF_DEC(OSD_JOY_LEFT) | IPF_INC(OSD_JOY_RIGHT) | IPF_DELTA(4) },
	{ IPT_TRACKBALL_X | IPF_PLAYER2, "Track X 2", IPF_DELTA(4), IPF_DELTA(4) },
	{ IPT_TRACKBALL_X | IPF_PLAYER3, "Track X 3", IPF_DELTA(4), IPF_DELTA(4) },
	{ IPT_TRACKBALL_X | IPF_PLAYER4, "Track X 4", IPF_DELTA(4), IPF_DELTA(4) },
	{ IPT_TRACKBALL_Y,         "Trak Y",          IPF_DEC(OSD_KEY_UP)   | IPF_INC(OSD_KEY_DOWN)  | IPF_DELTA(4), \
	                                              IPF_DEC(OSD_JOY_UP)   | IPF_INC(OSD_JOY_DOWN)  | IPF_DELTA(4) },
	{ IPT_TRACKBALL_Y | IPF_PLAYER2, "Track Y 2", IPF_DELTA(4), IPF_DELTA(4) },
	{ IPT_TRACKBALL_Y | IPF_PLAYER3, "Track Y 3", IPF_DELTA(4), IPF_DELTA(4) },
	{ IPT_TRACKBALL_Y | IPF_PLAYER4, "Track Y 4", IPF_DELTA(4), IPF_DELTA(4) },
	{ IPT_AD_STICK_X,          "AD Stick X",      IPF_DEC(OSD_KEY_LEFT) | IPF_INC(OSD_KEY_RIGHT) | IPF_DELTA(4), \
	                                              IPF_DEC(OSD_JOY_LEFT) | IPF_INC(OSD_JOY_RIGHT) | IPF_DELTA(4) },
	{ IPT_AD_STICK_X | IPF_PLAYER2, "AD Stick X 2", IPF_DELTA(4), IPF_DELTA(4) },
	{ IPT_AD_STICK_X | IPF_PLAYER3, "AD Stick X 3", IPF_DELTA(4), IPF_DELTA(4) },
	{ IPT_AD_STICK_X | IPF_PLAYER4, "AD Stick X 4", IPF_DELTA(4), IPF_DELTA(4) },
	{ IPT_AD_STICK_Y,          "AD Stick Y",      IPF_DEC(OSD_KEY_UP)   | IPF_INC(OSD_KEY_DOWN)  | IPF_DELTA(4), \
	                                              IPF_DEC(OSD_JOY_UP)   | IPF_INC(OSD_JOY_DOWN)  | IPF_DELTA(4) },
	{ IPT_AD_STICK_Y | IPF_PLAYER2, "AD Stick Y 2", IPF_DELTA(4), IPF_DELTA(4) },
	{ IPT_AD_STICK_Y | IPF_PLAYER3, "AD Stick Y 3", IPF_DELTA(4), IPF_DELTA(4) },
	{ IPT_AD_STICK_Y | IPF_PLAYER4, "AD Stick Y 4", IPF_DELTA(4), IPF_DELTA(4) },
	{ IPT_NONCENTERING_X,				"Non Centering X",          IPF_DEC(OSD_KEY_LEFT) | IPF_INC(OSD_KEY_RIGHT) | IPF_DELTA(4), \
																	IPF_DEC(OSD_JOY_LEFT) | IPF_INC(OSD_JOY_RIGHT) | IPF_DELTA(4) },
	{ IPT_NONCENTERING_X | IPF_PLAYER2,	"Non Centering X 2",          IPF_DELTA(4), IPF_DELTA(4) },
	{ IPT_NONCENTERING_X | IPF_PLAYER3,	"Non Centering X 3",          IPF_DELTA(4), IPF_DELTA(4) },
	{ IPT_NONCENTERING_X | IPF_PLAYER4,	"Non Centering X 4",          IPF_DELTA(4), IPF_DELTA(4) },
	{ IPT_NONCENTERING_Y,				"Non Centering Y",          IPF_DEC(OSD_KEY_UP)   | IPF_INC(OSD_KEY_DOWN)  | IPF_DELTA(4), \
																	IPF_DEC(OSD_KEY_UP)   | IPF_INC(OSD_KEY_DOWN)  | IPF_DELTA(4) },
	{ IPT_NONCENTERING_Y | IPF_PLAYER2,	"Non Centering Y 2",          IPF_DELTA(4), IPF_DELTA(4) },
	{ IPT_NONCENTERING_Y | IPF_PLAYER3,	"Non Centering Y 3",          IPF_DELTA(4), IPF_DELTA(4) },
	{ IPT_NONCENTERING_Y | IPF_PLAYER4,	"Non Centering Y 4",          IPF_DELTA(4), IPF_DELTA(4) },
	{ IPT_UNKNOWN,             "UNKNOWN",         IP_KEY_NONE,     IP_JOY_NONE },
	{ IPT_END,                 0,                 IP_KEY_NONE,     IP_JOY_NONE }
};

/* Note that the following 3 routines have slightly different meanings with analog ports */
const char *default_name(const struct InputPort *in)
{
	int i;


	if (in->name != IP_NAME_DEFAULT) return in->name;

	i = 0;
	while (inputport_defaults[i].type != IPT_END &&
			inputport_defaults[i].type != (in->type & (~IPF_MASK | IPF_PLAYERMASK)))
		i++;

	return inputport_defaults[i].name;
}

int default_key(const struct InputPort *in)
{
	int i;


	while (in->keyboard == IP_KEY_PREVIOUS) in--;

	if (in->keyboard != IP_KEY_DEFAULT) return in->keyboard;

	i = 0;
	while (inputport_defaults[i].type != IPT_END &&
			inputport_defaults[i].type != (in->type & (~IPF_MASK | IPF_PLAYERMASK)))
		i++;

	return inputport_defaults[i].keyboard;
}

int default_joy(const struct InputPort *in)
{
	int i;


	while (in->joystick == IP_JOY_PREVIOUS) in--;

	if (in->joystick != IP_JOY_DEFAULT)
		return in->joystick;

	i = 0;
	while (inputport_defaults[i].type != IPT_END &&
			inputport_defaults[i].type != (in->type & (~IPF_MASK | IPF_PLAYERMASK)))
		i++;

	return inputport_defaults[i].joystick;
}

void update_analog_port(int port)
{
	struct InputPort *in;
	int current, delta, type, sensitivity, clip, min, max, default_value;
	int axis, is_stick, check_bounds;
	int inckey, deckey, keydelta, incjoy, decjoy, joydelta;
	int key,joy;

	/* get input definition */
	in=input_analog[port];
	if (nocheat && (in->type & IPF_CHEAT)) return;
	type=(in->type & ~IPF_MASK);


	key = default_key(in);
	joy = default_joy(in);

	deckey =   key & 0x000000ff;
	inckey =   (key & 0x0000ff00) >> 8;
	keydelta = (key & 0x00ff0000) >> 16;
	decjoy =   joy & 0x000000ff;
	incjoy =   (joy & 0x0000ff00) >> 8;
	/* I am undecided if the joydelta is really needed. LBO 120897 */
	/* We probably don't, but this teases the compiler. BW 120997 */
	joydelta = (joy & 0x00ff0000) >> 16;

	switch (type)
	{
		case IPT_PADDLE:
			axis = X_AXIS; is_stick = 0; check_bounds = 1; break;
		case IPT_DIAL:
			axis = X_AXIS; is_stick = 0; check_bounds = 0; break;
		case IPT_TRACKBALL_X:
			axis = X_AXIS; is_stick = 0; check_bounds = 0; break;
		case IPT_TRACKBALL_Y:
			axis = Y_AXIS; is_stick = 0; check_bounds = 0; break;
		case IPT_AD_STICK_X:
			axis = X_AXIS; is_stick = 1; check_bounds = 1; break;
		case IPT_AD_STICK_Y:
			axis = Y_AXIS; is_stick = 1; check_bounds = 1; break;
		case IPT_NONCENTERING_X:
			axis = X_AXIS; is_stick = 0; check_bounds = 1; break;
		case IPT_NONCENTERING_Y:
			axis = Y_AXIS; is_stick = 0; check_bounds = 1; break;
		default:
			/* Use some defaults to prevent crash */
			axis = X_AXIS; is_stick = 0; check_bounds = 0;
	}


	if( axis == X_AXIS && x_sensitivity!=-1) {
		in->arg = in->arg & 0xffffff00;
		in->arg = in->arg | x_sensitivity;
	}
	if( axis == Y_AXIS && y_sensitivity!=-1) {
		in->arg = in->arg & 0xffffff00;
		in->arg = in->arg | y_sensitivity;
	}

	sensitivity = in->arg & 0x000000ff;
	clip = (in->arg & 0x0000ff00) >> 8;
	min = (in->arg & 0x00ff0000) >> 16;
	max = (in->arg & 0xff000000) >> 24;
	default_value = in->default_value * 100 / sensitivity;
	/* extremes can be either signed or unsigned */
	if (min > max) min = min - 256;

	/* if IPF_CENTER go back to the default position, but without */
	/* throwing away sub-precision movements which might have been done. */
	/* sticks are handled later... */
	if ((in->type & IPF_CENTER) && (!is_stick))
		input_analog_value[port] -=
				(input_analog_value[port] * sensitivity / 100 - in->default_value) * 100 / sensitivity;

	current = input_analog_value[port];

	delta = 0;

	if (axis == X_AXIS)
	{
		int now;

		now = cpu_scalebyfcount(mouse_current_x - mouse_previous_x) + mouse_previous_x;
		delta = now - mouse_last_x;
		mouse_last_x = now;
	}
	else
	{
		int now;

		now = cpu_scalebyfcount(mouse_current_y - mouse_previous_y) + mouse_previous_y;
		delta = now - mouse_last_y;
		mouse_last_y = now;
	}

	if (osd_key_pressed(deckey)) delta -= keydelta;
	if (osd_key_pressed(inckey)) delta += keydelta;
	if (osd_joy_pressed(decjoy)) delta -= keydelta; /* LBO 120897 */
	if (osd_joy_pressed(incjoy)) delta += keydelta;

	if (clip != 0)
	{
		if (delta*sensitivity/100 < -clip)
			delta = -clip*100/sensitivity;
		else if (delta*sensitivity/100 > clip)
			delta = clip*100/sensitivity;
	}

	if (in->type & IPF_REVERSE) delta = -delta;
	if ( (axis == X_AXIS && x_reversed) || (axis == Y_AXIS && y_reversed) ) {
		delta = -delta;
	}

	if (is_stick)
	{
		int thenew, prev;

		/* center stick */
		if ((delta == 0) && (in->type & IPF_CENTER))
		{
			if (current > default_value)
				delta = -100 / sensitivity;
			if (current < default_value)
				delta =  100 / sensitivity;
		}

		/* An analog joystick which is not at zero position (or has just */
		/* moved there) takes precedence over all other computations */
		/* analog_x/y holds values from -128 to 128 (yes, 128, not 127) */

		if (axis == X_AXIS)
		{
			thenew  = analog_current_x;
			prev = analog_previous_x;
		}
		else
		{
			thenew  = analog_current_y;
			prev = analog_previous_y;
		}

		if ((thenew != 0) || (thenew-prev != 0))
		{
			delta=0;

			if (in->type & IPF_REVERSE)
			{
				thenew  = -thenew;
				prev = -prev;
			}
			if ( (axis == X_AXIS && x_reversed) || (axis == Y_AXIS && y_reversed) ) {
				thenew  = -thenew;
				prev = -prev;
			}


			/* scale by time */

			thenew = cpu_scalebyfcount(thenew - prev) + prev;

			current = default_value + (thenew * (max-min) * 100) / (256 * sensitivity);

		}
	}

	current += delta;

	if (check_bounds)
	{
		if (current*sensitivity/100 < min)
			current=min*100/sensitivity;
		if (current*sensitivity/100 > max)
			current=max*100/sensitivity;
	}

	input_analog_value[port]=current;

	input_port_value[port] &= ~in->mask;
	input_port_value[port] |= (current * sensitivity / 100) & in->mask;
}

#define MAX_INPUT_BITS 1024
static int impulsecount[MAX_INPUT_BITS];
static int waspressed[MAX_INPUT_BITS];
#define MAX_JOYSTICKS 3
#define MAX_PLAYERS 4
#ifdef MRU_JOYSTICK
static int update_serial_number = 1;
static int joyserial[MAX_JOYSTICKS*MAX_PLAYERS][4];
#else
static int joystick[MAX_JOYSTICKS*MAX_PLAYERS][4];
#endif

	/* clear all the values before proceeding */
void update_input_ports(void)
{
	int port,ib;
	struct InputPort *in;
	for (port = 0;port < MAX_INPUT_PORTS;port++)
	{
		input_port_value[port] = 0;
		input_vblank[port] = 0;
		input_analog[port] = 0;
	}

#ifndef MRU_JOYSTICK
	for (i = 0;i < 4*MAX_JOYSTICKS*MAX_PLAYERS;i++)
		joystick[i>>2][i&3] = 0;
#endif

	in = Machine->input_ports;

	if (in->type == IPT_END) return; 	/* nothing to do */

	/* make sure the InputPort definition is correct */
	if (in->type != IPT_PORT)
	{
		return;
	}
	else in++;

#ifdef MRU_JOYSTICK
	/* scan all the joystick ports */
	port = 0;
	while (in->type != IPT_END && port < MAX_INPUT_PORTS)
	{
		while (in->type != IPT_END && in->type != IPT_PORT)
		{
			if ((in->type & ~IPF_MASK) >= IPT_JOYSTICK_UP &&
				(in->type & ~IPF_MASK) <= IPT_JOYSTICKLEFT_RIGHT)
			{
				int key,joy;

				key = default_key(in);
				joy = default_joy(in);

				if ((key != 0 && key != IP_KEY_NONE) ||
					(joy != 0 && joy != IP_JOY_NONE))
				{
					int joynum,joydir,player;

					player = 0;
					if ((in->type & IPF_PLAYERMASK) == IPF_PLAYER2)
						player = 1;
					else if ((in->type & IPF_PLAYERMASK) == IPF_PLAYER3)
						player = 2;
					else if ((in->type & IPF_PLAYERMASK) == IPF_PLAYER4)
						player = 3;

					joynum = player * MAX_JOYSTICKS +
							 ((in->type & ~IPF_MASK) - IPT_JOYSTICK_UP) >> 2;
					joydir = ((in->type & ~IPF_MASK) - IPT_JOYSTICK_UP) & 3;

					if ((key != 0 && key != IP_KEY_NONE && osd_key_pressed(key)) ||
						(joy != 0 && joy != IP_JOY_NONE && osd_joy_pressed(joy)))
					{
						if (joyserial[joynum][joydir] == 0)
							joyserial[joynum][joydir] = update_serial_number;
					}
					else
						joyserial[joynum][joydir] = 0;
				}
			}
			in++;
		}

		port++;
		if (in->type == IPT_PORT) in++;
	}
	update_serial_number += 1;

	in = Machine->input_ports;

	/* already made sure the InputPort definition is correct */
	in++;
#endif


	/* scan all the input ports */
	port = 0;
	ib = 0;
	while (in->type != IPT_END && port < MAX_INPUT_PORTS)
	{
		struct InputPort *start;


		/* first of all, scan the whole input port definition and build the */
		/* default value. I must do it before checking for input because otherwise */
		/* multiple keys associated with the same input bit wouldn't work (the bit */
		/* would be reset to its default value by the second entry, regardless if */
		/* the key associated with the first entry was pressed) */
		start = in;
		while (in->type != IPT_END && in->type != IPT_PORT)
		{
			if ((in->type & ~IPF_MASK) != IPT_DIPSWITCH_SETTING)	/* skip dipswitch definitions */
			{
				input_port_value[port] =
						(input_port_value[port] & ~in->mask) | (in->default_value & in->mask);
			}

			in++;
		}

		/* now get back to the beginning of the input port and check the input bits. */
		for (in = start;
			 in->type != IPT_END && in->type != IPT_PORT;
			 in++, ib++)
		{
			if ((in->type & ~IPF_MASK) != IPT_DIPSWITCH_SETTING &&	/* skip dipswitch definitions */
					(in->type & IPF_UNUSED) == 0 &&	/* skip unused bits */
					!(nocheat && (in->type & IPF_CHEAT))) /* skip cheats if cheats disabled */
			{
				if ((in->type & ~IPF_MASK) == IPT_VBLANK)
				{
					input_vblank[port] ^= in->mask;
					input_port_value[port] ^= in->mask;
				}
				/* If it's an analog control, handle it appropriately */
				else if (((in->type & ~IPF_MASK) > IPT_ANALOG_START)
					  && ((in->type & ~IPF_MASK) < IPT_ANALOG_END  )) /* LBO 120897 */
				{
					input_analog[port]=in;
					/* reset the analog port on first access */
					if (input_analog_init[port])
					{
						input_analog_init[port] = 0;
						input_analog_value[port] = in->default_value * 100 / (in->arg & 0x000000ff);
					}
				}
				else
				{
					int key,joy;


					key = default_key(in);
					joy = default_joy(in);

					if ((key != 0 && key != IP_KEY_NONE && osd_key_pressed(key)) ||
							(joy != 0 && joy != IP_JOY_NONE && osd_joy_pressed(joy)))
					{
						/* skip if coin input and it's locked out */
						if ((in->type & ~IPF_MASK) >= IPT_COIN1 &&
							(in->type & ~IPF_MASK) <= IPT_COIN4 &&
                            				coinlockedout[(in->type & ~IPF_MASK) - IPT_COIN1])
						{
							continue;
						}

						/* if IPF_RESET set, reset the first CPU */
						if (in->type & IPF_RESETCPU && waspressed[ib] == 0)
							cpu_reset(0);

						if (in->type & IPF_IMPULSE)
						{
							if (waspressed[ib] == 0)
								impulsecount[ib] = in->arg;
								/* the input bit will be toggled later */
						}
						else if (in->type & IPF_TOGGLE)
						{
							if (waspressed[ib] == 0)
							{
								in->default_value ^= in->mask;
								input_port_value[port] ^= in->mask;
							}
						}
						else if ((in->type & ~IPF_MASK) >= IPT_JOYSTICK_UP &&
								(in->type & ~IPF_MASK) <= IPT_JOYSTICKLEFT_RIGHT)
						{
							int joynum,joydir,mask,player;


							player = 0;
							if ((in->type & IPF_PLAYERMASK) == IPF_PLAYER2) player = 1;
							else if ((in->type & IPF_PLAYERMASK) == IPF_PLAYER3) player = 2;
							else if ((in->type & IPF_PLAYERMASK) == IPF_PLAYER4) player = 3;

							joynum = player * MAX_JOYSTICKS +
									((in->type & ~IPF_MASK) - IPT_JOYSTICK_UP) >> 2;
							joydir = ((in->type & ~IPF_MASK) - IPT_JOYSTICK_UP) & 3;

							mask = in->mask;

#ifndef MRU_JOYSTICK
							/* avoid movement in two opposite directions */
							if (joystick[joynum][joydir ^ 1] != 0)
								mask = 0;
							else if (in->type & IPF_4WAY)
							{
								int dir;


								/* avoid diagonal movements */
								for (dir = 0;dir < 4;dir++)
								{
									if (joystick[joynum][dir] != 0)
										mask = 0;
								}
							}

							joystick[joynum][joydir] = 1;
#else
							/* avoid movement in two opposite directions */
							if (joyserial[joynum][joydir ^ 1] != 0)
								mask = 0;
							else if (in->type & IPF_4WAY)
							{
								int mru_dir = joydir;
								int mru_serial = 0;
								int dir;


								/* avoid diagonal movements, use mru button */
								for (dir = 0;dir < 4;dir++)
								{
									if (joyserial[joynum][dir] > mru_serial)
									{
										mru_serial = joyserial[joynum][dir];
										mru_dir = dir;
									}
								}

								if (mru_dir != joydir)
									mask = 0;
							}
#endif

							input_port_value[port] ^= mask;
						}
						else
							input_port_value[port] ^= in->mask;

						waspressed[ib] = 1;
					}
					else
						waspressed[ib] = 0;

					if ((in->type & IPF_IMPULSE) && impulsecount[ib] > 0)
					{
						impulsecount[ib]--;
						waspressed[ib] = 1;
						input_port_value[port] ^= in->mask;
					}
				}
			}
		}

		port++;
		if (in->type == IPT_PORT) in++;
	}
}



/* used the the CPU interface to notify that VBlank has ended, so we can update */
/* IPT_VBLANK input ports. */
void inputport_vblank_end(void)
{
	int port;
	int deltax,deltay;


	for (port = 0;port < MAX_INPUT_PORTS;port++)
	{
		if (input_vblank[port])
		{
			input_port_value[port] ^= input_vblank[port];
			input_vblank[port] = 0;
		}
	}

	/* update joysticks */
	analog_previous_x = analog_current_x;
	analog_previous_y = analog_current_y;
	osd_poll_joystick();
	osd_analogjoy_read(&analog_current_x, &analog_current_y);

	/* update mouse position */
	mouse_previous_x = mouse_current_x;
	mouse_previous_y = mouse_current_y;
	osd_trak_read(&deltax,&deltay);
	mouse_current_x += deltax;
	mouse_current_y += deltay;
}



int readinputport(int port)
{
	struct InputPort *in;

	/* Update analog ports on demand, unless IPF_CUSTOM_UPDATE type */
	/* In that case, the driver must call osd_update_port (int port) */
	/* directly. BW 980112 */
	in=input_analog[port];
	if (in)
	{
		if (!(in->type & IPF_CUSTOM_UPDATE))
			update_analog_port(port);
	}

	return input_port_value[port];
}

int input_port_0_r(int offset) { return readinputport(0); }
int input_port_1_r(int offset) { return readinputport(1); }
int input_port_2_r(int offset) { return readinputport(2); }
int input_port_3_r(int offset) { return readinputport(3); }
int input_port_4_r(int offset) { return readinputport(4); }
int input_port_5_r(int offset) { return readinputport(5); }
int input_port_6_r(int offset) { return readinputport(6); }
int input_port_7_r(int offset) { return readinputport(7); }
int input_port_8_r(int offset) { return readinputport(8); }
int input_port_9_r(int offset) { return readinputport(9); }
int input_port_10_r(int offset) { return readinputport(10); }
int input_port_11_r(int offset) { return readinputport(11); }
int input_port_12_r(int offset) { return readinputport(12); }
int input_port_13_r(int offset) { return readinputport(13); }
int input_port_14_r(int offset) { return readinputport(14); }
int input_port_15_r(int offset) { return readinputport(15); }
