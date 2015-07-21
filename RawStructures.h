// Major thanks for the format documentation from https://github.com/kreativekorp/ksfl/wiki/Macintosh-Resource-File-Format

#ifndef __RAWSTRUCTURES_H__
#define __RAWSTRUCTURES_H__

#include <stdlib.h>
#include <libkern/OSByteOrder.h>

// Data structures corresponding to the raw bytes in the file.
// The sizeof() these structures is used for reading out of the stream
// so if additional data has to be computed and stored it must be elsewhere.
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
    uint8_t dataOffset[3];
    uint32_t resourcePointer;
};

// Read the data of a single struct Resource from the current position in file
size_t ReadResourceDataDefinition(uint8_t** pData, uint32_t* pSize, FILE* f)
{
    void* pSizeBuffer = malloc(sizeof(uint32_t));
    size_t readSize = fread(pSizeBuffer, sizeof(uint32_t), 1, f);
    uint32_t dataSize = OSReadBigInt32(pSizeBuffer, 0);
    free(pSizeBuffer);

    *pData = malloc(sizeof(uint8_t) * dataSize);
    readSize = fread(*pData, sizeof(uint8_t), dataSize, f);

    *pSize = dataSize;
    return readSize;
}

// Read a struct ResourceDefinition from the current position in file
size_t ReadResourceDefinitionList(struct ResourceDefinition*** pResources, uint16_t count, FILE* f)
{
    void* pBuffer = malloc(sizeof(struct ResourceDefinition) * count);
    size_t readSize = fread(pBuffer, sizeof(struct ResourceDefinition), count, f);

    *pResources = malloc(sizeof(struct ResourceDefinition*) * count);
    for (uint16_t i = 0; i < count; i++)
    {
        uint32_t bufferIndex = i * sizeof(struct ResourceDefinition);
        (*pResources)[i] = malloc(sizeof(struct ResourceDefinition));
        (*pResources)[i]->ID = OSReadBigInt16(pBuffer, bufferIndex + 0);
        (*pResources)[i]->nameOffset = OSReadBigInt16(pBuffer, bufferIndex + 2);
        (*pResources)[i]->attributes = *((uint8_t*)(pBuffer) + bufferIndex + 4);
        memcpy(&(*pResources)[i]->dataOffset, pBuffer + bufferIndex + 5, 3);
        (*pResources)[i]->resourcePointer = OSReadBigInt32(pBuffer, bufferIndex + 8);
    }
    free(pBuffer);
    return readSize;
}

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
    *pTypes = malloc(sizeof(struct TypeDefinition*) * typeCount);
    for (uint16_t i = 0; i < typeCount; i++)
    {
        uint32_t bufferIndex = i * sizeof(struct TypeDefinition);
        (*pTypes)[i] = malloc(sizeof(struct TypeDefinition));
        memcpy(&(*pTypes)[i]->identifier, pBuffer + bufferIndex, 4);
        (*pTypes)[i]->resourceCount = OSReadBigInt16(pBuffer, bufferIndex + 4) + 1;
        (*pTypes)[i]->resourceListOffset = OSReadBigInt16(pBuffer, bufferIndex + 6);
    }
    free(pBuffer);
    *pTypeCount = typeCount;
    return readSize;
}

// Reads a name from the current position in file
size_t ReadNameFromList(char* pName, uint16_t bufferSize, FILE* f)
{
    void* pSizeBuffer = malloc(sizeof(uint8_t));
    size_t readSize = fread(pSizeBuffer, sizeof(uint8_t), 1, f);
    uint8_t dataCount;
    memcpy(&dataCount, pSizeBuffer, sizeof(uint8_t));
    free(pSizeBuffer);
    memset(pName, 0, bufferSize);
    readSize = fread(pName, sizeof(char), dataCount, f);
    return readSize;
}

#endif
