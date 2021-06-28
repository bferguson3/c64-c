// player.h
void UpdatePlayerState();
bool CheckSpriteBottomCollision(u8 spr_no);

enum pstate { standing, starting, running, jumping, falling, reloading };
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

//gun stuff
static bool fireReleased = true; // did we release the joy button?
static bool p_Reloaded = true; // have we reloaded?
// the entire time p_Reloaded is false, we need to be trying to reload. 
static bool justFired = false; // for flash effect
s8 gunFlashCtr = 0; 	// for the flash effect
s8 waitToReload = 15;   // 15 frames until we can reload
bool reloadAnimPlaying = false; // 15 frames of reloading
void FireGun();
void GunMaintenance();


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
	if(playerState == reloading) goto _reloading;
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
				if(!CheckPlayerWallCollision(true)) \
					player_x += p_speed;
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
				if(!CheckPlayerWallCollision(false)) \
					player_x -= p_speed;
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
		if(!CheckSpriteBottomCollision(0)) {
			player_y += p_jump_pos[timer_j];
		}
		else {
			playerState = standing;
		}
		timer_j--;
		if(timer_j == 2) timer_j = 3;
	}
	// add 5 for left facing sprites
_reloading:
	
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


void FireGun()
{
	// set the flash sprite at gun location
	if(playerFacing == right){
		SetSpritePointer(2, 148);
		SetSpritePosition(2, player_x+21, player_y+3);
	}
	else {
		SetSpritePointer(2, 149);
		SetSpritePosition(2, player_x-29, player_y+3);
	}
	SPRITECOLOR_2(WHITE);
	// and toggle the gun states
	justFired = true;
	p_Reloaded = false;
	fireReleased = false;
}

void GunMaintenance()
{
	// Immediately after, animate the flash sprite
	if(!justFired) // then, reload the gun automatically...
	{ 
		if(!p_Reloaded) // if not already reloaded
		{
			if(!reloadAnimPlaying) // and not actively reloading
			{
				if(playerState == standing) 
				{ 	// and we're standing still 
					waitToReload--;
					if(waitToReload < 0) {
						// countdown the reload timer.
						waitToReload = 15;
						reloadAnimPlaying = true;
					}
				}
				else	// but if we aren't still, reset it
					waitToReload = 15;
			}
			else
			{	// only change to reload animation and tick
				// down waitToReload if we are standing still
				if(!(JOY2_STATE & 0b00001111)\
				  && (playerState == standing)) 
				{
					reloadAnimPlaying = true;
					playerState = reloading;
					waitToReload--;
					
				}
				if(playerState == reloading)
				{ // while reloading, we can't move for 15f
					waitToReload--;
					if(waitToReload < 0)
					{
						waitToReload = 15;
						reloadAnimPlaying = false;
						p_Reloaded = true;
						playerState = standing;
					}
				}
			}
		}
	}
	if((JOY2_STATE & JOYBTN) && (fireReleased) && (p_Reloaded))
	{	// if our gun is reloaded
		FireGun();
	}
	if(!(JOY2_STATE & JOYBTN) && (!fireReleased))
	{	// so it won't constantly try to fire if the button is held
		fireReleased = true;
	}
}
