#CC65PATH=~/Downloads/cc65/bin
CC=cc65
CL=cl65
EMU=x64sc
APPNAME=main
PROJECT=projects/rpj2
INCLUDES=src
RUN=1

default:
	rm -rf disk.d64
	$(CL) -v --target c64 -C c64ben.cfg -I$(INCLUDES) -l build/main.lst \
		-m build/main.map $(PROJECT)/main.c -o build/$(APPNAME) 
	mkd64 -m cbmdos -g -o disk.d64 -f build/$(APPNAME) \
		-n $(APPNAME) -w > mkd64_out.txt
	$(EMU) disk.d64 > emulator_out.txt

clean:
	rm -rf build/*
	rm -rf disk.d64
