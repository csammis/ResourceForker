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

char* CreateFilename(const char* original, const char* extension)
{
    uint32_t nameSize = strlen(original);
    uint32_t extSize = strlen(extension);
    char* filename = malloc(nameSize + extSize + 1);
    memset(filename, 0, nameSize + extSize + 1);
    strncpy(filename, original, nameSize);
    strncpy(filename + nameSize, extension, extSize);
    return filename;
}

void ReleaseFilename(char* filename)
{
    free(filename);
}

#endif
