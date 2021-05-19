# byte to string
# testing formula script. ignore me.
import sys 
byte = int(sys.argv[1],16)

t,d,h = 0,0,0
t = byte
d = 0
while t > 9:
	d += 1
	t -= 10
h = 0
while d > 9:
	h += 1
	d -= 10
print(h, d, t)