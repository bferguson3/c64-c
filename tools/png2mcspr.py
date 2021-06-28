#!/usr/bin/python3
# png2mcspr.py
# Input sprite sheet 12 x [21*numsprites] indexed png
#  COLOR 0 is transparent! Use a different color in the png
#  for sprites that use color BLACK.
# output C byte array, merged by color
#  0 - TRANSPARENT
#  1 - MULTICOLOR 0 (d025) (white)
#  2 - SPRITE COLOR
#  3 - MULTICOLOR 1 (d026) (dark blue) [for rpj2]

# ONLY USE 3 COLOR PALETTE PNG

import numpy,sys,os
from PIL import Image,ImageDraw

# open image 
f = Image.open(sys.argv[1])
px = f.load() 
imsize = f.size
f.close()

bn = os.path.basename(sys.argv[1])
bn = bn.split('.')[0]

outarrs=[]
k = 0
outno = imsize[1]/21
while k < outno:
        outim=[]
        lm = (k+1)*21
        y = k * 21  
        while y < lm:
                x = 0
                orow=[]
                while x < imsize[0]:
                        ob = ''
                        #print(px[x,y], end='')
                        #print(px[x+1,y], end='')
                        #print(px[x+2,y], end='')
                        #print(px[x+3,y], end='')
                        ob = px[x,y] << 6
                        ob += px[x+1,y] << 4
                        ob += px[x+2, y] << 2
                        ob += px[x+3, y]
                        orow.append(ob)
                        x += 4
                #print('', orow)
                h = 0
                while h < 3:
                        outim.append(orow[h])
                        h += 1
                y += 1
        outarrs.append(outim)
        k += 1

h = 0
otstr = ''
while h < len(outarrs):
        otstr += 'const unsigned char ' + bn + 'mc_' + str(h) + '[] = { \\\n\t'
        j = 0
        while j < len(outarrs[h]):
                otstr += str(outarrs[h][j]) + ','
                if (j+1) % 16 == 0:
                        otstr += '\n\t'
                j += 1
        otstr += ' \n};\n'
        h += 1
print(otstr)