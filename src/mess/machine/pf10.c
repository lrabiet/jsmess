/***************************************************************************

    Epson PF-10

    Serial floppy drive

    Skeleton driver, not working

***************************************************************************/

#include "driver.h"
#include "pf10.h"
#include "cpu/m6800/m6800.h"
#include "machine/nec765.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _pf10_state pf10_state;
struct _pf10_state
{
	UINT8 dummy;
};


/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE pf10_state *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == PF10);

	return (pf10_state *)device->token;
}


/*****************************************************************************
    ADDRESS MAPS
*****************************************************************************/

static ADDRESS_MAP_START( pf10_mem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0xe000, 0xffff) AM_ROM AM_REGION("pf10", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( pf10_io, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END


/*****************************************************************************
    MACHINE CONFIG
*****************************************************************************/

static const nec765_interface pf10_nec765a_intf =
{
	DEVCB_NULL, /* interrupt line */
	NULL,
	NULL,
	NEC765_RDY_PIN_NOT_CONNECTED /* ??? */
};

static MACHINE_DRIVER_START( pf10 )
	MDRV_CPU_ADD("pf10", M6803, XTAL_4MHz /* ??? */) /* HD63A03 */
	MDRV_CPU_PROGRAM_MAP(pf10_mem)
	MDRV_CPU_IO_MAP(pf10_io)

	MDRV_NEC765A_ADD("nec765a", pf10_nec765a_intf)
MACHINE_DRIVER_END


/***************************************************************************
    ROM DEFINITIONS
***************************************************************************/

ROM_START( pf10 )
	ROM_REGION(0x2000, "pf10", ROMREGION_LOADBYNAME)
	ROM_LOAD("k3pf1.bin", 0x0000, 0x2000, CRC(eef4593a) SHA1(bb176e4baf938fe58c2d32f7c46d7bb7b0627755))
ROM_END


/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( pf10 )
{
	pf10_state *pf10 = get_safe_token(device);

	pf10->dummy = 0;
}

static DEVICE_RESET( pf10 )
{
}

DEVICE_GET_INFO( pf10 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(pf10_state);					break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:	info->i = 0;									break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_OTHER;					break;

		/* --- the following bits of info are returned as pointers --- */
		case DEVINFO_PTR_MACHINE_CONFIG:		info->machine_config = MACHINE_DRIVER_NAME(pf10);	break;
		case DEVINFO_PTR_ROM_REGION:			info->romregion = ROM_NAME(pf10); 				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(pf10);			break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(pf10);			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "PF-10");						break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Floppy drive");				break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MESS Team");			break;
	}
}


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/* serial interface in (to the host computer) */
READ_LINE_DEVICE_HANDLER( pf10_txd1_r )
{
	logerror("%s: pf10_txd1_r\n", cpuexec_describe_context(device->machine));

	return 0;
}

WRITE_LINE_DEVICE_HANDLER( pf10_rxd1_w )
{
	logerror("%s: pf10_rxd1_w %u\n", cpuexec_describe_context(device->machine), state);
}


/* serial interface out (to another floppy drive) */
READ_LINE_DEVICE_HANDLER( pf10_txd2_r )
{
	logerror("%s: pf10_txd2_r\n", cpuexec_describe_context(device->machine));

	return 0;
}

WRITE_LINE_DEVICE_HANDLER( pf10_rxd2_w )
{
	logerror("%s: pf10_rxd2_w %u\n", cpuexec_describe_context(device->machine), state);
}
