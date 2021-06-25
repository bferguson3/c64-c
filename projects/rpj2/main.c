// main.c

#include "bent64.h"
#include "gfx.h"

//
void UpdatePlayerState();
void vblirq();

bool CheckSpriteBottomCollision(u8 spr_no);
void LoadSID(u8 n_tr, u8 n_sc);

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
u8 player_y = 20;
bool jumpReleased = true;
static u8 f_o;
static s8 spa_o = 0;
u8 p_anm_frames[] = { 0, 1, 2, 3, 4, 3 };
//u8 p_jump_pos[] = { 0, 13, 24, 33, 40, 47, 52, 56, 60, 62 }; < px offset
// slowed down by a factor of 2 and reduced by ~5px for a jump height of 6
u8 p_jump_pos[] = { 0, 7, 6, 6, 5, 5, 4, 4, 3, 3, 3, 2, 2, 2, 1, 1, 1 };
//u8 last_tr;

static u8 mapbuffer[2000]; // Dont forget to use this as general purpose RAM!
static u8 collisionmask[1000]; // 1500-1999 decompressed (0-15)

bool frameFinished = false;

void vblirq()
{
	u8 j_ofs = 0;
	asm("pha \
	txa \
	pha \
	tya \
	pha");
	
	  SETBORDER(RED);

	// Play music!
#define C_MUSIC_PLAY 0x1003
        asm("jsr %w", C_MUSIC_PLAY);

	// Set player sprites 0 and 1 position
	if(playerState == jumping) player_y -= p_jump_pos[timer_j];
	SetSpritePosition(0, (u16)(player_x + spa_o), \
		(u8)(player_y));
	SetSpritePosition(1, player_x + spa_o, \
		player_y);

	// Set the animation frame
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
	s8 z;

	timer_a = 0;
	timer_j = 2;
	p_frame = 0;
	p_speed = 3;
	playerFacing = right;
	//playerState = standing;
	
	k_CLS()
	SETBACKGROUND(BLACK);

	CHARSET_B()	
	print(&teststr, sizeof(teststr)-1, 0, 0, LIGHTGREY);

	// clean mapbuffer
	for(ii = 0; ii < 2000; ii++)
	{
		mapbuffer[ii] = 0x0;
	}

	LoadSID(1, 0); // sid is at track 1, sector 0

	dest = (u8*)&mapbuffer;
	LoadDiskFile(1, 1, (u8*)dest); // map1.rle @ 1,1
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
	// Decompress collision mask to collisionmask[]
	dl = SCREENRAM;
	dest = (u8*)&collisionmask[0];
	for(; ii < 2000; ii++)
	{
		n_sc = mapbuffer[ii];
		*dest++ = (n_sc >> 4);
		*dest++ = (n_sc & 0xf);
	/*
	# 4 = >
	# 1 = ^
	# 2 = <
	# 3 = v
	# 0 = ' '
	*/
	}
	// just 0 and 2
	ENABLE_SPRITES(0b00000011);
	SET_MCSPRITES(0)
	// Hero:
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

	// Start music
#define C_MUSIC_INIT 0x1000
        asm("lda #0 \
        ldx #0 \
        ldy #0 \
        jsr %w", C_MUSIC_INIT);

	// set player to falling
	playerState = falling;

	while(1) 
	{
	// MAIN GAME LOOP:
		// Do nothing until vblank returns 
		while(!frameFinished){}
		frameFinished = false;
		
		  SETBORDER(WHITE);
		UpdatePlayerState();	
		POLL_INPUT();
		if(player_x > 319) player_x = 319;
		//if(player_y > 199) player_y = 0;
		
		
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
	LoadSectorFromDisk(1, 0, dl);
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

bool CheckPlayerWallCollision(bool right)
{
	s8 x, y;
	u8 c;
	//u8* cp;
	x = 0; y = 0;
	if(right)
	{
		x = (u8)((player_x + p_speed) >> 3) - 1;
		y = ((player_y - 21) >> 3) - 2;
	}
	else 
	{
		x = (u8)((player_x - 16 - p_speed) >> 3) - 1;
		y = ((player_y - 21) >> 3) - 2;
	}
	if(x < 0) x = 0;
	if(y < 0) y = 0;
	c = collisionmask[(y*40) + x]; //SCREENRAM + (y*40) + x;
	//cp = SCREENRAM + (y*40) + x;
	//*cp = 1;
	if(right)
	{
		if (c != 0) // wall pushes left or up
		{
			player_x = ((x+1) * 8);//1;
			return true;
		}
	}
	else {
		if (c != 0)
		{
			player_x = ((x+4) * 8);
			return true;
		}
	}
	return false;
}

void UpdatePlayerState()
{
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
	// TODO change this set playerState to jumping WHILE its held down
	if ((JOY2_STATE & JOYUP) && (playerState != jumping) \
		&& (playerState != falling) && (jumpReleased == true))
	{
		jumpReleased = false;
		playerState = jumping;
		timer_j = 0;
	}
	if(!(JOY2_STATE & JOYUP) && (jumpReleased == false))
	{
		jumpReleased = true;
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
	// Normal walking speed
	if(JOY2_STATE & JOYRIGHT) 
	{
		if((playerState == falling) || (playerState == jumping)) 
		{
			if(!CheckPlayerWallCollision(true)) player_x += p_speed; 
		}
		else {
			if(!CheckSpriteBottomCollision(0)) playerState = falling;
			else 
			{
				if(!CheckPlayerWallCollision(true)) player_x += p_speed;
			}
		}
	}
	else if(JOY2_STATE & JOYLEFT) 
	{
		if((playerState == falling) || (playerState == jumping)) 
		{
			if(!CheckPlayerWallCollision(false)) player_x -= p_speed; 
		}
		else {
			if(!CheckSpriteBottomCollision(0)) playerState = falling;
			else 
			{
				if(!CheckPlayerWallCollision(false)) player_x -= p_speed;
			}
		}
	}
	
	
	if (playerState == jumping)
	{
		// JUMPING POSITION!
		timer_j ++ ;
		if(timer_j > sizeof(p_jump_pos)) playerState = falling;
	}
	// TODO 
	// continue to 'jump' while the button is held down.
	// if its released, set timer_j to 0,
	// and when falling, set position to y - j_pos_arr
	else if (playerState == falling)
	{
		if(!CheckSpriteBottomCollision(0)) player_y += p_jump_pos[timer_j];
		else playerState = standing;
		timer_j--;
		if(timer_j == 2) timer_j = 3;
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
}

#define FLOOR_COLLISION 1

bool CheckSpriteBottomCollision(u8 spr_no)
{
	u8 y;
	u16 x;
	u8* dr;
	x = *(u8*)(0xd000 + (spr_no*2));
	y = *(u8*)(0xd001 + (spr_no*2)) - 21; // sprites are 24x21 px
	// we only need to check the tile below,
	// +1, and +2. 
	if ( *(u8*)(0xd010) & (1 << spr_no) )
		x += 256;
	x = ((x-8) >> 3) - 1;
	// sprite facing: am I the player? (FIXME)
	y = (y >> 3) - 1;
	if(collisionmask[(y*40)+x] == FLOOR_COLLISION) {
		// am I the player?
		player_y = ((y + 1)*8) + 21;
		return 1;
	}
	x++;
	if(collisionmask[(y*40)+x] == FLOOR_COLLISION) {
		player_y = ((y + 1)*8) + 21;
		return 1;
	}
	else {
		if(playerState != falling) // FIRST FRAME FALLING?
		{
			//so adjust my pos
			player_x = (x + 2)*8;
		}
		return 0;
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