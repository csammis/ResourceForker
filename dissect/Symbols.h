#ifndef __DISSECT_NAME_H__
#define __DISSECT_NAME_H__

#include "../NiceStructures.h"
#include "../Common.h"

void DissectSingleNAME(Resource* pResource)
{
    char* filename = CreateFilename(pResource->name, ".txt");
    FILE* out = fopen(filename, "w");
    uint32_t index = 0;
    while (index < pResource->dataSize)
    {
        uint16_t length = OSReadBigInt16(pResource->data, index);
        index += 2;
        uint8_t stringLength = pResource->data[index];
        fwrite(pResource->data + index + 1, stringLength, 1, out);
        fwrite(CRLF, 2, 1, out);
        index += length;
    }
    fclose(out);
    ReleaseFilename(filename);
}

void DissectNAME(ResourceType* pResourceType)
{
    for (uint16_t i = 0; i < pResourceType->resourceCount; i++)
    {
        DissectSingleNAME(pResourceType->resources[i]);
    }
}

void DissectDLLN(ResourceType* pResourceType)
{
    for (uint16_t i = 0; i < pResourceType->resourceCount; i++)
    {
        char* filename = CreateFilename(pResourceType->resources[i]->name, ".txt");
        FILE* out = fopen(filename, "w");
        uint32_t index = 2;
        while (index < pResourceType->resources[i]->dataSize)
        {
            uint8_t length = pResourceType->resources[i]->data[index++];
            fwrite(pResourceType->resources[i]->data + index, length, 1, out);
            fwrite(CRLF, 2, 1, out);
            index += length;
        }
        fclose(out);
        ReleaseFilename(filename);
    }
}

#endif
