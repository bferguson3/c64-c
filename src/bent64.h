#ifndef BENT64
#define BENT64

//
typedef unsigned char u8;
typedef signed char bool;

//
void print(const u8* a);

//
#define CHARSET_A() asm("lda #21"); \
	asm("sta $d018"); \
	g_charsetMode = 0;
#define CHARSET_B() asm("lda #23"); \
	asm("sta $d018"); \
	g_charsetMode = 1;

//
static u8 g_charsetMode; 

//
void print(const u8* a)
{
	volatile unsigned char* scp;
	u8 i;
	u8 c;
	u8 sz;
	while(a[sz] != 0) sz++;
	scp = (volatile u8*)0x400;
	if(g_charsetMode == 0)
	{

	}
	else if(g_charsetMode == 1)
	{
		for(i = 0; i < sz; i++) 
		{
			c = a[i];
			if(c >= 0xc0 && c <= 0xdf) c -= 0x80;
			else if(c >= 0x40 && c <= 0x5f) c -= 0x40;
			*scp++ = c;
		}
	}
}

#endif