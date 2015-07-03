// Major thanks for the format documentation from https://github.com/kreativekorp/ksfl/wiki/Macintosh-Resource-File-Format

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libkern/OSByteOrder.h>

struct Header
{
    uint32_t resourceDataOffset; // From start of file
    uint32_t resourceMapOffset; // From start of file
    uint32_t resourceDataSize;
    uint32_t resourceMapSize;
};

struct ResourceMap
{
    struct Header header;
    uint32_t nextResourceMap;
    uint16_t fileRef;
    uint16_t attributes;
    uint16_t typeListOffset; // From resource map start
    uint16_t nameListOffset; // From resource map start
};

struct Type
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
    uint8_t dataOffset[3]; // From start of data section. This is a 24bit unsigned value in its raw form.
    uint32_t resourcePointer;
};

// Read a struct Header from the current position in file
size_t ReadHeader(struct Header* pHeader, FILE* f)
{
    memset(pHeader, 0, sizeof(struct Header));
    void* pBuffer = malloc(sizeof(struct Header));
    size_t readSize = fread(pBuffer, sizeof(struct Header), 1, f);
    pHeader->resourceDataOffset = OSReadBigInt32(pBuffer, 0);
    pHeader->resourceMapOffset = OSReadBigInt32(pBuffer, 4);
    pHeader->resourceDataSize = OSReadBigInt32(pBuffer, 8);
    pHeader->resourceMapSize = OSReadBigInt32(pBuffer, 12);
    free(pBuffer);
    return readSize;
}

// Read a struct ResourceMap from the current position in file
size_t ReadResourceMap(struct ResourceMap* pMap, FILE* f)
{
    memset(pMap, 0, sizeof(struct ResourceMap));
    void* pBuffer = malloc(sizeof(struct ResourceMap));
    size_t readSize = fread(pBuffer, sizeof(struct ResourceMap), 1, f);
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

// Reads a list of struct Type from the current position in file
size_t ReadTypeList(struct Type*** pTypes, uint16_t* pTypeCount, FILE* f)
{
    void* pSizeBuffer = malloc(sizeof(uint16_t));
    size_t readSize = fread(pSizeBuffer, sizeof(uint16_t), 1, f);
    uint16_t typeCount = OSReadBigInt16(pSizeBuffer, 0) + 1;
    free(pSizeBuffer);
    void* pBuffer = malloc(sizeof(struct Type) * typeCount);
    readSize = fread(pBuffer, sizeof(struct Type), typeCount, f);
    *pTypes = (struct Type**)malloc(sizeof(struct Type) * typeCount);
    for (uint16_t i = 0; i < typeCount; i++)
    {
        uint32_t bufferIndex = i * sizeof(struct Type);
        (*pTypes)[i] = (struct Type*)malloc(sizeof(struct Type));
        memcpy(&(*pTypes)[i]->identifier, pBuffer + bufferIndex, 4);
        (*pTypes)[i]->resourceCount = OSReadBigInt16(pBuffer, bufferIndex + 4) + 1;
        (*pTypes)[i]->resourceListOffset = OSReadBigInt16(pBuffer, bufferIndex + 6);
    }
    free(pBuffer);
    *pTypeCount = typeCount;
    return readSize;
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("Usage: ResourceForker file\n");
        return 1;
    }

    printf("Processing file %s\n", argv[1]);

    FILE* f = fopen(argv[1], "rb");

    struct Header header;
    struct ResourceMap resourceMap;
    struct Type** typeList;
    uint16_t typeCount;

    ReadHeader(&header, f);
    fseek(f, header.resourceMapOffset, SEEK_SET);
    ReadResourceMap(&resourceMap, f);
    fseek(f, header.resourceMapOffset + resourceMap.typeListOffset, SEEK_SET);
    ReadTypeList(&typeList, &typeCount, f);

    printf("Reading %u resource types\n", typeCount);
    char fourCharType[5];
    for (uint16_t i = 0; i < typeCount; i++)
    {
        memset(&fourCharType, 0, 5);
        memcpy(&fourCharType, &typeList[i]->identifier, 4);
        printf("Reading %u objects of type %s\n", typeList[i]->resourceCount, fourCharType);
    }

    fclose(f);
    for (int i = 0; i < typeCount; i++)
    {
        free(typeList[i]);
    }
    free(typeList);
    return 0;
}
