import sys 
f = open(sys.argv[1], 'rb')
inby = f.read()
f.close()
# RLE scheme:
# 0x11 0xaa ..  
# copy next aa bytes through
# 0x13 0xbb 0xcc  
# copy in bb cc times
# 0x12 0xdd
# copy 0xdd twice
out = []
i = 2
length = inby[0] + inby[1]*256
while i < length:
        if(inby[i] == 0x11):
                i += 1
                tocp = inby[i]
                i += 1
                while tocp > 0:
                        out.append(inby[i])
                        i += 1
                        tocp -= 1
        if(inby[i] == 0x12):
                i += 1
                tocp = inby[i]
                out.append(tocp)
                out.append(tocp)
        if(inby[i] == 0x13):
                i += 1
                tocp = inby[i]
                i += 1
                n = inby[i]
                while n > 0:
                        out.append(tocp)
                        n -= 1
        i += 1
print(len(out))
while i < len(inby):
        out.append(inby[i] >> 4)
        out.append(inby[i] & 0x0f)
        i += 1
i = 0
while i < len(out):
        print(out[i], end=',')
        i += 1
        if i % 40 == 0:
                print('')
print(len(out))