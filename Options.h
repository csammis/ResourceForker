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
    bool readRawFile;
    bool disassemblePEF;
};

void PrintUsage()
{
    printf("Usage: ResourceForker -[<flags>] <filename>\n");
    printf("Supported flags:\n");
    printf("\tv: Verbose output\n");
    printf("\td: Dump the individual resources as binary datafiles to the 'dump' subdirectory\n");
    printf("\te: Extract known resource types and write datafiles to the 'resources' subdirectory\n");
    printf("\tr: <filename> is the raw content of a resource fork (see Resource Forks)\n");
    printf("\tp: Disassemble a Preferred Executable File\n");
    printf("\th, ?: Display usage\n");

    printf("Known resource types (extracted with -e):\n");
    printf("\t'icl8', extracted as 24bpp uncompressed BMPs\n");
    printf("\t'ICN#', extracted as 24bpp uncompressed BMPs\n");
    printf("\t'snd ', depends on compression.\n");
    printf("\t\tIMA4: extracted as raw signed 16bit PCM with a descriptor file\n");
    printf("\t\tOther: not extracted\n");
    printf("\t'STR', 'STR#', extracted with CR-LF line endings\n");
    printf("\t'TEXT', extracted with CR-LF line endings\n");
    printf("\t'NAME', 'DLL#', extracted as lists with one signature per line\n");

    printf("Input file options:\n");
    printf("\tThe -r flag is used when <filename> is a raw binary file representing the resource fork of an application.\n");
    printf("\tWhen -r is not used, ResourceForker will try to read <filename>'s 'com.apple.ResourceFork' extended attribute and analyze that instead.\n");
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
        pOptions->readRawFile = (strnstr(argv[1], "r", length) != NULL);
        pOptions->disassemblePEF = (strnstr(argv[1], "p", length) != NULL);

        if (pOptions->writeBinaryData == false
                && pOptions->verbose == false
                && pOptions->extractKnownTypes == false
                && pOptions->readRawFile == false
                && pOptions->disassemblePEF == false)
        {
            printf("Unknown flags specified: %s\n", argv[1] + 1);
            return false;
        }

        if (pOptions->readRawFile && pOptions->disassemblePEF)
        {
            printf("-r and -p cannot be used together.\n");
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
