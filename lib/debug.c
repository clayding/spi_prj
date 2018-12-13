#include <stdio.h>
#include "debug.h"



void dump_debug_log(char *name, void* buf, int len)
{
    unsigned char *ptr = (unsigned char*)buf;
    unsigned char i = 0;
    printf("name:%s",name);
    for(;i < len;i++)
    {
        printf("-%02x",*ptr++);
    }
    printf("\n");
}