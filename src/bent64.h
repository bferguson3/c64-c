#ifndef BENT64
#define BENT64

// Memory map:
// Entry ~ $800
// Text screen default location, $400
// stack at $cfff (memory top) and grows downward.
//  Kernel requires e0-ff.
// C heap is at the end of program data (so allot some space.)

//
typedef unsigned char u8;
typedef signed char bool;
typedef unsigned int u16;
typedef signed char s8;
typedef signed int s16;

//
void print(const u8* a, u8 len, u8 x, u8 y, u8 color);
void WaitVBLANK();
void save2file(u8 nam[8], \
		u8 ftype, \
		u8* data_start, \
		u8* data_end, \
		u8 drive, \
		u8 f_no);
void SetSpritePosition(u8 sprNo, u16 x, u8 y);
void SetSpritePointer(u8 sprNo, u8 ptr);
void LoadSectorFromDisk(u8 trackNo, u8 secNo, u8* tgt);
void byteToString(u8 byte, u8* arr);
void setup_irq(void* interrupt, u16 scanline);
void LoadDiskFile(u8 n_tr, u8 n_sec, u8* dest);

static unsigned char globalSubA, globalSubB;
static unsigned int globalSubC, globalSubD;

//
#define BLACK 0
#define WHITE 1
#define RED 2
#define CYAN 3
#define VIOLET 4
#define GREEN 5
#define BLUE 6
#define YELLOW 7
#define ORANGE 8
#define BROWN 9
#define LIGHTRED 10
#define DARKGREY 11
#define GREY 12
#define LIGHTGREEN 13
#define LIGHTBLUE 14
#define LIGHTGREY 15

#define TRUE 1
#define FALSE 0
#define True TRUE 
#define true True 
#define False FALSE 
#define false False 

#define COLORRAM (volatile u8*)(0xd800)
#define SCREENRAM (volatile u8*)(0x400)
#define CHARSET_A() asm("lda #21"); \
	asm("sta $d018"); \
	g_charsetMode = 0;
#define CHARSET_B() asm("lda #23"); \
	asm("sta $d018"); \
	g_charsetMode = 1;

#define SETNAM 0xFFBD
#define SETLFS 0xFFBA
#define OPEN 0xFFC0
#define CHKOUT 0xFFC9
#define READST 0xFFB7
#define CLOSE 0xFFC3
#define CLRCHN 0xFFCC 
#define CHKIN 0xffc6
#define CHROUT 0xFFD2 
#define BASIN 0xffcf 
#define BSOUT 0xffd2 
/*
+----+----------------------+-------------------------------------------------------------------------------------------------------+
|    |                      |                                Peek from $dc01 (code in paranthesis):                                 |
|row:| $dc00:               +------------+------------+------------+------------+------------+------------+------------+------------+
|    |                      |   BIT 7    |   BIT 6    |   BIT 5    |   BIT 4    |   BIT 3    |   BIT 2    |   BIT 1    |   BIT 0    |
+----+----------------------+------------+------------+------------+------------+------------+------------+------------+------------+
|1.  | #%11111110 (254/$fe) | DOWN  ($  )|   F5  ($  )|   F3  ($  )|   F1  ($  )|   F7  ($  )| RIGHT ($  )| RETURN($  )|DELETE ($  )|
|2.  | #%11111101 (253/$fd) |LEFT-SH($  )|   e   ($05)|   s   ($13)|   z   ($1a)|   4   ($34)|   a   ($01)|   w   ($17)|   3   ($33)|
|3.  | #%11111011 (251/$fb) |   x   ($18)|   t   ($14)|   f   ($06)|   c   ($03)|   6   ($36)|   d   ($04)|   r   ($12)|   5   ($35)|
|4.  | #%11110111 (247/$f7) |   v   ($16)|   u   ($15)|   h   ($08)|   b   ($02)|   8   ($38)|   g   ($07)|   y   ($19)|   7   ($37)|
|5.  | #%11101111 (239/$ef) |   n   ($0e)|   o   ($0f)|   k   ($0b)|   m   ($0d)|   0   ($30)|   j   ($0a)|   i   ($09)|   9   ($39)|
|6.  | #%11011111 (223/$df) |   ,   ($2c)|   @   ($00)|   :   ($3a)|   .   ($2e)|   -   ($2d)|   l   ($0c)|   p   ($10)|   +   ($2b)|
|7.  | #%10111111 (191/$bf) |   /   ($2f)|   ^   ($1e)|   =   ($3d)|RGHT-SH($  )|  HOME ($  )|   ;   ($3b)|   *   ($2a)|   £   ($1c)|
|8.  | #%01111111 (127/$7f) | STOP  ($  )|   q   ($11)|COMMODR($  )| SPACE ($20)|   2   ($32)|CONTROL($  )|  <-   ($1f)|   1   ($31)|
+----+----------------------+------------+------------+------------+------------+------------+------------+------------+------------+
*/
/* safe keys:
 ,  @ :  .  -  l  p  + 
 /  ^ =  rs hm ;  *  E
 st q cm sp 2  ct <- 1
 *********
 [ ]
 < > 
 space 
 ctrl 
 Q 
 <-
 1 2 
 + - =
 / Rshft
*/
#define DRIVE1 8

#define ENABLE_KERNEL() asm("lda #$36 \
			sta $0001 ");
#define k_CLS() asm("jsr $e544");	
#define return_irq asm("asl $d019 \
			jmp $ea81");
#define ENABLE_SPRITES(n) asm("lda #%b", (u8)n); \
			asm("sta $d015 ");	
#define SET_MCSPRITES(n) asm("lda #%b", (u8)n); \
			asm("sta $d01c");

static u8 globalSubE;
#define SETBORDER(n) globalSubE = n; \
	asm("lda %v", globalSubE); \
	asm("sta $d020");
#define SETBACKGROUND(n) globalSubE = n; \
	asm("lda %v", globalSubE); \
	asm("sta $d021");


// Colors for quickness						 
#define SPRITECOLOR_0(n) asm("lda #%b", (u8)n); \
			asm("sta $d027");
#define SPRITECOLOR_1(n) asm("lda #%b", (u8)n); \
			asm("sta $d028");
#define SPRITECOLOR_2(n) asm("lda #%b", (u8)n); \
			asm("sta $d029");
#define SPRITECOLOR_3(n) asm("lda #%b", (u8)n); \
			asm("sta $d02a");
#define SPRITECOLOR_4(n) asm("lda #%b", (u8)n); \
			asm("sta $d02b");
#define SPRITECOLOR_5(n) asm("lda #%b", (u8)n); \
			asm("sta $d02c");
#define SPRITECOLOR_6(n) asm("lda #%b", (u8)n); \
			asm("sta $d02d");
#define SPRITECOLOR_7(n) asm("lda #%b", (u8)n); \
			asm("sta $d02e");
			
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


void setup_irq(void* interrupt, u16 scanline)
{
	globalSubA = (u16)(interrupt) >> 8;
	globalSubB = (u16)(interrupt) & 0xff;
	asm("sei"); // disable irq flag
	asm("lda %v", globalSubB);
	asm("sta $0314");
	asm("lda %v", globalSubA);
	asm("sta $0315"); // vblank function location
	asm("lda #$7f \
	sta $dc0d \
	sta $dd0d \
	lda #$81 \
	sta $d01a"); // turn off timers and set raster irqs
	asm("lda #$1b \
	sta $d011 ");//<- screen control
	globalSubC = scanline & 0xff;
	/*
	if(scanline > 0xff)
	{
		asm("lda $d011 \
		and #$7f \
		ora #$80 \
		sta $d011");
	}
	else 
	{
		asm("lda $d011 \
		and #$7f \
		sta $d011");
	}
	*/
	asm("lda %v", globalSubC);
	asm("sta $d012"); // IRQ raster loc
	asm("lda $dc0d \
	lda $dd0d \
	asl $d019"); // clear cia flags and irq flag
	asm("cli");
}

void WaitVBLANK()
{
	_vbla:	
	asm("lda $d011\n\
	and #$80\n\
	beq %g",_vbla);
	_vblb:
	asm("lda $d012\n\
	cmp #0\n\
	bne %g", _vblb);
}

void SetSpritePointer(u8 sprNo, u8 ptr)
{
	u8* sp = (u8*)(0x7f8 + sprNo);
	*sp = ptr;
}

void SetSpritePosition(u8 sprNo, u16 x, u8 y)
{
	u8* yl;
	yl = (u8*)(0xd000 + (sprNo * 2)); // X1,Y1,X2,Y2...
	*yl++ = (u8)(x & 0xff);
	*yl = y;
	if(x > 0xff)
	{
		globalSubA = 1 << sprNo;
		asm("lda $d010");
		asm("ora %v", globalSubA);
		asm("sta $d010");
	}
	else
	{
		globalSubA = ~(1 << sprNo);
		asm("lda $d010");
		asm("and %v", globalSubA);
		asm("sta $d010");	
	}
}

//
static u8 g_charsetMode; 
void print(const u8* a, u8 len, u8 x, u8 y, u8 color)
{
	unsigned char* scp;
	u8* cpr;
	u8 i;
	u8 c;
	
	scp = (u8*)(SCREENRAM + x + (y*40));
	cpr = (u8*)(COLORRAM + x + (y*40));
	
	if(g_charsetMode == 0)
	{

	}
	else if(g_charsetMode == 1)
	{
		for(i = 0; i < len; i++) 
		{
			c = a[i];
			if(c >= 0xc0 && c <= 0xdf) c -= 0x80;
			else if(c >= 0x40 && c <= 0x5f) c -= 0x40;
			*scp++ = (u8)c;
			*cpr++ = (u8)color;
		}
	}
}

void byteToString(u8 byte, u8* arr)
{
	u8 t, d, h;
	t = byte;
	d = 0;
	while(t > 9)
	{
		d += 1;
		t -= 10;
	}
	h = 0;
	while(d > 9)
	{
		h += 1;
		d -= 10;
	}
	arr[0] = h + 0x30;
	arr[1] = d + 0x30;
	arr[2] = t + 0x30;
}

// MUST BE LOWERCASE
static u8 fname[] = "@0:filetest,s,w";
// Disk vars
//const u8 a[] = "Writing to disk. Please wait...";
const u8 file_error[] = "Error opening file.";
const u8 write_error[] = "Write error!";
static u8 _errcode;

void save2file(u8 nam[8], 
		u8 ftype, 
		u8* data_start, 
		u8* data_end, 
		u8 drive, 
		u8 f_no) 
{
	u8 c;
	// set filename and file type
	for(c = 3; c < 3+8; c++) fname[c] = nam[c-3];
	fname[12] = ftype;
	
	globalSubC = data_start;
	globalSubD = data_end;
	globalSubA = drive;
	globalSubB = f_no;

	asm("lda #%b", sizeof(fname) - 1); 
	asm("ldx #<%v", fname);
	asm("ldy #>%v", fname); 
	asm("jsr %w", SETNAM); 
	asm("lda %v", globalSubB); 
	asm("ldx $ba");  // device 8
	asm("bne %g", DEFAULTDEVICE);
	asm("ldx %v", globalSubA);
	DEFAULTDEVICE:
	asm("ldy %v", globalSubB); 
	asm("jsr %w", SETLFS); 
	asm("jsr %w", OPEN); 
	asm("bcs %g", ERROR1);
	asm("ldx %v", globalSubB);
	asm("jsr %w", CHKOUT); // set file 2 as output
	asm("lda #<%v", globalSubC);
	asm("sta $ae");
	asm("lda #>%v", globalSubC);
	asm("sta $af \
	ldy #0");
	WRITELOOP:
	asm("jsr %w", READST);
	asm("bne %g", WERROR);
	asm("lda ($ae),y");
	asm("jsr %w", CHROUT);
	asm("inc $ae");
	asm("bne %g", ENDWRITE);
	asm("inc $af");
	ENDWRITE:
	asm("lda $ae");
	asm("cmp #<%v", globalSubD);
	asm("lda $af");
	asm("sbc #>%v", globalSubD);
	asm("bcc %g", WRITELOOP);
	CLOSEF:
	asm("lda %v", globalSubB);
	asm("jsr %w", CLOSE);
	asm("jsr %w", CLRCHN);
	return;
	ERROR1:
	asm("lda %v", globalSubB);
	asm("jsr %w", CLOSE);
	asm("jsr %w", CLRCHN);
	print(&file_error, sizeof(file_error)-1, 0, 5, LIGHTGREY);
	return;
	WERROR:
	asm("sta %v", _errcode);
	asm("lda %v", globalSubB);
	asm("jsr %w", CLOSE);
	asm("jsr %w", CLRCHN);
}

// Uses only DRIVE 1 as FILE 2, memory as FILE 15

const u8 cname = '#';
static u8 uname[] = "u1 2 0 17 00"; // exactly $15000 offset in D64
void LoadSectorFromDisk(u8 trackNo, u8 secNo, u8* tgt)
{
	// track - 1 - 35, sec- 0-21
	const u8 errorString[] = "Error loading!";
	u8 dec[3] = "000";
	byteToString(trackNo, &dec);
	uname[7] = dec[1];
	uname[8] = dec[2];
	byteToString(secNo, &dec);
	uname[10] = dec[1];
	uname[11] = dec[2];
	asm("sei");
	asm("lda #1 \
		ldx #<%v", cname);
	asm("ldy #>%v", cname); // SETNAM open input: 2,8,2
	asm("jsr $ffbd \
		lda #2 \
		ldx #8 \
		ldy #2 \
		jsr $ffba \
		jsr $ffc0 \
		bcs %g", LDERROR); // fail catch
	asm("lda #12");
	asm("ldx #<%v", uname); 
	asm("ldy #>%v", uname); //OPEN the disk on file 15
	asm("jsr $ffbd \
		lda #15 \
		ldx $ba \
		ldy #15 \
		jsr $ffba \
		jsr $ffc0 \
		bcs %g", LDERROR); // fail catch
	globalSubA = (u16)tgt >> 8;
	globalSubB = (u16)tgt & 0xff;
	asm("ldx #2 \
		jsr $ffc6 \
		lda %v", globalSubB); //chkin file 2 for writing
	asm("sta $ae \
		lda %v", globalSubA); // ram write addr
	asm("sta $af \
		ldy #0");
	RDLOOP:
	asm("jsr $ffcf \
		sta ($ae),y \
		iny \
		bne %g", RDLOOP);
	CLOSEFL:				// close both 2 and 15
	asm("lda #15 \
		jsr $ffc3 \
		lda #2 \
		jsr $ffc3 \
		jsr $ffcc"); // clear channels
	asm("cli");
	return;
	LDERROR:
	print(&errorString, sizeof(errorString)-1, 5, 5, RED);
	goto CLOSEFL;
}

static u8 disk_buffer[256];
// Standard load file from disk code 
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


#endif
