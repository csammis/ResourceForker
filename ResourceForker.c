// Major thanks for the format documentation from https://github.com/kreativekorp/ksfl/wiki/Macintosh-Resource-File-Format

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NiceStructures.h"
#include <sys/stat.h>
#include <unistd.h>
#include "Options.h"

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
        mkdir("resources", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        chdir("resources");
    }

    if (options.verbose)
    {
        printf("Reading %u resource types\n", map.resourceTypeCount);
    }

    for (uint16_t i = 0; i < map.resourceTypeCount; i++)
    {
        if (options.verbose)
        {
            printf("Reading %u objects of type '%s'\n", map.resourceTypes[i]->resourceCount, map.resourceTypes[i]->identifier);
        }

        if (options.writeBinaryData)
        {
            mkdir(map.resourceTypes[i]->identifier, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            chdir(map.resourceTypes[i]->identifier);
        }

        for (uint16_t j = 0; j < map.resourceTypes[i]->resourceCount; j++)
        {
            struct Resource* pCurrent = map.resourceTypes[i]->resources[j];
            if (options.verbose)
            {
                printf("  %d: %s is %d bytes\n", j + 1, pCurrent->name, pCurrent->dataSize);
            }

            if (options.writeBinaryData)
            {
                //cstodo still buggy when there are multiple resources with the same type / name
                FILE* writer = fopen(pCurrent->name, "wb");
                fwrite(pCurrent->data, pCurrent->dataSize, 1, writer);
                fclose(writer);
            }
        }

        if (options.writeBinaryData)
        {
            chdir("..");
        }
    }

    FreeResourceMap(&map);
    fclose(f);
    return 0;
}
