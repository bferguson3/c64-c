// main.c

#include "bent64.h"

#include "gfx.h"
u16 ii;
u8* dest;
//
void vblirq();
void LoadSID(u8 n_tr, u8 n_sc);
void LoadMap(u8 tr, u8 sec);

//
bool frameFinished = false; // for vsync
static bool oddFrame;

const u8 teststr[] = "loading...";

// mappy stuff
static u8 mapbuffer[2000]; // Dont forget to use this as general purpose RAM!
static u8 collisionmask[1000]; // 1500-1999 decompressed (0-15)

// Player!
#include "player.h"
static s8 playerHealth = 3;

// Enemies!
#include "enemies.h"

void vblirq()
{
	
	u8 j_ofs = 0;
	asm("pha \
	txa \
	pha \
	tya \
	pha");
	
	  SETBORDER(LIGHTRED);

	// Play music!
#define C_MUSIC_PLAY 0x1003
        asm("jsr %w", C_MUSIC_PLAY);

	  SETBORDER(RED);

	// Set player sprites 0 and 1 position
	if(playerState == jumping) player_y -= p_jump_pos[timer_j];
	SetSpritePosition(0, (u16)(player_x + spa_o), \
		(u8)(player_y));
	SetSpritePosition(1, player_x + spa_o, \
		player_y);
#define PLAYER_BASE 128
	// Set the animation frame
	if((playerState != jumping) && (playerState != falling) \
	&& (playerState != reloading))
	{
		SetSpritePointer(0, PLAYER_BASE + p_anm_frames[p_frame] + f_o);
		SetSpritePointer(1, PLAYER_BASE + 10 + p_anm_frames[p_frame] + f_o);
	}
	else if(playerState == reloading)
	{
		if(playerFacing == left)
		{
			j_ofs = 1;
		}
		SetSpritePointer(1, 22 + PLAYER_BASE + j_ofs);
		SetSpritePointer(0, 24 + PLAYER_BASE + j_ofs);
	}
	else 
	{ 
		SetSpritePointer(0, PLAYER_BASE + 4 + f_o);
		SetSpritePointer(1, PLAYER_BASE + 10 + 4 + f_o);
	}
	// Gun muzzle flash
	if(justFired) {
		gunFlashCtr++;
		if(gunFlashCtr == 1)
		{
			ENABLE_SPRITES(0b11111011);
		}
		else if(gunFlashCtr == 2)
		{
			ENABLE_SPRITES(0b11111111);
		}
		else if(gunFlashCtr > 2)
		{
			gunFlashCtr = 0;
			justFired = false;
			SetSpritePosition(2, 0, 0);
		}
	}
	DrawEnemies();
	
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
// Decompress collision mask to collisionmask[]
/*# 4 = >
# 1 = ^
# 2 = <
# 3 = v
# 0 = ' '*/
void LoadMap(u8 tr, u8 sec)
{

	u8* dl;
	u8 n_sc;// = 0;
	u8 n_tr;// = 0;	
	u8 e;
	u8 ey;
	u16 ex;
	u16 o;
	
	enemycount = 0;
		
	dest = (u8*)&mapbuffer;
	LoadDiskFile(tr, sec, (u8*)dest); // map1.rle @ 1,1
	// Load in uncompressed map
	ii = 0;
	dl = SCREENRAM;
	for(; ii < 1000; ii++)
	{
		*dl++ = mapbuffer[ii];
	}
	// Color the screen
	dl = COLORRAM;
	for(; ii < 1500/*(len+500)*/; ii++)
	{
		*dl++ = mapbuffer[ii] >> 4;
		*dl++ = mapbuffer[ii];
	}

	//dl = SCREENRAM;
	dest = (u8*)&collisionmask[0];
	//e_ct = 3;
	for(; ii < 2000; ii++)
	{
		n_sc = mapbuffer[ii];
		if((n_sc == 5) || (n_sc == (5<<4)))
		{	
			o = (ii - 1500) * 2; // 0-499 = 0-998
			ey = (u8)(o / 40) + 2; //17
			ex = (o % 40) + 1;
			e = n_sc;
			if (e > 15) e = (e >> 4);
			enemies[(u8)enemycount].e_type = e;
			enemies[(u8)enemycount].x = (u16)(ex * 8) + 12;
			enemies[(u8)enemycount].y = (u8)(ey * 8) + 21;
			enemies[(u8)enemycount].timer = 10;
			enemies[(u8)enemycount].state = patrolling;
#define SNAKEMONSTER_ID 5
			switch(e){
				case SNAKEMONSTER_ID:
					enemies[(u8)enemycount].base_sprite = 154;
					enemies[(u8)enemycount].sprite_id = 154;
				break;
			}
			enemycount++;
			dest+=2; //dl+=2;
		}
		else
		{
			*dest++ = (n_sc >> 4);
			*dest++ = (n_sc & 0xf);
			//*dl++ = (n_sc >> 4);
			//*dl++ = (n_sc & 0xf);
			
		}
	}
}

void UpdateGUI()
{	
	u8 i;
	u8* cl;
	u8* hr;
	//GUI
	//cl = SCREENRAM + (40 * 24) +22;
	hr = (u8*)COLORRAM + (40 * 24) +22;
	if(p_Reloaded) *hr = 10;
	else *hr = 0;
	
}

static const u8 htxt[] = "HELLTH";
static const u8 gtxt[] = "GUNZ";

void main()
{
	u16 ii;
	u8 pw;

	u16* clr;
	u16 len;
	u8 tcp;
	u8 n;
	s8 z;
	
	timer_a = 0;
	timer_j = 2;
	p_frame = 0;
	p_speed = 3;
	playerFacing = right;
	//playerState = standing;
	
	k_CLS();
	SETBACKGROUND(BLACK);
	SETBORDER(BLACK);
	CHARSET_B();	// lowercase and petscii
	print(&teststr, sizeof(teststr)-1, 0, 0, LIGHTGREY); // "loading..."

	// clean mapbuffer
	for(ii = 0; ii < 2000; ii++)
	{
		mapbuffer[ii] = 0x0;
	}

// mac loads the sid last
	LoadSID(1, 2); // sid is at track 1, sector 2

	// on mac, map2 is track 1 sector 1
	LoadMap(1, 1);

	ENABLE_SPRITES(0b11111111);
	n = 0;
	
	SET_MCSPRITES(0b11110000);
	// Hero:
	SetSpritePointer(0, 128);
	SetSpritePosition(0, 90, 90);
	SPRITECOLOR_0(WHITE);
	SetSpritePointer(1, 138);
	SetSpritePosition(1, 90, 90);
	SPRITECOLOR_1(RED);
	
	// WHITE and BLUE MULTICOLOR SPRITE colors , rest green
	SPRITECOLOR_4(LIGHTGREEN);
	SPRITECOLOR_5(LIGHTGREEN);
	SPRITECOLOR_6(LIGHTGREEN);
	SPRITECOLOR_7(LIGHTGREEN);
	asm("lda #1 \
	sta $d025 \
	lda #6 \
	sta $d026 ");

	// Set: Only 'safe' kb keys as readable, support joy 1 + 2.
	// JOY1 <-> KB (either or)
	// JOY2 (doesn't matter)
	asm("lda #$e0 \
	sta $dc02 \
	lda #0 \
	sta $dc03 ");
	
	// enable vblank
	setup_irq(&vblirq, 226);

	// Start music
#define C_MUSIC_INIT 0x1000
        asm("lda #0 \
        ldx #0 \
        ldy #0 \
        jsr %w", C_MUSIC_INIT);

	// set player to falling
	playerState = jumping;
		
	
	while(1) 
	{
	// MAIN GAME LOOP:
		// Do nothing until vblank returns 
		while(!frameFinished)
		{
			// wait until vblank is done
		}
		frameFinished = false;
		
		  SETBORDER(WHITE);
		//////
		if(oddFrame) oddFrame = false;
		else oddFrame = true;

		UpdatePlayerState();
		GunMaintenance();
		POLL_INPUT();
		if(player_x > 319) player_x = 319;
		UpdateEnemyState();
		UpdateGUI();

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

// RLE code that doesn't work for some reason
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