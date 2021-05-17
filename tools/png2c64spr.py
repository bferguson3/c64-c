#!/usr/bin/python3
# png2c64spr.py
# Input sprite sheet 24 x [21*numsprites] indexed png
#  COLOR 0 is transparent! Use a different color in the png
#  for sprites that use color BLACK.
# output C byte array, color by color

import numpy,sys,os
from PIL import Image,ImageDraw

# open image 
f = Image.open(sys.argv[1])
px = f.load() 
imsize = f.size
f.close()

bn = os.path.basename(sys.argv[1])
bn = bn.split('.')[0]

colors = []
c = 0
while c < 16:
	colors.append([])
	c += 1

# create the color arrays
y = 0
while y < imsize[1]:
	x = 0
	while x < imsize[0]:
		if(px[x,y] != 0):
			colors[(px[x,y])].append(int('1'))
			c = 0
			while c < 16: #pad all 
				if(c != px[x,y]): #other arrays
					colors[c].append(int('0'))
				c += 1
		else:
			c = 0
			while c < 16: #pad all arrays
				colors[c].append(int('0'))
				c += 1
		x += 1
	y += 1

which_colors=[]
palette = [ 'black', 'white', 'red', 'cyan', 'violet', 'green', 'blue', \
		    'yellow', 'orange', 'brown', 'lightred', 'darkgrey', 'grey', \
		    'lightgreen', 'lightblue', 'lightgrey' ]

# determine which files to output
c = 0
while c < 16:
	i = 0
	f = False
	while i < (len(colors[c])):
		if(colors[c][i] != 0):
			f = True
		i += 1
	if (f != False):
		which_colors.append(c)
	c += 1

# write the string
for c in which_colors:
	outstr = ''
	outstr += 'const unsigned char ' + bn + '_' + palette[c] + '[] = {\n\t'
	i = 0
	while i < len(colors[c]):
		ob = colors[c][i] << 7
		ob |= colors[c][i+1] << 6
		ob |= colors[c][i+2] << 5
		ob |= colors[c][i+3] << 4
		ob |= colors[c][i+4] << 3
		ob |= colors[c][i+5] << 2
		ob |= colors[c][i+6] << 1
		ob |= colors[c][i+7]
		outstr += hex(ob) + ', '
		i += 8
		if(i%(63*8)==0):
			outstr += '0x00\n};\nconst unsigned char ' + bn + '_' + str(i) + palette[c] + '[] = {\n\t'
		if(i%64 == 0):
			outstr += '\n\t'
	outstr += '\n};\n'
	f = open(bn + '_' + palette[c] + '.h', 'w')
	f.write(outstr)
	f.close()
