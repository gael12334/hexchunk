Copyright (c) 2026 Gaël Fortier

## Hexchunk

A small CLI app to display hex values of a large file in chunks of ~~1024~~ 8192 bytes. I made this as a tool to help me recover a large corrupted archive (and later found out some parts where whiped out).

Available commands: 

- `open <path> `: open a file
- `close` : close the opened file
- `skip <offset>` : step over N bytes in file (new: next with no arguments skips the next 8192 bytes)
- `next <size>` : display the next N bytes in file (new: next with no arguments shows the next 8192 bytes)
- `set <pointer>` : set the current position in file
- `tell` : show the current position in file
- `scan <output-path>` : scans the entire device or file for non-zeroed memory chunks.

## Disclamer

This tool is intended for my own personal use. I will not be responsible for any damage inflicted by a malfunction of my program. You may use it at your own risk and expanses. You may not sell, modify or redistribute this program without my prior consent. While it is impossible for me to enforce it, you may not use my code in a collection of data for the training of any sort of generative algorithm or machine learning (begone, clankers.)
