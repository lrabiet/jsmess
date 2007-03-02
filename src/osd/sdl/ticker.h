//============================================================
//
//	ticker.h - Win32 timing code
//
//============================================================

#ifndef __SDL_TICKER__
#define __SDL_TICKER__

#include "osd_cpu.h"

//============================================================
//	TYPE DEFINITIONS
//============================================================

typedef INT64 TICKER;



//============================================================
//	MACROS
//============================================================

#define TICKS_PER_SEC ticks_per_sec



//============================================================
//	GLOBAL VARIABLES
//============================================================

extern TICKER		ticks_per_sec;
extern TICKER		(*ticker)(void);

#endif
