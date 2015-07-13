#ifndef __OPTIONS_H__
#define __OPTIONS_H__

#include <stdbool.h>
#include <stdio.h>

struct ResourceForkerOptions
{
    char* filename;
    bool writeBinaryData;
    bool verbose;
    bool extractKnownTypes;
};

void PrintUsage()
{
    printf("Usage: ResourceForker -[<flags>] <filename>\n");
    printf("Supported flags:\n");
    printf("\tv: Verbose output\n");
    printf("\td: Dump the individual resources as binary datafiles to the 'dump' subdirectory\n");
    printf("\te: Extract known resource types and write datafiles to the 'resources' subdirectory\n");
    printf("\th, ?: Display usage\n");

    printf("Known types (extracted with -e):\n");
    printf("\t'icl8', extracted as 24bpp uncompressed BMPs\n");
    printf("\t'ICN#', extracted as 24bpp uncompressed BMPs\n");
    printf("\t'snd ', depends on compression.\n");
    printf("\t\tIMA4: extracted as raw signed 16bit PCM with a descriptor file\n");
    printf("\t\tOther: not extracted\n");
    printf("\t'TEXT', extracted with CR-LF line endings\n");
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

        pOptions->writeBinaryData = (strnstr(argv[1], "d", length) != NULL);
        pOptions->verbose = (strnstr(argv[1], "v", length) != NULL);
        pOptions->extractKnownTypes = (strnstr(argv[1], "e", length) != NULL);

        if (pOptions->writeBinaryData == false
                && pOptions->verbose == false
                && pOptions->extractKnownTypes == false)
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
