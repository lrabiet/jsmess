/*****************************************************************************
 *
 * includes/pp01.h
 *
 ****************************************************************************/

#ifndef PP01_H_
#define PP01_H_

#include "machine/pit8253.h"
#include "machine/8255ppi.h"

/*----------- defined in machine/pp01.c -----------*/
extern const struct pit8253_config pp01_pit8253_intf;
extern const ppi8255_interface pp01_ppi8255_interface;

extern DRIVER_INIT( pp01 );
extern MACHINE_START( pp01 );
extern MACHINE_RESET( pp01 );
extern WRITE8_HANDLER (pp01_mem_block_w);
/*----------- defined in video/pp01.c -----------*/

extern VIDEO_START( pp01 );
extern VIDEO_UPDATE( pp01 );
extern PALETTE_INIT( pp01 );

#endif
