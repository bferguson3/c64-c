#ifndef BENT64
#define BENT64

//
typedef unsigned char u8;
typedef signed char bool;
typedef unsigned int u16;

//
void print(const u8* a, u8 len, u8 x, u8 y, u8 color);
void WaitVBLANK();
void save2file(u8 nam[8], \
			   u8 ftype, \
			   u8* data_start, \
			   u8* data_end, \
			   u8 drive, \
			   u8 f_no);

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

#define DRIVE1 8

static unsigned char globalSubA, globalSubB;
static unsigned int globalSubC, globalSubD;

static u8 g_charsetMode; 

//
void print(const u8* a, u8 len, u8 x, u8 y, u8 color)
{
	volatile unsigned char* scp;
	volatile u8* cpr;
	u8 i;
	u8 c;
	scp = SCREENRAM + x + (y*40);
	cpr = COLORRAM + x + (y*40);
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
			*scp++ = c;
			*cpr++ = color;
		}
	}
}

void WaitVBLANK()
{
	_vbla:	
	asm("lda $d011\n\
		and #$80\n\
		beq %g",_vbla);
	_vblb:
	asm("lda $d011\n\
		and #$80\n\
		bne %g", _vblb);
}

// MUST BE LOWERCASE
static u8 fname[] = "@0:filetest,s,w";
// Disk vars
const u8 a[] = "Writing to disk. Please wait...";
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
	asm("sta $af");
	asm("ldy #0");
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

#endif