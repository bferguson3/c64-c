# c64-c

###`make PROJECT=projectdirectory`

Make project. Structure:<br>
```
-root
|-main.c
|-diskfiles/
|-makedisk 
|-...
```
`main.c` - Program start.
`diskfiles/`  - Files here will be added to disk as-is.
This folder is created if it does not exist.
`makedisk` - Config file for mkd64. Generated automatically.<br>
###`make binary DATAFILE=file.c`<br>
Converts a C format read-only data format to a binary file, i.e. to place in diskfiles/ folder.<br>
