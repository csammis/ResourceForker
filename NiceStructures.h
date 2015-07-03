#ifndef __NICESTRUCTURES_H__
#define __NICESTRUCTURES_H__

#include "RawStructures.h"
#include <stdio.h>

struct ResourceType
{
    char identifier[5];
    uint16_t resourceCount;
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
    fseek(f, header.resourceMapOffset + resourceMap.typeListOffset, SEEK_SET);
    ReadTypeDefinitionList(&typeList, &pResourceMap->resourceTypeCount, f);

    pResourceMap->resourceTypes = malloc(sizeof(struct ResourceType) * pResourceMap->resourceTypeCount);
    for (uint16_t i = 0; i < pResourceMap->resourceTypeCount; i++)
    {
        pResourceMap->resourceTypes[i] = malloc(sizeof(struct ResourceType));
        memset(&pResourceMap->resourceTypes[i]->identifier, 0, 5);
        memcpy(&pResourceMap->resourceTypes[i]->identifier, &typeList[i]->identifier, 4);
        pResourceMap->resourceTypes[i]->resourceCount = typeList[i]->resourceCount;
    }
}

void FreeResourceMap(struct ResourceMap* pResourceMap)
{
    for (uint16_t i = 0; i < pResourceMap->resourceTypeCount; i++)
    {
        free(pResourceMap->resourceTypes[i]);
    }
    free(pResourceMap->resourceTypes);
}

#endif
