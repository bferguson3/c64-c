#CC65PATH=~/Downloads/cc65/bin
CC=cc65
CL=cl65
EMU=x64sc
APPNAME=main
DISKNAME=app
PROJECT=./
INCLUDES=src
RUN=$(EMU) $(DISKNAME).d64 > emulator_out.txt
#EXOPATH=~/Projects/exomizer/src
EXOPATH=~/Projects/c64/exomizer/src
DATAFILE=.
PY=python3
FNO=0

####################################################

default: d64 main d64files
	$(RUN)

main:
	$(CL) -v --target c64 \
		  -C c64app.cfg \
		  -I$(INCLUDES) \
		  -l build/main.lst \
		  -m build/main.map \
		  -Ors \
		  $(PROJECT)/main.c -o build/$(APPNAME)
	$(EXOPATH)/exomizer sfx sys -c -t 64 \
		-o build/main.exo build/main  
	$(PY) tools/maked64.py -f $(DISKNAME).d64 \
		build/$(APPNAME).exo -t19 -s0

d64files: $(PROJECT)/diskfiles/*
	for file in $^ ; do \
		$(PY) tools/maked64.py -f $(DISKNAME).d64 $${file} ;\
	done

d64: 
	mkdir -p $(PROJECT)/diskfiles
	mkdir -p build
	rm -rf *.d64
	$(PY) tools/maked64.py -n $(DISKNAME)

binary:
	$(CL) -v \
		-C datafile.cfg \
		$(DATAFILE)

clean:
	rm -rf build
	rm -rf app.d64
	rm -rf diskfiles.txt 
	rm -rf emulator_out.txt 
	rm -rf mkd64_out.txt 
	
