#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>

void Indent(int spaces)
{
    for (int i = 0; i < spaces; i++)
    {
        printf(" ");
    }
}

#endif
