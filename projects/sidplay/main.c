// sidplay/main.c
#include <bent64.h>

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

        k_CLS();
        // 17,1 taken from diskfiles.txt
        // Default sector interleave is 10 we used 1.
        dl = &disk_buffer[0];//(u8*)0x8000-2-0x7c+6; // f88
        //void LoadSectorFromDisk(u8 trackNo, u8 secNo, u8* tgt);
        LoadSectorFromDisk(1, 0, dl);
        dl = dl + 0x7c + 4; // discard sector header and SID header
        for(i = 0; i < 254 - 0x7c - 2; i++) //254 = sector size without header
        {
                *cl++ = *dl++;
        }
        // Standard load looks like this:
        for(c = 1; c < 7; c++) // c+=1 is interleave of 1
        {
                dl = &disk_buffer[0];
                LoadSectorFromDisk(1, c, dl);
                dl += 2; // discard sector header
                for(i = 0; i < 254; i++)
                {
                        *cl++ = *dl++;
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
