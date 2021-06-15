// main.c

#include "bent64.h"
#include "gfx.h"

//
void UpdatePlayerState();
void vblirq();
void LoadDiskFile(u8 n_tr, u8 n_sec, u8* dest);
static u8 disk_buffer[256];
//
const u8 teststr[] = "loading";

enum pstate { standing, starting, running, jumping, falling };
enum pstate playerState;
enum pfacing { left, right };
enum pfacing playerFacing;

u8 timer_a;
u8 p_frame;
u8 p_speed;
u8 timer_j;
u16 player_x = 100;
static u8 f_o;
static s8 spa_o = 0;
u8 p_anm_frames[] = { 0, 1, 2, 3, 4, 3 };
u8 p_jump_pos[] = { 13, 24, 33, 40, 47, 52, 56, 60, 62 };
//u8 last_tr;

static u8 mapbuffer[2000];

bool frameFinished = false;

void vblirq()
{
	asm("pha \
	txa \
	pha \
	tya \
	pha");
	
	  SETBORDER(RED);

#define C_MUSIC_PLAY 0x1003
        asm("jsr %w", C_MUSIC_PLAY);

	// Set player sprites 0 and 1 position + frame
	SetSpritePosition(0, (u16)(player_x + spa_o), \
		(u8)(180 - p_jump_pos[timer_j]));
	SetSpritePosition(1, player_x + spa_o, \
		180 - p_jump_pos[timer_j]);
	if((playerState != jumping) && (playerState != falling))
	{
		SetSpritePointer(0, 128 + p_anm_frames[p_frame] + f_o);
		SetSpritePointer(1, 138 + p_anm_frames[p_frame] + f_o);
	}
	else 
	{ 
		SetSpritePointer(0, 128 + 4 + f_o);
		SetSpritePointer(1, 138 + 4 + f_o);
	}
	
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


void LoadSID(u8 n_tr, u8 n_sc)
{
	u16* clr;
	u8* dl;
	u8 i;
	u8* dest;
	u16 ii;
	clr = (u16*)0x1000;
	for(ii = 0; ii < 0x7ff; ii++)
		*clr++ = 0x0;
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

}

void main()
{
	u8 i;
	u8 pw;
	u16 ii;
	u8* dest;
	u8* dl;
	u8* cl;
	u8 n_sc;// = 0;
	u8 n_tr;// = 0;	
	u16* clr;
	u16 len;
	u8 tcp;
	u8 n;

	timer_a = 0;
	timer_j = 0;
	p_frame = 0;
	p_speed = 3;
	playerFacing = right;
	playerState = standing;
	
	k_CLS()
	SETBACKGROUND(BLACK);

	CHARSET_B()	
	print(&teststr, sizeof(teststr)-1, 0, 0, LIGHTGREY);

	// clean mapbuffer
	for(ii = 0; ii < 2000; ii++)
	{
		mapbuffer[ii] = 0x0;
	}


	// SID LOADER:
	LoadSID(1, 0); // sid is at track 1, sector 0
	
	// Standard load looks like this:
	// memory target:
	dest = (u8*)&mapbuffer;
	//n_tr = 1; // file start sector and track
	//n_sc = 1;
	LoadDiskFile(1, 1, (u8*)dest);
	// load bg
	// if uncompressed:
	// 0x0 - 0x3e7 map data
	// 0x3e8 - 0x7cf color data 
	// we compress with comprescr.py
	//  00 - 01 = offset for color table
	//  02 .. RLE-encoded data
	ii = 0;//2;
	dl = SCREENRAM;
	//len = mapbuffer[0] + (mapbuffer[1]*256);
	for(; ii < 1000/*len*/; ii++)
	{
		*dl++ = mapbuffer[ii];
		/*
		if(mapbuffer[ii] == 0x11)
		{
			ii++;
			tcp = mapbuffer[ii];
			ii++;
			while (tcp > 0)
			{
				*dl++ = mapbuffer[ii];
				ii++;
				tcp--;
			}
		}
		else if(mapbuffer[ii] == 0x12)
		{
			ii++;
			tcp = mapbuffer[ii];
			*dl++ = tcp;
			*dl++ = tcp;
		}
		else if(mapbuffer[ii] == 0x13)
		{
			ii++;
			tcp = mapbuffer[ii];
			ii++;
			n = mapbuffer[ii];
			while(n > 0)
			{
				*dl++ = tcp;
				n--;
			}
		}
		*/
	}
	dl = COLORRAM;
	for(; ii < 1500/*(len+500)*/; ii++)
	{
		*dl++ = mapbuffer[ii] >> 4;
		*dl++ = mapbuffer[ii];
	}

	// just 0 and 2
	ENABLE_SPRITES(0b00000011);
	SET_MCSPRITES(0)
	
	SetSpritePointer(0, 128);
	SetSpritePosition(0, 90, 90);
	SPRITECOLOR_0(WHITE);
	
	SetSpritePointer(1, 138);
	SetSpritePosition(1, 90, 90);
	SPRITECOLOR_1(RED);

	// Set: Only 'safe' kb keys as readable, support joy 1 + 2.
	// JOY1 <-> KB (either or)
	// JOY2 (doesn't matter)
	asm("lda #$e0 \
	sta $dc02 \
	lda #0 \
	sta $dc03 ");
	
	// enable vblank
	setup_irq(&vblirq, 0);

#define C_MUSIC_INIT 0x1000
        asm("lda #0 \
        ldx #0 \
        ldy #0 \
        jsr %w", C_MUSIC_INIT);

	while(1) 
	{
		while(!frameFinished){}
		frameFinished = false;
		
		  SETBORDER(WHITE);
		UpdatePlayerState();	
		POLL_INPUT();
		
		  SETBORDER(BLACK);
		
	}
	// end of main()
}

void UpdatePlayerState()
{
///////////////////////////////////////////////////////
// PLAYER STATE CONTROL 
//
	// Turn / run left or right
	if((JOY2_STATE & JOYRIGHT) \
	&& (playerState != jumping)\
	&& (playerState != falling))
	{
		if(playerFacing == left)
		{
			playerFacing = right;
			playerState = starting;
		}
		else playerState = running;	
	}
	else if((JOY2_STATE & JOYLEFT) \
		&& (playerState != jumping)\
		&& (playerState != falling))
	{
		if(playerFacing == right)
		{
			playerFacing = left;
			playerState = starting;
		}
		else playerState = running;
	}
	else
	{	// if not left or right, make standing
		if((playerState != jumping) && (playerState != falling))
		{
			playerState = standing;
			timer_a = 0; 
		}
	}
	if(JOY2_STATE & JOYRIGHT) player_x += p_speed;
	else if(JOY2_STATE & JOYLEFT) player_x -= p_speed;
	if ((JOY2_STATE & JOYUP) && (playerState != jumping) \
		&& (playerState != falling))
	{
		playerState = jumping;
		timer_j = 0;
	}
	// Make sure we start off frame-exact
	if(playerState == running && p_frame == 0) p_frame = 1;
	// animation ticker for running left/right
	if( (playerState == starting) || (playerState == running) )
	{
		timer_a++;
		if(timer_a > 4)
		{
			timer_a = 0;
			p_frame++;
			if(p_frame > 5) p_frame = 2;
		}
	}
	else if (playerState == standing)
	{
		p_frame = 0; // standing=0
	}
	if (playerState == jumping)
	{
		// JUMPING POSITION!
		timer_j ++ ;
		if(timer_j > 7) playerState = falling;
	}
	else if (playerState == falling)
	{
		timer_j --;
		if(timer_j == 0) playerState = standing;
	}
	// add 5 for left facing sprites
	if ( playerFacing == left ) {
		f_o = 5;
		spa_o = -8;
	}
	else {
		f_o = 0;
		spa_o = 0;
	}
//
//////////////////////////////////////////////////////

}

void LoadDiskFile(u8 n_tr, u8 n_sc, u8* dest)
{
	u8* dl;
	u8 i;
	u8 tb;
	tb = 0;
	while(n_tr != 0)
	{
		dl = (u8*)&disk_buffer[0];
                LoadSectorFromDisk(n_tr, n_sc, (u8*)dl); // store it in buffer 1st
		n_tr = *dl++;
		n_sc = *dl++; // header = next track/sector
		if(n_tr == 0) {
			tb = n_sc; }
		else { tb = 254; }
                for(i = 0; i < tb; i++)
                { 	// copy rest from buffer to destination
                        *dest++ = *dl++;
                }  
	}
}