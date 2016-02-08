/*	$OpenBSD: trap.c,v 1.1 2003/09/01 19:45:28 miod Exp $	*/

/*
 * Simple test program to check what happens when userland tries to trap.
 * Written by Miodrag Vallat 2003 AD -- public domain
 */

#include <stdio.h>

#define	TRAP(num) \
	case num: \
		__asm__ __volatile__ ("tb0 0, r0, " __STRING(num) ); \
		break;

int
main(int argc, char *argv[])
{
	int trapnum = 503;	/* software divide by zero */

	if (argc > 1)
		trapnum = atoi(argv[1]);

	switch (trapnum) {
TRAP(0x000)
TRAP(0x001)
TRAP(0x002)
TRAP(0x003)
TRAP(0x004)
TRAP(0x005)
TRAP(0x006)
TRAP(0x007)
TRAP(0x008)
TRAP(0x009)
TRAP(0x00a)
TRAP(0x00b)
TRAP(0x00c)
TRAP(0x00d)
TRAP(0x00e)
TRAP(0x00f)
TRAP(0x010)
TRAP(0x011)
TRAP(0x012)
TRAP(0x013)
TRAP(0x014)
TRAP(0x015)
TRAP(0x016)
TRAP(0x017)
TRAP(0x018)
TRAP(0x019)
TRAP(0x01a)
TRAP(0x01b)
TRAP(0x01c)
TRAP(0x01d)
TRAP(0x01e)
TRAP(0x01f)
TRAP(0x020)
TRAP(0x021)
TRAP(0x022)
TRAP(0x023)
TRAP(0x024)
TRAP(0x025)
TRAP(0x026)
TRAP(0x027)
TRAP(0x028)
TRAP(0x029)
TRAP(0x02a)
TRAP(0x02b)
TRAP(0x02c)
TRAP(0x02d)
TRAP(0x02e)
TRAP(0x02f)
TRAP(0x030)
TRAP(0x031)
TRAP(0x032)
TRAP(0x033)
TRAP(0x034)
TRAP(0x035)
TRAP(0x036)
TRAP(0x037)
TRAP(0x038)
TRAP(0x039)
TRAP(0x03a)
TRAP(0x03b)
TRAP(0x03c)
TRAP(0x03d)
TRAP(0x03e)
TRAP(0x03f)
TRAP(0x040)
TRAP(0x041)
TRAP(0x042)
TRAP(0x043)
TRAP(0x044)
TRAP(0x045)
TRAP(0x046)
TRAP(0x047)
TRAP(0x048)
TRAP(0x049)
TRAP(0x04a)
TRAP(0x04b)
TRAP(0x04c)
TRAP(0x04d)
TRAP(0x04e)
TRAP(0x04f)
TRAP(0x050)
TRAP(0x051)
TRAP(0x052)
TRAP(0x053)
TRAP(0x054)
TRAP(0x055)
TRAP(0x056)
TRAP(0x057)
TRAP(0x058)
TRAP(0x059)
TRAP(0x05a)
TRAP(0x05b)
TRAP(0x05c)
TRAP(0x05d)
TRAP(0x05e)
TRAP(0x05f)
TRAP(0x060)
TRAP(0x061)
TRAP(0x062)
TRAP(0x063)
TRAP(0x064)
TRAP(0x065)
TRAP(0x066)
TRAP(0x067)
TRAP(0x068)
TRAP(0x069)
TRAP(0x06a)
TRAP(0x06b)
TRAP(0x06c)
TRAP(0x06d)
TRAP(0x06e)
TRAP(0x06f)
TRAP(0x070)
TRAP(0x071)
TRAP(0x072)
TRAP(0x073)
TRAP(0x074)
TRAP(0x075)
TRAP(0x076)
TRAP(0x077)
TRAP(0x078)
TRAP(0x079)
TRAP(0x07a)
TRAP(0x07b)
TRAP(0x07c)
TRAP(0x07d)
TRAP(0x07e)
TRAP(0x07f)
TRAP(0x080)
TRAP(0x081)
TRAP(0x082)
TRAP(0x083)
TRAP(0x084)
TRAP(0x085)
TRAP(0x086)
TRAP(0x087)
TRAP(0x088)
TRAP(0x089)
TRAP(0x08a)
TRAP(0x08b)
TRAP(0x08c)
TRAP(0x08d)
TRAP(0x08e)
TRAP(0x08f)
TRAP(0x090)
TRAP(0x091)
TRAP(0x092)
TRAP(0x093)
TRAP(0x094)
TRAP(0x095)
TRAP(0x096)
TRAP(0x097)
TRAP(0x098)
TRAP(0x099)
TRAP(0x09a)
TRAP(0x09b)
TRAP(0x09c)
TRAP(0x09d)
TRAP(0x09e)
TRAP(0x09f)
TRAP(0x0a0)
TRAP(0x0a1)
TRAP(0x0a2)
TRAP(0x0a3)
TRAP(0x0a4)
TRAP(0x0a5)
TRAP(0x0a6)
TRAP(0x0a7)
TRAP(0x0a8)
TRAP(0x0a9)
TRAP(0x0aa)
TRAP(0x0ab)
TRAP(0x0ac)
TRAP(0x0ad)
TRAP(0x0ae)
TRAP(0x0af)
TRAP(0x0b0)
TRAP(0x0b1)
TRAP(0x0b2)
TRAP(0x0b3)
TRAP(0x0b4)
TRAP(0x0b5)
TRAP(0x0b6)
TRAP(0x0b7)
TRAP(0x0b8)
TRAP(0x0b9)
TRAP(0x0ba)
TRAP(0x0bb)
TRAP(0x0bc)
TRAP(0x0bd)
TRAP(0x0be)
TRAP(0x0bf)
TRAP(0x0c0)
TRAP(0x0c1)
TRAP(0x0c2)
TRAP(0x0c3)
TRAP(0x0c4)
TRAP(0x0c5)
TRAP(0x0c6)
TRAP(0x0c7)
TRAP(0x0c8)
TRAP(0x0c9)
TRAP(0x0ca)
TRAP(0x0cb)
TRAP(0x0cc)
TRAP(0x0cd)
TRAP(0x0ce)
TRAP(0x0cf)
TRAP(0x0d0)
TRAP(0x0d1)
TRAP(0x0d2)
TRAP(0x0d3)
TRAP(0x0d4)
TRAP(0x0d5)
TRAP(0x0d6)
TRAP(0x0d7)
TRAP(0x0d8)
TRAP(0x0d9)
TRAP(0x0da)
TRAP(0x0db)
TRAP(0x0dc)
TRAP(0x0dd)
TRAP(0x0de)
TRAP(0x0df)
TRAP(0x0e0)
TRAP(0x0e1)
TRAP(0x0e2)
TRAP(0x0e3)
TRAP(0x0e4)
TRAP(0x0e5)
TRAP(0x0e6)
TRAP(0x0e7)
TRAP(0x0e8)
TRAP(0x0e9)
TRAP(0x0ea)
TRAP(0x0eb)
TRAP(0x0ec)
TRAP(0x0ed)
TRAP(0x0ee)
TRAP(0x0ef)
TRAP(0x0f0)
TRAP(0x0f1)
TRAP(0x0f2)
TRAP(0x0f3)
TRAP(0x0f4)
TRAP(0x0f5)
TRAP(0x0f6)
TRAP(0x0f7)
TRAP(0x0f8)
TRAP(0x0f9)
TRAP(0x0fa)
TRAP(0x0fb)
TRAP(0x0fc)
TRAP(0x0fd)
TRAP(0x0fe)
TRAP(0x0ff)
TRAP(0x100)
TRAP(0x101)
TRAP(0x102)
TRAP(0x103)
TRAP(0x104)
TRAP(0x105)
TRAP(0x106)
TRAP(0x107)
TRAP(0x108)
TRAP(0x109)
TRAP(0x10a)
TRAP(0x10b)
TRAP(0x10c)
TRAP(0x10d)
TRAP(0x10e)
TRAP(0x10f)
TRAP(0x110)
TRAP(0x111)
TRAP(0x112)
TRAP(0x113)
TRAP(0x114)
TRAP(0x115)
TRAP(0x116)
TRAP(0x117)
TRAP(0x118)
TRAP(0x119)
TRAP(0x11a)
TRAP(0x11b)
TRAP(0x11c)
TRAP(0x11d)
TRAP(0x11e)
TRAP(0x11f)
TRAP(0x120)
TRAP(0x121)
TRAP(0x122)
TRAP(0x123)
TRAP(0x124)
TRAP(0x125)
TRAP(0x126)
TRAP(0x127)
TRAP(0x128)
TRAP(0x129)
TRAP(0x12a)
TRAP(0x12b)
TRAP(0x12c)
TRAP(0x12d)
TRAP(0x12e)
TRAP(0x12f)
TRAP(0x130)
TRAP(0x131)
TRAP(0x132)
TRAP(0x133)
TRAP(0x134)
TRAP(0x135)
TRAP(0x136)
TRAP(0x137)
TRAP(0x138)
TRAP(0x139)
TRAP(0x13a)
TRAP(0x13b)
TRAP(0x13c)
TRAP(0x13d)
TRAP(0x13e)
TRAP(0x13f)
TRAP(0x140)
TRAP(0x141)
TRAP(0x142)
TRAP(0x143)
TRAP(0x144)
TRAP(0x145)
TRAP(0x146)
TRAP(0x147)
TRAP(0x148)
TRAP(0x149)
TRAP(0x14a)
TRAP(0x14b)
TRAP(0x14c)
TRAP(0x14d)
TRAP(0x14e)
TRAP(0x14f)
TRAP(0x150)
TRAP(0x151)
TRAP(0x152)
TRAP(0x153)
TRAP(0x154)
TRAP(0x155)
TRAP(0x156)
TRAP(0x157)
TRAP(0x158)
TRAP(0x159)
TRAP(0x15a)
TRAP(0x15b)
TRAP(0x15c)
TRAP(0x15d)
TRAP(0x15e)
TRAP(0x15f)
TRAP(0x160)
TRAP(0x161)
TRAP(0x162)
TRAP(0x163)
TRAP(0x164)
TRAP(0x165)
TRAP(0x166)
TRAP(0x167)
TRAP(0x168)
TRAP(0x169)
TRAP(0x16a)
TRAP(0x16b)
TRAP(0x16c)
TRAP(0x16d)
TRAP(0x16e)
TRAP(0x16f)
TRAP(0x170)
TRAP(0x171)
TRAP(0x172)
TRAP(0x173)
TRAP(0x174)
TRAP(0x175)
TRAP(0x176)
TRAP(0x177)
TRAP(0x178)
TRAP(0x179)
TRAP(0x17a)
TRAP(0x17b)
TRAP(0x17c)
TRAP(0x17d)
TRAP(0x17e)
TRAP(0x17f)
TRAP(0x180)
TRAP(0x181)
TRAP(0x182)
TRAP(0x183)
TRAP(0x184)
TRAP(0x185)
TRAP(0x186)
TRAP(0x187)
TRAP(0x188)
TRAP(0x189)
TRAP(0x18a)
TRAP(0x18b)
TRAP(0x18c)
TRAP(0x18d)
TRAP(0x18e)
TRAP(0x18f)
TRAP(0x190)
TRAP(0x191)
TRAP(0x192)
TRAP(0x193)
TRAP(0x194)
TRAP(0x195)
TRAP(0x196)
TRAP(0x197)
TRAP(0x198)
TRAP(0x199)
TRAP(0x19a)
TRAP(0x19b)
TRAP(0x19c)
TRAP(0x19d)
TRAP(0x19e)
TRAP(0x19f)
TRAP(0x1a0)
TRAP(0x1a1)
TRAP(0x1a2)
TRAP(0x1a3)
TRAP(0x1a4)
TRAP(0x1a5)
TRAP(0x1a6)
TRAP(0x1a7)
TRAP(0x1a8)
TRAP(0x1a9)
TRAP(0x1aa)
TRAP(0x1ab)
TRAP(0x1ac)
TRAP(0x1ad)
TRAP(0x1ae)
TRAP(0x1af)
TRAP(0x1b0)
TRAP(0x1b1)
TRAP(0x1b2)
TRAP(0x1b3)
TRAP(0x1b4)
TRAP(0x1b5)
TRAP(0x1b6)
TRAP(0x1b7)
TRAP(0x1b8)
TRAP(0x1b9)
TRAP(0x1ba)
TRAP(0x1bb)
TRAP(0x1bc)
TRAP(0x1bd)
TRAP(0x1be)
TRAP(0x1bf)
TRAP(0x1c0)
TRAP(0x1c1)
TRAP(0x1c2)
TRAP(0x1c3)
TRAP(0x1c4)
TRAP(0x1c5)
TRAP(0x1c6)
TRAP(0x1c7)
TRAP(0x1c8)
TRAP(0x1c9)
TRAP(0x1ca)
TRAP(0x1cb)
TRAP(0x1cc)
TRAP(0x1cd)
TRAP(0x1ce)
TRAP(0x1cf)
TRAP(0x1d0)
TRAP(0x1d1)
TRAP(0x1d2)
TRAP(0x1d3)
TRAP(0x1d4)
TRAP(0x1d5)
TRAP(0x1d6)
TRAP(0x1d7)
TRAP(0x1d8)
TRAP(0x1d9)
TRAP(0x1da)
TRAP(0x1db)
TRAP(0x1dc)
TRAP(0x1dd)
TRAP(0x1de)
TRAP(0x1df)
TRAP(0x1e0)
TRAP(0x1e1)
TRAP(0x1e2)
TRAP(0x1e3)
TRAP(0x1e4)
TRAP(0x1e5)
TRAP(0x1e6)
TRAP(0x1e7)
TRAP(0x1e8)
TRAP(0x1e9)
TRAP(0x1ea)
TRAP(0x1eb)
TRAP(0x1ec)
TRAP(0x1ed)
TRAP(0x1ee)
TRAP(0x1ef)
TRAP(0x1f0)
TRAP(0x1f1)
TRAP(0x1f2)
TRAP(0x1f3)
TRAP(0x1f4)
TRAP(0x1f5)
TRAP(0x1f6)
TRAP(0x1f7)
TRAP(0x1f8)
TRAP(0x1f9)
TRAP(0x1fa)
TRAP(0x1fb)
TRAP(0x1fc)
TRAP(0x1fd)
TRAP(0x1fe)
TRAP(0x1ff)
	default:
		printf("invalid trap number %x\n", trapnum);
		return 1;
	}

	return 0;
}