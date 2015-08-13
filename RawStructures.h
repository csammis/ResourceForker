// Major thanks for the format documentation from https://github.com/kreativekorp/ksfl/wiki/Macintosh-Resource-File-Format

#ifndef __RAWSTRUCTURES_H__
#define __RAWSTRUCTURES_H__

#include <stdlib.h>
#include <libkern/OSByteOrder.h>

// Data structures corresponding to the raw bytes in the file.
// The sizeof() these structures is used for reading out of the stream
// so if additional data has to be computed and stored it must be elsewhere.
typedef struct _HeaderDefinition
{
    uint32_t resourceDataOffset; // From start of file
    uint32_t resourceMapOffset; // From start of file
    uint32_t resourceDataSize;
    uint32_t resourceMapSize;
} RawHeaderDefinition;

typedef struct _ResourceMapDefinition
{
    RawHeaderDefinition header;
    uint32_t nextResourceMap;
    uint16_t fileRef;
    uint16_t attributes;
    uint16_t typeListOffset; // From resource map start
    uint16_t nameListOffset; // From resource map start
} RawResourceMapDefinition;

typedef struct _TypeDefinition
{
    uint32_t identifier;
    uint16_t resourceCount;
    uint16_t resourceListOffset; // From start of type list
} RawTypeDefinition;

typedef struct _ResourceDefinition
{
    uint16_t ID;
    uint16_t nameOffset; // From start of name list
    uint8_t attributes;
    uint8_t dataOffset[3];
    uint32_t resourcePointer;
} RawResourceDefinition;

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

// Read a RawResourceDefinition from the current position in file
size_t ReadResourceDefinitionList(RawResourceDefinition*** pResources, uint16_t count, FILE* f)
{
    void* pBuffer = malloc(sizeof(RawResourceDefinition) * count);
    size_t readSize = fread(pBuffer, sizeof(RawResourceDefinition), count, f);

    *pResources = malloc(sizeof(RawResourceDefinition*) * count);
    for (uint16_t i = 0; i < count; i++)
    {
        uint32_t bufferIndex = i * sizeof(RawResourceDefinition);
        (*pResources)[i] = malloc(sizeof(RawResourceDefinition));
        (*pResources)[i]->ID = OSReadBigInt16(pBuffer, bufferIndex + 0);
        (*pResources)[i]->nameOffset = OSReadBigInt16(pBuffer, bufferIndex + 2);
        (*pResources)[i]->attributes = *((uint8_t*)(pBuffer) + bufferIndex + 4);
        memcpy(&(*pResources)[i]->dataOffset, pBuffer + bufferIndex + 5, 3);
        (*pResources)[i]->resourcePointer = OSReadBigInt32(pBuffer, bufferIndex + 8);
    }
    free(pBuffer);
    return readSize;
}

// Read a RawHeaderDefinition from the current position in file
size_t ReadHeaderDefinition(RawHeaderDefinition* pHeaderDefinition, FILE* f)
{
    memset(pHeaderDefinition, 0, sizeof(RawHeaderDefinition));
    void* pBuffer = malloc(sizeof(RawHeaderDefinition));
    size_t readSize = fread(pBuffer, sizeof(RawHeaderDefinition), 1, f);
    pHeaderDefinition->resourceDataOffset = OSReadBigInt32(pBuffer, 0);
    pHeaderDefinition->resourceMapOffset = OSReadBigInt32(pBuffer, 4);
    pHeaderDefinition->resourceDataSize = OSReadBigInt32(pBuffer, 8);
    pHeaderDefinition->resourceMapSize = OSReadBigInt32(pBuffer, 12);
    free(pBuffer);
    return readSize;
}

// Read a RawResourceMapDefinition from the current position in file
size_t ReadResourceMapDefinition(RawResourceMapDefinition* pMap, FILE* f)
{
    memset(pMap, 0, sizeof(RawResourceMapDefinition));
    void* pBuffer = malloc(sizeof(RawResourceMapDefinition));
    size_t readSize = fread(pBuffer, sizeof(RawResourceMapDefinition), 1, f);
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

// Reads a list of RawTypeDefinition from the current position in file
size_t ReadTypeDefinitionList(RawTypeDefinition*** pTypes, uint16_t* pTypeCount, FILE* f)
{
    void* pSizeBuffer = malloc(sizeof(uint16_t));
    size_t readSize = fread(pSizeBuffer, sizeof(uint16_t), 1, f);
    uint16_t typeCount = OSReadBigInt16(pSizeBuffer, 0) + 1;
    free(pSizeBuffer);
    void* pBuffer = malloc(sizeof(RawTypeDefinition) * typeCount);
    readSize = fread(pBuffer, sizeof(RawTypeDefinition), typeCount, f);
    *pTypes = malloc(sizeof(RawTypeDefinition*) * typeCount);
    for (uint16_t i = 0; i < typeCount; i++)
    {
        uint32_t bufferIndex = i * sizeof(RawTypeDefinition);
        (*pTypes)[i] = malloc(sizeof(RawTypeDefinition));
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
