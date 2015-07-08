#ifndef __DISSECT_SND_H__
#define __DISSECT_SND_H__

#include "../NiceStructures.h"
#include <stdbool.h>

void DissectSoundHeader(struct Resource* pResource, uint32_t offset)
{
    uint32_t dataPointer = OSReadBigInt32(pResource->data, offset);
    uint32_t numChannels = OSReadBigInt32(pResource->data, offset + 4);
    uint32_t sampleRate = OSReadBigInt32(pResource->data, offset + 8);

    printf("*** Channels: %d\n", numChannels);
    printf("*** Sample rate: 0x%08x\n", sampleRate);
}

void DissectSingleSoundCommand(struct Resource* pResource, uint16_t rawCommand, uint16_t param1, uint32_t param2)
{
    bool hasAssociatedSoundData = ((rawCommand & 0x8000) != 0);
    uint16_t command = (rawCommand & 0x7FFF);

    switch (command)
    {
        case 0x51:
            printf("** bufferCmd");
            if (hasAssociatedSoundData == false)
            {
                printf(" with sound header located outside resource at 0x%08x\n", param2);
            }
            else
            {
                printf(" with sound header located at offset 0x%08x\n", param2);
                DissectSoundHeader(pResource, param2);
            }
            break;
        default:
            printf("** Unrecognized sound command: 0x%04x (param1: 0x%04x param2: 0x%08x)\n", command, param1, param2);
    }
}

void DissectSingleSoundResource(struct Resource* pResource)
{
    printf("Dissecting named resource '%s'\n", pResource->name);
    printf("Sound Resource Header\n");
    printf("* Format Type: 0x%04x\n", OSReadBigInt16(pResource->data, 0));
    
    uint16_t dataTypeCount = OSReadBigInt16(pResource->data, 2);
    printf("* Number of data types: 0x%04x\n", dataTypeCount);

    uint16_t offset = 4;
    for (uint16_t i = 0; i < dataTypeCount; i++)
    {
        printf("** Data type: 0x%04x\n", OSReadBigInt16(pResource->data, offset));
        printf("** Initialization options: 0x%08x\n", OSReadBigInt32(pResource->data, offset + 2));
        offset += 6;
    }

    printf("Sound Commands\n");
    uint16_t commandCount = OSReadBigInt16(pResource->data, offset);
    offset += 2;
    for (uint16_t i = 0; i < commandCount; i++)
    {
        uint16_t command = OSReadBigInt16(pResource->data, offset);
        uint16_t param1 = OSReadBigInt16(pResource->data, offset + 2);
        uint32_t param2 = OSReadBigInt32(pResource->data, offset + 4);

        printf("* Command %d:\n", i + 1);
        DissectSingleSoundCommand(pResource, command, param1, param2);
        offset += 8;
    }
}

void DissectSound(struct ResourceType* pResourceType)
{
    for (uint16_t i = 0; i < pResourceType->resourceCount; i++)
    {
        DissectSingleSoundResource(pResourceType->resources[i]);
    }
}

#endif
