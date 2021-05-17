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
	$(CC) --target c64 -I$(INCLUDES) $(PROJECT)/main.c -o build/main.s 	# Rel
	$(CL) --listing build/main.lst build/main.s   		# Lst
	$(CL) --target c64 -I$(INCLUDES) $(PROJECT)/main.c -o build/$(APPNAME) 
	mkd64 -m cbmdos -g -o disk.d64 -f build/$(APPNAME) \
		-n $(APPNAME) -w > mkd64_out.txt
	$(EMU) disk.d64 > emulator_out.txt

clean:
	rm -rf build/*
	rm -rf disk.d64
