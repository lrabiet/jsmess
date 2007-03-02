//============================================================
//
//  input.c - SDL implementation of MAME input routines
//
//  Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

// standard sdl header
#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include <ctype.h>

// MAME headers
#include "osdepend.h"
#include "driver.h"
#include "window.h"
#include "options.h"
#include "input.h"


//============================================================
//	IMPORTS
//============================================================

extern int verbose;


//============================================================
//	PARAMETERS
//============================================================

#define MAX_KEYBOARDS		1
#define MAX_MICE			8
#define MAX_JOYSTICKS		8
#define MAX_LIGHTGUNS		2

#define MAX_KEYS			256

#define MAX_JOY				256
#define MAX_AXES			8
#define MAX_BUTTONS			32
#define MAX_POV				4

enum
{
	ANALOG_TYPE_PADDLE = 0,
	ANALOG_TYPE_ADSTICK,
	ANALOG_TYPE_LIGHTGUN,
	ANALOG_TYPE_PEDAL,
	ANALOG_TYPE_DIAL,
	ANALOG_TYPE_TRACKBALL,
#ifdef MESS
	ANALOG_TYPE_MOUSE,
#endif // MESS
	ANALOG_TYPE_COUNT
};

enum
{
	SELECT_TYPE_KEYBOARD = 0,
	SELECT_TYPE_MOUSE,
	SELECT_TYPE_JOYSTICK,
	SELECT_TYPE_LIGHTGUN
};



//============================================================
//	MACROS
//============================================================

#define ELEMENTS(x)			(sizeof(x) / sizeof((x)[0]))



//============================================================
//	GLOBAL VARIABLES
//============================================================

UINT8						win_trying_to_quit;
int						win_use_mouse;



//============================================================
//	LOCAL VARIABLES
//============================================================

// this will be filled in dynamically
static os_code_info	codelist[MAX_KEYS+MAX_JOY];
static int					total_codes;

// global states
static int					input_paused;

// Controller override options
static float				a2d_deadzone;
static int					use_joystick;
static int					steadykey;
static UINT8				analog_type[ANALOG_TYPE_COUNT];

// keyboard states
static INT8					keyboard_state[0x3ff];

// additional key data
static INT8					oldkey[MAX_KEYS];
static INT8					currkey[MAX_KEYS];

// mouse states
static int					mouse_active;
static struct {
	int lX, lY;
	int buttons[MAX_BUTTONS];
} mouse_state[MAX_JOYSTICKS];

// joystick states
static int					joystick_count;
static UINT8					joystick_digital[MAX_JOYSTICKS][MAX_AXES];
//static char					joystick_name[MAX_JOYSTICKS][1024];
static SDL_Joystick			*joystick_device[MAX_JOYSTICKS];
static struct {
	int axes[MAX_AXES];
	int buttons[MAX_BUTTONS];
} joystick_state[MAX_JOYSTICKS];

// gun states
//static INT32					gun_axis[MAX_LIGHTGUNS][2];


//============================================================
//	PROTOTYPES
//============================================================

static void extract_input_config(void);
static void updatekeyboard(void);
static void init_keycodes(void);
static void add_joylist_entry(const char *name, os_code code, input_code standardcode);


//============================================================
//	KEYBOARD/JOYSTICK LIST
//============================================================

// macros for building/mapping keyboard codes
#define KEYCODE(sdlk, vk, ascii)    		((sdlk) | ((vk) << 10) | ((ascii) << 18))
#define SDLCODE(keycode)     			((keycode) & 0x3ff)
#define VKCODE(keycode)				(((keycode) >> 10) & 0xff)
#define ASCIICODE(keycode)			(((keycode) >> 18) & 0xff)

// macros for building/mapping joystick codes
#define JOYCODE(joy, type, index)	((index) | ((type) << 8) | ((joy) << 12) | 0x80000000)
#define JOYINDEX(joycode)			((joycode) & 0xff)
#define CODETYPE(joycode)			(((joycode) >> 8) & 0xf)
#define JOYNUM(joycode)				(((joycode) >> 12) & 0xf)

// macros for differentiating the two
#define IS_KEYBOARD_CODE(code)		(((code) & 0x80000000) == 0)
#define IS_JOYSTICK_CODE(code)		(((code) & 0x80000000) != 0)

// joystick types
enum
{
	CODETYPE_KEYBOARD = 0,
	CODETYPE_AXIS_NEG,
	CODETYPE_AXIS_POS,
	CODETYPE_POV_UP,
	CODETYPE_POV_DOWN,
	CODETYPE_POV_LEFT,
	CODETYPE_POV_RIGHT,
	CODETYPE_BUTTON,
	CODETYPE_JOYAXIS,
	CODETYPE_JOYAXIS_NEG,
	CODETYPE_JOYAXIS_POS,
	CODETYPE_MOUSEAXIS,
	CODETYPE_MOUSE_NEG,
	CODETYPE_MOUSE_POS,
	CODETYPE_MOUSEBUTTON,
	CODETYPE_GUNAXIS
};

// master keyboard translation table
const int def_key_trans_table[][4] =
{
	// MAME key			SDL key			vkey	ascii
	{ KEYCODE_ESC, 			SDLK_ESCAPE,	     	27,	27 },
	{ KEYCODE_1, 			SDLK_1,		     	'1',	'1' },
	{ KEYCODE_2, 			SDLK_2,		     	'2',	'2' },
	{ KEYCODE_3, 			SDLK_3,		     	'3',	'3' },
	{ KEYCODE_4, 			SDLK_4,		     	'4',	'4' },
	{ KEYCODE_5, 			SDLK_5,		     	'5',	'5' },
	{ KEYCODE_6, 			SDLK_6,		     	'6',	'6' },
	{ KEYCODE_7, 			SDLK_7,		     	'7',	'7' },
	{ KEYCODE_8, 			SDLK_8,		     	'8',	'8' },
	{ KEYCODE_9, 			SDLK_9,		     	'9',	'9' },
	{ KEYCODE_0, 			SDLK_0,		     	'0',	'0' },
	{ KEYCODE_MINUS, 		SDLK_MINUS, 	     	0xbd,	'-' },
	{ KEYCODE_EQUALS, 		SDLK_EQUALS,	      	0xbb,	'=' },
	{ KEYCODE_BACKSPACE,		SDLK_BACKSPACE,         8, 	8 },
	{ KEYCODE_TAB, 			SDLK_TAB, 	     	9, 	9 },
	{ KEYCODE_Q, 			SDLK_q,		     	'Q',	'Q' },
	{ KEYCODE_W, 			SDLK_w,		     	'W',	'W' },
	{ KEYCODE_E, 			SDLK_e,		     	'E',	'E' },
	{ KEYCODE_R, 			SDLK_r,		     	'R',	'R' },
	{ KEYCODE_T, 			SDLK_t,		     	'T',	'T' },
	{ KEYCODE_Y, 			SDLK_y,		     	'Y',	'Y' },
	{ KEYCODE_U, 			SDLK_u,		     	'U',	'U' },
	{ KEYCODE_I, 			SDLK_i,		     	'I',	'I' },
	{ KEYCODE_O, 			SDLK_o,		     	'O',	'O' },
	{ KEYCODE_P, 			SDLK_p,		     	'P',	'P' },
	{ KEYCODE_OPENBRACE,		SDLK_LEFTBRACKET, 	0xdb,	'[' },
	{ KEYCODE_CLOSEBRACE,		SDLK_RIGHTBRACKET, 	0xdd,	']' },
	{ KEYCODE_ENTER, 		SDLK_RETURN, 		13, 	13 },
	{ KEYCODE_LCONTROL, 		SDLK_LCTRL, 	        0, 	0 },
	{ KEYCODE_A, 			SDLK_a,		     	'A',	'A' },
	{ KEYCODE_S, 			SDLK_s,		     	'S',	'S' },
	{ KEYCODE_D, 			SDLK_d,		     	'D',	'D' },
	{ KEYCODE_F, 			SDLK_f,		     	'F',	'F' },
	{ KEYCODE_G, 			SDLK_g,		     	'G',	'G' },
	{ KEYCODE_H, 			SDLK_h,		     	'H',	'H' },
	{ KEYCODE_J, 			SDLK_j,		     	'J',	'J' },
	{ KEYCODE_K, 			SDLK_k,		     	'K',	'K' },
	{ KEYCODE_L, 			SDLK_l,		     	'L',	'L' },
	{ KEYCODE_COLON, 		SDLK_SEMICOLON,		0xba,	';' },
	{ KEYCODE_QUOTE, 		SDLK_QUOTE,		0xde,	'\'' },
	{ KEYCODE_TILDE, 		SDLK_BACKQUOTE,      	0xc0,	'`' },
	{ KEYCODE_LSHIFT, 		SDLK_LSHIFT, 		0, 	0 },
	{ KEYCODE_BACKSLASH,		SDLK_BACKSLASH, 	0xdc,	'\\' },
	{ KEYCODE_Z, 			SDLK_z,		     	'Z',	'Z' },
	{ KEYCODE_X, 			SDLK_x,		     	'X',	'X' },
	{ KEYCODE_C, 			SDLK_c,		     	'C',	'C' },
	{ KEYCODE_V, 			SDLK_v,		     	'V',	'V' },
	{ KEYCODE_B, 			SDLK_b,		     	'B',	'B' },
	{ KEYCODE_N, 			SDLK_n,		     	'N',	'N' },
	{ KEYCODE_M, 			SDLK_m,		     	'M',	'M' },
	{ KEYCODE_COMMA, 		SDLK_COMMA,	     	0xbc,	',' },
	{ KEYCODE_STOP, 		SDLK_PERIOD, 		0xbe,	'.' },
	{ KEYCODE_SLASH, 		SDLK_SLASH, 	     	0xbf,	'/' },
	{ KEYCODE_RSHIFT, 		SDLK_RSHIFT, 		0, 	0 },
	{ KEYCODE_ASTERISK, 		SDLK_ASTERISK, 		'*',	'*' },
	{ KEYCODE_LALT, 		SDLK_LALT, 		0, 	0 },
	{ KEYCODE_SPACE, 		SDLK_SPACE, 		' ',	' ' },
	{ KEYCODE_CAPSLOCK, 		SDLK_CAPSLOCK, 	     	0, 	0 },
	{ KEYCODE_F1, 			SDLK_F1,		0, 	0 },
	{ KEYCODE_F2, 			SDLK_F2,		0, 	0 },
	{ KEYCODE_F3, 			SDLK_F3,		0, 	0 },
	{ KEYCODE_F4, 			SDLK_F4,		0, 	0 },
	{ KEYCODE_F5, 			SDLK_F5,		0, 	0 },
	{ KEYCODE_F6, 			SDLK_F6,		0, 	0 },
	{ KEYCODE_F7, 			SDLK_F7,		0, 	0 },
	{ KEYCODE_F8, 			SDLK_F8,		0, 	0 },
	{ KEYCODE_F9, 			SDLK_F9,	    	0, 	0 },
	{ KEYCODE_F10, 			SDLK_F10,		0, 	0 },
	{ KEYCODE_NUMLOCK, 		SDLK_NUMLOCK,		0, 	0 },
	{ KEYCODE_SCRLOCK, 		SDLK_SCROLLOCK,		0, 	0 },
	{ KEYCODE_7_PAD, 		SDLK_KP7,		0, 	0 },
	{ KEYCODE_8_PAD, 		SDLK_KP8,		0, 	0 },
	{ KEYCODE_9_PAD, 		SDLK_KP9,		0, 	0 },
	{ KEYCODE_MINUS_PAD,		SDLK_KP_MINUS,	   	0, 	0 },
	{ KEYCODE_4_PAD, 		SDLK_KP4,		0, 	0 },
	{ KEYCODE_5_PAD, 		SDLK_KP5,		0, 	0 },
	{ KEYCODE_6_PAD, 		SDLK_KP6,		0, 	0 },
	{ KEYCODE_PLUS_PAD, 		SDLK_KP_PLUS,	   	0, 	0 },
	{ KEYCODE_1_PAD, 		SDLK_KP1,		0, 	0 },
	{ KEYCODE_2_PAD, 		SDLK_KP2,		0, 	0 },
	{ KEYCODE_3_PAD, 		SDLK_KP3,		0, 	0 },
	{ KEYCODE_0_PAD, 		SDLK_KP0,		0, 	0 },
	{ KEYCODE_DEL_PAD, 		SDLK_KP_PERIOD,		0, 	0 },
	{ KEYCODE_F11, 			SDLK_F11,		0,     	0 },
	{ KEYCODE_F12, 			SDLK_F12,		0,     	0 },
	{ KEYCODE_F13, 			SDLK_F13,		0,     	0 },
	{ KEYCODE_F14, 			SDLK_F14,		0,     	0 },
	{ KEYCODE_F15, 			SDLK_F15,		0,     	0 },
	{ KEYCODE_ENTER_PAD,		SDLK_KP_ENTER,		0, 	0 },
	{ KEYCODE_RCONTROL, 		SDLK_RCTRL,		0, 	0 },
	{ KEYCODE_SLASH_PAD,		SDLK_KP_DIVIDE,		0, 	0 },
	{ KEYCODE_PRTSCR, 		SDLK_PRINT, 		0, 	0 },
	{ KEYCODE_RALT, 		SDLK_RALT,		0,     	0 },
	{ KEYCODE_HOME, 		SDLK_HOME,		0,     	0 },
	{ KEYCODE_UP, 			SDLK_UP,		0,     	0 },
	{ KEYCODE_PGUP, 		SDLK_PAGEUP,		0,     	0 },
	{ KEYCODE_LEFT, 		SDLK_LEFT,		0,     	0 },
	{ KEYCODE_RIGHT, 		SDLK_RIGHT,		0,     	0 },
	{ KEYCODE_END, 			SDLK_END,		0,     	0 },
	{ KEYCODE_DOWN, 		SDLK_DOWN,		0,     	0 },
	{ KEYCODE_PGDN, 		SDLK_PAGEDOWN,		0,     	0 },
	{ KEYCODE_INSERT, 		SDLK_INSERT,		0, 	0 },
	{ KEYCODE_DEL, 			SDLK_DELETE,		0, 	0 },
	{ KEYCODE_LWIN, 		SDLK_LSUPER,		0,     	0 },
	{ KEYCODE_RWIN, 		SDLK_RSUPER,		0,     	0 },
	{ KEYCODE_MENU, 		SDLK_MENU,		0,     	0 },
	{ -1, -1, -1, -1 }
};

static const char *key_name_table[] =
{
	"ESC",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"0",
	"-",
	"=",
	"BS",
	"TAB",
	"Q",
	"W",
	"E",
	"R",
	"T",
	"Y",
	"U",
	"I",
	"O",
	"P",
	"[",
	"]",
	"ENTER",
	"LCTRL",
	"A",
	"S",
	"D",
	"F",
	"G",
	"H",
	"J",
	"K",
	"L",
	";",
	"\"",
	"`",
	"LSHIFT",
	"\\",
	"Z",
	"X",
	"C",
	"V",
	"B",
	"N",
	"M",
	",",
	".",
	"/",
	"RSHIFT",
	"*",
	"LALT",
	"SPACE",
	"CAPSLOCK",
	"F1",
	"F2",
	"F3",
	"F4",
	"F5",
	"F6",
	"F7",
	"F8",
	"F9",
	"F10",
	"NUMLOCK",
	"SCROLLLOCK",
	"KP 7",
	"KP 8",
	"KP 9",
	"KP -",
	"KP 4",
	"KP 5",
	"KP 6",
	"KP +",
	"KP 1",
	"KP 2",
	"KP 3",
	"KP 0",
	"KP .",
	"F11",
	"F12",
	"F13",
	"F14",
	"F15",
	"KP ENTER",
	"RCTRL",
	"KP /",
	"PRINTSCRN",
	"RALT",
	"HOME",
	"UP",
	"PAGEUP",
	"LEFT",
	"RIGHT",
	"END",
	"DOWN",
	"PAGEDN",
	"INS",
	"DEL",
	"LWIN",
	"RWIN",
	"MENU",
	" ",
};

// master joystick translation table
static int joy_trans_table[][2] =
{
	// internal code					MAME code
	{ JOYCODE(0, CODETYPE_AXIS_NEG, 0),		JOYCODE_1_LEFT },
	{ JOYCODE(0, CODETYPE_AXIS_POS, 0),		JOYCODE_1_RIGHT },
	{ JOYCODE(0, CODETYPE_AXIS_NEG, 1),		JOYCODE_1_UP },
	{ JOYCODE(0, CODETYPE_AXIS_POS, 1),		JOYCODE_1_DOWN },
	{ JOYCODE(0, CODETYPE_BUTTON, 0),		JOYCODE_1_BUTTON1 },
	{ JOYCODE(0, CODETYPE_BUTTON, 1),		JOYCODE_1_BUTTON2 },
	{ JOYCODE(0, CODETYPE_BUTTON, 2),		JOYCODE_1_BUTTON3 },
	{ JOYCODE(0, CODETYPE_BUTTON, 3),		JOYCODE_1_BUTTON4 },
	{ JOYCODE(0, CODETYPE_BUTTON, 4),		JOYCODE_1_BUTTON5 },
	{ JOYCODE(0, CODETYPE_BUTTON, 5),		JOYCODE_1_BUTTON6 },
	{ JOYCODE(0, CODETYPE_BUTTON, 6),		JOYCODE_1_BUTTON7 },
	{ JOYCODE(0, CODETYPE_BUTTON, 7),		JOYCODE_1_BUTTON8 },
	{ JOYCODE(0, CODETYPE_BUTTON, 8),		JOYCODE_1_BUTTON9 },
	{ JOYCODE(0, CODETYPE_BUTTON, 9),		JOYCODE_1_BUTTON10 },
	{ JOYCODE(0, CODETYPE_BUTTON, 10),		JOYCODE_1_BUTTON11 },
	{ JOYCODE(0, CODETYPE_BUTTON, 11),		JOYCODE_1_BUTTON12 },
	{ JOYCODE(0, CODETYPE_BUTTON, 12),		JOYCODE_1_BUTTON13 },
	{ JOYCODE(0, CODETYPE_BUTTON, 13),		JOYCODE_1_BUTTON14 },
	{ JOYCODE(0, CODETYPE_BUTTON, 14),		JOYCODE_1_BUTTON15 },
	{ JOYCODE(0, CODETYPE_BUTTON, 15),		JOYCODE_1_BUTTON16 },
	{ JOYCODE(0, CODETYPE_JOYAXIS, 0),		JOYCODE_1_ANALOG_X },
	{ JOYCODE(0, CODETYPE_JOYAXIS_NEG, 0),	JOYCODE_1_ANALOG_X_NEG },
	{ JOYCODE(0, CODETYPE_JOYAXIS_POS, 0),	JOYCODE_1_ANALOG_X_POS },
	{ JOYCODE(0, CODETYPE_JOYAXIS, 1),		JOYCODE_1_ANALOG_Y },
	{ JOYCODE(0, CODETYPE_JOYAXIS_NEG, 1),	JOYCODE_1_ANALOG_Y_NEG },
	{ JOYCODE(0, CODETYPE_JOYAXIS_POS, 1),	JOYCODE_1_ANALOG_Y_POS },
	{ JOYCODE(0, CODETYPE_JOYAXIS, 2),		JOYCODE_1_ANALOG_Z },
	{ JOYCODE(0, CODETYPE_JOYAXIS_NEG, 2),	JOYCODE_1_ANALOG_Z_NEG },
	{ JOYCODE(0, CODETYPE_JOYAXIS_POS, 2),	JOYCODE_1_ANALOG_Z_POS },

	{ JOYCODE(1, CODETYPE_AXIS_NEG, 0),		JOYCODE_2_LEFT },
	{ JOYCODE(1, CODETYPE_AXIS_POS, 0),		JOYCODE_2_RIGHT },
	{ JOYCODE(1, CODETYPE_AXIS_NEG, 1),		JOYCODE_2_UP },
	{ JOYCODE(1, CODETYPE_AXIS_POS, 1),		JOYCODE_2_DOWN },
	{ JOYCODE(1, CODETYPE_BUTTON, 0),		JOYCODE_2_BUTTON1 },
	{ JOYCODE(1, CODETYPE_BUTTON, 1),		JOYCODE_2_BUTTON2 },
	{ JOYCODE(1, CODETYPE_BUTTON, 2),		JOYCODE_2_BUTTON3 },
	{ JOYCODE(1, CODETYPE_BUTTON, 3),		JOYCODE_2_BUTTON4 },
	{ JOYCODE(1, CODETYPE_BUTTON, 4),		JOYCODE_2_BUTTON5 },
	{ JOYCODE(1, CODETYPE_BUTTON, 5),		JOYCODE_2_BUTTON6 },
	{ JOYCODE(1, CODETYPE_BUTTON, 6),		JOYCODE_2_BUTTON7 },
	{ JOYCODE(1, CODETYPE_BUTTON, 7),		JOYCODE_2_BUTTON8 },
	{ JOYCODE(1, CODETYPE_BUTTON, 8),		JOYCODE_2_BUTTON9 },
	{ JOYCODE(1, CODETYPE_BUTTON, 9),		JOYCODE_2_BUTTON10 },
	{ JOYCODE(1, CODETYPE_BUTTON, 10),		JOYCODE_2_BUTTON11 },
	{ JOYCODE(1, CODETYPE_BUTTON, 11),		JOYCODE_2_BUTTON12 },
	{ JOYCODE(1, CODETYPE_BUTTON, 12),		JOYCODE_2_BUTTON13 },
	{ JOYCODE(1, CODETYPE_BUTTON, 13),		JOYCODE_2_BUTTON14 },
	{ JOYCODE(1, CODETYPE_BUTTON, 14),		JOYCODE_2_BUTTON15 },
	{ JOYCODE(1, CODETYPE_BUTTON, 15),		JOYCODE_2_BUTTON16 },
	{ JOYCODE(1, CODETYPE_JOYAXIS, 0),		JOYCODE_2_ANALOG_X },
	{ JOYCODE(1, CODETYPE_JOYAXIS_NEG, 0),	JOYCODE_2_ANALOG_X_NEG },
	{ JOYCODE(1, CODETYPE_JOYAXIS_POS, 0),	JOYCODE_2_ANALOG_X_POS },
	{ JOYCODE(1, CODETYPE_JOYAXIS, 1),		JOYCODE_2_ANALOG_Y },
	{ JOYCODE(1, CODETYPE_JOYAXIS_NEG, 1),	JOYCODE_2_ANALOG_Y_NEG },
	{ JOYCODE(1, CODETYPE_JOYAXIS_POS, 1),	JOYCODE_2_ANALOG_Y_POS },
	{ JOYCODE(1, CODETYPE_JOYAXIS, 2),		JOYCODE_2_ANALOG_Z },
	{ JOYCODE(1, CODETYPE_JOYAXIS_NEG, 2),	JOYCODE_2_ANALOG_Z_NEG },
	{ JOYCODE(1, CODETYPE_JOYAXIS_POS, 2),	JOYCODE_2_ANALOG_Z_POS },

	{ JOYCODE(2, CODETYPE_AXIS_NEG, 0),		JOYCODE_3_LEFT },
	{ JOYCODE(2, CODETYPE_AXIS_POS, 0),		JOYCODE_3_RIGHT },
	{ JOYCODE(2, CODETYPE_AXIS_NEG, 1),		JOYCODE_3_UP },
	{ JOYCODE(2, CODETYPE_AXIS_POS, 1),		JOYCODE_3_DOWN },
	{ JOYCODE(2, CODETYPE_BUTTON, 0),		JOYCODE_3_BUTTON1 },
	{ JOYCODE(2, CODETYPE_BUTTON, 1),		JOYCODE_3_BUTTON2 },
	{ JOYCODE(2, CODETYPE_BUTTON, 2),		JOYCODE_3_BUTTON3 },
	{ JOYCODE(2, CODETYPE_BUTTON, 3),		JOYCODE_3_BUTTON4 },
	{ JOYCODE(2, CODETYPE_BUTTON, 4),		JOYCODE_3_BUTTON5 },
	{ JOYCODE(2, CODETYPE_BUTTON, 5),		JOYCODE_3_BUTTON6 },
	{ JOYCODE(2, CODETYPE_BUTTON, 6),		JOYCODE_3_BUTTON7 },
	{ JOYCODE(2, CODETYPE_BUTTON, 7),		JOYCODE_3_BUTTON8 },
	{ JOYCODE(2, CODETYPE_BUTTON, 8),		JOYCODE_3_BUTTON9 },
	{ JOYCODE(2, CODETYPE_BUTTON, 9),		JOYCODE_3_BUTTON10 },
	{ JOYCODE(2, CODETYPE_BUTTON, 10),		JOYCODE_3_BUTTON11 },
	{ JOYCODE(2, CODETYPE_BUTTON, 11),		JOYCODE_3_BUTTON12 },
	{ JOYCODE(2, CODETYPE_BUTTON, 12),		JOYCODE_3_BUTTON13 },
	{ JOYCODE(2, CODETYPE_BUTTON, 13),		JOYCODE_3_BUTTON14 },
	{ JOYCODE(2, CODETYPE_BUTTON, 14),		JOYCODE_3_BUTTON15 },
	{ JOYCODE(2, CODETYPE_BUTTON, 15),		JOYCODE_3_BUTTON16 },
	{ JOYCODE(2, CODETYPE_JOYAXIS, 0),		JOYCODE_3_ANALOG_X },
	{ JOYCODE(2, CODETYPE_JOYAXIS_NEG, 0),	JOYCODE_3_ANALOG_X_NEG },
	{ JOYCODE(2, CODETYPE_JOYAXIS_POS, 0),	JOYCODE_3_ANALOG_X_POS },
	{ JOYCODE(2, CODETYPE_JOYAXIS, 1),		JOYCODE_3_ANALOG_Y },
	{ JOYCODE(2, CODETYPE_JOYAXIS_NEG, 1),	JOYCODE_3_ANALOG_Y_NEG },
	{ JOYCODE(2, CODETYPE_JOYAXIS_POS, 1),	JOYCODE_3_ANALOG_Y_POS },
	{ JOYCODE(2, CODETYPE_JOYAXIS, 2),		JOYCODE_3_ANALOG_Z },
	{ JOYCODE(2, CODETYPE_JOYAXIS_NEG, 2),	JOYCODE_3_ANALOG_Z_NEG },
	{ JOYCODE(2, CODETYPE_JOYAXIS_POS, 2),	JOYCODE_3_ANALOG_Z_POS },

	{ JOYCODE(3, CODETYPE_AXIS_NEG, 0),		JOYCODE_4_LEFT },
	{ JOYCODE(3, CODETYPE_AXIS_POS, 0),		JOYCODE_4_RIGHT },
	{ JOYCODE(3, CODETYPE_AXIS_NEG, 1),		JOYCODE_4_UP },
	{ JOYCODE(3, CODETYPE_AXIS_POS, 1),		JOYCODE_4_DOWN },
	{ JOYCODE(3, CODETYPE_BUTTON, 0),		JOYCODE_4_BUTTON1 },
	{ JOYCODE(3, CODETYPE_BUTTON, 1),		JOYCODE_4_BUTTON2 },
	{ JOYCODE(3, CODETYPE_BUTTON, 2),		JOYCODE_4_BUTTON3 },
	{ JOYCODE(3, CODETYPE_BUTTON, 3),		JOYCODE_4_BUTTON4 },
	{ JOYCODE(3, CODETYPE_BUTTON, 4),		JOYCODE_4_BUTTON5 },
	{ JOYCODE(3, CODETYPE_BUTTON, 5),		JOYCODE_4_BUTTON6 },
	{ JOYCODE(3, CODETYPE_BUTTON, 6),		JOYCODE_4_BUTTON7 },
	{ JOYCODE(3, CODETYPE_BUTTON, 7),		JOYCODE_4_BUTTON8 },
	{ JOYCODE(3, CODETYPE_BUTTON, 8),		JOYCODE_4_BUTTON9 },
	{ JOYCODE(3, CODETYPE_BUTTON, 9),		JOYCODE_4_BUTTON10 },
	{ JOYCODE(3, CODETYPE_BUTTON, 10),		JOYCODE_4_BUTTON11 },
	{ JOYCODE(3, CODETYPE_BUTTON, 11),		JOYCODE_4_BUTTON12 },
	{ JOYCODE(3, CODETYPE_BUTTON, 12),		JOYCODE_4_BUTTON13 },
	{ JOYCODE(3, CODETYPE_BUTTON, 13),		JOYCODE_4_BUTTON14 },
	{ JOYCODE(3, CODETYPE_BUTTON, 14),		JOYCODE_4_BUTTON15 },
	{ JOYCODE(3, CODETYPE_BUTTON, 15),		JOYCODE_4_BUTTON16 },
	{ JOYCODE(3, CODETYPE_JOYAXIS, 0),		JOYCODE_4_ANALOG_X },
	{ JOYCODE(3, CODETYPE_JOYAXIS_NEG, 0),	JOYCODE_4_ANALOG_X_NEG },
	{ JOYCODE(3, CODETYPE_JOYAXIS_POS, 0),	JOYCODE_4_ANALOG_X_POS },
	{ JOYCODE(3, CODETYPE_JOYAXIS, 1),		JOYCODE_4_ANALOG_Y },
	{ JOYCODE(3, CODETYPE_JOYAXIS_NEG, 1),	JOYCODE_4_ANALOG_Y_NEG },
	{ JOYCODE(3, CODETYPE_JOYAXIS_POS, 1),	JOYCODE_4_ANALOG_Y_POS },
	{ JOYCODE(3, CODETYPE_JOYAXIS, 2),		JOYCODE_4_ANALOG_Z },
	{ JOYCODE(3, CODETYPE_JOYAXIS_NEG, 2),	JOYCODE_4_ANALOG_Z_NEG },
	{ JOYCODE(3, CODETYPE_JOYAXIS_POS, 2),	JOYCODE_4_ANALOG_Z_POS },

	{ JOYCODE(4, CODETYPE_AXIS_NEG, 0),		JOYCODE_5_LEFT },
	{ JOYCODE(4, CODETYPE_AXIS_POS, 0),		JOYCODE_5_RIGHT },
	{ JOYCODE(4, CODETYPE_AXIS_NEG, 1),		JOYCODE_5_UP },
	{ JOYCODE(4, CODETYPE_AXIS_POS, 1),		JOYCODE_5_DOWN },
	{ JOYCODE(4, CODETYPE_BUTTON, 0),		JOYCODE_5_BUTTON1 },
	{ JOYCODE(4, CODETYPE_BUTTON, 1),		JOYCODE_5_BUTTON2 },
	{ JOYCODE(4, CODETYPE_BUTTON, 2),		JOYCODE_5_BUTTON3 },
	{ JOYCODE(4, CODETYPE_BUTTON, 3),		JOYCODE_5_BUTTON4 },
	{ JOYCODE(4, CODETYPE_BUTTON, 4),		JOYCODE_5_BUTTON5 },
	{ JOYCODE(4, CODETYPE_BUTTON, 5),		JOYCODE_5_BUTTON6 },
	{ JOYCODE(4, CODETYPE_BUTTON, 6),		JOYCODE_5_BUTTON7 },
	{ JOYCODE(4, CODETYPE_BUTTON, 7),		JOYCODE_5_BUTTON8 },
	{ JOYCODE(4, CODETYPE_BUTTON, 8),		JOYCODE_5_BUTTON9 },
	{ JOYCODE(4, CODETYPE_BUTTON, 9),		JOYCODE_5_BUTTON10 },
	{ JOYCODE(4, CODETYPE_BUTTON, 10),		JOYCODE_5_BUTTON11 },
	{ JOYCODE(4, CODETYPE_BUTTON, 11),		JOYCODE_5_BUTTON12 },
	{ JOYCODE(4, CODETYPE_BUTTON, 12),		JOYCODE_5_BUTTON13 },
	{ JOYCODE(4, CODETYPE_BUTTON, 13),		JOYCODE_5_BUTTON14 },
	{ JOYCODE(4, CODETYPE_BUTTON, 14),		JOYCODE_5_BUTTON15 },
	{ JOYCODE(4, CODETYPE_BUTTON, 15),		JOYCODE_5_BUTTON16 },
	{ JOYCODE(4, CODETYPE_JOYAXIS, 0),		JOYCODE_5_ANALOG_X },
	{ JOYCODE(4, CODETYPE_JOYAXIS_NEG, 0),	JOYCODE_5_ANALOG_X_NEG },
	{ JOYCODE(4, CODETYPE_JOYAXIS_POS, 0),	JOYCODE_5_ANALOG_X_POS },
	{ JOYCODE(4, CODETYPE_JOYAXIS, 1),		JOYCODE_5_ANALOG_Y },
	{ JOYCODE(4, CODETYPE_JOYAXIS_NEG, 1),	JOYCODE_5_ANALOG_Y_NEG },
	{ JOYCODE(4, CODETYPE_JOYAXIS_POS, 1),	JOYCODE_5_ANALOG_Y_POS },
	{ JOYCODE(4, CODETYPE_JOYAXIS, 2),		JOYCODE_5_ANALOG_Z },
	{ JOYCODE(4, CODETYPE_JOYAXIS_NEG, 2),	JOYCODE_5_ANALOG_Z_NEG },
	{ JOYCODE(4, CODETYPE_JOYAXIS_POS, 2),	JOYCODE_5_ANALOG_Z_POS },

	{ JOYCODE(5, CODETYPE_AXIS_NEG, 0),		JOYCODE_6_LEFT },
	{ JOYCODE(5, CODETYPE_AXIS_POS, 0),		JOYCODE_6_RIGHT },
	{ JOYCODE(5, CODETYPE_AXIS_NEG, 1),		JOYCODE_6_UP },
	{ JOYCODE(5, CODETYPE_AXIS_POS, 1),		JOYCODE_6_DOWN },
	{ JOYCODE(5, CODETYPE_BUTTON, 0),		JOYCODE_6_BUTTON1 },
	{ JOYCODE(5, CODETYPE_BUTTON, 1),		JOYCODE_6_BUTTON2 },
	{ JOYCODE(5, CODETYPE_BUTTON, 2),		JOYCODE_6_BUTTON3 },
	{ JOYCODE(5, CODETYPE_BUTTON, 3),		JOYCODE_6_BUTTON4 },
	{ JOYCODE(5, CODETYPE_BUTTON, 4),		JOYCODE_6_BUTTON5 },
	{ JOYCODE(5, CODETYPE_BUTTON, 5),		JOYCODE_6_BUTTON6 },
	{ JOYCODE(5, CODETYPE_BUTTON, 6),		JOYCODE_6_BUTTON7 },
	{ JOYCODE(5, CODETYPE_BUTTON, 7),		JOYCODE_6_BUTTON8 },
	{ JOYCODE(5, CODETYPE_BUTTON, 8),		JOYCODE_6_BUTTON9 },
	{ JOYCODE(5, CODETYPE_BUTTON, 9),		JOYCODE_6_BUTTON10 },
	{ JOYCODE(5, CODETYPE_BUTTON, 10),		JOYCODE_6_BUTTON11 },
	{ JOYCODE(5, CODETYPE_BUTTON, 11),		JOYCODE_6_BUTTON12 },
	{ JOYCODE(5, CODETYPE_BUTTON, 12),		JOYCODE_6_BUTTON13 },
	{ JOYCODE(5, CODETYPE_BUTTON, 13),		JOYCODE_6_BUTTON14 },
	{ JOYCODE(5, CODETYPE_BUTTON, 14),		JOYCODE_6_BUTTON15 },
	{ JOYCODE(5, CODETYPE_BUTTON, 15),		JOYCODE_6_BUTTON16 },
	{ JOYCODE(5, CODETYPE_JOYAXIS, 0),		JOYCODE_6_ANALOG_X },
	{ JOYCODE(5, CODETYPE_JOYAXIS_NEG, 0),	JOYCODE_6_ANALOG_X_NEG },
	{ JOYCODE(5, CODETYPE_JOYAXIS_POS, 0),	JOYCODE_6_ANALOG_X_POS },
	{ JOYCODE(5, CODETYPE_JOYAXIS, 1),		JOYCODE_6_ANALOG_Y },
	{ JOYCODE(5, CODETYPE_JOYAXIS_NEG, 1),	JOYCODE_6_ANALOG_Y_NEG },
	{ JOYCODE(5, CODETYPE_JOYAXIS_POS, 1),	JOYCODE_6_ANALOG_Y_POS },
	{ JOYCODE(5, CODETYPE_JOYAXIS, 2),		JOYCODE_6_ANALOG_Z },
	{ JOYCODE(5, CODETYPE_JOYAXIS_NEG, 2),	JOYCODE_6_ANALOG_Z_NEG },
	{ JOYCODE(5, CODETYPE_JOYAXIS_POS, 2),	JOYCODE_6_ANALOG_Z_POS },

	{ JOYCODE(6, CODETYPE_AXIS_NEG, 0),		JOYCODE_7_LEFT },
	{ JOYCODE(6, CODETYPE_AXIS_POS, 0),		JOYCODE_7_RIGHT },
	{ JOYCODE(6, CODETYPE_AXIS_NEG, 1),		JOYCODE_7_UP },
	{ JOYCODE(6, CODETYPE_AXIS_POS, 1),		JOYCODE_7_DOWN },
	{ JOYCODE(6, CODETYPE_BUTTON, 0),		JOYCODE_7_BUTTON1 },
	{ JOYCODE(6, CODETYPE_BUTTON, 1),		JOYCODE_7_BUTTON2 },
	{ JOYCODE(6, CODETYPE_BUTTON, 2),		JOYCODE_7_BUTTON3 },
	{ JOYCODE(6, CODETYPE_BUTTON, 3),		JOYCODE_7_BUTTON4 },
	{ JOYCODE(6, CODETYPE_BUTTON, 4),		JOYCODE_7_BUTTON5 },
	{ JOYCODE(6, CODETYPE_BUTTON, 5),		JOYCODE_7_BUTTON6 },
	{ JOYCODE(6, CODETYPE_BUTTON, 6),		JOYCODE_7_BUTTON7 },
	{ JOYCODE(6, CODETYPE_BUTTON, 7),		JOYCODE_7_BUTTON8 },
	{ JOYCODE(6, CODETYPE_BUTTON, 8),		JOYCODE_7_BUTTON9 },
	{ JOYCODE(6, CODETYPE_BUTTON, 9),		JOYCODE_7_BUTTON10 },
	{ JOYCODE(6, CODETYPE_BUTTON, 10),		JOYCODE_7_BUTTON11 },
	{ JOYCODE(6, CODETYPE_BUTTON, 11),		JOYCODE_7_BUTTON12 },
	{ JOYCODE(6, CODETYPE_BUTTON, 12),		JOYCODE_7_BUTTON13 },
	{ JOYCODE(6, CODETYPE_BUTTON, 13),		JOYCODE_7_BUTTON14 },
	{ JOYCODE(6, CODETYPE_BUTTON, 14),		JOYCODE_7_BUTTON15 },
	{ JOYCODE(6, CODETYPE_BUTTON, 15),		JOYCODE_7_BUTTON16 },
	{ JOYCODE(6, CODETYPE_JOYAXIS, 0),		JOYCODE_7_ANALOG_X },
	{ JOYCODE(6, CODETYPE_JOYAXIS_NEG, 0),	JOYCODE_7_ANALOG_X_NEG },
	{ JOYCODE(6, CODETYPE_JOYAXIS_POS, 0),	JOYCODE_7_ANALOG_X_POS },
	{ JOYCODE(6, CODETYPE_JOYAXIS, 1),		JOYCODE_7_ANALOG_Y },
	{ JOYCODE(6, CODETYPE_JOYAXIS_NEG, 1),	JOYCODE_7_ANALOG_Y_NEG },
	{ JOYCODE(6, CODETYPE_JOYAXIS_POS, 1),	JOYCODE_7_ANALOG_Y_POS },
	{ JOYCODE(6, CODETYPE_JOYAXIS, 2),		JOYCODE_7_ANALOG_Z },
	{ JOYCODE(6, CODETYPE_JOYAXIS_NEG, 2),	JOYCODE_7_ANALOG_Z_NEG },
	{ JOYCODE(6, CODETYPE_JOYAXIS_POS, 2),	JOYCODE_7_ANALOG_Z_POS },

	{ JOYCODE(7, CODETYPE_AXIS_NEG, 0),		JOYCODE_8_LEFT },
	{ JOYCODE(7, CODETYPE_AXIS_POS, 0),		JOYCODE_8_RIGHT },
	{ JOYCODE(7, CODETYPE_AXIS_NEG, 1),		JOYCODE_8_UP },
	{ JOYCODE(7, CODETYPE_AXIS_POS, 1),		JOYCODE_8_DOWN },
	{ JOYCODE(7, CODETYPE_BUTTON, 0),		JOYCODE_8_BUTTON1 },
	{ JOYCODE(7, CODETYPE_BUTTON, 1),		JOYCODE_8_BUTTON2 },
	{ JOYCODE(7, CODETYPE_BUTTON, 2),		JOYCODE_8_BUTTON3 },
	{ JOYCODE(7, CODETYPE_BUTTON, 3),		JOYCODE_8_BUTTON4 },
	{ JOYCODE(7, CODETYPE_BUTTON, 4),		JOYCODE_8_BUTTON5 },
	{ JOYCODE(7, CODETYPE_BUTTON, 5),		JOYCODE_8_BUTTON6 },
	{ JOYCODE(7, CODETYPE_BUTTON, 6),		JOYCODE_8_BUTTON7 },
	{ JOYCODE(7, CODETYPE_BUTTON, 7),		JOYCODE_8_BUTTON8 },
	{ JOYCODE(7, CODETYPE_BUTTON, 8),		JOYCODE_8_BUTTON9 },
	{ JOYCODE(7, CODETYPE_BUTTON, 9),		JOYCODE_8_BUTTON10 },
	{ JOYCODE(7, CODETYPE_BUTTON, 10),		JOYCODE_8_BUTTON11 },
	{ JOYCODE(7, CODETYPE_BUTTON, 11),		JOYCODE_8_BUTTON12 },
	{ JOYCODE(7, CODETYPE_BUTTON, 12),		JOYCODE_8_BUTTON13 },
	{ JOYCODE(7, CODETYPE_BUTTON, 13),		JOYCODE_8_BUTTON14 },
	{ JOYCODE(7, CODETYPE_BUTTON, 14),		JOYCODE_8_BUTTON15 },
	{ JOYCODE(7, CODETYPE_BUTTON, 15),		JOYCODE_8_BUTTON16 },
	{ JOYCODE(7, CODETYPE_JOYAXIS, 0),		JOYCODE_8_ANALOG_X },
	{ JOYCODE(7, CODETYPE_JOYAXIS_NEG, 0),	JOYCODE_8_ANALOG_X_NEG },
	{ JOYCODE(7, CODETYPE_JOYAXIS_POS, 0),	JOYCODE_8_ANALOG_X_POS },
	{ JOYCODE(7, CODETYPE_JOYAXIS, 1),		JOYCODE_8_ANALOG_Y },
	{ JOYCODE(7, CODETYPE_JOYAXIS_NEG, 1),	JOYCODE_8_ANALOG_Y_NEG },
	{ JOYCODE(7, CODETYPE_JOYAXIS_POS, 1),	JOYCODE_8_ANALOG_Y_POS },
	{ JOYCODE(7, CODETYPE_JOYAXIS, 2),		JOYCODE_8_ANALOG_Z },
	{ JOYCODE(7, CODETYPE_JOYAXIS_NEG, 2),	JOYCODE_8_ANALOG_Z_NEG },
	{ JOYCODE(7, CODETYPE_JOYAXIS_POS, 2),	JOYCODE_8_ANALOG_Z_POS },

	{ JOYCODE(0, CODETYPE_MOUSEBUTTON, 0), 	MOUSECODE_1_BUTTON1 },
	{ JOYCODE(0, CODETYPE_MOUSEBUTTON, 1), 	MOUSECODE_1_BUTTON2 },
	{ JOYCODE(0, CODETYPE_MOUSEBUTTON, 2), 	MOUSECODE_1_BUTTON3 },
	{ JOYCODE(0, CODETYPE_MOUSEBUTTON, 3), 	MOUSECODE_1_BUTTON4 },
	{ JOYCODE(0, CODETYPE_MOUSEBUTTON, 4), 	MOUSECODE_1_BUTTON5 },
	{ JOYCODE(0, CODETYPE_MOUSEAXIS, 0),	MOUSECODE_1_ANALOG_X },
	{ JOYCODE(0, CODETYPE_MOUSEAXIS, 1),	MOUSECODE_1_ANALOG_Y },
	{ JOYCODE(0, CODETYPE_MOUSEAXIS, 2),	MOUSECODE_1_ANALOG_Z },
	{ JOYCODE(0, CODETYPE_MOUSE_NEG, 0),	MOUSECODE_1_X_NEG },
	{ JOYCODE(0, CODETYPE_MOUSE_POS, 0),	MOUSECODE_1_X_POS },
	{ JOYCODE(0, CODETYPE_MOUSE_NEG, 1),	MOUSECODE_1_Y_NEG },
	{ JOYCODE(0, CODETYPE_MOUSE_POS, 1),	MOUSECODE_1_Y_POS },
	{ JOYCODE(0, CODETYPE_MOUSE_NEG, 2),	MOUSECODE_1_Z_NEG },
	{ JOYCODE(0, CODETYPE_MOUSE_POS, 2),	MOUSECODE_1_Z_POS },

	{ JOYCODE(1, CODETYPE_MOUSEBUTTON, 0), 	MOUSECODE_2_BUTTON1 },
	{ JOYCODE(1, CODETYPE_MOUSEBUTTON, 1), 	MOUSECODE_2_BUTTON2 },
	{ JOYCODE(1, CODETYPE_MOUSEBUTTON, 2), 	MOUSECODE_2_BUTTON3 },
	{ JOYCODE(1, CODETYPE_MOUSEBUTTON, 3), 	MOUSECODE_2_BUTTON4 },
	{ JOYCODE(1, CODETYPE_MOUSEBUTTON, 4), 	MOUSECODE_2_BUTTON5 },
	{ JOYCODE(1, CODETYPE_MOUSEAXIS, 0),	MOUSECODE_2_ANALOG_X },
	{ JOYCODE(1, CODETYPE_MOUSEAXIS, 1),	MOUSECODE_2_ANALOG_Y },
	{ JOYCODE(1, CODETYPE_MOUSEAXIS, 2),	MOUSECODE_2_ANALOG_Z },
	{ JOYCODE(1, CODETYPE_MOUSE_NEG, 0),	MOUSECODE_2_X_NEG },
	{ JOYCODE(1, CODETYPE_MOUSE_POS, 0),	MOUSECODE_2_X_POS },
	{ JOYCODE(1, CODETYPE_MOUSE_NEG, 1),	MOUSECODE_2_Y_NEG },
	{ JOYCODE(1, CODETYPE_MOUSE_POS, 1),	MOUSECODE_2_Y_POS },
	{ JOYCODE(1, CODETYPE_MOUSE_NEG, 2),	MOUSECODE_2_Z_NEG },
	{ JOYCODE(1, CODETYPE_MOUSE_POS, 2),	MOUSECODE_2_Z_POS },

	{ JOYCODE(2, CODETYPE_MOUSEBUTTON, 0), 	MOUSECODE_3_BUTTON1 },
	{ JOYCODE(2, CODETYPE_MOUSEBUTTON, 1), 	MOUSECODE_3_BUTTON2 },
	{ JOYCODE(2, CODETYPE_MOUSEBUTTON, 2), 	MOUSECODE_3_BUTTON3 },
	{ JOYCODE(2, CODETYPE_MOUSEBUTTON, 3), 	MOUSECODE_3_BUTTON4 },
	{ JOYCODE(2, CODETYPE_MOUSEBUTTON, 4), 	MOUSECODE_3_BUTTON5 },
	{ JOYCODE(2, CODETYPE_MOUSEAXIS, 0),	MOUSECODE_3_ANALOG_X },
	{ JOYCODE(2, CODETYPE_MOUSEAXIS, 1),	MOUSECODE_3_ANALOG_Y },
	{ JOYCODE(2, CODETYPE_MOUSEAXIS, 2),	MOUSECODE_3_ANALOG_Z },
	{ JOYCODE(2, CODETYPE_MOUSE_NEG, 0),	MOUSECODE_3_X_NEG },
	{ JOYCODE(2, CODETYPE_MOUSE_POS, 0),	MOUSECODE_3_X_POS },
	{ JOYCODE(2, CODETYPE_MOUSE_NEG, 1),	MOUSECODE_3_Y_NEG },
	{ JOYCODE(2, CODETYPE_MOUSE_POS, 1),	MOUSECODE_3_Y_POS },
	{ JOYCODE(2, CODETYPE_MOUSE_NEG, 2),	MOUSECODE_3_Z_NEG },
	{ JOYCODE(2, CODETYPE_MOUSE_POS, 2),	MOUSECODE_3_Z_POS },

	{ JOYCODE(3, CODETYPE_MOUSEBUTTON, 0), 	MOUSECODE_4_BUTTON1 },
	{ JOYCODE(3, CODETYPE_MOUSEBUTTON, 1), 	MOUSECODE_4_BUTTON2 },
	{ JOYCODE(3, CODETYPE_MOUSEBUTTON, 2), 	MOUSECODE_4_BUTTON3 },
	{ JOYCODE(3, CODETYPE_MOUSEBUTTON, 3), 	MOUSECODE_4_BUTTON4 },
	{ JOYCODE(3, CODETYPE_MOUSEBUTTON, 4), 	MOUSECODE_4_BUTTON5 },
	{ JOYCODE(3, CODETYPE_MOUSEAXIS, 0),	MOUSECODE_4_ANALOG_X },
	{ JOYCODE(3, CODETYPE_MOUSEAXIS, 1),	MOUSECODE_4_ANALOG_Y },
	{ JOYCODE(3, CODETYPE_MOUSEAXIS, 2),	MOUSECODE_4_ANALOG_Z },
	{ JOYCODE(3, CODETYPE_MOUSE_NEG, 0),	MOUSECODE_4_X_NEG },
	{ JOYCODE(3, CODETYPE_MOUSE_POS, 0),	MOUSECODE_4_X_POS },
	{ JOYCODE(3, CODETYPE_MOUSE_NEG, 1),	MOUSECODE_4_Y_NEG },
	{ JOYCODE(3, CODETYPE_MOUSE_POS, 1),	MOUSECODE_4_Y_POS },
	{ JOYCODE(3, CODETYPE_MOUSE_NEG, 2),	MOUSECODE_4_Z_NEG },
	{ JOYCODE(3, CODETYPE_MOUSE_POS, 2),	MOUSECODE_4_Z_POS },

	{ JOYCODE(4, CODETYPE_MOUSEBUTTON, 0), 	MOUSECODE_5_BUTTON1 },
	{ JOYCODE(4, CODETYPE_MOUSEBUTTON, 1), 	MOUSECODE_5_BUTTON2 },
	{ JOYCODE(4, CODETYPE_MOUSEBUTTON, 2), 	MOUSECODE_5_BUTTON3 },
	{ JOYCODE(4, CODETYPE_MOUSEBUTTON, 3), 	MOUSECODE_5_BUTTON4 },
	{ JOYCODE(4, CODETYPE_MOUSEBUTTON, 4), 	MOUSECODE_5_BUTTON5 },
	{ JOYCODE(4, CODETYPE_MOUSEAXIS, 0),	MOUSECODE_5_ANALOG_X },
	{ JOYCODE(4, CODETYPE_MOUSEAXIS, 1),	MOUSECODE_5_ANALOG_Y },
	{ JOYCODE(4, CODETYPE_MOUSEAXIS, 2),	MOUSECODE_5_ANALOG_Z },
	{ JOYCODE(4, CODETYPE_MOUSE_NEG, 0),	MOUSECODE_5_X_NEG },
	{ JOYCODE(4, CODETYPE_MOUSE_POS, 0),	MOUSECODE_5_X_POS },
	{ JOYCODE(4, CODETYPE_MOUSE_NEG, 1),	MOUSECODE_5_Y_NEG },
	{ JOYCODE(4, CODETYPE_MOUSE_POS, 1),	MOUSECODE_5_Y_POS },
	{ JOYCODE(4, CODETYPE_MOUSE_NEG, 2),	MOUSECODE_5_Z_NEG },
	{ JOYCODE(4, CODETYPE_MOUSE_POS, 2),	MOUSECODE_5_Z_POS },

	{ JOYCODE(5, CODETYPE_MOUSEBUTTON, 0), 	MOUSECODE_6_BUTTON1 },
	{ JOYCODE(5, CODETYPE_MOUSEBUTTON, 1), 	MOUSECODE_6_BUTTON2 },
	{ JOYCODE(5, CODETYPE_MOUSEBUTTON, 2), 	MOUSECODE_6_BUTTON3 },
	{ JOYCODE(5, CODETYPE_MOUSEBUTTON, 3), 	MOUSECODE_6_BUTTON4 },
	{ JOYCODE(5, CODETYPE_MOUSEBUTTON, 4), 	MOUSECODE_6_BUTTON5 },
	{ JOYCODE(5, CODETYPE_MOUSEAXIS, 0),	MOUSECODE_6_ANALOG_X },
	{ JOYCODE(5, CODETYPE_MOUSEAXIS, 1),	MOUSECODE_6_ANALOG_Y },
	{ JOYCODE(5, CODETYPE_MOUSEAXIS, 2),	MOUSECODE_6_ANALOG_Z },
	{ JOYCODE(5, CODETYPE_MOUSE_NEG, 0),	MOUSECODE_6_X_NEG },
	{ JOYCODE(5, CODETYPE_MOUSE_POS, 0),	MOUSECODE_6_X_POS },
	{ JOYCODE(5, CODETYPE_MOUSE_NEG, 1),	MOUSECODE_6_Y_NEG },
	{ JOYCODE(5, CODETYPE_MOUSE_POS, 1),	MOUSECODE_6_Y_POS },
	{ JOYCODE(5, CODETYPE_MOUSE_NEG, 2),	MOUSECODE_6_Z_NEG },
	{ JOYCODE(5, CODETYPE_MOUSE_POS, 2),	MOUSECODE_6_Z_POS },

	{ JOYCODE(6, CODETYPE_MOUSEBUTTON, 0), 	MOUSECODE_7_BUTTON1 },
	{ JOYCODE(6, CODETYPE_MOUSEBUTTON, 1), 	MOUSECODE_7_BUTTON2 },
	{ JOYCODE(6, CODETYPE_MOUSEBUTTON, 2), 	MOUSECODE_7_BUTTON3 },
	{ JOYCODE(6, CODETYPE_MOUSEBUTTON, 3), 	MOUSECODE_7_BUTTON4 },
	{ JOYCODE(6, CODETYPE_MOUSEBUTTON, 4), 	MOUSECODE_7_BUTTON5 },
	{ JOYCODE(6, CODETYPE_MOUSEAXIS, 0),	MOUSECODE_7_ANALOG_X },
	{ JOYCODE(6, CODETYPE_MOUSEAXIS, 1),	MOUSECODE_7_ANALOG_Y },
	{ JOYCODE(6, CODETYPE_MOUSEAXIS, 2),	MOUSECODE_7_ANALOG_Z },
	{ JOYCODE(6, CODETYPE_MOUSE_NEG, 0),	MOUSECODE_7_X_NEG },
	{ JOYCODE(6, CODETYPE_MOUSE_POS, 0),	MOUSECODE_7_X_POS },
	{ JOYCODE(6, CODETYPE_MOUSE_NEG, 1),	MOUSECODE_7_Y_NEG },
	{ JOYCODE(6, CODETYPE_MOUSE_POS, 1),	MOUSECODE_7_Y_POS },
	{ JOYCODE(6, CODETYPE_MOUSE_NEG, 2),	MOUSECODE_7_Z_NEG },
	{ JOYCODE(6, CODETYPE_MOUSE_POS, 2),	MOUSECODE_7_Z_POS },

	{ JOYCODE(7, CODETYPE_MOUSEBUTTON, 0), 	MOUSECODE_8_BUTTON1 },
	{ JOYCODE(7, CODETYPE_MOUSEBUTTON, 1), 	MOUSECODE_8_BUTTON2 },
	{ JOYCODE(7, CODETYPE_MOUSEBUTTON, 2), 	MOUSECODE_8_BUTTON3 },
	{ JOYCODE(7, CODETYPE_MOUSEBUTTON, 3), 	MOUSECODE_8_BUTTON4 },
	{ JOYCODE(7, CODETYPE_MOUSEBUTTON, 4), 	MOUSECODE_8_BUTTON5 },
	{ JOYCODE(7, CODETYPE_MOUSEAXIS, 0),	MOUSECODE_8_ANALOG_X },
	{ JOYCODE(7, CODETYPE_MOUSEAXIS, 1),	MOUSECODE_8_ANALOG_Y },
	{ JOYCODE(7, CODETYPE_MOUSEAXIS, 2),	MOUSECODE_8_ANALOG_Z },
	{ JOYCODE(7, CODETYPE_MOUSE_NEG, 0),	MOUSECODE_8_X_NEG },
	{ JOYCODE(7, CODETYPE_MOUSE_POS, 0),	MOUSECODE_8_X_POS },
	{ JOYCODE(7, CODETYPE_MOUSE_NEG, 1),	MOUSECODE_8_Y_NEG },
	{ JOYCODE(7, CODETYPE_MOUSE_POS, 1),	MOUSECODE_8_Y_POS },
	{ JOYCODE(7, CODETYPE_MOUSE_NEG, 2),	MOUSECODE_8_Z_NEG },
	{ JOYCODE(7, CODETYPE_MOUSE_POS, 2),	MOUSECODE_8_Z_POS },

	{ JOYCODE(0, CODETYPE_GUNAXIS, 0),		GUNCODE_1_ANALOG_X },
	{ JOYCODE(0, CODETYPE_GUNAXIS, 1),		GUNCODE_1_ANALOG_Y },

	{ JOYCODE(1, CODETYPE_GUNAXIS, 0),		GUNCODE_2_ANALOG_X },
	{ JOYCODE(1, CODETYPE_GUNAXIS, 1),		GUNCODE_2_ANALOG_Y },

	{ JOYCODE(2, CODETYPE_GUNAXIS, 0),		GUNCODE_3_ANALOG_X },
	{ JOYCODE(2, CODETYPE_GUNAXIS, 1),		GUNCODE_3_ANALOG_Y },

	{ JOYCODE(3, CODETYPE_GUNAXIS, 0),		GUNCODE_4_ANALOG_X },
	{ JOYCODE(3, CODETYPE_GUNAXIS, 1),		GUNCODE_4_ANALOG_Y },

	{ JOYCODE(4, CODETYPE_GUNAXIS, 0),		GUNCODE_5_ANALOG_X },
	{ JOYCODE(4, CODETYPE_GUNAXIS, 1),		GUNCODE_5_ANALOG_Y },

	{ JOYCODE(5, CODETYPE_GUNAXIS, 0),		GUNCODE_6_ANALOG_X },
	{ JOYCODE(5, CODETYPE_GUNAXIS, 1),		GUNCODE_6_ANALOG_Y },

	{ JOYCODE(6, CODETYPE_GUNAXIS, 0),		GUNCODE_7_ANALOG_X },
	{ JOYCODE(6, CODETYPE_GUNAXIS, 1),		GUNCODE_7_ANALOG_Y },

	{ JOYCODE(7, CODETYPE_GUNAXIS, 0),		GUNCODE_8_ANALOG_X },
	{ JOYCODE(7, CODETYPE_GUNAXIS, 1),		GUNCODE_8_ANALOG_Y },
};

static int win_has_menu(void)
{
	return 0;
}

//============================================================
//	autoselect_analog_devices
//============================================================

static void autoselect_analog_devices(const input_port_entry *inp, int type1, int type2, int type3, int anatype, const char *ananame)
{
	// loop over input ports
	for ( ; inp->type != IPT_END; inp++)
	
		// if this port type is in use, apply the autoselect criteria
		if ((type1 != 0 && inp->type == type1) || 
			(type2 != 0 && inp->type == type2) ||
			(type3 != 0 && inp->type == type3))
		{
			// autoenable mouse devices
			if (analog_type[anatype] == SELECT_TYPE_MOUSE && !win_use_mouse)
			{
				win_use_mouse = 1;
				if (verbose)
					printf("Autoenabling mice due to presence of a %s\n", ananame);
			}
				
			// autoenable joystick devices
			if (analog_type[anatype] == SELECT_TYPE_JOYSTICK && !use_joystick)
			{
				use_joystick = 1;
				if (verbose)
					printf("Autoenabling joysticks due to presence of a %s\n", ananame);
			}
				
			// all done
			break;
		}	
}

//============================================================
//  sdl_pause_input
//============================================================

void sdl_pause_input(running_machine *machine, int paused)
{
	input_paused = paused;
}

//============================================================
//	win_init_input
//============================================================

int win_init_input(running_machine *machine)
{
	const input_port_entry *inp;
	int stick, axis, button, mouse;
	char tempname[512];
	SDL_Joystick *joy;

	input_paused = 0;
	add_pause_callback(machine, sdl_pause_input);

	// decode the options
	extract_input_config();

	init_keycodes();
	memset(keyboard_state, 0, sizeof(keyboard_state));

	// enable devices based on autoselect
	if (Machine != NULL && Machine->gamedrv != NULL)
	{
		begin_resource_tracking();
		inp = input_port_allocate(Machine->gamedrv->ipt, NULL);
		autoselect_analog_devices(inp, IPT_PADDLE,     IPT_PADDLE_V,   0,             ANALOG_TYPE_PADDLE,   "paddle");
		autoselect_analog_devices(inp, IPT_AD_STICK_X, IPT_AD_STICK_Y, IPT_AD_STICK_Z,ANALOG_TYPE_ADSTICK,  "analog joystick");
		autoselect_analog_devices(inp, IPT_LIGHTGUN_X, IPT_LIGHTGUN_Y, 0,             ANALOG_TYPE_LIGHTGUN, "lightgun");
		autoselect_analog_devices(inp, IPT_PEDAL,      IPT_PEDAL2,     IPT_PEDAL3,    ANALOG_TYPE_PEDAL,    "pedal");
		autoselect_analog_devices(inp, IPT_DIAL,       IPT_DIAL_V,     0,             ANALOG_TYPE_DIAL,     "dial");
		autoselect_analog_devices(inp, IPT_TRACKBALL_X,IPT_TRACKBALL_X,0,             ANALOG_TYPE_TRACKBALL,"trackball");
#ifdef MESS
		autoselect_analog_devices(inp, IPT_MOUSE_X,    IPT_MOUSE_Y,    0,             ANALOG_TYPE_MOUSE,    "mouse");
#endif // MESS
		end_resource_tracking();
	}

	if (win_use_mouse)
	{
		mouse = 0;
		sprintf(tempname, "Mouse %d X", mouse + 1);
		add_joylist_entry(tempname, JOYCODE(mouse, CODETYPE_MOUSEAXIS, 0), CODE_OTHER_ANALOG_RELATIVE);
		// add negative & positive digital values
		sprintf(tempname, "Mouse %d X -", mouse + 1);
		add_joylist_entry(tempname, JOYCODE(mouse, CODETYPE_MOUSE_NEG, 0), CODE_OTHER_DIGITAL);
		sprintf(tempname, "Mouse %d X +", mouse + 1);
		add_joylist_entry(tempname, JOYCODE(mouse, CODETYPE_MOUSE_POS, 0), CODE_OTHER_DIGITAL);

		sprintf(tempname, "Mouse %d Y", mouse + 1);
		add_joylist_entry(tempname, JOYCODE(mouse, CODETYPE_MOUSEAXIS, 1), CODE_OTHER_ANALOG_RELATIVE);
		// add negative & positive digital values
		sprintf(tempname, "Mouse %d Y -", mouse + 1);
		add_joylist_entry(tempname, JOYCODE(mouse, CODETYPE_MOUSE_NEG, 1), CODE_OTHER_DIGITAL);
		sprintf(tempname, "Mouse %d Y +", mouse + 1);
		add_joylist_entry(tempname, JOYCODE(mouse, CODETYPE_MOUSE_POS, 1), CODE_OTHER_DIGITAL);

		for (button = 0; button < 4; button++)
		{
			sprintf(tempname, "Mouse %d B%d", mouse + 1, button + 1);
			add_joylist_entry(tempname, JOYCODE(mouse, CODETYPE_MOUSEBUTTON, button), CODE_OTHER_DIGITAL);
		}
	}

	joystick_count = 0;
	if (use_joystick)
	{
		for (stick = 0; stick < SDL_NumJoysticks(); stick++)
		{
			joystick_count++;

			// loop over all axes
			joy = SDL_JoystickOpen(stick);
			joystick_device[stick] = joy;

//			printf("stick %d (%s) has %d axes, %d buttons, and %d hats\n", stick+1, SDL_JoystickName(stick),
//				SDL_JoystickNumAxes(joy), SDL_JoystickNumButtons(joy), SDL_JoystickNumHats(joy));

			for (axis = 0; axis < SDL_JoystickNumAxes(joy); axis++)
			{
				// add analog axes
				if (!joystick_digital[stick][axis])
				{
					sprintf(tempname, "J%d %s", stick + 1, SDL_JoystickName(stick));
					add_joylist_entry(tempname, JOYCODE(stick, CODETYPE_JOYAXIS, axis), CODE_OTHER_ANALOG_ABSOLUTE);
					// add negative & positive analog axis
					sprintf(tempname, "J%d + %s", stick + 1, SDL_JoystickName(stick));
					add_joylist_entry(tempname, JOYCODE(stick, CODETYPE_JOYAXIS_POS, axis), CODE_OTHER_ANALOG_ABSOLUTE);
					sprintf(tempname, "J%d - %s", stick + 1, SDL_JoystickName(stick));
					add_joylist_entry(tempname, JOYCODE(stick, CODETYPE_JOYAXIS_NEG, axis), CODE_OTHER_ANALOG_ABSOLUTE);
				}
		
				// add negative & positive digital values
				sprintf(tempname, "J%d axis %d -", stick + 1, axis);
				add_joylist_entry(tempname, JOYCODE(stick, CODETYPE_AXIS_NEG, axis), CODE_OTHER_DIGITAL);
				sprintf(tempname, "J%d axis %d +", stick + 1, axis);
				add_joylist_entry(tempname, JOYCODE(stick, CODETYPE_AXIS_POS, axis), CODE_OTHER_DIGITAL);
			}

			// loop over all buttons
			for (button = 0; button < SDL_JoystickNumButtons(joy); button++)
			{
				sprintf(tempname, "J%d button %d", stick + 1, button);
				add_joylist_entry(tempname, JOYCODE(stick, CODETYPE_BUTTON, button), CODE_OTHER_DIGITAL);
			}
		}
	}

	return 0;
}


//============================================================
//	win_process_events
//============================================================

void sdlwindow_resize(INT32 width, INT32 height);

void win_process_events(void)
{
	SDL_Event event;
	int i;
	for (i=0;i<MAX_JOYSTICKS;i++)
	{
	        mouse_state[i].lX = 0;
	        mouse_state[i].lY = 0;
	}
	while(SDL_PollEvent(&event)) {
		switch(event.type) {
		case SDL_QUIT:
			mame_schedule_exit(Machine);
			break;
		case SDL_KEYDOWN:
			keyboard_state[event.key.keysym.sym] = 1;
			updatekeyboard();
			break;
		case SDL_KEYUP:
			keyboard_state[event.key.keysym.sym] = 0;
			updatekeyboard();
			break;
		case SDL_JOYAXISMOTION:
 			joystick_state[event.jaxis.which].axes[event.jaxis.axis] = (event.jaxis.value * 2);
			break;
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
			joystick_state[event.jbutton.which].buttons[event.jbutton.button] = event.jbutton.state == SDL_PRESSED;
			break;
		case SDL_MOUSEBUTTONDOWN:
			mouse_state[0].buttons[event.button.button-1] = 1;
			break;
		case SDL_MOUSEBUTTONUP:
			mouse_state[0].buttons[event.button.button-1] = 0;
			break;
		case SDL_MOUSEMOTION:
			mouse_state[0].lX = event.motion.xrel;
			mouse_state[0].lY = event.motion.yrel;
			break;
		case SDL_VIDEORESIZE:
			sdlwindow_resize(event.resize.w, event.resize.h);
			break;
		}
	}
}

//============================================================
//  extract_input_options
//============================================================

static void parse_analog_select(int type, const char *option)
{
	const char *stemp = options_get_string(option);

	if (strcmp(stemp, "keyboard") == 0)
		analog_type[type] = SELECT_TYPE_KEYBOARD;
	else if (strcmp(stemp, "mouse") == 0)
		analog_type[type] = SELECT_TYPE_MOUSE;
	else if (strcmp(stemp, "joystick") == 0)
		analog_type[type] = SELECT_TYPE_JOYSTICK;
	else
	{
		fprintf(stderr, "Invalid %s value %s; reverting to keyboard\n", option, stemp);
		analog_type[type] = SELECT_TYPE_KEYBOARD;
	}
}


static void parse_digital(const char *option)
{
	const char *soriginal = options_get_string(option);
	const char *stemp = soriginal;

	if (strcmp(stemp, "none") == 0)
		memset(joystick_digital, 0, sizeof(joystick_digital));
	else if (strcmp(stemp, "all") == 0)
		memset(joystick_digital, 1, sizeof(joystick_digital));
	else
	{
		/* scan the string */
		while (1)
		{
			int joynum = 0;
			int axisnum = 0;

			/* stop if we hit the end */
			if (stemp[0] == 0)
				break;

			/* we require the next bits to be j<N> */
			if (tolower(stemp[0]) != 'j' || sscanf(&stemp[1], "%d", &joynum) != 1)
				goto usage;
			stemp++;
			while (stemp[0] != 0 && isdigit(stemp[0]))
				stemp++;

			/* if we are followed by a comma or an end, mark all the axes digital */
			if (stemp[0] == 0 || stemp[0] == ',')
			{
				if (joynum != 0 && joynum - 1 < MAX_JOYSTICKS)
					memset(&joystick_digital[joynum - 1], 1, sizeof(joystick_digital[joynum - 1]));
				if (stemp[0] == 0)
					break;
				stemp++;
				continue;
			}

			/* loop over axes */
			while (1)
			{
				/* stop if we hit the end */
				if (stemp[0] == 0)
					break;

				/* if we hit a comma, skip it and break out */
				if (stemp[0] == ',')
				{
					stemp++;
					break;
				}

				/* we require the next bits to be a<N> */
				if (tolower(stemp[0]) != 'a' || sscanf(&stemp[1], "%d", &axisnum) != 1)
					goto usage;
				stemp++;
				while (stemp[0] != 0 && isdigit(stemp[0]))
					stemp++;

				/* set that axis to digital */
				if (joynum != 0 && joynum - 1 < MAX_JOYSTICKS && axisnum < MAX_AXES)
					joystick_digital[joynum - 1][axisnum] = 1;
			}
		}
	}
	return;

usage:
	fprintf(stderr, "Invalid %s value %s; reverting to all -- valid values are:\n", option, soriginal);
	fprintf(stderr, "         none -- no axes on any joysticks are digital\n");
	fprintf(stderr, "         all -- all axes on all joysticks are digital\n");
	fprintf(stderr, "         j<N> -- all axes on joystick <N> are digital\n");
	fprintf(stderr, "         j<N>a<M> -- axis <M> on joystick <N> is digital\n");
	fprintf(stderr, "    Multiple axes can be specified for one joystick:\n");
	fprintf(stderr, "         j1a5a6 -- axes 5 and 6 on joystick 1 are digital\n");
	fprintf(stderr, "    Multiple joysticks can be specified separated by commas:\n");
	fprintf(stderr, "         j1,j2a2 -- all joystick 1 axes and axis 2 on joystick 2 are digital\n");
}

static void extract_input_config(void)
{
	// extract boolean options
	win_use_mouse = options_get_bool("mouse");
	use_joystick = options_get_bool("joystick");
	steadykey = options_get_bool("steadykey");
	a2d_deadzone = options_get_float("a2d_deadzone");
	options.controller = options_get_string("ctrlr");
	parse_analog_select(ANALOG_TYPE_PADDLE, "paddle_device");
	parse_analog_select(ANALOG_TYPE_ADSTICK, "adstick_device");
	parse_analog_select(ANALOG_TYPE_PEDAL, "pedal_device");
	parse_analog_select(ANALOG_TYPE_DIAL, "dial_device");
	parse_analog_select(ANALOG_TYPE_TRACKBALL, "trackball_device");
	parse_analog_select(ANALOG_TYPE_LIGHTGUN, "lightgun_device");
#ifdef MESS
	parse_analog_select(ANALOG_TYPE_MOUSE, "mouse_device");
#endif
	parse_digital("digital");
}
 
void win_clear_keyboard(void)
{
	memset(keyboard_state, 0, sizeof(keyboard_state));
}

//============================================================
//	is_mouse_captured
//============================================================

int sdl_is_mouse_captured(void)
{
	return (!input_paused && mouse_active && win_use_mouse); // && !win_has_menu());
}



//============================================================
//	updatekeyboard
//============================================================

// since the keyboard controller is slow, it is not capable of reporting multiple
// key presses fast enough. We have to delay them in order not to lose special moves
// tied to simultaneous button presses.

static void updatekeyboard(void)
{
	int i, changed = 0;

	// see if any keys have changed state
	for (i = 0; i < MAX_KEYS; i++)
		if (keyboard_state[i] != oldkey[i])
		{
			changed = 1;

			// keypress was missed, turn it on for one frame
			if (keyboard_state[i] == 0 && currkey[i] == 0)
				currkey[i] = -1;
		}

	// if keyboard state is stable, copy it over
	if (!changed)
		memcpy(currkey, &keyboard_state[0], sizeof(currkey));

	// remember the previous state
	memcpy(oldkey, &keyboard_state[0], sizeof(oldkey));
}



//============================================================
//	is_key_pressed
//============================================================

static int is_key_pressed(os_code keycode)
{
	int sdlk = SDLCODE(keycode);

	// special case: if we're trying to quit, fake up/down/up/down
	if (sdlk == SDLK_ESCAPE && win_trying_to_quit)
	{
		static int dummy_state = 1;
		return dummy_state ^= 1;
	}

	// otherwise, just return the current keystate
	if (steadykey)
		return currkey[sdlk];
	else
		return keyboard_state[sdlk];
}



//============================================================
//	init_keycodes
//============================================================

static void init_keycodes(void)
{
	int key = 0;
	int (*key_trans_table)[4] = NULL;

	if (options_get_bool("keymap"))
	{
		char *keymap_filename;
		FILE *keymap_file;
		int line = 0, rc;

		keymap_filename = (char *)options_get_string("keymap_file");
		keymap_file = fopen(keymap_filename, "r");

		if (keymap_file == NULL)
		{
			printf("Unable to open keymap %s, using default\n", keymap_filename);
			key_trans_table = (int (*)[4])def_key_trans_table;
		}
		else
		{
			do
			{
				key_trans_table = (int (*)[4])realloc( key_trans_table, sizeof(int) * 4 * ( line + 1));

				rc = fscanf(keymap_file, "%x %x %x %x\n",
				&key_trans_table[line][MAME_KEY],
				&key_trans_table[line][SDL_KEY],
				&key_trans_table[line][VIRTUAL_KEY],
				&key_trans_table[line][ASCII_KEY]
				);

				line++;
			}
			while(rc != EOF);

			fclose(keymap_file);
		}
	}
	else
	{
		key_trans_table = (int (*)[4])def_key_trans_table;
	}

	// iterate over all possible keys
	while (key_trans_table[key][0] >= 0) 
	{
		// copy the name
		char *namecopy = auto_malloc(strlen(key_name_table[key])+1);

		if (namecopy)
		{
			input_code standardcode;
			os_code code;

			// compute the code, which encodes DirectInput, virtual, and ASCII codes
			code = KEYCODE(key_trans_table[key][SDL_KEY], key_trans_table[key][VIRTUAL_KEY], key_trans_table[key][ASCII_KEY]);
			standardcode = key_trans_table[key][MAME_KEY];

			// fill in the key description
			codelist[total_codes].name = strcpy(namecopy, key_name_table[key]);
			codelist[total_codes].oscode = code;
			codelist[total_codes].inputcode = standardcode;
			total_codes++;
		}

		key++;
	} 
}



//============================================================
//	add_joylist_entry
//============================================================

static void add_joylist_entry(const char *name, os_code code, input_code standardcode)
{
	// copy the name
	char *namecopy = auto_malloc(strlen(name) + 1);
	if (namecopy)
	{
		int entry;

		// find the table entry, if there is one
		for (entry = 0; entry < ELEMENTS(joy_trans_table); entry++)
			if (joy_trans_table[entry][0] == code)
				break;

		// fill in the joy description
		codelist[total_codes].name = strcpy(namecopy, name);
		codelist[total_codes].oscode = code;
		if (entry < ELEMENTS(joy_trans_table))
			standardcode = joy_trans_table[entry][1];
		codelist[total_codes].inputcode = standardcode;
		total_codes++;
	}
}

//============================================================
//	get_joycode_value
//============================================================

static INT32 get_joycode_value(os_code joycode)
{
	int joyindex = JOYINDEX(joycode);
	int codetype = CODETYPE(joycode);
	int joynum = JOYNUM(joycode);
	INT32 retv = 0;

	// switch off the type
	switch (codetype)
	{
		case CODETYPE_MOUSEBUTTON:
			return mouse_state[joynum].buttons[joyindex];
		case CODETYPE_BUTTON:
			return joystick_state[joynum].buttons[joyindex];

		case CODETYPE_AXIS_POS:
		{
			int val = joystick_state[joynum].axes[joyindex];
			int top = 32767;
			int middle = 0;

			return (val > middle + ((top - middle) * a2d_deadzone));
		}

		case CODETYPE_AXIS_NEG:
		{
			int val = joystick_state[joynum].axes[joyindex];
			int bottom = -32768;
			int middle = 0;

			return (val < middle - ((middle - bottom) * a2d_deadzone));
		}

#if 0
		// anywhere from 0-45 (315) deg to 0+45 (45) deg
		case CODETYPE_POV_UP:
			pov = joystick_state[joynum].rgdwPOV[joyindex];
			return ((pov & 0xffff) != 0xffff && (pov >= 31500 || pov <= 4500));

		// anywhere from 90-45 (45) deg to 90+45 (135) deg
		case CODETYPE_POV_RIGHT:
			pov = joystick_state[joynum].rgdwPOV[joyindex];
			return ((pov & 0xffff) != 0xffff && (pov >= 4500 && pov <= 13500));

		// anywhere from 180-45 (135) deg to 180+45 (225) deg
		case CODETYPE_POV_DOWN:
			pov = joystick_state[joynum].rgdwPOV[joyindex];
			return ((pov & 0xffff) != 0xffff && (pov >= 13500 && pov <= 22500));

		// anywhere from 270-45 (225) deg to 270+45 (315) deg
		case CODETYPE_POV_LEFT:
			pov = joystick_state[joynum].rgdwPOV[joyindex];
			return ((pov & 0xffff) != 0xffff && (pov >= 22500 && pov <= 31500));
#endif

		// analog joystick axis
		case CODETYPE_JOYAXIS:
		{
			int val = ((int *)&joystick_state[joynum].axes)[joyindex];

			if (!use_joystick)
				return 0;
			if (val < ANALOG_VALUE_MIN) val = ANALOG_VALUE_MIN;
			if (val > ANALOG_VALUE_MAX) val = ANALOG_VALUE_MAX;
			return val;
		}

		// analog mouse axis
		case CODETYPE_MOUSEAXIS:
		// digital mouse direction
		case CODETYPE_MOUSE_NEG:
		case CODETYPE_MOUSE_POS:
			// if the mouse isn't yet active, make it so
			if (!mouse_active && win_use_mouse && !win_has_menu())
			{
				mouse_active = 1;
			}

			// return the latest mouse info
			if (joyindex == 0)
			{
				retv = mouse_state[joynum].lX * 512;
			}
			if (joyindex == 1)
			{
				retv = mouse_state[joynum].lY * 512;
			}

			if (codetype == CODETYPE_MOUSEAXIS)
			{
				return retv;
			}
			else
			{
				if (codetype == CODETYPE_MOUSE_POS)
					return retv > 0;
				else
					return retv < 0;
			}

			return 0;

#if 0		
		// analog gun axis
		case CODETYPE_GUNAXIS:
			// return the latest gun info
			if (joynum >= MAX_LIGHTGUNS)
				return 0;
			if (joyindex >= 2)
				return 0;
			return gun_axis[joynum][joyindex];
#endif
	}

	// keep the compiler happy
	return 0;
}



//============================================================
//	osd_is_code_pressed
//============================================================

INT32 osd_get_code_value(os_code code)
{
	if (IS_KEYBOARD_CODE(code))
		return is_key_pressed(code);
	else
		return get_joycode_value(code);
}



//============================================================
//	osd_get_code_list
//============================================================

const os_code_info *osd_get_code_list(void)
{
	return codelist;
}



//============================================================
//	osd_joystick_needs_calibration
//============================================================

int osd_joystick_needs_calibration(void)
{
	return 0;
}



//============================================================
//	osd_joystick_start_calibration
//============================================================

void osd_joystick_start_calibration(void)
{
}



//============================================================
//	osd_joystick_calibrate_next
//============================================================

const char *osd_joystick_calibrate_next(void)
{
	return 0;
}



//============================================================
//	osd_joystick_calibrate
//============================================================

void osd_joystick_calibrate(void)
{
}



//============================================================
//	osd_joystick_end_calibration
//============================================================

void osd_joystick_end_calibration(void)
{
}



//============================================================
//	osd_customize_inputport_list
//============================================================

void osd_customize_inputport_list(input_port_default_entry *defaults)
{
	static input_seq no_alt_tab_seq = SEQ_DEF_5(KEYCODE_TAB, CODE_NOT, KEYCODE_LALT, CODE_NOT, KEYCODE_RALT);
	input_port_default_entry *idef = defaults;

	// loop over all the defaults
	while (idef->type != IPT_END)
	{
		// map in some OSD-specific keys
		switch (idef->type)
		{
			// alt-enter for fullscreen
			case IPT_OSD_1:
				idef->token = "TOGGLE_FULLSCREEN";
				idef->name = "Toggle Fullscreen";
				seq_set_2(&idef->defaultseq, KEYCODE_ENTER, KEYCODE_LALT);
				break;
			// disable UI_SELECT when LALT is down, this stops selecting
			// things in the menu when toggling fullscreen with LALT+ENTER
			case IPT_UI_SELECT:
				seq_set_3(&idef->defaultseq, KEYCODE_ENTER, CODE_NOT, KEYCODE_LALT);
				break;

			// page down for fastforward
			case IPT_OSD_2:
				idef->token = "FAST_FORWARD";
				idef->name = "Fast Forward";
				seq_set_1(&idef->defaultseq, KEYCODE_PGDN);
				break;
			
			// OSD hotkeys use LCTRL and start at F3, they start at
			// F3 because F1-F2 are hardcoded into many drivers to
			// various dipswitches, and pressing them together with
			// LCTRL will still press/toggle these dipswitches.

			// LCTRL-F3 to toggle fullstretch
			case IPT_OSD_3:
				idef->token = "TOGGLE_FULLSTRETCH";
				idef->name = "Toggle Fullstretch";
				seq_set_2(&idef->defaultseq, KEYCODE_F3, KEYCODE_LCONTROL);
				break;
			// add a Not lcrtl condition to the reset key
			case IPT_UI_SOFT_RESET:
				seq_set_5(&idef->defaultseq, KEYCODE_F3, CODE_NOT, KEYCODE_LCONTROL, CODE_NOT, KEYCODE_LSHIFT);
				break;
			
			// LCTRL-F4 to toggle keep aspect
			case IPT_OSD_4:
				idef->token = "TOGGLE_KEEP_ASPECT";
				idef->name = "Toggle Keepaspect";
				seq_set_2(&idef->defaultseq, KEYCODE_F4, KEYCODE_LCONTROL);
				break;
			// add a Not lcrtl condition to the show gfx key
			case IPT_UI_SHOW_GFX:
				seq_set_3(&idef->defaultseq, KEYCODE_F4, CODE_NOT, KEYCODE_LCONTROL);
				break;
			
			// LCTRL-F5 to toggle OpenGL filtering
			case IPT_OSD_5:
				idef->token = "TOGGLE_FILTER";
				idef->name = "Toggle Filter";
				seq_set_2(&idef->defaultseq, KEYCODE_F5, KEYCODE_LCONTROL);
				break;
			// add a Not lcrtl condition to the toggle debug key
			case IPT_UI_TOGGLE_DEBUG:
				seq_set_3(&idef->defaultseq, KEYCODE_F5, CODE_NOT, KEYCODE_LCONTROL);
				break;
			
			// LCTRL-F6 to decrease OpenGL prescaling
			case IPT_OSD_6:
				idef->token = "DECREASE_PRESCALE";
				idef->name = "Decrease Prescaling";
				seq_set_2(&idef->defaultseq, KEYCODE_F6, KEYCODE_LCONTROL);
				break;
			// add a Not lcrtl condition to the toggle cheat key
			case IPT_UI_TOGGLE_CHEAT:
				seq_set_3(&idef->defaultseq, KEYCODE_F6, CODE_NOT, KEYCODE_LCONTROL);
				break;
			
			// LCTRL-F7 to increase OpenGL prescaling
			case IPT_OSD_7:
				idef->token = "INCREASE_PRESCALE";
				idef->name = "Increase Prescaling";
				seq_set_2(&idef->defaultseq, KEYCODE_F7, KEYCODE_LCONTROL);
				break;
			// add a Not lcrtl condition to the load state key
			case IPT_UI_LOAD_STATE:
				seq_set_5(&idef->defaultseq, KEYCODE_F7, CODE_NOT, KEYCODE_LCONTROL, CODE_NOT, KEYCODE_LSHIFT);
				break;
			
			// LCTRL-F8 to decrease prescaling effect #
			case IPT_OSD_8:
				if (getenv("SDLMAME_UNSUPPORTED")) {
					idef->token = "DECREASE_EFFECT";
					idef->name = "Decrease Effect";
					seq_set_2(&idef->defaultseq, KEYCODE_F8, KEYCODE_LCONTROL);
				}
				break;
			// add a Not lcrtl condition to frameskip decrease
			case IPT_UI_FRAMESKIP_DEC:
				if (getenv("SDLMAME_UNSUPPORTED"))
					seq_set_3(&idef->defaultseq, KEYCODE_F8, CODE_NOT, KEYCODE_LCONTROL);
				break;
			
			// LCTRL-F9 to increase prescaling effect #
			case IPT_OSD_9:
				if (getenv("SDLMAME_UNSUPPORTED")) {
					idef->token = "INCREASE_EFFECT";
					idef->name = "Increase Effect";
					seq_set_2(&idef->defaultseq, KEYCODE_F9, KEYCODE_LCONTROL);
				}
				break;
			// add a Not lcrtl condition to frameskip increase
			case IPT_UI_FRAMESKIP_INC:
				if (getenv("SDLMAME_UNSUPPORTED"))
					seq_set_3(&idef->defaultseq, KEYCODE_F9, CODE_NOT, KEYCODE_LCONTROL);
				break;
			
			// LCTRL-F10 to toggle the renderer (software vs opengl)
			case IPT_OSD_10:
				idef->token = "TOGGLE_RENDERER";
				idef->name = "Toggle OpenGL/software rendering";
				seq_set_2(&idef->defaultseq, KEYCODE_F10, KEYCODE_LCONTROL);
				break;
			// add a Not lcrtl condition to the throttle key
			case IPT_UI_THROTTLE:
				seq_set_3(&idef->defaultseq, KEYCODE_F10, CODE_NOT, KEYCODE_LCONTROL);
				break;
			
			// disable the config menu if the ALT key is down
			// (allows ALT-TAB to switch between windows apps)
			case IPT_UI_CONFIGURE:
				seq_copy(&idef->defaultseq, &no_alt_tab_seq);
				break;
		}

		// find the next one
		idef++;
	}
}
