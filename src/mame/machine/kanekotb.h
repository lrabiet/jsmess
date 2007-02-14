/***************************************************************************

    TOYBOX MCU data for Bonk's Adventure

***************************************************************************/

// bonkadv_mcu_4_33 has 32 zeroed bytes every 64 bytes: it has been verified
// that these zeroes are effectively written by the MCU

// MCU executed command: 0400 0E00 0034
/*
200E00: 7071                     moveq   #$71, D0
200E02: 7273                     moveq   #$73, D1
200E04: 7475                     moveq   #$75, D2
200E06: 7677                     moveq   #$77, D3
200E08: 7879                     moveq   #$79, D4
200E0A: 7A7B                     moveq   #$7b, D5
200E0C: 7C7D                     moveq   #$7d, D6
200E0E: 7E7F                     moveq   #$7f, D7
200E10: 4E75                     rts
*/
static UINT16 bonkadv_mcu_4_34[] = {
	0x7071,0x7273,0x7475,0x7677,0x7879,0x7a7b,0x7c7d,0x7e7f,
	0x4e75
};

// MCU executed command: 0400 0180 0032 - 128 bytes at $200180
static UINT16 bonkadv_mcu_4_32[] = {
	0x00cc,0xcc0c,0xc0c0,0xc080,0x0484,0xb6a6,0x0404,0x80c0,
	0x80b1,0xb1a1,0xa1b2,0xa2b3,0xb3a3,0xa3b1,0xb1b1,0xb1c0,
	0xc0a1,0xa1a1,0xa1b2,0xb2a2,0xa290,0x9090,0xb9b9,0xa9a9,
	0xbaaa,0xbbbb,0xabab,0xb9b9,0xb9b9,0xc8c8,0xa9a9,0xa9a9,
	0xbaba,0xaaaa,0x9898,0x98b9,0xb9a9,0xa9ba,0xaabb,0xbbab,
	0xabb9,0xb9b9,0xb9c8,0xc8a9,0xa9a9,0xa9ba,0xbaaa,0xaa98,
	0x9898,0xb1b1,0xa1a1,0xb2a2,0xb3b3,0xa3a3,0xb1b1,0xb1b1,
	0xc0c0,0xa1a1,0xa1a1,0xb2b2,0xa2a2,0x9090,0x9000,0x0000
};

// MCU executed command: 0400 0280 0031 - 128 bytes at $200280
static UINT16 bonkadv_mcu_4_31[] = {
	0x1013,0x1411,0x1216,0x1519,0x1a17,0x1824,0x2322,0x211f,
	0x201e,0x1d1c,0x1b27,0x2825,0x2629,0x2a2b,0x2e2f,0x2c2d,
	0x3130,0x3435,0x3233,0x3f3e,0x3d3c,0x3a3b,0x3938,0x3736,
	0x4243,0x4041,0x4445,0x4649,0x4a47,0x484c,0x4b4f,0x504d,
	0x4e5a,0x5958,0x5755,0x5654,0x5352,0x515d,0x5e5b,0x5c5f,
	0x6061,0x6465,0x6263,0x6766,0x6a6b,0x6869,0x7574,0x7372,
	0x7071,0x6f6e,0x6d6c,0x7879,0x7677,0x7a7b,0x7c7d,0x7e7f,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
};

// MCU executed command: 0400 0E50 0030 - 688 bytes at $200E50
static UINT16 bonkadv_mcu_4_30[] = {
	0x8c00,0xa000,0x9700,0xa700,0xb200,0xb700,0xc000,0xcc00,
	0xd600,0xdf00,0xef00,0xf500,0xfd00,0x0501,0x0b01,0x1801,
	0x1e01,0x2901,0x3101,0x3701,0x4101,0x4901,0x0b01,0x5d01,
	0xa700,0xb200,0x6c01,0x8101,0xb200,0x8101,0x8101,0x8101,
	0x8101,0x8101,0x8101,0x8101,0x8101,0x8101,0x8101,0x8101,
	0x8101,0x8101,0x8101,0x8101,0x8101,0x8101,0x8101,0x8101,
	0x8101,0x8101,0x8101,0x8101,0x8101,0x8101,0x8101,0x2e02,
	0x3602,0x3d02,0x4102,0x4f02,0x5502,0x5502,0x5f02,0x6402,
	0x6d02,0x7302,0x7802,0x7d02,0x8302,0x8a02,0x0165,0x712b,
	0x57fe,0x57a2,0xff00,0x0003,0x7c3a,0x436c,0x66ff,0x0000,
	0x026c,0x6d43,0xff00,0x0000,0x74a5,0x8373,0x3599,0x9fff,

	0x0000,0x074a,0xff00,0x0002,0x8788,0x3331,0x32ff,0x0000,
	0x0276,0x8c71,0x2d8c,0xfe12,0x8cff,0x0000,0x0171,0x0d29,
	0xfe0d,0x29ff,0x0000,0x0559,0x5a5b,0x8a8b,0xff00,0x0003,
	0x0d0e,0x576a,0x641e,0xfe0d,0x0efe,0x64a3,0xff00,0x0006,
	0x0262,0xff00,0x0007,0x3e3f,0x7a7b,0xff00,0x0007,0x7584,
	0x4c53,0xff00,0x0001,0x6645,0xff00,0x0007,0x0d18,0x2984,
	0x5384,0xfe0d,0x29ff,0x0000,0x0457,0x5cff,0x0000,0x0751,
	0x5202,0x45fe,0x4575,0xff00,0x0000,0x191a,0x1b6e,0xff00,
	0x0006,0x3472,0xff00,0x0000,0x6569,0x090f,0x8b23,0xff00,
	0x0004,0x3b3c,0x3d79,0xff00,0x0080,0x770a,0x8176,0x2a65,
	0x64fe,0x7776,0xfe2a,0x76fe,0x650a,0xff00,0x0000,0x1011,
	0x1226,0x2728,0xa655,0x940f,0x68ff,0x0000,0x8045,0x6477,
	0x0a81,0x762a,0x65fe,0x7776,0xfe2a,0x76fe,0x650a,0xff00,
	0x0000,0x166b,0xff00,0x0007,0x1e30,0x39ff,0x0000,0x0345,
	0x3a77,0x7843,0x4647,0xff00,0x0000,0x7335,0x8399,0x9fff,
	0x0000,0x0237,0x38ff,0x0000,0x0044,0x303b,0xff00,0x0000,

	0x7d7e,0x7fa1,0x0aff,0x0000,0x0070,0xff00,0x0000,0x3e3f,
	0x7a7b,0x6180,0x89a4,0xff00,0x0006,0x86a5,0x4f42,0xff00,
	0x0000,0x1011,0x1255,0x0f68,0xff00,0x0002,0x8788,0x4033,
	0xff00,0x0005,0x650f,0xff00,0x0008,0x9596,0x9ba7,0xff00,
	0x0000,0x4849,0x82ff,0x0000,0x022e,0x2f30,0xff00,0x0001,
	0x7778,0x5826,0x2728,0xa6ff,0x0000,0x0813,0x1402,0x51ff,
	0x0000,0x072e,0x5c5d,0xff00,0x0006,0x57ff,0x0000,0x0230,
	0x391e,0x65fe,0x6530,0xff00,0x0000,0x4bff,0x0000,0x008e,
	0x8f90,0x91ff,0x0000,0x0092,0x9d17,0xff00,0x0000,0xff00,
	0x0000,0x8d9b,0xa77d,0x7e7f,0xa10a,0x9596,0xff00,0x0001,
	0x1993,0xff00,0x0000,0x67ff,0x0000,0x0067,0xff00,0x0000,
	0x27ff,0x0000,0x0390,0x8e23,0x978b,0xff00,0x0000,0x1d9e,
	0xff00,0x0007,0x3bff,0x0000,0x001d,0xff00,0x0000,0x8e90,
	0xff00,0x0005,0x2495,0x96ff,0x0000,0x009a,0xff00,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
};

// MCU executed command: 0400 0400 0033 - 2560 bytes at $200400
static UINT16 bonkadv_mcu_4_33[] = {
	0x00a4,0x0001,0x00a5,0x005a,0x00a6,0x0074,0x00a7,0x009b,
	0x00a8,0x00d0,0x00a9,0x00fe,0x00aa,0x015d,0x00ab,0x01b0,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0001,0x0002,0x0019,0x005b,0x0022,0x0075,0x002d,0x009c,
	0x0041,0x00d1,0x004e,0x00ff,0x0060,0x015f,0x0072,0x01b1,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0002,0x0008,0x001a,0x0061,0x0023,0x007b,0x002e,0x00a2,
	0x0042,0x00d7,0x004f,0x0107,0x0061,0x0165,0x0073,0x01b7,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x000a,0x0000,0x0063,0x0000,0x007d,0x0000,0x00a4,
	0x0000,0x00d9,0x0000,0x0109,0x0000,0x0167,0x0000,0x01b9,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0003,0x000b,0x001b,0x0064,0x0024,0x007e,0x002f,0x00a5,
	0x0043,0x00da,0x0050,0x010a,0x0062,0x0168,0x0074,0x01ba,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x000c,0x0000,0x0065,0x0000,0x007f,0x0000,0x00a6,
	0x0000,0x00db,0x0000,0x010b,0x0000,0x0169,0x0000,0x01bb,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0004,0x0008,0x001c,0x0061,0x0025,0x007b,0x0030,0x00a2,
	0x0044,0x00d7,0x0051,0x0107,0x0063,0x0165,0x0075,0x01b7,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0005,0x000d,0x001d,0x0066,0x0025,0x007b,0x0031,0x00a7,
	0x0045,0x00dc,0x0051,0x0107,0x0063,0x0165,0x0075,0x01b7,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0000,0x0017,0x0000,0x0069,0x0000,0x0089,0x0000,0x00af,
	0x0000,0x00df,0x0000,0x0115,0x0000,0x0173,0x0000,0x01c5,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0009,0x0015,0x001e,0x0067,0x0029,0x0087,0x0034,0x00ad,
	0x0046,0x00dd,0x0055,0x0113,0x0067,0x0171,0x0079,0x01c3,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0006,0x000e,0x0006,0x000e,0x0026,0x0080,0x0006,0x000e,
	0x0000,0x0000,0x0052,0x010c,0x0064,0x016a,0x0076,0x01bc,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0007,0x0010,0x0007,0x0010,0x0027,0x0082,0x0032,0x00a8,
	0x0000,0x0000,0x0053,0x010e,0x0065,0x016c,0x0077,0x01be,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0000,0x0013,0x0000,0x0013,0x0000,0x0085,0x0000,0x00ab,
	0x0000,0x0000,0x0000,0x0111,0x0000,0x016f,0x0000,0x01c1,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0008,0x0014,0x0008,0x0014,0x0028,0x0086,0x0033,0x00ac,
	0x0000,0x0000,0x0054,0x0112,0x0066,0x0170,0x0078,0x01c2,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0018,0x0000,0x006a,0x0000,0x008a,0x0000,0x00b0,
	0x0000,0x00e0,0x0000,0x0116,0x0000,0x0174,0x0000,0x01c6,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x000c,0x0000,0x0065,0x0000,0x007f,0x0000,0x00a6,
	0x0000,0x00db,0x0000,0x010b,0x0000,0x0169,0x0000,0x01bb,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x000a,0x0019,0x001f,0x006b,0x002a,0x008b,0x0035,0x00b1,
	0x0047,0x00e1,0x0056,0x0117,0x0068,0x0175,0x007a,0x01c7,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x000b,0x0019,0x0020,0x006b,0x002b,0x008b,0x0036,0x00b1,
	0x0048,0x00e1,0x0057,0x0117,0x0069,0x0175,0x007a,0x01c7,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0015,0x003e,0x0015,0x003e,0x0099,0x0095,0x0015,0x003e,
	0x0000,0x0000,0x009e,0x013a,0x006d,0x018f,0x007e,0x01dc,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x000c,0x001d,0x000c,0x001d,0x000c,0x001d,0x000c,0x001d,
	0x0049,0x00e5,0x0058,0x011b,0x006a,0x0179,0x007b,0x01c9,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0000,0x001f,0x0000,0x001f,0x0000,0x001f,0x0000,0x001f,
	0x0000,0x00e7,0x0000,0x011d,0x0000,0x017b,0x0000,0x01cb,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x000d,0x0020,0x0021,0x006f,0x002c,0x008f,0x0037,0x00b5,
	0x004a,0x00e8,0x0059,0x011e,0x006b,0x017c,0x007c,0x01cc,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0022,0x0000,0x0071,0x0000,0x0091,0x0000,0x00b7,
	0x0000,0x00ea,0x0000,0x0120,0x0000,0x017e,0x0000,0x01ce,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0014,0x003b,0x0014,0x003b,0x005d,0x0092,0x0014,0x003b,
	0x0014,0x003b,0x009d,0x0137,0x00a0,0x018c,0x00a1,0x01d9,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0012,0x0036,0x0012,0x0036,0x0012,0x0036,0x0012,0x0036,
	0x008a,0x00e5,0x00be,0x013c,0x006c,0x0184,0x007d,0x01d1,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0013,0x0038,0x0013,0x0038,0x0013,0x0038,0x0013,0x0038,
	0x008a,0x00e5,0x00bf,0x013d,0x006c,0x0184,0x007d,0x01d1,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0038,0x00ba,0x0038,0x00ba,0x0038,0x00ba,0x0038,0x00ba,
	0x0038,0x00ba,0x00a2,0x0128,0x006e,0x019c,0x007f,0x01e7,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x005c,0x014d,0x005c,0x014d,0x005c,0x014d,0x005c,0x014d,
	0x0000,0x0000,0x009c,0x0124,0x0071,0x01a5,0x0082,0x01f0,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x005b,0x014d,0x005b,0x014d,0x005b,0x014d,0x005b,0x014d,
	0x0000,0x0000,0x009b,0x0124,0x0070,0x01a1,0x0081,0x01ec,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0044,0x0000,0x0044,0x0000,0x0044,0x0000,0x0044,
	0x0000,0x00db,0x0000,0x010b,0x0000,0x0169,0x0000,0x01bb,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0018,0x0045,0x0018,0x0045,0x0018,0x0045,0x0018,0x0045,
	0x0044,0x00d7,0x0051,0x0107,0x0063,0x0165,0x0075,0x01b7,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x008b,0x000e,0x008b,0x000e,0x008c,0x0080,0x008b,0x000e,
	0x0000,0x0000,0x008d,0x010c,0x008e,0x016a,0x008f,0x01bc,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0090,0x0010,0x0090,0x0010,0x0091,0x0082,0x0092,0x00a8,
	0x0000,0x0000,0x0093,0x010e,0x0094,0x016c,0x0095,0x01be,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0096,0x003e,0x0096,0x003e,0x009a,0x0095,0x0096,0x003e,
	0x0000,0x0000,0x009f,0x013a,0x0097,0x018f,0x0098,0x01dc,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0049,0x0000,0x0069,0x0000,0x0098,0x0000,0x00af,
	0x0000,0x00df,0x0000,0x0145,0x0000,0x0173,0x0000,0x01c5,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x005e,0x0048,0x005f,0x0068,0x006f,0x0097,0x0080,0x00ae,
	0x00ac,0x00de,0x00ad,0x0144,0x00ae,0x0172,0x00af,0x01c4,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x004b,0x00ed,0x004b,0x00ed,0x004b,0x00ed,0x004b,0x00ed,
	0x004b,0x00ed,0x00b0,0x012b,0x00b6,0x0186,0x00ba,0x01d3,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0087,0x00f2,0x0087,0x00f2,0x0087,0x00f2,0x0087,0x00f2,
	0x0087,0x00f2,0x00b3,0x0130,0x00b8,0x018b,0x00bc,0x01d8,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x00c1,0x0050,0x00c1,0x0050,0x00c1,0x0050,0x00c1,0x0050,
	0x00c1,0x0050,0x00c3,0x0148,0x00c5,0x019a,0x00c7,0x01e5,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x00c2,0x0050,0x00c2,0x0050,0x00c2,0x0050,0x00c2,0x0050,
	0x00c2,0x0050,0x00c4,0x0148,0x00c6,0x019a,0x00c8,0x01e5,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
};

// dynamic, per-level (29), in level order
// PC=078896 : MCU executed command: 0400 0300 00xx - xx = subcmd
static UINT16 bonkadv_mcu_4_00[] = { 0x0000,0x04C0,0x0000,0x0100,0x0600,0x0100 };
static UINT16 bonkadv_mcu_4_02[] = { 0x0000,0x0760,0x0000,0x0100,0x0270,0x0100,0x0320,0x01B0,
                                       0x0460,0x01B0,0x0510,0x0100,0x05E0,0x0100,0x0620,0x0120,
                                       0x08A0,0x0120 };
static UINT16 bonkadv_mcu_4_01[] = { 0x0000,0x0670,0x0000,0x0100,0x0200,0x0100,0x02E0,0x0090,
                                       0x0400,0x0090,0x04E0,0x0100,0x07B0,0x0100 };
static UINT16 bonkadv_mcu_4_05[] = { 0x0000,0x0610,0x0000,0x0100,0x0750,0x0100 };
static UINT16 bonkadv_mcu_4_07[] = { 0x0000,0x03C0,0x0000,0x0100,0x0500,0x0100 };
static UINT16 bonkadv_mcu_4_06[] = { 0x0000,0x05F0,0x0000,0x0100,0x0730,0x0100 };
static UINT16 bonkadv_mcu_4_09[] = { 0x0000,0x05C0,0x0000,0x0100,0x0700,0x0100 };
static UINT16 bonkadv_mcu_4_0D[] = { 0x0000,0x07A0,0x0000,0x0100,0x08E0,0x0100 };
static UINT16 bonkadv_mcu_4_03[] = { 0x0000,0x0920,0x0000,0x0100,0x0A60,0x0100 };
static UINT16 bonkadv_mcu_4_08[] = { 0x0000,0x0730,0x0000,0x0100,0x0870,0x0100 };
static UINT16 bonkadv_mcu_4_04[] = { 0x0000,0x05C0,0x0000,0x0100,0x0200,0x0100,0x0280,0x00C0,
                                       0x02E0,0x0060,0x03A0,0x0000,0x0700,0x0000 };
static UINT16 bonkadv_mcu_4_0C[] = { 0x0000,0x06E0,0x0000,0x0100,0x0820,0x0100 };
static UINT16 bonkadv_mcu_4_0A[] = { 0x0000,0x05A0,0x0000,0x0100,0x06E0,0x0100 };
static UINT16 bonkadv_mcu_4_0B[] = { 0x0000,0x0470,0x0000,0x0100,0x05B0,0x0100 };
static UINT16 bonkadv_mcu_4_10[] = { 0x0000,0x06C0,0x0000,0x0100,0x03B0,0x0100,0x0460,0x0050,
                                       0x05E0,0x0050,0x0650,0xFFE0,0x0800,0xFFE0 };
static UINT16 bonkadv_mcu_4_0E[] = { 0x0000,0x0630,0x0000,0x0100,0x0770,0x0100 };
static UINT16 bonkadv_mcu_4_13[] = { 0x0000,0x05C0,0x0000,0x0100,0x0700,0x0100 };
static UINT16 bonkadv_mcu_4_0F[] = { 0x0000,0x05C0,0x0000,0x0100,0x0700,0x0100 };
static UINT16 bonkadv_mcu_4_11[] = { 0x0000,0x05C0,0x0000,0x0100,0x0700,0x0100 };
static UINT16 bonkadv_mcu_4_14[] = { 0x0000,0x05C0,0x0000,0x0100,0x0700,0x0100 };
static UINT16 bonkadv_mcu_4_12[] = { 0x0000,0x05C0,0x0000,0x0100,0x0700,0x0100 };
static UINT16 bonkadv_mcu_4_17[] = { 0x0000,0x0520,0x0000,0x0100,0x0660,0x0100 };
static UINT16 bonkadv_mcu_4_1A[] = { 0x0000,0x03C0,0x0000,0x0100,0x0500,0x0100 };
static UINT16 bonkadv_mcu_4_15[] = { 0x0000,0x02C0,0x0000,0x0100,0x0400,0x0100 };
static UINT16 bonkadv_mcu_4_18[] = { 0x0000,0x03C0,0x0000,0x0100,0x0500,0x0100 };
static UINT16 bonkadv_mcu_4_16[] = { 0x0000,0x02F0,0x0000,0x0100,0x0430,0x0100 };
static UINT16 bonkadv_mcu_4_19[] = { 0x0000,0x03A0,0x0000,0x0100,0x04E0,0x0100 };
static UINT16 bonkadv_mcu_4_1B[] = { 0x0000,0x0460,0x0000,0x0100,0x05A0,0x0100 };
static UINT16 bonkadv_mcu_4_1C[] = { 0x0000,0x02C0,0x0000,0x0100,0x0400,0x0100 };


/***************************************************************************

    TOYBOX MCU data for Blood Warrior

***************************************************************************/

// --- unknown data ---
static UINT16 bloodwar_mcu_4_01[] = {0x0000}; // Warrior 1
static UINT16 bloodwar_mcu_4_02[] = {0x0000}; // Warrior 2
static UINT16 bloodwar_mcu_4_03[] = {0x0000}; // Warrior 3
static UINT16 bloodwar_mcu_4_04[] = {0x0000}; // Warrior 4
static UINT16 bloodwar_mcu_4_05[] = {0x0000}; // Warrior 5
static UINT16 bloodwar_mcu_4_06[] = {0x0000}; // Warrior 6
static UINT16 bloodwar_mcu_4_07[] = {0x0000}; // Warrior 7
static UINT16 bloodwar_mcu_4_08[] = {0x0000}; // Warrior 8
static UINT16 bloodwar_mcu_4_09[] = {0x0000}; // Warrior 9


// --- palette data ---
//  number of palettes (>=1)
//  palette data follows (each palette is 0x200 bytes long)
//  a negative word will end the palette
static UINT16 bloodwar_mcu_4_0a[] = {0x0001,0x8000}; // Warrior 1 Player 1
static UINT16 bloodwar_mcu_4_0b[] = {0x0001,0x8000}; // Warrior 1 Player 2
static UINT16 bloodwar_mcu_4_0c[] = {0x0001,0x8000}; // Warrior 5 Player 1
static UINT16 bloodwar_mcu_4_0d[] = {0x0001,0x8000}; // Warrior 5 Player 2
static UINT16 bloodwar_mcu_4_0e[] = {0x0001,0x8000}; // Warrior 4 Player 2
static UINT16 bloodwar_mcu_4_0f[] = {0x0001,0x8000}; // Warrior 4 Player 1
static UINT16 bloodwar_mcu_4_10[] = {0x0001,0x8000}; // Warrior 6 Player 1
static UINT16 bloodwar_mcu_4_11[] = {0x0001,0x8000}; // Warrior 6 Player 2
static UINT16 bloodwar_mcu_4_12[] = {0x0001,0x8000}; // Warrior 9 Player 1
static UINT16 bloodwar_mcu_4_13[] = {0x0001,0x8000}; // Warrior 9 Player 2
static UINT16 bloodwar_mcu_4_14[] = {0x0001,0x8000}; // Warrior 7 Player 1
static UINT16 bloodwar_mcu_4_15[] = {0x0001,0x8000}; // Warrior 7 Player 2
static UINT16 bloodwar_mcu_4_16[] = {0x0001,0x8000}; // Warrior 8 Player 1
static UINT16 bloodwar_mcu_4_17[] = {0x0001,0x8000}; // Warrior 8 Player 2
static UINT16 bloodwar_mcu_4_18[] = {0x0001,0x8000}; // Warrior 2 Player 2
static UINT16 bloodwar_mcu_4_19[] = {0x0001,0x8000}; // Warrior 2 Player 1
static UINT16 bloodwar_mcu_4_1a[] = {0x0001,0x8000}; // Warrior 3 Player 1
static UINT16 bloodwar_mcu_4_1b[] = {0x0001,0x8000}; // Warrior 3 Player 2


// --- tilemap data ---
//  tile data (ff means no tiles) followed by routine index
static UINT16 bloodwar_mcu_4_1c[] = {0xff00}; // Warrior 8
static UINT16 bloodwar_mcu_4_1d[] = {0xff00}; // Warrior 2
static UINT16 bloodwar_mcu_4_1e[] = {0xff00}; // Warrior 3
static UINT16 bloodwar_mcu_4_1f[] = {0xff00}; // Warrior 5
static UINT16 bloodwar_mcu_4_20[] = {0xff00}; // Warrior 4
static UINT16 bloodwar_mcu_4_21[] = {0xff00}; // Warrior 6
static UINT16 bloodwar_mcu_4_22[] = {0xff00}; // Warrior 1
static UINT16 bloodwar_mcu_4_23[] = {0xff00}; // Warrior 9
static UINT16 bloodwar_mcu_4_24[] = {0xff00}; // Warrior 7


// --- fighter data: pointers to ROM data ---
static UINT16 bloodwar_mcu_4_25[] = {0x0000,0x0000}; // Warrior 1
static UINT16 bloodwar_mcu_4_26[] = {0x0000,0x0000}; // Warrior 2
static UINT16 bloodwar_mcu_4_27[] = {0x0000,0x0000}; // Warrior 3
static UINT16 bloodwar_mcu_4_28[] = {0x0000,0x0000}; // Warrior 4
static UINT16 bloodwar_mcu_4_29[] = {0x0000,0x0000}; // Warrior 5
static UINT16 bloodwar_mcu_4_2a[] = {0x0000,0x0000}; // Warrior 6
static UINT16 bloodwar_mcu_4_2b[] = {0x0000,0x0000}; // Warrior 7
static UINT16 bloodwar_mcu_4_2c[] = {0x0000,0x0000}; // Warrior 8
static UINT16 bloodwar_mcu_4_2d[] = {0x0000,0x0000}; // Warrior 9

