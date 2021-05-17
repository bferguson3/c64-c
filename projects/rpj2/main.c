// main.c

#include "bent64.h"
#include "gfx.h"



const u8 teststr[] = "Display sprite test";


void main()
{
	u8 i;
	u8 pw;
	
	k_CLS()

	CHARSET_B()	
	print(&teststr, sizeof(teststr)-1, 0, 0, LIGHTGREY);
		//save2file("data    ", 's', &DATASTART, &DATASTART + 8, DRIVE1, 2);

	// just 0 and 2
	ENABLE_SPRITES(0b00000101);
	SET_MCSPRITES(0)
	
	SET_SPRITEPTR(0, 128); // sprites start at $2000, so 128+
	SetSpritePosition(0, 90, 90);
	SPRITECOLOR_0(1);
	
	SET_SPRITEPTR(2, 138);
	SetSpritePosition(2, 90, 90);
	SPRITECOLOR_2(2);

	asm("inc $d020");
	
	while(1) 
	{
		for(pw = 0; pw < 10; pw++)
		{
			for(i = 0; i < 10; i++) WaitVBLANK();
			asm("inc $07f8");
			asm("inc $07fa");	
		}
		asm("lda #128 \
			 sta $07f8 \
			 lda #138 \
			 sta $07fa ");
	}

}
