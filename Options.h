#ifndef __OPTIONS_H__
#define __OPTIONS_H__

#include <stdbool.h>
#include <stdio.h>

struct ResourceForkerOptions
{
    char* filename;
    bool writeBinaryData;
    bool verbose;
};

bool ReadOptions(struct ResourceForkerOptions* pOptions, int argc, char** argv)
{
    if (argc < 2)
    {
        printf("Usage: ResourceForker [-wv] file\n");
        return false;
    }

    if (strncmp(argv[1], "-", 1) == 0 && argc >= 2)
    {
        int length = strlen(argv[1]);
        pOptions->writeBinaryData = (strnstr(argv[1], "w", length) != NULL);
        pOptions->verbose = (strnstr(argv[1], "v", length) != NULL);
    }

    pOptions->filename = argv[argc - 1];

    return true;
}

#endif
