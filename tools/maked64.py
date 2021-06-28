#!/usr/bin/python3

# maked64.py v0.3

### @retrodevdiscord
### http://discord.gg/Js3uUrc

# Run $ python3 maked64.py without arguments
#  to print the help.

### Issues:
# - Only supports 8 files
# - Does not interleave on the same track
# - All files are of type PRG
###

# Standard d64 format: 174848 byte file
#  256 bytes x 35 tracks, 683 sectors total
#  Tracks numbered 1-35, sectors 0-20, 18, 17, or 16.

# T1-17: 21 sec ea, 357 total, 7820by
# t18-24: 19 sec ea, 133 total, 7170by
# t25-30: 18 sec ea, 108 total, 6300by

# first two bytes are the following TRACK/SECTOR of this file
# then 254 bytes of data 

import math
import sys, os

fn = 'blank'
dn = 'blank.d64'

MAKE_BLANK = 0
ADD_FILE = 1
mode = MAKE_BLANK

# PARSE ARGUMENTS
if(len(sys.argv) == 1):
    print("MakeD64 usage:\n\n\
$ python3 maked64.py -n <diskname>\
\n\t Make a blank d64 file.\
\n\n$ maked64.py -f <diskname> <filename> [-i# | -s# | -t#]\
\n\t Adds filename to diskname with optional parameters interleave,\
\n\tstart sector, start track; where i=(0,99), s=(0,20), t=(1,35) \n\
\t With no argument, new files will be placed at the first available\n\
\tsector with an interleave of 10.\n")
    sys.exit()
if(sys.argv[1] == '-n'):
    if(len(sys.argv) == 2):
        print("Syntax: $ python3 maked64.py -n <diskname>\n")
        sys.exit()
    else:
        fn = sys.argv[2]
if(sys.argv[1] == '-f'):
    if(len(sys.argv) < 4):
        print("Syntax: $ python3 maked64.py -f <diskname> <filename> [-i# | \
-s# | -t#]\n")
        sys.exit()
    else:
        fn = sys.argv[3]
        dn = sys.argv[2]
        i = 4
        interleave = 10 # interleave*256 bytes are skipped every 254* bytes when
        # writing files to the image (*+2 track/sector bytes)
        startsec = -1
        starttrac = 0
        while i < len(sys.argv):
            if(sys.argv[i][:2] == '-i'):
                interleave = int(sys.argv[i][2:4])
            if(sys.argv[i][:2] == '-s'):
                startsec = int(sys.argv[i][2:4])
            if(sys.argv[i][:2] == '-t'):
                starttrac = int(sys.argv[i][2:4])
            i += 1
        starttrac -= 1 # change 1-35 to 0-34!
            
    mode = ADD_FILE


# fill in the sector count array for reference
seccounts = []
s = 0
h = 1
while h <= 17:
    seccounts.append(s)
    s += 21
    h += 1
while h <= 24:
    seccounts.append(s)
    s += 19
    h += 1
while h <= 30:
    seccounts.append(s)
    s += 18
    h += 1
while h <= 35:
    seccounts.append(s)
    s += 17
    h += 1

## Helper function definitions ##
def PrintBAM(dat):
    # 0x165004 - 8f: BAM entries
    i = 0x16504
    t = 0
    print('track no', 'free sectors', '   visualization')
    while i < 0x16590:
        frsec = dat[i]
        print(' ',t+1, '\t\t',frsec, end='\t')
        # ff ff 1f
        # bits are read left to right
        dmt = ''
        b = 0
        while b < 8:
            if(dat[i+1] & (1<<b)):
                dmt += '.'
            else:
                dmt += str(b)
            b += 1
        b = 0
        while b < 8:
            if(dat[i+2] & (1<<b)):
                dmt += '.'
            else:
                dmt += str(b)
            b += 1
        if(t < 17):
            se = 4
        elif (t < 24):
            se = 2
        elif (t < 30):
            se = 1
        else:
            se = 0
        b = 0
        while b < se:
            if(dat[i+3] & (1<<b)):
                dmt += '.'
            else:
                dmt += str(b)
            b += 1
        print(dmt)
        t += 1
        i += 4
    return

def GetTrackFromSec(s):
    if(s < 357):
        a = 1 + math.floor(s/21)
    elif(s < 490):
        a = 18 + math.floor((s-357)/19)
    elif(s < 598):
        a = 25 + math.floor((s-490)/18)
    else:
        a = 31 + math.floor((s-598)/17)
    return a

def GetNumSecs(tr):
    if(tr < 18):
        return 21
    elif(tr < 25):
        return 19
    elif(tr < 31):
        return 18
    else:
        return 17

def GetFirstEmpty(startsec = 0):
    global ds
    tr_, se_ = 0, 0
    r = startsec * 256
    while r < len(ds):
        if(ds[r] == 0):
            if(ds[r+1] == 0):
                # is blank
                se_ = r/256
                r = len(ds)
        r += 256
    tr_ = GetTrackFromSec(se_) - 1
    if(tr_ > 0):
        se_ = se_ - GetNumSecs(tr_) 
    return int(tr_), int(se_)

def GetDiskOffset(tr, se):
    o = 0
    if (tr < 17): # 0-17
        o = 0 + (tr*0x1500)
    elif (tr < 24): # 18+
        o = 0x16500 + ((tr-17)*0x1300)
    elif (tr < 30): # 25+
        o = 0x1ea00 + ((tr-24)*0x1200)
    else: # 31+
        o = 0x25600 + ((tr-30)*0x1100)
    return o + (se*256)

def TickBam(ds, s, final=False):
    t=GetTrackFromSec(s)-1 # what track are we on?
    si = s - seccounts[t]
    ofs = GetDiskOffset(t, si) # where in the disk?
    nofs = ofs + ((interleave)*256) # where is our next sector for this file
    i = 0
    while (nofs/256) > seccounts[i]:
        i += 1
    # next track found
    if not final:
        #ds[ofs] = i # so set it on the sector...
        nsi = int((nofs/256)) - seccounts[i-1] # get the next
        if(nsi >= GetNumSecs(i)): # sector
            nsi -= GetNumSecs(i) # of the target track
        ds[ofs+1] = nsi  # and set that on the sector header as well!
        ds[ofs] = int(GetTrackFromSec(nofs/256))
    else: #last sector, so
        ds[ofs] = 0
        rem = len(infile)%254
        ds[ofs+1] = rem+1 
    # subtract one from available sectors in this track 
    ds[0x16504 + (t*4)] -= 1
    if(si < 8):
        ds[0x16504 + (t*4) + 1] ^= (1 << si) # sectors 0-7
    elif(si < 16):
        si -= 8
        ds[0x16504 + (t*4) + 2] ^= (1 << si) # sectors 8-15
    else:
        si -= 16
        ds[0x16504 + (t*4) + 3] ^= (1 << (si)) # sectors 16-20
    return ds


######################################
###### MAKE BLANK
######################################
if mode == MAKE_BLANK:
    d64data = []
    
    def append_track(starttrack, endtrack, numsecs):
        global d64data 
        t = starttrack 
        while t <= endtrack:
            s = 0
            tra = []
            while s < numsecs:
                sec = [0,0]#[t, s]
                b = 0
                while b < 254:
                    sec.append(0)
                    b += 1
                tra.append(sec) 
                s += 1
            d64data.append(tra)
            t += 1
    append_track(1, 17, 21)
    append_track(18, 24, 19)
    append_track(25, 30, 18)
    append_track(31, 35, 17)

    # BAM sector (18/0)
    bam = d64data[17][0]
    # end of directory header (for blank disk!):
    d64data[17][1][0] = 0
    d64data[17][1][0] = 0

    bam[1] = 0x1  # should be 18/1!
    bam[2] = 0x41 #'A', original dos mode
    # .4 + 35 * 4 bytes = sector free space
    # 15 ff ff 1f < default
    # 
    b = 4
    while b <= (17*4):
        bam[b] = 0x15
        bam[b+1] = 0xff
        bam[b+2] = 0xff
        bam[b+3] = 0x1f
        b += 4
    while b <= (24*4):
        bam[b] = 19
        bam[b+1] = 0xff
        bam[b+2] = 0xff
        bam[b+3] = 0x7
        b += 4
    while b <= (30*4):
        bam[b] = 18
        bam[b+1] = 0xff
        bam[b+2] = 0xff
        bam[b+3] = 0x3
        b += 4
    while b <= (35*4):
        bam[b] = 17
        bam[b+1] = 0xff
        bam[b+2] = 0xff
        bam[b+3] = 0x1
        b += 4
    # set bam for directory...
    bam[(17*4)+4] = 16
    bam[(17*4)+5] = 0b11101100
    dn = dn.upper().split('.')[0]
    i = 0x90
    while (i-0x90) < len(dn):
        bam[i] = ord(dn[i-0x90])
        i += 1
    while i <= 0xaa:#9f: # disk name padding
        bam[i] = 0xa0
        i += 1
# Don't do this, it breaks stuff #
    #bam[0xa0] = 0xa0 # two blank bytes
    #bam[0xa1] = 0xa0
    #bam[0xa2] = 0x0 # disk ID
    #bam[0xa3] = 0x0
    #bam[0xa4] = 0xa0 # 'usually' a0
    #bam[0xa5] = 0x0
    #bam[0xa6] = 0x2a # DOS type 'A'
#                               #
    i = 0xab
    while i <= 0xff:
        bam[i] = 0x00 
        i += 1

    # write final bytes
    f = open(fn + '.d64', 'wb')
    i = 0
    while i < len(d64data):
        j = 0
        while j < len(d64data[i]):
            k = 0
            while k < len(d64data[i][j]):
                f.write(bytes([d64data[i][j][k]]))
                k += 1
            j += 1
        i += 1
    f.close()
    print(fn + '.d64 created.')

#################################
## ADD FILE
#################################

if mode == ADD_FILE:
    global ds 
    # track 18, sector 1 contains the first
    #  directory listing
    # first two bytes: 
    # IF FIRST FILE IN SECTOR LIST:
    #  track/sector of directory listing part 2
    # IF NOT FIRST FILE:
    #  is just 0 0.
    # if [0] == 00 then its the end of the listing,
    #   and [1] should contain ff.

    # TODO add support for + 8 files...
    f = open(dn, 'rb')
    a = f.read()
    f.close()
    indisk = []
    i = 0
    while i < len(a):
        indisk.append(a[i])
        i += 1
    f = open(fn, 'rb')
    b = f.read()
    infile = []
    f.close()
    i = 0
    while i < len(b):
        infile.append(b[i])
        i += 1
    dir_ofs = 0x16600
    ds = indisk
    # get first empty dir entry
    while(ds[dir_ofs+2] != 0x00):
        dir_ofs += 0x20
    ds[0+dir_ofs] = 0
    ds[1+dir_ofs] = 0xff
    #TODO select file type
    file_type = 0x82 #prg, closed
    ds[2+dir_ofs] = file_type
    # file location
    if(starttrac == -1):
        starttrac, startsec = GetFirstEmpty()
    ds[3+dir_ofs] = starttrac+1
    ds[4+dir_ofs] = startsec
    # 16 char petascii name / a0 pad
    dfn = os.path.basename(fn).split('.')[0].upper()
    i = 5
    while i < len(dfn)+5:
        ds[i+dir_ofs] = ord(dfn[i-5])
        i += 1
    while i <= 0x14:
        ds[i+dir_ofs] = 0xa0
        i += 1
    #15-16 rel only, t/s loc of 1st side/sec block
    #17 rel record lenght
    #18-1d unused except geos
    #1e-1f file size in sectors: (1e) + (1f)*256
    #  filesize is #sec * 254
    ussec = math.ceil(len(infile)/254)
    ds[0x1e+dir_ofs] = ussec & 0xff
    ds[0x1f+dir_ofs] = (ussec & 0xff00) >> 8

    f_ofs = GetDiskOffset(starttrac, startsec)
    print(starttrac, startsec)
    # then, copy in file data @ 
    #          ~ + given interleave
    # interleave broken: adjust headers FIXME
    # ( not really ! interleave is arbitrary!)
    j = 0
    f_start = f_ofs
    f_ofs += 2
    i = 2
    while j < len(infile):
        #print(j, f_ofs+j)
        ds[f_ofs+j] = infile[j]
        i += 1
        if(i > 255):
            f_ofs += (256*(interleave-1)) # skip sectors
            # TODO f_ofs needs to wrap around if its over the limit
            #if((f_ofs+j) > 174848):
            #    f_ofs -= 174848
            f_ofs += 2 # skip header
            i = 2
        j += 1
    print('file offset',hex(f_start))
    print('start track/sector',starttrac,startsec, 'interleave',interleave)
    print('filesize',len(infile), '(' + str(ussec),'sectors)')
    print('name set to: ',fn)

    i = ussec 
    ss = seccounts[starttrac] + startsec 
    while i > 1:
        # for every used sector, tick thru the bam
        ds = TickBam(ds, ss) # and write the track headers
        ss += interleave
        #if((ss > 650)):
        #    ss -= 650
        #    print(ss)
        i -= 1
    ds = TickBam(ds, ss, True) #final sector 
    
    PrintBAM(ds)

    # then write the file!
    f = open(dn,'wb')
    b = 0
    while b < len(ds):
        #if(ds[b] == -1): # if the byte conversion fails...?
        #    ds[b] = 0
        f.write(bytes([ds[b]]))
        b += 1
    f.close()
