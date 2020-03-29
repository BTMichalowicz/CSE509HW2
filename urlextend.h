#ifndef __URLEXTEND_H__
#define __URLEXTEND_H__

//Purpose: To include everything so you don't have to!

#include<sys/user.h>
#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<stddef.h>
#include<stdint.h>
#include<inttypes.h>
#include<unistd.h>
#include<fcntl.h>
#include<errno.h>
#include<time.h>
#include<assert.h>
#include<syscall.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<string.h>
#include<sys/mman.h>
#include<sys/ptrace.h>

#define FATAL(...) \
    do{ \
        fprintf(stderr, "urlextend trace: " __VA_ARGS__);\
        fputc('\n', stderr); \
        exit(EXIT_FAILURE); \
    }while (0);
/*Above: for fatal elements*/


typedef unsigned int uint;
typedef unsigned char uchar;

int capture(int, char**);
int bad_approach(int, char**);


#endif
