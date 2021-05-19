// main.c
#include "bent64.h"

void main()
{
	LoadSectorFromDisk(17, 0, (u8*)0x8000);

	while(1){}
}

