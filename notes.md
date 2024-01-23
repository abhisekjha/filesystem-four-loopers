# File System

Run: `make` then `./file`

## Tasks

- ~~Setup Env - Makefile~~ BG
- Meet and discuss program Task Distribution

### Damber
- ~~encrypt~~
- ~~decrypt~~


### Diwas
Remaining Commands:
- ~~delete~~
- ~~undelete~~
- ~attrib~

### Abhishek
- read
- retrieve

### Bishal
~~createfs~~
~~savefs~~
~~openfs~~
~~closefs~~
~~insert~~
~~list~~
~~df~~
~~quit~~

``` bash
# Notes from Lets code video(s)

createfs + savefs test:
createfs foo.db > savefs > quit = should produce a 67mb file (with whole bunch of zeros)

open:
open foo.db = shouldn't segfault

list:
open foo.db > list = should list 

around 47 min in 2nd vid he scrolls all the way up - slow down maybe

insert works too.

block copy code: starting at line 130 is the retrieve code (get)

delete: set directoryEntry in_use and inode in_use to false
and set the blocks as free, dont 0 em out, you keep in there but you set them to free, 
undel: you look in your directory for the file, set it to be in use, you find the inode that represents that file and for every block that's still defined in the inode structure you mark it in use
```

---

Initial Notes - Bishal (from 4/12/2021 lecture)
4/19/2023

- 64 mb will be the max size of our file system
- print prompt of msh
  - read command from user
    - parse command
    - **insert**
    - **retrieve** - pull file out of disk image to hdd to working directory, another one will renaming ability
    - **read** `read main.c 0 10` - read 10 bytes from main.c starting at byte 0
    - **list** - list all files, `-h` list hidden files, `-a` list file attributes (print out as binary value)
    - **df** - disk space left in image
    - **createfs** - create a new file system, new image
    - **attrib** - set file attributes, `attrib main.c -h` - hide main.c, `attrib main.c +h` - unhide main.c, and `attrib main.c +r` - make main.c read only, `attrib main.c -r` - make main.c read/write
    - **encypit** - xor encryption, 8 bit cipher value will do an xor encryption to a file, 8 bits so byte by byte through the file, [xor test website](https://gchq.github.io/CyberChef/#recipe=XOR(%7B'option':'Hex','string':''%7D,'Standard',false))
    - **quit** 

- use mav shell for the input parsing (TODO)
- example directory - pass in a file on cmd line, it will read from the first file, chop it into fixed size block (block size of 1024) then open a new file and put the blocks back in that file (shows how to chop into blocks and put back together)
- for `read` use write, read, fseek()
