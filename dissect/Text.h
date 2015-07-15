#ifndef __DISSECT_TEXT_H__
#define __DISSECT_TEXT_H__

#include "../NiceStructures.h"
#include "../Common.h"

void DissectTEXT(struct ResourceType* pResourceType)
{
    for (uint16_t i = 0; i < pResourceType->resourceCount; i++)
    {
        char* filename = CreateFilename(pResourceType->resources[i]->name, ".txt");

        FILE* out = fopen(filename, "w");
        uint8_t lf = 0x0A;
        uint32_t dataSize = pResourceType->resources[i]->dataSize;
        for (uint32_t j = 0; j < dataSize; j++)
        {
            uint8_t byte = pResourceType->resources[i]->data[j];
            fwrite(&byte, 1, 1, out);
            if (byte == 0x0D && (pResourceType->resources[i]->data[j + 1] != 0x0A || j + 1 == dataSize))
            {
                fwrite(&lf, 1, 1, out);
            }
        }
        fclose(out);

        ReleaseFilename(filename);
    }
}

#endif
