// main.c
// c64 C test!

// Memory map:
// Entry ~ $800
// Text screen default location, $400
// stack at $cfff (memory top) and grows downward.
// C heap is at the end of program data (so allot some space.)

#include "bent64.h"

// C64 uses oddball char map, need to adjust from ascii
const u8 a[] = "Hello World!";

void WaitVBLANK()
{
	_vbla:	
	asm("lda $d011\n\
		and #$80\n\
		beq %g",_vbla);
	_vblb:
	asm("lda $d011\n\
		and #$80\n\
		bne %g", _vblb);
}

void main()
{
	CHARSET_B()	
	print(&a);
	
	while(1)
	{
		WaitVBLANK();
		asm("inc $d020");
	}
}
