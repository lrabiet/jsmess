/* tmmjprd.c

 - split from rabbit.c  (it uses the same GFX chip, but is otherwise a different PCB, and until the methods
   of configuring the graphic chip are understood it's easier to work on them here)

 - in 16x16 tile mode, the offset into tileram doesn't neccessarily align to 16x16 tiles!  This makes using the
   tilemap system excessively difficult, as it expects predecoded tiles which simply isn't possible here.
   This is used for the girls in the intro at least, they specify 16x16 tiles on non 16x16 boundaries.
   (basically the physical tile rom addressing doesn't change between modes even if the data type does)
   (handling this in the tilemap system is very messy, might just be best with custom renderer)

 - Rom Test not hooked up (can read the gfx roms via a BANK
    - should hook this up as a test to help determine if the tmpdoki roms should really be different

 - EEPROM might be wrong, pressing what might be coin3 causes the game to hang
   (see input ports)

 - Video has a 'blitter' but it isn't used by these games, it is used by Rabbit

 - tmpdoki isn't a dual screen game, we should remove the dual screen layout from VIDEO_UPDATE and from the MACHINE_DRIVER, also notice that MAME
   doesn't have a macro for removing previously declared screens.

 - sound (see rabbit.c for preliminary details, not copied here)

 - sprites from one screen are overlapping on the other, probably there's a way to limit them to a single screen

 - priority is wrong.

*/


#include "driver.h"
#include "cpu/m68000/m68000.h"
#include "deprecat.h"
#include "machine/eeprom.h"
#include "rendlay.h"


static UINT32 *tmmjprd_tilemap_regs[4];
static UINT32 *tmmjprd_spriteregs;
//static UINT32 *tmmjprd_blitterregs;

static UINT32 *tmmjprd_tilemap_ram[4];

static UINT32 *tmmjprd_spriteram;

static WRITE32_HANDLER( tmmjprd_tilemap0_w )
{
	COMBINE_DATA(&tmmjprd_tilemap_ram[0][offset]);
}



static WRITE32_HANDLER( tmmjprd_tilemap1_w )
{
	COMBINE_DATA(&tmmjprd_tilemap_ram[1][offset]);
}

static WRITE32_HANDLER( tmmjprd_tilemap2_w )
{
	COMBINE_DATA(&tmmjprd_tilemap_ram[2][offset]);
}

static WRITE32_HANDLER( tmmjprd_tilemap3_w )
{
	COMBINE_DATA(&tmmjprd_tilemap_ram[3][offset]);
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int xoffs )
{
	int xpos,ypos,tileno,xflip,yflip, colr;
	const gfx_element *gfx = machine->gfx[0];
//	int todraw = (tmmjprd_spriteregs[5]&0x0fff0000)>>16; // how many sprites to draw (start/end reg..) what is the other half?


//	UINT32 *source = (tmmjprd_spriteram+ (todraw*2))-2;
//	UINT32 *finish = tmmjprd_spriteram;

	UINT32 *source = tmmjprd_spriteram+(0xc000/4)-2;
	UINT32 *finish = tmmjprd_spriteram;


	while( source>finish )
	{

		xpos = (source[0]&0x00000fff);
		ypos = (source[0]&0x0fff0000)>>16;

		xflip = (source[0]&0x00008000)>>15;
		yflip = (source[0]&0x00004000)>>14;
		colr = (source[1]&0x0ff00000)>>15;


		tileno = (source[1]&0x0003ffff);

		// the tiles in this are 8bpp, it can probably do the funky sub-tile addressing for them too tho..
		tileno >>=1;

		colr =   (source[1]&0x0ff00000)>>20;

		if(xpos&0x800)xpos-=0x1000;

		// 255 for 8bpp
		drawgfx_transpen(bitmap,cliprect,gfx,tileno,colr,!xflip,yflip,(xpos-xoffs)-8,(ypos)-8,255);

		source-=2;
	}
}

static void ttmjprd_draw_tile(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int x,int y,int sizex,int sizey, UINT32 tiledata, UINT8* rom)
{
	/* note, it's tile address _NOT_ tile number, 'sub-tile' access is possible, hence using the custom rendering */
	int tileaddr = (tiledata&0x000fffff)>>0;
	int colour   = (tiledata&0x0ff00000)>>20;
	int depth    = (tiledata&0x10000000)>>28;
	//int flipxy   = (tiledata&0x60000000)>>29;
	//		                 0x80000000   (blank tile like metro.c?)
	int drawx,drawy;
	int count;

	// entirely off right
	if (x > cliprect->max_x)
		return;

	// entirely off left
	if ((x+sizex) < cliprect->min_x)
		return;

	// entirely off bottom
	if (y > cliprect->max_y)
		return;

	// entirely off bottom
	if ((y+sizey) < cliprect->min_y)
		return;

	count = 0;
	for (drawy=y;drawy<y+sizey;drawy++)
	{
		for (drawx=x;drawx<x+sizex;drawx++)
		{
			UINT16 dat;
			UINT16* dst;

			if (!depth)
			{
				if ((drawx < cliprect->max_x) && (drawx > cliprect->min_x) && (drawy < cliprect->max_y) && (drawy > cliprect->min_y))
				{
					dat = (rom[(tileaddr*32)+count] & 0xf0)>>4;
					if (dat!=15)
					{
						//dat += (colour<<8);
						dst = BITMAP_ADDR16(bitmap, drawy, drawx);
						dst[0] = dat;
					}
				}
				drawx++;
				if ((drawx < cliprect->max_x) && (drawx > cliprect->min_x) && (drawy < cliprect->max_y) && (drawy > cliprect->min_y))
				{
					dat = (rom[(tileaddr*32)+count] & 0x0f);
					if (dat!=15)
					{
						//dat += (colour<<8);
						dst = BITMAP_ADDR16(bitmap, drawy, drawx);
						dst[0] = dat;
					}
				}

				count++;
			}
			else
			{
				if ((drawx < cliprect->max_x) && (drawx > cliprect->min_x) && (drawy < cliprect->max_y) && (drawy > cliprect->min_y))
				{
					dat = (rom[(tileaddr*32)+count] & 0xff);
					if (dat!=255)
					{
						dat += (colour<<8);
						dst = BITMAP_ADDR16(bitmap, drawy, drawx);
						dst[0] = dat;
					}
				}
				count++;
			}
		}
	}
}

static void ttmjprd_draw_tilemap(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, UINT32*tileram, UINT32*tileregs, UINT8*rom )
{
	int y,x;
	int count;

	int tilemap_sizex = 64;
	int tilemap_sizey = 64;

	int tile_sizex;
	int tile_sizey;

	int scrolly, scrollx;

	if (tileregs[0]&0x00400000)
	{
		tile_sizex = 16;
		tile_sizey = 16;
	}
	else
	{
		tile_sizex = 8;
		tile_sizey = 8;
	}

	scrolly = (tileregs[2] & 0xfff00000) >> 20;
	scrollx = (tileregs[2] & 0x0000fff0) >> 4;

	count = 0;
	for (y=0;y<tilemap_sizey;y++)
	{
		for (x=0;x<tilemap_sizex;x++)
		{
			UINT32 tiledata = tileram[count];
			// todo: handle wraparound
			ttmjprd_draw_tile(machine,bitmap,cliprect,(x*tile_sizex)-scrollx,(y*tile_sizey)-scrolly,tile_sizex,tile_sizey, tiledata, rom);
			count++;
		}
	}

}

static VIDEO_UPDATE( tmmjprd )
{
	UINT8* gfxroms = memory_region(screen->machine,"gfx2");
	const device_config *left_screen  = devtag_get_device(screen->machine, "lscreen");
	const device_config *right_screen = devtag_get_device(screen->machine, "rscreen");

	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));

	if (screen == left_screen)
	{
		ttmjprd_draw_tilemap( screen->machine, bitmap, cliprect, tmmjprd_tilemap_ram[3], tmmjprd_tilemap_regs[3], gfxroms );
		draw_sprites(screen->machine,bitmap,cliprect, 320);
		ttmjprd_draw_tilemap( screen->machine, bitmap, cliprect, tmmjprd_tilemap_ram[2], tmmjprd_tilemap_regs[2], gfxroms );
	}
	if (screen == right_screen)
	{
		ttmjprd_draw_tilemap( screen->machine, bitmap, cliprect, tmmjprd_tilemap_ram[1], tmmjprd_tilemap_regs[1], gfxroms );
		draw_sprites(screen->machine,bitmap,cliprect, 0);
		ttmjprd_draw_tilemap( screen->machine, bitmap, cliprect, tmmjprd_tilemap_ram[0], tmmjprd_tilemap_regs[0], gfxroms );
	}

	/*
	popmessage("%08x %08x %08x %08x %08x %08x",
	tmmjprd_tilemap_regs[2][0],
	tmmjprd_tilemap_regs[2][1],
	tmmjprd_tilemap_regs[2][2],
	tmmjprd_tilemap_regs[2][3],
	tmmjprd_tilemap_regs[2][4],
	tmmjprd_tilemap_regs[2][5]);
	*/

/*
	popmessage("%08x %08x %08x %08x %08x %08x %08x",
	tmmjprd_spriteregs[0],
	tmmjprd_spriteregs[1],
	tmmjprd_spriteregs[2],
	tmmjprd_spriteregs[3],
	tmmjprd_spriteregs[4],
	tmmjprd_spriteregs[5],
	tmmjprd_spriteregs[6]);
*/

	return 0;
}

static VIDEO_START(tmmjprd)
{
	/* the tilemaps are bigger than the regions the cpu can see, need to allocate the ram here */
	/* or maybe not for this game/hw .... */
	tmmjprd_tilemap_ram[0] = auto_alloc_array_clear(machine, UINT32, 0x8000);
	tmmjprd_tilemap_ram[1] = auto_alloc_array_clear(machine, UINT32, 0x8000);
	tmmjprd_tilemap_ram[2] = auto_alloc_array_clear(machine, UINT32, 0x8000);
	tmmjprd_tilemap_ram[3] = auto_alloc_array_clear(machine, UINT32, 0x8000);
}

static READ32_HANDLER( tmmjprd_tilemap0_r )
{
	return tmmjprd_tilemap_ram[0][offset];
}

static READ32_HANDLER( tmmjprd_tilemap1_r )
{
	return tmmjprd_tilemap_ram[1][offset];
}

static READ32_HANDLER( tmmjprd_tilemap2_r )
{
	return tmmjprd_tilemap_ram[2][offset];
}

static READ32_HANDLER( tmmjprd_tilemap3_r )
{
	return tmmjprd_tilemap_ram[3][offset];
}

static READ32_HANDLER( randomtmmjprds )
{
	return 0x0000;//mame_rand(space->machine);
}


#define BLITCMDLOG 0
#define BLITLOG 0

#if 0
static TIMER_CALLBACK( tmmjprd_blit_done )
{
	cputag_set_input_line(machine, "maincpu", 3, HOLD_LINE);
}

static void tmmjprd_do_blit(running_machine *machine)
{
	UINT8 *blt_data = memory_region(machine, "gfx1");
	int blt_source = (tmmjprd_blitterregs[0]&0x000fffff)>>0;
	int blt_column = (tmmjprd_blitterregs[1]&0x00ff0000)>>16;
	int blt_line   = (tmmjprd_blitterregs[1]&0x000000ff);
	int blt_tilemp = (tmmjprd_blitterregs[2]&0x0000e000)>>13;
	int blt_oddflg = (tmmjprd_blitterregs[2]&0x00000001)>>0;
	int mask,shift;


	if(BLITCMDLOG) mame_printf_debug("BLIT command %08x %08x %08x\n", tmmjprd_blitterregs[0], tmmjprd_blitterregs[1], tmmjprd_blitterregs[2]);

	if (blt_oddflg&1)
	{
		mask = 0xffff0000;
		shift= 0;
	}
	else
	{
		mask = 0x0000ffff;
		shift= 16;
	}

	blt_oddflg>>=1; /* blt_oddflg is now in dword offsets*/
	blt_oddflg+=0x80*blt_line;

	blt_source<<=1; /* blitsource is in word offsets */

	while(1)
	{
		int blt_commnd = blt_data[blt_source+1];
		int blt_amount = blt_data[blt_source+0];
		int blt_value;
		int loopcount;
		int writeoffs;
		blt_source+=2;

		switch (blt_commnd)
		{
			case 0x00: /* copy nn bytes */
				if (!blt_amount)
				{
					if(BLITLOG) mame_printf_debug("end of blit list\n");
					timer_set(machine, ATTOTIME_IN_USEC(500), NULL,0,tmmjprd_blit_done);
					return;
				}

				if(BLITLOG) mame_printf_debug("blit copy %02x bytes\n", blt_amount);
				for (loopcount=0;loopcount<blt_amount;loopcount++)
				{
					blt_value = ((blt_data[blt_source+1]<<8)|(blt_data[blt_source+0]));
					blt_source+=2;
					writeoffs=blt_oddflg+blt_column;
					tmmjprd_tilemap_ram[blt_tilemp][writeoffs]=(tmmjprd_tilemap_ram[blt_tilemp][writeoffs]&mask)|(blt_value<<shift);
					tilemap_mark_tile_dirty(tmmjprd_tilemap[blt_tilemp],writeoffs);

					blt_column++;
					blt_column&=0x7f;
				}

				break;

			case 0x02: /* fill nn bytes */
				if(BLITLOG) mame_printf_debug("blit fill %02x bytes\n", blt_amount);
				blt_value = ((blt_data[blt_source+1]<<8)|(blt_data[blt_source+0]));
				blt_source+=2;

				for (loopcount=0;loopcount<blt_amount;loopcount++)
				{
					writeoffs=blt_oddflg+blt_column;
					tmmjprd_tilemap_ram[blt_tilemp][writeoffs]=(tmmjprd_tilemap_ram[blt_tilemp][writeoffs]&mask)|(blt_value<<shift);
					tilemap_mark_tile_dirty(tmmjprd_tilemap[blt_tilemp],writeoffs);
					blt_column++;
					blt_column&=0x7f;
				}

				break;

			case 0x03: /* next line */
				if(BLITLOG) mame_printf_debug("blit: move to next line\n");
				blt_column = (tmmjprd_blitterregs[1]&0x00ff0000)>>16; /* --CC---- */
				blt_oddflg+=128;
				break;

			default: /* unknown / illegal */
				if(BLITLOG) mame_printf_debug("uknown blit command %02x\n",blt_commnd);
				break;
		}
	}

}



static WRITE32_HANDLER( tmmjprd_blitter_w )
{
	COMBINE_DATA(&tmmjprd_blitterregs[offset]);

	if (offset == 0x0c/4)
	{
		tmmjprd_do_blit(space->machine);
	}
}
#endif

static UINT8 mux_data;

static WRITE32_HANDLER( tmmjprd_eeprom_write )
{
	// don't disturb the EEPROM if we're not actually writing to it
	// (in particular, data & 0x100 here with mask = ffff00ff looks to be the watchdog)
	if (mem_mask == 0x000000ff)
		mux_data = (~data & 0xff);

	if (mem_mask == 0xff000000)
	{
		// latch the bit
		eeprom_write_bit(data & 0x01000000);

		// reset line asserted: reset.
		eeprom_set_cs_line((data & 0x04000000) ? CLEAR_LINE : ASSERT_LINE );

		// clock line asserted: write latch or select next bit to read
		eeprom_set_clock_line((data & 0x02000000) ? ASSERT_LINE : CLEAR_LINE );
	}
}

static READ32_HANDLER( tmmjprd_mux_r )
{
	static UINT8 system_in;

	system_in = input_port_read(space->machine, "SYSTEM");

	switch(mux_data)
	{
		case 0x01: return (system_in & 0xff) | (input_port_read(space->machine, "PL1_1")<<8) | (input_port_read(space->machine, "PL2_1")<<16) | 0xff000000;
		case 0x02: return (system_in & 0xff) | (input_port_read(space->machine, "PL1_2")<<8) | (input_port_read(space->machine, "PL2_2")<<16) | 0xff000000;
		case 0x04: return (system_in & 0xff) | (input_port_read(space->machine, "PL1_3")<<8) | (input_port_read(space->machine, "PL2_3")<<16) | 0xff000000;
		case 0x08: return (system_in & 0xff) | (input_port_read(space->machine, "PL1_4")<<8) | (input_port_read(space->machine, "PL2_4")<<16) | 0xff000000;
		case 0x10: return (system_in & 0xff) | (input_port_read(space->machine, "PL1_5")<<8) | (input_port_read(space->machine, "PL2_5")<<16) | 0xff000000;
	}

	return (system_in & 0xff) | 0xffffff00;
}

static INPUT_PORTS_START( tmmjprd )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("Left Screen Coin A")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("Left Screen Coin B")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN ) // might actually be coin 3 button.. but hangs the game? (eeprom issue?)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("Right Screen Coin A")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_NAME("Right Screen Coin B") // might actually be service 1
	PORT_SERVICE( 0x20, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(eeprom_bit_r, NULL)	// CHECK!

	PORT_START("PL1_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL1_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) //bet button
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL1_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL1_4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(1)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL1_5")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) //bet button
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_5")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static WRITE32_HANDLER( tmmjprd_paletteram_dword_w )
{
	int r,g,b;
	COMBINE_DATA(&paletteram32[offset]);

	b = ((paletteram32[offset] & 0x000000ff) >>0);
	r = ((paletteram32[offset] & 0x0000ff00) >>8);
	g = ((paletteram32[offset] & 0x00ff0000) >>16);

	palette_set_color(space->machine,offset,MAKE_RGB(r,g,b));
}

static double old_brt1, old_brt2;

/* notice that data & 0x4 is always cleared on brt_1 and set on brt_2.        *
 * My wild guess is that bits 0,1 and 2 controls what palette entries to dim. */
static WRITE32_HANDLER( tmmjprd_brt_1_w )
{
	int i;
	double brt;
	int bank;

	data>>=24;
	brt = ((data & 0x78)>>3) / 16.0;
	bank = data & 0x4 ? 0x800 : 0; //guess

	if(data & 0x80 && old_brt1 != brt)
	{
		old_brt1 = brt;
		for (i = bank; i < 0x800+bank; i++)
			palette_set_pen_contrast(space->machine, i, brt);
	}
}

static WRITE32_HANDLER( tmmjprd_brt_2_w )
{
	int i;
	double brt;
	int bank;

	data>>=24;
	brt = ((data & 0x78)>>3) / 16.0;
	bank = data & 0x4 ? 0x800 : 0; //guess

	if(data & 0x80 && old_brt2 != brt)
	{
		old_brt2 = brt;
		for (i = bank; i < 0x800+bank; i++)
			palette_set_pen_contrast(space->machine, i, brt);
	}
}

static ADDRESS_MAP_START( tmmjprd_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM
	AM_RANGE(0x200010, 0x200013) AM_READ(randomtmmjprds) // gfx chip status?
	AM_RANGE(0x200980, 0x200983) AM_READ(randomtmmjprds) // sound chip status?
	AM_RANGE(0x200984, 0x200987) AM_READ(randomtmmjprds) // sound chip status?
	/* check these are used .. */
//  AM_RANGE(0x200010, 0x200013) AM_WRITEONLY AM_BASE( &tmmjprd_viewregs0 )
	AM_RANGE(0x200100, 0x200117) AM_WRITEONLY AM_BASE( &tmmjprd_tilemap_regs[0] ) // tilemap regs1
	AM_RANGE(0x200120, 0x200137) AM_WRITEONLY AM_BASE( &tmmjprd_tilemap_regs[1] ) // tilemap regs2
	AM_RANGE(0x200140, 0x200157) AM_WRITEONLY AM_BASE( &tmmjprd_tilemap_regs[2] ) // tilemap regs3
	AM_RANGE(0x200160, 0x200177) AM_WRITEONLY AM_BASE( &tmmjprd_tilemap_regs[3] ) // tilemap regs4
	AM_RANGE(0x200200, 0x20021b) AM_WRITEONLY AM_BASE( &tmmjprd_spriteregs ) // sprregs?
//  AM_RANGE(0x200300, 0x200303) AM_WRITE(tmmjprd_rombank_w) // used during rom testing, rombank/area select + something else?
  	AM_RANGE(0x20040c, 0x20040f) AM_WRITE(tmmjprd_brt_1_w)
    AM_RANGE(0x200410, 0x200413) AM_WRITE(tmmjprd_brt_2_w)
//  AM_RANGE(0x200500, 0x200503) AM_WRITEONLY AM_BASE( &tmmjprd_viewregs7 )
//  AM_RANGE(0x200700, 0x20070f) AM_WRITE(tmmjprd_blitter_w) AM_BASE( &tmmjprd_blitterregs )
//  AM_RANGE(0x200800, 0x20080f) AM_WRITEONLY AM_BASE( &tmmjprd_viewregs9 ) // never changes?
//  AM_RANGE(0x200900, 0x20098f) AM_WRITE(tmmjprd_audio_w)
	/* hmm */
//  AM_RANGE(0x279700, 0x279713) AM_WRITEONLY AM_BASE( &tmmjprd_viewregs10 )
	/* tilemaps */
	AM_RANGE(0x280000, 0x283fff) AM_READWRITE(tmmjprd_tilemap0_r,tmmjprd_tilemap0_w)
	AM_RANGE(0x284000, 0x287fff) AM_READWRITE(tmmjprd_tilemap1_r,tmmjprd_tilemap1_w)
	AM_RANGE(0x288000, 0x28bfff) AM_READWRITE(tmmjprd_tilemap2_r,tmmjprd_tilemap2_w)
	AM_RANGE(0x28c000, 0x28ffff) AM_READWRITE(tmmjprd_tilemap3_r,tmmjprd_tilemap3_w)
	/* ?? is palette ram shared with sprites in this case or just a different map */
	AM_RANGE(0x290000, 0x29bfff) AM_RAM AM_BASE(&tmmjprd_spriteram)
	AM_RANGE(0x29c000, 0x29ffff) AM_RAM_WRITE(tmmjprd_paletteram_dword_w) AM_BASE(&paletteram32)

	AM_RANGE(0x400000, 0x400003) AM_READ(tmmjprd_mux_r) AM_WRITE(tmmjprd_eeprom_write)
	AM_RANGE(0xf00000, 0xffffff) AM_RAM
ADDRESS_MAP_END



static const gfx_layout rabbit_sprite_16x16x8_layout =
{
	16,16,
	RGN_FRAC(1,2),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{
	40, 32, RGN_FRAC(1,2)+40,RGN_FRAC(1,2)+32,
	56, 48,RGN_FRAC(1,2)+56, RGN_FRAC(1,2)+48,
	8, 0,RGN_FRAC(1,2)+8   , RGN_FRAC(1,2)+0,
	24, 16,RGN_FRAC(1,2)+24, RGN_FRAC(1,2)+16,
	},


	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,8*64,9*64,10*64,11*64,12*64,13*64,14*64,15*64 },
	16*64
};


// gfx decoding is ugly.. 16*16 tiles can start at varying different offsets..
static GFXDECODE_START( tmmjprd )
	/* this seems to be sprites */
//	GFXDECODE_ENTRY( "gfx1", 0, tmmjprd_sprite_8x8x4_layout,   0x0, 0x1000  )
//	GFXDECODE_ENTRY( "gfx1", 0, rabbit_sprite_16x16x4_layout, 0x0, 0x1000  )
//	GFXDECODE_ENTRY( "gfx1", 0, tmmjprd_sprite_8x8x8_layout,   0x0, 0x1000  )
	GFXDECODE_ENTRY( "gfx1", 0, rabbit_sprite_16x16x8_layout, 0x0, 0x1000  )
GFXDECODE_END


static INTERRUPT_GEN( tmmjprd_interrupt )
{
	int intlevel = 0;

	if (cpu_getiloops(device)==0)
		intlevel = 5;
	else
		intlevel = 3;

	cpu_set_input_line(device, intlevel, HOLD_LINE);
}

static MACHINE_DRIVER_START( tmmjprd )
	MDRV_CPU_ADD("maincpu",M68EC020,24000000) /* 24 MHz */
	MDRV_CPU_PROGRAM_MAP(tmmjprd_map)
	MDRV_CPU_VBLANK_INT_HACK(tmmjprd_interrupt,2)
	MDRV_NVRAM_HANDLER(93C46)

	MDRV_GFXDECODE(tmmjprd)

//	MDRV_SCREEN_ADD("screen", RASTER)
//	MDRV_SCREEN_REFRESH_RATE(60)
//	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
//	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
//	MDRV_SCREEN_SIZE(64*16, 64*16)
//	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 28*8-1)
	MDRV_PALETTE_LENGTH(0x1000)


	MDRV_DEFAULT_LAYOUT(layout_dualhsxs)

	MDRV_SCREEN_ADD("lscreen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_SIZE(64*16, 64*16)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 28*8-1)
	//MDRV_SCREEN_VISIBLE_AREA(0*8, 64*16-1, 0*8, 64*16-1)

	MDRV_SCREEN_ADD("rscreen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_SIZE(64*16, 64*16)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 28*8-1)
	//MDRV_SCREEN_VISIBLE_AREA(0*8, 64*16-1, 0*8, 64*16-1)


	MDRV_VIDEO_START(tmmjprd)
	MDRV_VIDEO_UPDATE(tmmjprd)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( tmpdoki )
	MDRV_IMPORT_FROM(tmmjprd)
	MDRV_DEFAULT_LAYOUT(layout_horizont)
MACHINE_DRIVER_END




ROM_START( tmmjprd )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_BYTE( "p00.bin", 0x000000, 0x080000, CRC(a1efd960) SHA1(7f41ab58de32777bccbfe28e6e5a1f2dca35bb90) )
	ROM_LOAD32_BYTE( "p01.bin", 0x000001, 0x080000, CRC(9c325374) SHA1(1ddf1c292fc1bcf4dcefb5d4aa3abdeb1489c020) )
 	ROM_LOAD32_BYTE( "p02.bin", 0x000002, 0x080000, CRC(729a5f12) SHA1(615704d36afdceb4b1ff2e5dc34856e614181e16) )
	ROM_LOAD32_BYTE( "p03.bin", 0x000003, 0x080000, CRC(595615ab) SHA1(aca746d74aa6e7e856eb5c9b740d884778743b27) )

	ROM_REGION( 0x2000000, "gfx1", 0 ) /* Sprite Roms */
	ROM_LOAD32_WORD( "00.bin", 0x1000002, 0x400000, CRC(303e91a1) SHA1(c29a22061ab8af8b72e0e6bdb36915a0cb5b2a5c) )
	ROM_LOAD32_WORD( "01.bin", 0x0000002, 0x400000, CRC(3371b775) SHA1(131dd850bd01dac52fa82c41948d900c4833db3c) )
	ROM_LOAD32_WORD( "02.bin", 0x1000000, 0x400000, CRC(4c1e13b9) SHA1(d244eb74f755350604824670db58ab2a56a856cb) )
	ROM_LOAD32_WORD( "03.bin", 0x0000000, 0x400000, CRC(9cf86152) SHA1(e27e0d9befb12ad5c2acf547afe80d1c7921a4d1) )
	ROM_LOAD32_WORD( "10.bin", 0x1800002, 0x400000, CRC(5ab6af41) SHA1(e29cee23c84e17dd8dabd2ec71e622c25418646e) )
	ROM_LOAD32_WORD( "11.bin", 0x0800002, 0x400000, CRC(1d1fd633) SHA1(655be5b72bb70a90d23e49512ca84d9978d87b0b) )
	ROM_LOAD32_WORD( "12.bin", 0x1800000, 0x400000, CRC(5b8bb9d6) SHA1(ee93774077d8a2ddcf70869a9c2f4961219a85b4) )
 	ROM_LOAD32_WORD( "13.bin", 0x0800000, 0x400000, CRC(d950df0a) SHA1(3b109341ab4ad87005113fb481b5d1ed9a82f50f) )

	ROM_REGION( 0x2000000, "gfx2", 0 ) /* BG Roms */
	ROM_LOAD32_WORD( "40.bin", 0x0000000, 0x400000, CRC(8bedc606) SHA1(7159c8b86e8d7d5ae202c239638483ccdc7dfc25) )
	ROM_LOAD32_WORD( "41.bin", 0x0000002, 0x400000, CRC(e19713dd) SHA1(a8f1b716913f2e391abf277e5bf0e9986cc75898) )
	ROM_LOAD32_WORD( "50.bin", 0x0800000, 0x400000, CRC(85ca9ce9) SHA1(c5a7270507522e11e9485196be325508846fda90) )
	ROM_LOAD32_WORD( "51.bin", 0x0800002, 0x400000, CRC(6ba1d2ec) SHA1(bbe7309b33f213c8cb9ab7adb3221ea79f89e8b0) )
	ROM_LOAD32_WORD( "60.bin", 0x1000000, 0x400000, CRC(7cb132e0) SHA1(f9c366befec46c7f6e307111a62eede029202b16) )
	ROM_LOAD32_WORD( "61.bin", 0x1000002, 0x400000, CRC(caa7e854) SHA1(592867e001abd0781f83a5124bf9aa62ad1aa7f3) )
	ROM_LOAD32_WORD( "70.bin", 0x1800000, 0x400000, CRC(9b737ae4) SHA1(0b62a90d42ace81ee32db073a57731a55a32f989) )
	ROM_LOAD32_WORD( "71.bin", 0x1800002, 0x400000, CRC(189f694e) SHA1(ad0799d4aadade51be38d824910d299257a758a3) )

	ROM_REGION( 0x800000, "unknown", 0 ) /* Sound Roms? */
	ROM_LOAD16_BYTE( "21.bin", 0x0000001, 0x400000, CRC(bb5fa8da) SHA1(620e609b3e2524d06d58844625f186fd4682205f))
ROM_END

// single screen?
ROM_START( tmpdoki )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_BYTE( "u70_p0.bin", 0x000000, 0x080000, CRC(c0ee1942) SHA1(0cebc3e326d84e200c2399208d810c0ac767dbb4) )
	ROM_LOAD32_BYTE( "u72_p1.bin", 0x000001, 0x080000, CRC(3c1bc6f6) SHA1(7b3719d4bb52e45db793564b0ccee067fd7af4e4) )
 	ROM_LOAD32_BYTE( "u71_p2.bin", 0x000002, 0x080000, CRC(f2091cce) SHA1(88c6822eb1546e914c2644264367e71fb2a82be3) )
	ROM_LOAD32_BYTE( "u73_p3.bin", 0x000003, 0x080000, CRC(cca8ef13) SHA1(d5b077f3d8d38262e69d058a7d61e4563332abce) )

	ROM_REGION( 0x2000000, "gfx1", 0 ) /* Sprite Roms */
	ROM_LOAD32_WORD( "00.bin", 0x1000002, 0x400000, CRC(303e91a1) SHA1(c29a22061ab8af8b72e0e6bdb36915a0cb5b2a5c) )
	ROM_LOAD32_WORD( "01.bin", 0x0000002, 0x400000, CRC(3371b775) SHA1(131dd850bd01dac52fa82c41948d900c4833db3c) )
	ROM_LOAD32_WORD( "02.bin", 0x1000000, 0x400000, CRC(4c1e13b9) SHA1(d244eb74f755350604824670db58ab2a56a856cb) )
	ROM_LOAD32_WORD( "03.bin", 0x0000000, 0x400000, CRC(9cf86152) SHA1(e27e0d9befb12ad5c2acf547afe80d1c7921a4d1) )
	ROM_LOAD32_WORD( "10.bin", 0x1800002, 0x400000, CRC(5ab6af41) SHA1(e29cee23c84e17dd8dabd2ec71e622c25418646e) )
	ROM_LOAD32_WORD( "11.bin", 0x0800002, 0x400000, CRC(1d1fd633) SHA1(655be5b72bb70a90d23e49512ca84d9978d87b0b) )
	ROM_LOAD32_WORD( "12.bin", 0x1800000, 0x400000, CRC(5b8bb9d6) SHA1(ee93774077d8a2ddcf70869a9c2f4961219a85b4) )
 	ROM_LOAD32_WORD( "13.bin", 0x0800000, 0x400000, CRC(d950df0a) SHA1(3b109341ab4ad87005113fb481b5d1ed9a82f50f) )

	ROM_REGION( 0x2000000, "gfx2", 0 ) /* BG Roms */
	ROM_LOAD32_WORD( "40.bin", 0x0000000, 0x400000, CRC(8bedc606) SHA1(7159c8b86e8d7d5ae202c239638483ccdc7dfc25) )
	ROM_LOAD32_WORD( "41.bin", 0x0000002, 0x400000, CRC(e19713dd) SHA1(a8f1b716913f2e391abf277e5bf0e9986cc75898) )
	ROM_LOAD32_WORD( "50.bin", 0x0800000, 0x400000, CRC(85ca9ce9) SHA1(c5a7270507522e11e9485196be325508846fda90) )
	ROM_LOAD32_WORD( "51.bin", 0x0800002, 0x400000, CRC(6ba1d2ec) SHA1(bbe7309b33f213c8cb9ab7adb3221ea79f89e8b0) )

	/* I think these should be different, the game attempts to draw tiles from here for the title logo, but
	   the tiles are empty.  Once the ROM check is hooked up this will be easier to confirm */
	ROM_LOAD32_WORD( "60.bin", 0x1000000, 0x400000, BAD_DUMP CRC(7cb132e0) SHA1(f9c366befec46c7f6e307111a62eede029202b16) )
	ROM_LOAD32_WORD( "61.bin", 0x1000002, 0x400000, BAD_DUMP CRC(caa7e854) SHA1(592867e001abd0781f83a5124bf9aa62ad1aa7f3) )
	ROM_LOAD32_WORD( "70.bin", 0x1800000, 0x400000, BAD_DUMP CRC(9b737ae4) SHA1(0b62a90d42ace81ee32db073a57731a55a32f989) )
	ROM_LOAD32_WORD( "71.bin", 0x1800002, 0x400000, BAD_DUMP CRC(189f694e) SHA1(ad0799d4aadade51be38d824910d299257a758a3) )

	ROM_REGION( 0x800000, "unknown", 0 ) /* Sound Roms? */
	ROM_LOAD16_BYTE( "21.bin", 0x0000001, 0x400000, CRC(bb5fa8da) SHA1(620e609b3e2524d06d58844625f186fd4682205f))
ROM_END


GAME( 1997, tmmjprd,       0, tmmjprd, tmmjprd, 0, ROT0, "Media / Sonnet", "Tokimeki Mahjong Paradise - Dear My Love", GAME_IMPERFECT_GRAPHICS | GAME_NO_SOUND )
GAME( 1998, tmpdoki, tmmjprd, tmpdoki, tmmjprd, 0, ROT0, "Media / Sonnet", "Tokimeki Mahjong Paradise - Doki Doki Hen", GAME_IMPERFECT_GRAPHICS | GAME_NO_SOUND ) // missing gfx due to wrong roms?
