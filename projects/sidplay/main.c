// sidplay/main.c
#include <bent64.h>

/*
*
* Note:
*
* SID files must not use zero-page 02-03! !
*
*/

//
static u8 disk_buffer[256];

//
void vblirq();

//
void vblirq()
{
        asm("inc $d020");
#define C_MUSIC_PLAY 0x1003
        asm("jsr %w", C_MUSIC_PLAY);
        return_irq;
}

//
void main()
{
        u8 i;
        u8 c;
        u8* dl;
        u8* cl = (u8*)0x1000;
        u8* dest;
        u8 n_tr, n_sc;

        k_CLS();
        
	// SID LOADER:
	dl = (u8*)&disk_buffer[0];
	dest = (u8*)0x1000;
	LoadSectorFromDisk(1, 0, dl);
        n_tr = *dl++;
	n_sc = *dl++;
	dl = (dl + 0x7c + 2); // discard sector header and SID header
        for(i = 0; i < 254 - 0x7c - 2; i++) //254 = sector size without header
        {
                *dest++ = *dl++;
        }
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
        
        setup_irq(&vblirq, 0);
        
        // init music
#define C_MUSIC_INIT 0x1000
        asm("lda #0 \
        ldx #0 \
        ldy #0 \
        jsr %w", C_MUSIC_INIT);

        while(1){}
}
