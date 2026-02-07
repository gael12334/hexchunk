/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <linux/limits.h>

#define CHUNK 1024
#define OK 0
#define ERR_FOPEN 1
#define ERR_FILE_NULL 2
#define ERR_RANGE 3
#define ERR_ARGC 4
#define ERR_CMD 5
#define INPUT_BUFFER 512
#define LINE_LENGTH 16
#define BYTE_LENGTH 3
#define OFFSET_LENGTH 6
#define MAX_POINTERS 10

struct {
    long offset;
    long used;
    uint8_t data[CHUNK];
} chunk = {0};

struct {
    const char* path;
    FILE* file;
    long size;
} program = {0};

int str_is_int(char* str) {
    int valid = 1;
    for(char* pchr = str; *pchr != '\0' && valid == 1; pchr++) {
        valid &= isdigit(*pchr) ? 1 : 0;
    }
    return valid;
}

int close_file(void) {
    if(program.file == NULL) {
        return ERR_FILE_NULL;
    }

    fclose(program.file);
    program.file = NULL;
    program.size = 0;
    return OK;
}

int open_file(const char* path) {
    static char real_path[PATH_MAX];
    close_file();

    realpath(path, real_path);
    program.file = fopen(real_path, "rb");
    if(program.file == NULL) {
        return ERR_FOPEN;
    }

    fseek(program.file, 0, SEEK_END);
    
    program.size = ftell(program.file);
    program.path = path;

    fseek(program.file, 0, SEEK_SET);
    return OK;
}

int read_file(long off, long size) {
    if(program.file == NULL) {
        return ERR_FILE_NULL;
    } else if(off + size > program.size) { 
        return ERR_RANGE;
    } else if(size > CHUNK) {
        size = CHUNK;
    }

    fseek(program.file, off, SEEK_SET);

    chunk.offset = off;
    chunk.used = size;
    for(long i = 0; i < size; i++) {
        chunk.data[i] = fgetc(program.file);
    }

    fseek(program.file, 0, SEEK_SET);
    return OK;
}

void show_chunk(void) {
    const long line_length = 16;

    printf("%-6s ", "");
    for (long i = 0; i < line_length; i++) {
        printf(".%lX ", i);
    }
    
    long row = 0;
    long col = 0;
    long ndx = 0;

    typedef char hexstr[BYTE_LENGTH + 1];
    typedef char chrstr[LINE_LENGTH + 1];

    hexstr hexbuf[LINE_LENGTH];
    chrstr chrbuf;

    while(ndx < chunk.used) {
        printf("\n%06lx ", chunk.offset + ndx); 

        memset(hexbuf, 0, sizeof(hexbuf));
        memset(chrbuf, 0, sizeof(chrbuf));

        for(col = 0; col < LINE_LENGTH; col++) {
            snprintf(hexbuf[col], sizeof(hexbuf[col]), "%02X", chunk.data[ndx]);
            chrbuf[col] = (chunk.data[ndx] >= 0 && chunk.data[ndx] < 32) ? '.' : chunk.data[ndx];
            ndx++;
        }
        
        for(col = 0; col < LINE_LENGTH; col++) {
            printf("%s ", hexbuf[col]);
        }
        printf("%s", chrbuf);
    }
    puts("\n");
}

int cmd_openfile(int argc, char** argv) {
#define CMD "open"
    if(strcmp(argv[0], CMD) != 0) {
        return ERR_CMD;
    }

    if(argc != 2) {
        printf(CMD ": expected 2 arguments.\n");
        return ERR_ARGC;
    }

    int result = open_file(argv[1]);
    if(result == ERR_FOPEN) {
        printf(CMD ": unable to open %s\n", argv[1]);
        return result;
    }

    printf(CMD ": file %s loaded\n", argv[1]);
    return OK;
#undef CMD
}

int cmd_closefile(int argc, char** argv) {
#define CMD "close"
    if(strcmp(argv[0], CMD) != 0) {
        return ERR_CMD;
    }

    int result = close_file();
    if(result == ERR_FILE_NULL) {
        printf(CMD ": no file loaded.\n");
        return result;
    }

    printf(CMD ": file %s unloaded\n", program.path);
    return result;
#undef CMD
}

int cmd_showfile(int argc, char** argv) {
#define CMD "show"
    if(strcmp(argv[0], CMD) != 0) {
        return ERR_CMD;
    }

    if(argc != 3) {
        printf(CMD ": expected 3 arguments.\n");
        return ERR_ARGC;
    }

    int off_valid = str_is_int(argv[1]);
    int size_valid = str_is_int(argv[2]);

    if(!off_valid || !size_valid) {
        printf(CMD ": invalid offset or size.\n");
        return ERR_RANGE;
    }

    long off = strtol(argv[1], NULL, 10);
    long size = strtol(argv[2], NULL, 10);

    int result = read_file(off, size);
    if(result == ERR_FILE_NULL) {
        printf(CMD ": no file loaded.\n");
    }
    else if (result == ERR_RANGE) {
        printf(CMD ": range is invalid.\n");
    }
    else {
        show_chunk();
    }

    return result;
#undef CMD
}

int main(int argc, char** argv) {
    static char buffer[INPUT_BUFFER];
    static char* pointers[MAX_POINTERS];
    
    while(1) {
        printf("command > ");
        char* input = fgets(buffer, INPUT_BUFFER, stdin);
        if(input == NULL) {
            continue;
        }

        int index = strcspn(buffer, "\n\r");
        buffer[index] = '\0';

        int argc = 0;
        char* source[] = {input, NULL};
        char* token = strtok(input, " ");
        while(token != NULL && argc < MAX_POINTERS) {
            pointers[argc] = token;
            token = strtok(NULL, " ");
            argc++;
        }

        if(strcmp(pointers[0], "quit") == 0) 
            break;

        if(cmd_openfile(argc, pointers) == OK)
            continue;
        
        if(cmd_showfile(argc, pointers) == OK)
            continue;
        
        if(cmd_closefile(argc, pointers) == OK)
            continue;

        printf("unknown command\n");
    }

    close_file();
}