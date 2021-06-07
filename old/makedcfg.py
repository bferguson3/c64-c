#!/usr/bin/python3
# makedcfg.py
import os,sys
pa = "-m cbmdos -g \n-m separators -g \n-M diskfiles.txt \n-o disk.d64 \
\n-f build/" + sys.argv[2] + " -n " + sys.argv[2] + " -w \n"
out = pa
fs = os.listdir(sys.argv[1] + '/diskfiles')
i = 0
while i < len(fs):
	out += '-f '+ sys.argv[1] +'/diskfiles/' + fs[i] +' -n ' + fs[i] + ' -i 1 -w \n'
	i += 1
out += "-p line -w\n"
f = open(sys.argv[1] + '/makedisk', 'w')
f.write(out)
f.close()