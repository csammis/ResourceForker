#ifndef __NICESTRUCTURES_H__
#define __NICESTRUCTURES_H__

#include "RawStructures.h"
#include <stdio.h>

struct Resource
{
    char name[257];
    uint32_t dataSize;
    uint8_t* data;
};

struct ResourceType
{
    char identifier[5];
    uint16_t resourceCount;
    struct Resource** resources;
};

struct ResourceMap
{
    uint16_t resourceTypeCount;
    struct ResourceType** resourceTypes;
};

void BuildResourceMap(struct ResourceMap* pResourceMap, FILE* f)
{
    struct HeaderDefinition header;
    struct ResourceMapDefinition resourceMap;
    struct TypeDefinition** typeList;

    ReadHeaderDefinition(&header, f);
    fseek(f, header.resourceMapOffset, SEEK_SET);
    ReadResourceMapDefinition(&resourceMap, f);
    uint32_t startOfTypeList = header.resourceMapOffset + resourceMap.typeListOffset;
    uint32_t startOfNameList = header.resourceMapOffset + resourceMap.nameListOffset;
    fseek(f, startOfTypeList, SEEK_SET);
    ReadTypeDefinitionList(&typeList, &pResourceMap->resourceTypeCount, f);

    pResourceMap->resourceTypes = malloc(sizeof(struct ResourceType) * pResourceMap->resourceTypeCount);
    for (uint16_t i = 0; i < pResourceMap->resourceTypeCount; i++)
    {
        pResourceMap->resourceTypes[i] = malloc(sizeof(struct ResourceType));
        memset(&pResourceMap->resourceTypes[i]->identifier, 0, 5);
        memcpy(&pResourceMap->resourceTypes[i]->identifier, &typeList[i]->identifier, 4);

        struct ResourceDefinition** resourceDefinitionList;
        fseek(f, startOfTypeList + typeList[i]->resourceListOffset, SEEK_SET);
        ReadResourceDefinitionList(&resourceDefinitionList, typeList[i]->resourceCount, f);

        pResourceMap->resourceTypes[i]->resourceCount = typeList[i]->resourceCount;
        size_t mallocSize = sizeof(struct Resource*) * typeList[i]->resourceCount;
        pResourceMap->resourceTypes[i]->resources = (struct Resource**)malloc(mallocSize);
        struct Resource* currentResource;
        for (uint16_t j = 0; j < typeList[i]->resourceCount; j++)
        {
            pResourceMap->resourceTypes[i]->resources[j] = (struct Resource*)malloc(sizeof(struct Resource));
            currentResource = pResourceMap->resourceTypes[i]->resources[j];
            memset(&currentResource->name, 0, 257);
            if (resourceDefinitionList[j]->nameOffset != 0xFFFF)
            {
                fseek(f, startOfNameList + resourceDefinitionList[j]->nameOffset, SEEK_SET);
                snprintf(currentResource->name, 256, "%d (", resourceDefinitionList[j]->ID);
                uint32_t namelength = strlen(currentResource->name);
                ReadNameFromList(currentResource->name + namelength, 256 - namelength, f);
                namelength = strlen(currentResource->name);
                snprintf(currentResource->name + namelength, 256 - namelength, ")");

            }
            else
            {
                snprintf(currentResource->name, 256, "%d (No Name)", resourceDefinitionList[j]->ID);
            }

            // dataOffset is read unaligned so fix it up
            uint8_t alignBuffer[4];
            memset(alignBuffer, 0, 4);
            memcpy(alignBuffer + 1, resourceDefinitionList[j]->dataOffset, 3);
            uint32_t trueDataOffset = OSReadBigInt32(alignBuffer, 0);

            fseek(f, header.resourceDataOffset + trueDataOffset, SEEK_SET);
            ReadResourceDataDefinition(&currentResource->data, &currentResource->dataSize, f);
        }
    }
}

void FreeResourceMap(struct ResourceMap* pResourceMap)
{
    for (uint16_t i = 0; i < pResourceMap->resourceTypeCount; i++)
    {
        for (uint16_t j = 0; j < pResourceMap->resourceTypes[i]->resourceCount; j++)
        {
            free(pResourceMap->resourceTypes[i]->resources[j]->data);
            free(pResourceMap->resourceTypes[i]->resources[j]);
        }
        free(pResourceMap->resourceTypes[i]);
    }
    free(pResourceMap->resourceTypes);
}

#endif
