#ifndef __DISSECT_TEXT_H__
#define __DISSECT_TEXT_H__

#include "../NiceStructures.h"
#include "../Common.h"

void WriteMacStringToFile(uint8_t* pBuffer, uint32_t length, FILE* out)
{
    for (uint32_t i = 0; i < length; i++)
    {
        uint8_t byte = pBuffer[i];
        fwrite(&byte, 1, 1, out);
        if (byte == CR && (i + 1 == length || pBuffer[i + 1] != LF))
        {
            fwrite(CRLF + 1, 1, 1, out);
        }
    }
}

void DissectTEXT(struct ResourceType* pResourceType)
{
    for (uint16_t i = 0; i < pResourceType->resourceCount; i++)
    {
        char* filename = CreateFilename(pResourceType->resources[i]->name, ".txt");

        FILE* out = fopen(filename, "w");
        WriteMacStringToFile(pResourceType->resources[i]->data, pResourceType->resources[i]->dataSize, out);
        fclose(out);
        ReleaseFilename(filename);
    }
}

uint8_t DissectSingleSTR(uint8_t* pBuffer, FILE* out)
{
    uint8_t length = pBuffer[0];
    WriteMacStringToFile(pBuffer + 1, length, out);
    return length + 1;
}

void DissectSTR(struct ResourceType* pResourceType)
{
    for (uint16_t i = 0; i < pResourceType->resourceCount; i++)
    {
        char* filename = CreateFilename(pResourceType->resources[i]->name, ".txt");
        FILE* out = fopen(filename, "w");
        DissectSingleSTR(pResourceType->resources[i]->data, out);
        fclose(out);
        ReleaseFilename(filename);
    }
}

void DissectSTRN(struct ResourceType* pResourceType)
{
    for (uint16_t i = 0; i < pResourceType->resourceCount; i++)
    {
        char* filename = CreateFilename(pResourceType->resources[i]->name, ".txt");

        FILE* out = fopen(filename, "w");
        uint16_t stringCount = OSReadBigInt16(pResourceType->resources[i]->data, 0);
        uint16_t index = 2;
        for (uint16_t j = 0; j < stringCount; j++)
        {
            index += DissectSingleSTR(pResourceType->resources[i]->data + index, out);
            fwrite(CRLF, 2, 1, out);
        }
        fclose(out);
        ReleaseFilename(filename);
    }
}

#endif
