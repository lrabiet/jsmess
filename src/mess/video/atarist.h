#ifndef _VIDEO_ATARIST_H_
#define _VIDEO_ATARIST_H_

#define ATARIST_HBSTART_PAL		512
#define ATARIST_HBEND_PAL		0
#define ATARIST_HBSTART_NTSC	508
#define ATARIST_HBEND_NTSC		0
#define ATARIST_HTOT_PAL		516
#define ATARIST_HTOT_NTSC		512

#define ATARIST_HBDEND_PAL		14
#define ATARIST_HBDSTART_PAL	94
#define ATARIST_HBDEND_NTSC		13
#define ATARIST_HBDSTART_NTSC	93

#define ATARIST_VBEND_PAL		0
#define ATARIST_VBEND_NTSC		0
#define ATARIST_VBSTART_PAL		312
#define ATARIST_VBSTART_NTSC	262
#define ATARIST_VTOT_PAL		313
#define ATARIST_VTOT_NTSC		263

#define ATARIST_VBDEND_PAL		63
#define ATARIST_VBDSTART_PAL	263
#define ATARIST_VBDEND_NTSC		34
#define ATARIST_VBDSTART_NTSC	234

READ16_HANDLER( atarist_shifter_r );
WRITE16_HANDLER( atarist_shifter_w );
READ16_HANDLER( atariste_shifter_r );
WRITE16_HANDLER( atariste_shifter_w );

VIDEO_START( atarist );
VIDEO_UPDATE( atarist );

#endif
