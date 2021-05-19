// main.c

#include "bent64.h"
#include "gfx.h"

const u8 teststr[] = "Display sprite test";

static u8 JOY1_STATE = 0;
static u8 JOY2_STATE = 0;
#define POLL_INPUT() asm("lda $dc00 \
						 and #$1f \
						 eor #$1f \
						 sta %v", JOY2_STATE); \
					asm("lda $dc01 \
						 and #$1f \
						 eor #$1f \
						 sta %v", JOY1_STATE);
#define JOYUP (1)
#define JOYDOWN (1<<1)
#define JOYLEFT (1<<2)
#define JOYRIGHT (1<<3)
#define JOYBTN (1<<4)

enum pstate { standing, starting, running, jumping, falling };
enum pstate playerState;
enum pfacing { left, right };
enum pfacing playerFacing;

u8 timer_a;
u8 p_frame;
u8 p_speed;
u8 timer_j;
u16 player_x = 100;

u8 p_anm_frames[] = { 0, 1, 2, 3, 4, 3 };
//u8 p_jump_pos[] = { 0, 12, 12+8, 12+8+8, 12+8+8+6, 12+8+8+6+5, 12+8+8+6+5+4, \
//	12+8+8+6+5+4+2, 12+8+8+6+5+4+2+1 };
u8 p_jump_pos[] = { 13, 24, 33, 40, 47, 52, 56, 60, 62 };

void main()
{
	u8 i;
	u8 pw;
	u8 f_o;
	signed char spa_o = 0;
	timer_a = 0;
	timer_j = 0;
	p_frame = 0;
	p_speed = 3;
	playerFacing = right;
	playerState = standing;
	k_CLS()

	CHARSET_B()	
	print(&teststr, sizeof(teststr)-1, 0, 0, LIGHTGREY);
	//save2file("data    ", 's', &DATASTART, &DATASTART + 8, DRIVE1, 2);

	// just 0 and 2
	ENABLE_SPRITES(0b00000011);
	SET_MCSPRITES(0)
	
	SetSpritePointer(0, 128);
	SetSpritePosition(0, 90, 90);
	SPRITECOLOR_0(1);
	
	SetSpritePointer(1, 138);
	SetSpritePosition(1, 90, 90);
	SPRITECOLOR_1(2);

	asm("inc $d020");
	// Set: Only 'safe' kb keys as readable, support joy 1 + 2.
	// JOY1 <-> KB (either or)
	// JOY2 (doesn't matter)
	asm("lda #$e0 \
		 sta $dc02 \
		 lda #0 \
		 sta $dc03 ");
	
	while(1) 
	{
		POLL_INPUT();

		///////////////////////////////////////////////////////
		// PLAYER STATE CONTROL 
		//
		// Turn / run left or right
		if((JOY1_STATE & JOYRIGHT) \
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
		else if((JOY1_STATE & JOYLEFT) \
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
		if(JOY1_STATE & JOYRIGHT) player_x += p_speed;
		else if(JOY1_STATE & JOYLEFT) player_x -= p_speed;
		if ((JOY1_STATE & JOYUP) && (playerState != jumping) && (playerState != falling))
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

		// DRAW ONLY CODE NOW!
		WaitVBLANK();

		// Set player sprites 0 and 1 position + frame
		SetSpritePosition(0, player_x + spa_o, 180 - p_jump_pos[timer_j]);
		SetSpritePosition(1, player_x + spa_o, 180 - p_jump_pos[timer_j]);
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
		// flash screen if button pressed
		if(JOY1_STATE & JOYBTN) asm("inc $d020");
		
		// end of main loop
	}
	// end of main()
}
