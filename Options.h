#ifndef __OPTIONS_H__
#define __OPTIONS_H__

#include <stdbool.h>
#include <stdio.h>

struct ResourceForkerOptions
{
    char* filename;
    bool writeBinaryData;
};

bool ReadOptions(struct ResourceForkerOptions* pOptions, int argc, char** argv)
{
    if (argc < 2)
    {
        printf("Usage: ResourceForker [-w] file\n");
        return false;
    }

    pOptions->writeBinaryData = (strncmp(argv[1], "-w", 2) == 0);
    pOptions->filename = argv[argc - 1];

    return true;
}

#endif
