/***************************************************************************

    Epson PF-10

    Serial floppy drive

***************************************************************************/

#ifndef __PF10_H__
#define __PF10_H__

#include "devcb.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

#if 0
typedef struct _pf10_interface pf10_interface;
struct _pf10_interface
{
};
#endif


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

DEVICE_GET_INFO( pf10 );

/* serial interface in (to the host computer) */
READ_LINE_DEVICE_HANDLER( pf10_txd1_r );
WRITE_LINE_DEVICE_HANDLER( pf10_rxd1_w );

/* serial interface out (to another floppy drive) */
READ_LINE_DEVICE_HANDLER( pf10_txd2_r );
WRITE_LINE_DEVICE_HANDLER( pf10_rxd2_w );


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define PF10 DEVICE_GET_INFO_NAME(pf10)

#define MDRV_PF10_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, PF10, 0) \


#endif /* __PF10_H__ */
