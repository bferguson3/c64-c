#!/usr/bin/python3
# comprescr.py
import os,sys
coloronly = False
f = open(sys.argv[1], 'r')
ib = f.read()
f.close()
map = []
exec(ib)
if(len(sys.argv) > 2):
        for s in sys.argv:
                if s == '-c':
                        coloronly = True
                else:
                        ofn = sys.argv[2]
else:
        ofn = sys.argv[1]
# RLE scheme:
# 0x11 0xaa ..  
# copy next aa bytes through
# 0x13 0xbb 0xcc  
# copy in bb cc times
# 0x12 0xdd
# copy 0xdd twice
out = []
if not coloronly:
        i = 0
        while i < 0x3e7:
                if(map[i] == map[i+1]):
                        if(map[i] == map[i+2]):
                                # RLE it
                                out.append(0x13)
                                out.append(map[i])
                                j = 1
                                while map[i] == map[i+j]:
                                        j += 1
                                out.append(j)
                                i += j
                        else: # fc 
                                out.append(0x12)
                                out.append(map[i])
                                out.append(map[i])
                                i += 2
                else:
                        j = 0
                        out.append(0x11) 
                        while(map[i+j] != map[i+j+1]):
                                j += 1
                        out.append(j)
                        k = 0 
                        while k < j:
                                out.append(map[i+k])
                                k += 1
                        i += j
else:
        i = 0
        while i < 1000:
                out.append(map[i])
                i += 1
ofs = len(out)
while i < 2000:
        out.append((map[i] << 4) | map[i+1])
        i += 2
bn = sys.argv[1].split('.')[0]
b = 0
if(bn[-3:] != 'rle'):
        bn += '.rle'
f = open(bn, 'wb')
if not coloronly:
        f.write(bytes([ofs & 0xff]))
        f.write(bytes([ofs >> 8]))
while b < len(out):
        print(b, out[b])
        f.write(bytes([out[b]]))
        b += 1
print(str(len(map)) + ' bytes compressed to ' + str(len(out)))

print(bn + ' written OK.')