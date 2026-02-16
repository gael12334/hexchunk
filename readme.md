Copyright (c) 2026 Gaël Fortier

## Hexchunk

A small CLI app to display hex values of a large file in chunks of 1024 bytes. I made this as a tool to help me recover a large corrupted archive. 

Available commands: 

- `open <path> `: open a file
- `close` : close the opened file
- `skip <offset>` : step over N bytes in file
- `next <size>` : display the next N bytes in file
- `set <pointer>` : set the current position in file
- `tell` : show the current position in file
- `find <size> <ascii>` : find in the next N bytes an ascii pattern in file

## Disclamer

This tool is intended for my own personal use. I will not be responsible for any damage inflicted by a malfunction of my program. You may use it at your own risk and expanses. You may not sell, modify or redistribute this program without my prior consent. While it is impossible for me to enforce it, you may not use my code in a collection of data for the training of any sort of generative algorithm or machine learning (begone, clankers.)
