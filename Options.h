#ifndef __OPTIONS_H__
#define __OPTIONS_H__

#include <stdbool.h>
#include <stdio.h>

struct ResourceForkerOptions
{
    char* filename;
    bool writeBinaryData;
    bool verbose;
    bool dissectKnownTypes;
};

void PrintUsage()
{
    printf("Usage: ResourceForker -[<flags>] <filename>\n");
    printf("Supported flags:\n");
    printf("\tv: Verbose output\n");
    printf("\tw: Write the individual resources as binary datafiles to the 'resources' subdirectory\n");
    printf("\td: Dissect known resource types and print results to stdout\n");
    printf("\th, ?: Display usage\n");
}

bool ReadOptions(struct ResourceForkerOptions* pOptions, int argc, char** argv)
{
    if (argc < 2)
    {
        PrintUsage();
        return false;
    }

    if (strncmp(argv[1], "-?", 2) == 0 || strncmp(argv[1], "-h", 2) == 0)
    {
        PrintUsage();
        return false;
    }

    if (strncmp(argv[1], "-", 1) == 0 && argc >= 2)
    {
        int length = strlen(argv[1]);

        pOptions->writeBinaryData = (strnstr(argv[1], "w", length) != NULL);
        pOptions->verbose = (strnstr(argv[1], "v", length) != NULL);
        pOptions->dissectKnownTypes = (strnstr(argv[1], "d", length) != NULL);

        if (pOptions->writeBinaryData == false
                && pOptions->verbose == false
                && pOptions->dissectKnownTypes == false)
        {
            printf("Unknown flags specified: %s\n", argv[1] + 1);
            return false;
        }
    }
    else
    {
        printf("At least one flag must be specified.\n");
        return false;
    }

    pOptions->filename = argv[argc - 1];

    return true;
}

#endif
