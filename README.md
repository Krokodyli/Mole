# Mole
Mole is a small 2 week project for a second year university subject Operating Systems. Primary focus of this subject was POSIX API. The project uses various POSIX system calls in implementation of low level IO, asynchronicity, file manipulation, etc. Although resulting program probably has no practical uses, it could be rewritten to complete similiar actions to the command line utility "find".
## Help message
```
----HELP----
Mole - a small program for indexing and searching through directories, JPG, PNG, GZIP and ZIP files.
It loads filesystem tree to a structure in memory storing some informations about files.
It also saves and loads (if it's possible) informations about indexed directories to/from a file,
which allows to quickly resumy querying a big directory with many subdirectories.
Full indexing process can take several minutes, because the type of a file is detected from
a magic number, not from a filename extension, so the process is done in the background allowing
you to execute commands on previous data.

Parameters:

    -d path_d   -    specify the directory you want to index
if path_d is not specified, the program will try to read path from an enviromental variable MOLE_DIR.

    -f path_f   -    specify the file you to save to/load from information about the directory.
if path_f is not specified, the program will try to read path from an enviromental variable MOLE_INDEX_FILE.
If it is also not set, the program will try to load ~/.mole-index

    -t <SEC>    -    if t argument is specified, then every t seconds the program will perform indexing of
path_d directory in the background updating informations for your queries. <SEC> value must be an integer between 30
and 7200.

Commands: 
help             -  shows this message
count            -  shows a table with the number of files of every file type
largerthan <X>   -  shows files larger than <X> bytes
owner <X>        -  shows files with uid equal to <X>
namepart "<X>"   -  shows files whose name contains text <X>
example: namepart ".jpg"
Type \" to escape \ sign
Type \\ to escape \
NOTE: you can combine last 4 command for example "count owner 1000 largerthan 500000"
index            -  starts new indexing in the background
exit             -  waits for indexing to end and exits the program.
exit!            -  interrupts indexing and exits the program.
------------
```
