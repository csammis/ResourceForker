#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NiceStructures.h"
#include <sys/stat.h>
#include <sys/xattr.h>
#include <unistd.h>
#include "Options.h"

#include "dissect/Snd.h"
#include "dissect/Bitmaps.h"
#include "dissect/Text.h"
#include "dissect/Symbols.h"

#include "pef/PEF.h"

#define MKDIR_FLAGS S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH

#define DISSECT_TYPE(x, y) \
    if (strncmp(pMap->resourceTypes[i]->identifier, x, 4) == 0) \
    { \
        printf("Extracting '%s'...", x); \
        mkdir(x, MKDIR_FLAGS); \
        chdir(x); \
        y(pMap->resourceTypes[i]); \
        chdir(".."); \
        printf("...done\n"); \
    }

void ProcessMap(ResourceMap* pMap, ResourceForkerOptions* pOptions);

int main(int argc, char** argv)
{
    ResourceForkerOptions options;
    if (!ReadOptions(&options, argc, argv))
    {
        return 1;
    }

    ResourceMap map;

    printf("Processing file %s\n", options.filename);
    if (options.readRawFile == true)
    {
        FILE* resourceForkInput = fopen(options.filename, "r");
        if (resourceForkInput == NULL)
        {
            printf("! Unable to open '%s'\n", options.filename);
            return 1;
        }
        BuildResourceMap(&map, resourceForkInput);
        fclose(resourceForkInput);

        ProcessMap(&map, &options);
        FreeResourceMap(&map);
    }
    else
    {
        ssize_t attrSize = getxattr(options.filename, "com.apple.ResourceFork", NULL, 0, 0, 0);
        if (attrSize == -1)
        {
            printf("! Could not open extended attribute 'com.apple.ResourceFork' from %s\n", options.filename);
            return 1;
        }

        void* buffer = malloc(attrSize);
        attrSize = getxattr(options.filename, "com.apple.ResourceFork", buffer, attrSize, 0, 0);
        char* tmpFilename = CreateFilename(options.filename, ".tmp");
        FILE* tmp = fopen(tmpFilename, "w");
        fwrite(buffer, attrSize, 1, tmp);
        fclose(tmp);
        free(buffer);

        FILE* resourceForkInput = fopen(tmpFilename, "r");
        if (resourceForkInput == NULL)
        {
            printf("! Unable to open '%s'\n", tmpFilename);
            return 1;
        }
        BuildResourceMap(&map, resourceForkInput);
        fclose(resourceForkInput);
        remove(tmpFilename);

        ProcessMap(&map, &options);
        FreeResourceMap(&map);

        if (options.disassemblePEF)
        {
            FILE* pefInput = fopen(options.filename, "r");
            if (pefInput == NULL)
            {
                printf("! Unable to open '%s'\n", options.filename);
                return 1;
            }
            mkdir("disassembly", MKDIR_FLAGS);
            chdir("disassembly");
            ProcessPEF(pefInput);
            fclose(pefInput);
            chdir("..");
        }
    }

    return 0;
}

void ProcessMap(ResourceMap* pMap, ResourceForkerOptions* pOptions)
{
    if (pOptions->writeBinaryData)
    {
        mkdir("dump", MKDIR_FLAGS);
        chdir("dump");

        for (uint16_t i = 0; i < pMap->resourceTypeCount; i++)
        {
            mkdir(pMap->resourceTypes[i]->identifier, MKDIR_FLAGS);
            chdir(pMap->resourceTypes[i]->identifier);
            for (uint16_t j = 0; j < pMap->resourceTypes[i]->resourceCount; j++)
            {
                Resource* pCurrent = pMap->resourceTypes[i]->resources[j];

                //cstodo still buggy when there are multiple resources with the same type / name
                FILE* writer = fopen(pCurrent->name, "wb");
                fwrite(pCurrent->data, pCurrent->dataSize, 1, writer);
                fclose(writer);

            }
            chdir("..");
        }
    }

    if (pOptions->verbose)
    {
        printf("Reading %u resource types\n", pMap->resourceTypeCount);
        for (uint16_t i = 0; i < pMap->resourceTypeCount; i++)
        {
            printf("Reading %u objects of type '%s'\n", pMap->resourceTypes[i]->resourceCount, pMap->resourceTypes[i]->identifier);

            for (uint16_t j = 0; j < pMap->resourceTypes[i]->resourceCount; j++)
            {
                Resource* pCurrent = pMap->resourceTypes[i]->resources[j];
                printf("  %d: %s is %d bytes\n", j + 1, pCurrent->name, pCurrent->dataSize);
            }
        }
    }

    if (pOptions->extractKnownTypes)
    {
        mkdir("resources", MKDIR_FLAGS);
        chdir("resources");
        for (uint16_t i = 0; i < pMap->resourceTypeCount; i++)
        {
            if (strncmp(pMap->resourceTypes[i]->identifier, "snd ", 4) == 0)
            {
                printf("Extracting 'snd '...");
                mkdir("snd ", MKDIR_FLAGS);
                chdir("snd ");
                DissectSound(pMap->resourceTypes[i], pOptions->verbose);
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
}
