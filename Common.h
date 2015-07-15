#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>

#define CR 0x0D
#define LF 0x0A
uint8_t CRLF[2] = {CR, LF};

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
