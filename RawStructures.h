#ifndef __RAWSTRUCTURES_H__
#define __RAWSTRUCTURES_H__

#include <stdlib.h>
#include <libkern/OSByteOrder.h>

// Data structures corresponding to the raw bytes in the file
//
struct HeaderDefinition
{
    uint32_t resourceDataOffset; // From start of file
    uint32_t resourceMapOffset; // From start of file
    uint32_t resourceDataSize;
    uint32_t resourceMapSize;
};

struct ResourceMapDefinition
{
    struct HeaderDefinition header;
    uint32_t nextResourceMap;
    uint16_t fileRef;
    uint16_t attributes;
    uint16_t typeListOffset; // From resource map start
    uint16_t nameListOffset; // From resource map start
};

struct TypeDefinition
{
    uint32_t identifier;
    uint16_t resourceCount;
    uint16_t resourceListOffset; // From start of type list
};

struct ResourceDefinition
{
    uint16_t ID;
    uint16_t nameOffset; // From start of name list
    uint8_t attributes;
    uint8_t dataOffset[3]; // From start of data section.
    uint32_t resourcePointer;
};

// Read a struct HeaderDefinition from the current position in file
size_t ReadHeaderDefinition(struct HeaderDefinition* pHeaderDefinition, FILE* f)
{
    memset(pHeaderDefinition, 0, sizeof(struct HeaderDefinition));
    void* pBuffer = malloc(sizeof(struct HeaderDefinition));
    size_t readSize = fread(pBuffer, sizeof(struct HeaderDefinition), 1, f);
    pHeaderDefinition->resourceDataOffset = OSReadBigInt32(pBuffer, 0);
    pHeaderDefinition->resourceMapOffset = OSReadBigInt32(pBuffer, 4);
    pHeaderDefinition->resourceDataSize = OSReadBigInt32(pBuffer, 8);
    pHeaderDefinition->resourceMapSize = OSReadBigInt32(pBuffer, 12);
    free(pBuffer);
    return readSize;
}

// Read a struct ResourceMapDefinition from the current position in file
size_t ReadResourceMapDefinition(struct ResourceMapDefinition* pMap, FILE* f)
{
    memset(pMap, 0, sizeof(struct ResourceMapDefinition));
    void* pBuffer = malloc(sizeof(struct ResourceMapDefinition));
    size_t readSize = fread(pBuffer, sizeof(struct ResourceMapDefinition), 1, f);
    pMap->header.resourceDataOffset = OSReadBigInt32(pBuffer, 0);
    pMap->header.resourceMapOffset = OSReadBigInt32(pBuffer, 4);
    pMap->header.resourceDataSize = OSReadBigInt32(pBuffer, 8);
    pMap->header.resourceMapSize = OSReadBigInt32(pBuffer, 12);
    pMap->nextResourceMap = OSReadBigInt32(pBuffer, 16);
    pMap->fileRef = OSReadBigInt16(pBuffer, 20);
    pMap->attributes = OSReadBigInt16(pBuffer, 22);
    pMap->typeListOffset = OSReadBigInt16(pBuffer, 24);
    pMap->nameListOffset = OSReadBigInt16(pBuffer, 26);
    return readSize;
}

// Reads a list of struct TypeDefinition from the current position in file
size_t ReadTypeDefinitionList(struct TypeDefinition*** pTypes, uint16_t* pTypeCount, FILE* f)
{
    void* pSizeBuffer = malloc(sizeof(uint16_t));
    size_t readSize = fread(pSizeBuffer, sizeof(uint16_t), 1, f);
    uint16_t typeCount = OSReadBigInt16(pSizeBuffer, 0) + 1;
    free(pSizeBuffer);
    void* pBuffer = malloc(sizeof(struct TypeDefinition) * typeCount);
    readSize = fread(pBuffer, sizeof(struct TypeDefinition), typeCount, f);
    *pTypes = (struct TypeDefinition**)malloc(sizeof(struct TypeDefinition) * typeCount);
    for (uint16_t i = 0; i < typeCount; i++)
    {
        uint32_t bufferIndex = i * sizeof(struct TypeDefinition);
        (*pTypes)[i] = (struct TypeDefinition*)malloc(sizeof(struct TypeDefinition));
        memcpy(&(*pTypes)[i]->identifier, pBuffer + bufferIndex, 4);
        (*pTypes)[i]->resourceCount = OSReadBigInt16(pBuffer, bufferIndex + 4) + 1;
        (*pTypes)[i]->resourceListOffset = OSReadBigInt16(pBuffer, bufferIndex + 6);
    }
    free(pBuffer);
    *pTypeCount = typeCount;
    return readSize;
}

#endif
