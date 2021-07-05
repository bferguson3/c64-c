// enemies.h

#define ReHigh(s) globalSubA = (1 << (s));\
	asm("lda $d010 \
	ora %v", globalSubA);\
	asm("sta $d010");
#define UnHigh(s) globalSubA = ~(1 << (s));\
	asm("lda $d010 \
	and %v", globalSubA);\
	asm("sta $d010");

void DrawEnemies();

enum enemyState { patrolling, aiming, firing, injured };
// enemy stuff
static struct Enemy {
	u16 x;
	u8 y;
	u8 sprite_id;
	u8 base_sprite;
	u8 e_type;
	s8 health;
	s8 timer; // used for all animation cooldowns
	bool facingRight;
	enum enemyState state;
} enemies[8];

s8 enemycount;


void DrawEnemies()
{
	u8 jj;
	u8 ne;
	u8* v;
	u8* sprlo;
	u8* sprpt;
	/*
	Enemy = {
	u16 x;
	u8 y;
	u8 sprite_id;
	u8 e_type;
	s8 health;
	s8 timer;
	bool facingRight;
	enum enemyState state;
	*/
	SETBORDER(BLUE);

	sprlo = (u8*)0xd008;
	sprpt = (u8*)0x7fc; // sprite 04
	if(oddFrame) ne = 0;
	else {
		ne = 2;
		sprlo += 4;
		sprpt += 2;
	}
	v = (u8*)(&enemies[ne]);	// enemy A
	ne += 4; // enemy sprites are always system #s 4-7
	
	*sprlo++ = *v++; // x
	if(*v++ == 1) { ReHigh(ne); }
	 else { UnHigh(ne); } // x-hi
	*sprlo++ = *v++; // y
	*sprpt++ = *v++; // sprite id
	
	v += sizeof(struct Enemy) - 4; // enemy B this frame
	ne++; // sprite++
	*sprlo++ = *v++; // x
	if(*v++ == 1) { ReHigh(ne); }
	 else { UnHigh(ne); } // x-hi
	*sprlo = *v++; // y
	*sprpt = *v;

	//TODO sprite_id / e_type animation

	SETBORDER(RED);
}


void UpdateEnemyState()
{
	u8 ne, nx, ny;
	u8 jj, jk;
	u16 tgsq;
	u8* sc;
	struct Enemy* en;
	SETBORDER(GREEN);
	if(oddFrame) jj = 4;
	else jj = 6;
	jk = jj + 2;
	for(; jj < jk; jj++)
	{
		ne = jj - 4;
		en = (struct Enemy*)&enemies[ne];
		en->timer--;
		if(en->state == patrolling){
			en->facingRight ? en->x += 2 : en->x -= 2;
			tgsq = en->x >> 3;
			tgsq += (((en->y >> 3)-3) * 40) - 1; //even enough
			if(collisionmask[tgsq] != 1) {//acc for spr height 
				if(en->facingRight)
				{
				en->facingRight = false;
				//en->timer = -1;
				en->sprite_id -= 2; // left comes two frames before
				}
				else {
				en->facingRight = true;
				//en->timer = -1;
				en->sprite_id += 2;
				}
			}
			else 
			{ // patrolling, but grounded, so can I aim?
				ny = player_y + 21;// nx is lowest edge y, ny is highest edge y. within these 2, try to shoot
				if((en->y >= player_y) && (en->y <= ny))
				{
					en->state = aiming;
					en->timer = 15;
					if(player_x > en->x) // jj contains the sprite number 4-7
					{ // the player is to my right?
						en->facingRight = true;
						en->sprite_id = en->base_sprite + 2;
					}
					else
					{ // the player is to the left.
						en->facingRight = false;
						en->sprite_id = en->base_sprite;
					}
				}
			}
		}
		else if (en->state == aiming)
		{
			if(en->timer == 0)
			{
				en->state = firing;
				en->timer = 5;
			}
			else if( ((en->timer < 15) \
			&& (en->timer >= 10))\
			|| (en->timer < 5)){
				switch(jj){
					case(4):
					SPRITECOLOR_4(RED);
					break;
					case(5):
					SPRITECOLOR_5(RED);
					break;
					case(6):
					SPRITECOLOR_6(RED);
					break;
					case(7):
					SPRITECOLOR_7(RED);
					break;
				}
			}else
			{
				switch(jj){
					case(4):
					SPRITECOLOR_4(LIGHTGREEN);
					break;
					case(5):
					SPRITECOLOR_5(LIGHTGREEN);
					break;
					case(6):
					SPRITECOLOR_6(LIGHTGREEN);
					break;
					case(7):
					SPRITECOLOR_7(LIGHTGREEN);
					break;
				}
			
			} 
		} else if (en->state == firing)
		{
			// display a gun flash sprite for 5 frames
			//SPRITEC
			SetSpritePosition(2, en->x, en->y);
		}
		if(en->timer < 0)
		{
			en->timer = 15;
			switch(en->state)
			{
				nx = 0;
				case patrolling:
					en->sprite_id++;
					nx = en->base_sprite + (2 * en->facingRight);
					if(en->sprite_id >= (nx + 2)) \
						en->sprite_id = nx;
					break;
				case firing:
					SetSpritePosition(2, 0, 0);
					en->state = patrolling;
					switch(jj){
						case(4):
						SPRITECOLOR_4(LIGHTGREEN);
						break;
						case(5):
						SPRITECOLOR_5(LIGHTGREEN);
						break;
						case(6):
						SPRITECOLOR_6(LIGHTGREEN);
						break;
						case(7):
						SPRITECOLOR_7(LIGHTGREEN);
						break;
					}
					break;
			}
		}
		//if(en->x > 319) en->x = 319;
	}
	SETBORDER(RED);
}
