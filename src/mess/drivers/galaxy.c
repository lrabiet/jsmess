/***************************************************************************
Galaksija driver by Krzysztof Strzecha

18/04/2005 Possibilty to disable ROM 2. 2k, 22k, 38k and 54k memory
       configurations added.
13/03/2005 Memory mapping improved. Palette corrected. Supprort for newer
           version of snapshots added. Lot of cleanups. Keyboard mapping
           corrected.
19/09/2002 malloc() replaced by image_malloc().
15/09/2002 Snapshot loading fixed. Code cleanup.
31/01/2001 Snapshot loading corrected.
09/01/2001 Fast mode implemented (many thanks to Kevin Thacker).
07/01/2001 Keyboard corrected (still some keys unknown).
           Horizontal screen positioning in video subsystem added.
05/01/2001 Keyboard implemented (some keys unknown).
03/01/2001 Snapshot loading added.
01/01/2001 Preliminary driver.

To do:
-Video subsystem 'real' emulation
-Tape
-Galaksija Plus

***************************************************************************/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "includes/galaxy.h"
#include "devices/snapquik.h"

static ADDRESS_MAP_START (galaxy_readport, ADDRESS_SPACE_IO, 8)
ADDRESS_MAP_END

static ADDRESS_MAP_START (galaxy_writeport, ADDRESS_SPACE_IO, 8)
ADDRESS_MAP_END


static ADDRESS_MAP_START (galaxy_mem, ADDRESS_SPACE_PROGRAM, 8)
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x2000, 0x2037) AM_MIRROR(0x07c0) AM_READ( galaxy_keyboard_r )
	AM_RANGE(0x2038, 0x203f) AM_MIRROR(0x07c0) AM_READWRITE( galaxy_latch_r, galaxy_latch_w )
ADDRESS_MAP_END


static GFXDECODE_START( galaxy )
	GFXDECODE_ENTRY( REGION_GFX1, 0x0000, galaxy_charlayout, 0, 2 )
GFXDECODE_END

/* 2008-05 FP:
Small note about natural keyboard support. Currently:
- "List" is mapped to 'ESC'
- "Break" is mapped to 'F1'
- "Repeat" is mapped to 'F2'                           */

static INPUT_PORTS_START (galaxy)
	PORT_START /* line 0 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)		PORT_CHAR('A')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)		PORT_CHAR('B')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)		PORT_CHAR('C')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) 		PORT_CHAR('D')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) 		PORT_CHAR('E')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) 		PORT_CHAR('F')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) 		PORT_CHAR('G')

	PORT_START /* line 1 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) 		PORT_CHAR('H')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) 		PORT_CHAR('I')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) 		PORT_CHAR('J')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) 		PORT_CHAR('K')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) 		PORT_CHAR('L')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) 		PORT_CHAR('M')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) 		PORT_CHAR('N')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) 		PORT_CHAR('O')

	PORT_START /* line 2 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) 		PORT_CHAR('P')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) 		PORT_CHAR('Q')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)		PORT_CHAR('R')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) 		PORT_CHAR('S')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) 		PORT_CHAR('T')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) 		PORT_CHAR('U')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) 		PORT_CHAR('V')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) 		PORT_CHAR('W')

	PORT_START /* line 3 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) 		PORT_CHAR('X')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) 		PORT_CHAR('Y')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) 		PORT_CHAR('Z')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) 		PORT_CHAR(UCHAR_MAMEKEY(UP))
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) 	PORT_CHAR(UCHAR_MAMEKEY(DOWN))
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)	PORT_CHAR(UCHAR_MAMEKEY(LEFT))
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)	PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)	PORT_CHAR(' ')

	PORT_START /* line 4 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) 		PORT_CHAR('0') PORT_CHAR('_')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) 		PORT_CHAR('1') PORT_CHAR('!')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) 		PORT_CHAR('2') PORT_CHAR('"')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) 		PORT_CHAR('3') PORT_CHAR('#')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) 		PORT_CHAR('4') PORT_CHAR('$')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) 		PORT_CHAR('5') PORT_CHAR('%')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) 		PORT_CHAR('6') PORT_CHAR('&')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7  ||") PORT_CODE(KEYCODE_7) PORT_CHAR('7')

	PORT_START /* line 5 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) 		PORT_CHAR('8') PORT_CHAR('(')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) 		PORT_CHAR('9') PORT_CHAR(')')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)	PORT_CHAR(';') PORT_CHAR('+')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)	PORT_CHAR(':') PORT_CHAR('*')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)	PORT_CHAR(',') PORT_CHAR('<')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)	PORT_CHAR('=') PORT_CHAR('-')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)	PORT_CHAR('.') PORT_CHAR('>')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)	PORT_CHAR('/') PORT_CHAR('?')

	PORT_START /* line 6 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)	PORT_CHAR(13)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_PAUSE) PORT_CHAR(UCHAR_MAMEKEY(F1))
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Repeat") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(F2))
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Delete") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("List") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START /* port 7 */
		PORT_CONFNAME(0x01, 0x01, "ROM 2")
		PORT_CONFSETTING(	0x01, "Installed")
		PORT_CONFSETTING(	0x00, "Not installed")
INPUT_PORTS_END

static MACHINE_DRIVER_START( galaxy )
	/* basic machine hardware */
	MDRV_CPU_ADD(Z80, 3072000)
	MDRV_CPU_PROGRAM_MAP(galaxy_mem, 0)
	MDRV_CPU_IO_MAP(galaxy_readport, galaxy_writeport)
	MDRV_CPU_VBLANK_INT("main", galaxy_interrupt)
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(50)
	MDRV_SCREEN_VBLANK_TIME(0)
	MDRV_INTERLEAVE(1)

	MDRV_MACHINE_RESET( galaxy )

	/* video hardware */
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 208)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 0, 208-1)
	MDRV_GFXDECODE( galaxy )
	MDRV_PALETTE_LENGTH(sizeof (galaxy_palette) / 3)
	MDRV_PALETTE_INIT( galaxy )

	MDRV_VIDEO_START( galaxy )
	MDRV_VIDEO_UPDATE( galaxy )

	/* snapshot */
	MDRV_SNAPSHOT_ADD(galaxy, "gal", 0)
MACHINE_DRIVER_END

ROM_START (galaxy)
	ROM_REGION (0x10000, REGION_CPU1, ROMREGION_ERASEFF)
	ROM_LOAD ("galrom1.bin", 0x0000, 0x1000, CRC(365f3e24) SHA1(ffc6bf2ec09eabdad76604a63f5dd697c30c4358))
	ROM_LOAD_OPTIONAL ("galrom2.bin", 0x1000, 0x1000, CRC(5dc5a100) SHA1(5d5ab4313a2d0effe7572bb129193b64cab002c1))
	ROM_REGION(0x0800, REGION_GFX1,0)
	ROM_LOAD ("galchr.bin", 0x0000, 0x0800, CRC(5c3b5bb5) SHA1(19429a61dc5e55ddec3242a8f695e06dd7961f88))
ROM_END

SYSTEM_CONFIG_START(galaxy)
	CONFIG_RAM(2 * 1024)
	CONFIG_RAM_DEFAULT(6 * 1024)
	CONFIG_RAM((6+16) * 1024)
	CONFIG_RAM((6+32) * 1024)
	CONFIG_RAM((6+48) * 1024)
SYSTEM_CONFIG_END

/*    YEAR  NAME    PARENT  COMPAT  MACHINE INPUT   INIT    CONFIG  COMPANY FULLNAME */
COMP(1983,	galaxy,	0,	0,	galaxy,	galaxy,	galaxy,	galaxy,	"",	"Galaksija", 0)
