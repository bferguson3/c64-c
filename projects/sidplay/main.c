// main.c

#include "bent64.h"

u16 ii;
u8* dest;
//
void vblirq();
void LoadSID(u8 n_tr, u8 n_sc);

//
bool frameFinished = false; // for vsync
bool musicPlaying = false;
static bool oddFrame;


void vblirq()
{
	
	asm("pha \
	txa \
	pha \
	tya \
	pha");
	
	  SETBORDER(LIGHTRED);

	// Play music!
#define C_MUSIC_PLAY 0x1003
        if(musicPlaying) asm("jsr %w", C_MUSIC_PLAY);

	  SETBORDER(BLACK);
	frameFinished = true;
	
	// RETURN FROM INTERRUPT
	asm("pla \
	tay \
	pla \
	tax \
	pla");
	return_irq;
}


void main()
{
	u16 ii;
	u8 pw;

	u16* clr;
	u16 len;
	u8 tcp;
	u8 n;
	s8 z;
	
	k_CLS();
	SETBACKGROUND(BLACK);
	SETBORDER(BLACK);
	CHARSET_B();	// lowercase and petscii
	
	// Set: Only 'safe' kb keys as readable, support joy 1 + 2.
	// JOY1 <-> KB (either or)
	// JOY2 (doesn't matter)
	asm("lda #$e0 \
	sta $dc02 \
	lda #0 \
	sta $dc03 ");
	

        LoadSID(1, 0); 
	setup_irq(&vblirq, 226);
	// Start music
#define C_MUSIC_INIT 0x1000
        asm("lda #0 \
        ldx #0 \
        ldy #0 \
        jsr %w", C_MUSIC_INIT);
	musicPlaying = true;
	
	while(1) 
	{
		// Do nothing until vblank returns 
		while(!frameFinished)
		{
			// wait until vblank is done
		}
		frameFinished = false;
		
		  SETBORDER(WHITE);
		//////

		POLL_INPUT();

		//////
		  SETBORDER(BLACK);
		
	}
	// end of main()
}


void LoadSID(u8 n_tr, u8 n_sc)
{
	u16* clr;
	u8* dl;
	u8 i;
	u8* dest;
	u16 ii;
	// Clear the 4kb of SID RAM
	clr = (u16*)0x1000;
	for(ii = 0; ii < 0x7ff; ii++)
		*clr++ = 0x0;
	// First sector:
	dl = (u8*)&disk_buffer[0];
	dest = (u8*)0x1000;
	LoadSectorFromDisk(n_tr, n_sc, dl);
        n_tr = *dl++;
	n_sc = *dl++;
	dl = (dl + 0x7c + 2); // discard sector header and SID header
        for(i = 0; i < 254 - 0x7c - 2; i++) //254 = sector size without header
        {
                *dest++ = *dl++;
        }
	// Remaining sectors:
        while(n_tr != 0) 
        {
                u8 tb;
		dl = (u8*)&disk_buffer[0];
                LoadSectorFromDisk(n_tr, n_sc, dl);
		n_tr = *dl++;
		n_sc = *dl++;
		if(n_tr == 0) tb = n_sc;
		else tb = 254;
		for(i = 0; i < tb; i++)
                {
                        *dest++ = *dl++;
                }        
        }

}

///////////////////////////////////////////
