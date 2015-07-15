// Major thanks for the format documentation from https://github.com/kreativekorp/ksfl/wiki/Macintosh-Resource-File-Format

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NiceStructures.h"
#include <sys/stat.h>
#include <unistd.h>
#include "Options.h"
#include "dissect/Snd.h"
#include "dissect/Bitmaps.h"
#include "dissect/Text.h"
#include "dissect/Symbols.h"

#define MKDIR_FLAGS S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH

#define DISSECT_TYPE(x, y) \
    if (strncmp(map.resourceTypes[i]->identifier, x, 4) == 0) \
    { \
        printf("Extracting '%s'...", x); \
        mkdir(x, MKDIR_FLAGS); \
        chdir(x); \
        y(map.resourceTypes[i]); \
        chdir(".."); \
        printf("...done\n"); \
    }

int main(int argc, char** argv)
{
    struct ResourceForkerOptions options;
    if (!ReadOptions(&options, argc, argv))
    {
        return 1;
    }
    
    FILE* f = fopen(options.filename, "rb");
    if (f == NULL)
    {
        printf("! Unable to open '%s'\n", options.filename);
        return 1;
    }

    printf("Processing file %s\n", options.filename);

    struct ResourceMap map;
    BuildResourceMap(&map, f);

    if (options.writeBinaryData)
    {
        mkdir("dump", MKDIR_FLAGS);
        chdir("dump");

        for (uint16_t i = 0; i < map.resourceTypeCount; i++)
        {
            mkdir(map.resourceTypes[i]->identifier, MKDIR_FLAGS);
            chdir(map.resourceTypes[i]->identifier);
            for (uint16_t j = 0; j < map.resourceTypes[i]->resourceCount; j++)
            {
                struct Resource* pCurrent = map.resourceTypes[i]->resources[j];

                //cstodo still buggy when there are multiple resources with the same type / name
                FILE* writer = fopen(pCurrent->name, "wb");
                fwrite(pCurrent->data, pCurrent->dataSize, 1, writer);
                fclose(writer);

            }
            chdir("..");
        }
    }

    if (options.verbose)
    {
        printf("Reading %u resource types\n", map.resourceTypeCount);
        for (uint16_t i = 0; i < map.resourceTypeCount; i++)
        {
            printf("Reading %u objects of type '%s'\n", map.resourceTypes[i]->resourceCount, map.resourceTypes[i]->identifier);

            for (uint16_t j = 0; j < map.resourceTypes[i]->resourceCount; j++)
            {
                struct Resource* pCurrent = map.resourceTypes[i]->resources[j];
                printf("  %d: %s is %d bytes\n", j + 1, pCurrent->name, pCurrent->dataSize);
            }
        }
    }

    if (options.extractKnownTypes)
    {
        mkdir("resources", MKDIR_FLAGS);
        chdir("resources");
        for (uint16_t i = 0; i < map.resourceTypeCount; i++)
        {
            if (strncmp(map.resourceTypes[i]->identifier, "snd ", 4) == 0)
            {
                printf("Extracting 'snd '...");
                mkdir("snd ", MKDIR_FLAGS);
                chdir("snd ");
                DissectSound(map.resourceTypes[i], options.verbose);
                chdir("..");
                printf("done\n");
            }
            
            DISSECT_TYPE("icl8", DissectIcl8)
            DISSECT_TYPE("ICN#", DissectICN)
            DISSECT_TYPE("STR#", DissectSTRN)
            DISSECT_TYPE("STR ", DissectSTR)
            DISSECT_TYPE("NAME", DissectNAME)
            DISSECT_TYPE("TEXT", DissectTEXT)
            DISSECT_TYPE("dll#", DissectDLLN)
        }
        chdir("..");
    }
    
    FreeResourceMap(&map);
    fclose(f);
    return 0;
}
