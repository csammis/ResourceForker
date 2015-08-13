#ifndef __NICESTRUCTURES_H__
#define __NICESTRUCTURES_H__

#include "RawStructures.h"
#include <stdio.h>

typedef struct _Resource
{
    char name[257];
    uint32_t dataSize;
    uint8_t* data;
} Resource;

typedef struct _ResourceType
{
    char identifier[5];
    uint16_t resourceCount;
    Resource** resources;
} ResourceType;

typedef struct _ResourceMap
{
    uint16_t resourceTypeCount;
    ResourceType** resourceTypes;
} ResourceMap;

void BuildResourceMap(ResourceMap* pResourceMap, FILE* f)
{
    RawHeaderDefinition header;
    RawResourceMapDefinition resourceMap;
    RawTypeDefinition** typeList;

    ReadHeaderDefinition(&header, f);
    fseek(f, header.resourceMapOffset, SEEK_SET);
    ReadResourceMapDefinition(&resourceMap, f);
    uint32_t startOfTypeList = header.resourceMapOffset + resourceMap.typeListOffset;
    uint32_t startOfNameList = header.resourceMapOffset + resourceMap.nameListOffset;
    fseek(f, startOfTypeList, SEEK_SET);
    ReadTypeDefinitionList(&typeList, &pResourceMap->resourceTypeCount, f);

    pResourceMap->resourceTypes = malloc(sizeof(ResourceType) * pResourceMap->resourceTypeCount);
    for (uint16_t i = 0; i < pResourceMap->resourceTypeCount; i++)
    {
        pResourceMap->resourceTypes[i] = malloc(sizeof(ResourceType));
        memset(&pResourceMap->resourceTypes[i]->identifier, 0, 5);
        memcpy(&pResourceMap->resourceTypes[i]->identifier, &typeList[i]->identifier, 4);

        RawResourceDefinition** resourceDefinitionList;
        fseek(f, startOfTypeList + typeList[i]->resourceListOffset, SEEK_SET);
        ReadResourceDefinitionList(&resourceDefinitionList, typeList[i]->resourceCount, f);

        pResourceMap->resourceTypes[i]->resourceCount = typeList[i]->resourceCount;
        size_t mallocSize = sizeof(Resource*) * typeList[i]->resourceCount;
        pResourceMap->resourceTypes[i]->resources = malloc(mallocSize);
        Resource* currentResource;
        for (uint16_t j = 0; j < typeList[i]->resourceCount; j++)
        {
            pResourceMap->resourceTypes[i]->resources[j] = malloc(sizeof(Resource));
            currentResource = pResourceMap->resourceTypes[i]->resources[j];
            memset(&currentResource->name, 0, 257);
            if (resourceDefinitionList[j]->nameOffset != 0xFFFF)
            {
                char* resourceName = malloc(257);
                memset(resourceName, 0, 257);
                fseek(f, startOfNameList + resourceDefinitionList[j]->nameOffset, SEEK_SET);
                ReadNameFromList(resourceName, 256, f);
                snprintf(currentResource->name, 256, "%d (%s)", resourceDefinitionList[j]->ID, resourceName);
                free(resourceName);
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

void FreeResourceMap(ResourceMap* pResourceMap)
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
