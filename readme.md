Copyright (c) 2026 Gaël Fortier

## Hexchunk

A small CLI app to display hex values of a large file in chunks of 1024 bytes.

- `open <path>` : Opens a file specified by path.
```
command > open /media/user/archive.rar
open: file /media/user/archive.rar loaded
```
- `show <offset> <size>`: Shows a chuck of data, starting at `offset`, spanning `size` bytes. Parameter `size` must not exceed 1024 (maximum length of a chunk).

```
command > show 0 50
       .0 .1 .2 .3 .4 .5 .6 .7 .8 .9 .A .B .C .D .E .F 
000000 52 61 72 21 1A 07 01 00 E7 F0 B4 05 10 01 05 0C Rar!....��.....
000010 00 0B 01 01 92 E5 91 F2 AA 80 80 80 00 5C 02 E5 ....��򪀀�.\.�
000020 89 25 02 03 0B D5 02 04 9E 2C 20 73 84 1F 53 80 �%...�..�, s�.S�
000030 43 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 C...............
```

- `close` : closes the file.
```
command > close
close: file  unloaded
```
- `quit` : quits. 
