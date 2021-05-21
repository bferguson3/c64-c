#CC65PATH=~/Downloads/cc65/bin
CC=cc65
CL=cl65
EMU=x64sc
APPNAME=main
PROJECT=projects/rpj2
INCLUDES=src
RUN=$(EMU) disk.d64 > emulator_out.txt
DATAFILE=.

default:
	mkdir -p $(PROJECT)/diskfiles
	mkdir -p build
	rm -rf disk.d64
	$(CL) -v --target c64 \
		  -C c64app.cfg \
		  -I$(INCLUDES) \
		  -l build/main.lst \
		  -m build/main.map \
		  -Ors \
		  $(PROJECT)/main.c -o build/$(APPNAME) 
	python3 tools/makedcfg.py $(PROJECT) $(APPNAME)
	mkd64 -C $(PROJECT)/makedisk > mkd64_out.txt
	$(RUN)

binary:
	$(CL) -v \
		-C datafile.cfg \
		$(DATAFILE)

clean:
	rm -rf build
	rm -rf disk.d64
	rm -rf diskfiles.txt 
	rm -rf emulator_out.txt 
	rm -rf mkd64_out.txt 
	