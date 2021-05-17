// main.c
// c64 C test!

// Memory map:
// Entry ~ $800
// Text screen default location, $400
// stack at $cfff (memory top) and grows downward.
// C heap is at the end of program data (so allot some space.)

#include "bent64.h"

const u8 DATASTART[] = "ASDFASDF";




void main()
{
	u8 i;
	// Enable kernel @ $e0-$ff
	asm("lda #%b", 0b00110110);
	asm("sta $0001");

	CHARSET_B()	
	print(&a, sizeof(a)-1, 0, 0, LIGHTGREY);
	
	save2file("data    ", 's', &DATASTART, &DATASTART + 8, DRIVE1, 2);
	
	WaitVBLANK();
	asm("inc $d020");
	
	// to draw a tile
	// 

}
