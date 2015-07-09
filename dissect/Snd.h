#ifndef __DISSECT_SND_H__
#define __DISSECT_SND_H__

#include "../NiceStructures.h"
#include <stdbool.h>

void DissectCompressedSoundHeader(struct Resource* pResource, uint32_t offset)
{
    uint32_t frameCount = OSReadBigInt32(pResource->data, offset);
    // +10 for the AIFF sample rate 4
    // +4 for a marker chunk unused in 'snd ' 14
    char compressionFormat[5];
    memset(&compressionFormat, 0, 5);
    memcpy(&compressionFormat, pResource->data + offset + 18, 4);
    // +4 reserved for future use, not used for 'snd ' 22
    // +4 for state variables, not used for 'snd ' 26
    // +4 for leftover block storage, not used in analysis 30
    short compressionID = (short)OSReadBigInt16(pResource->data, offset + 34);
    uint16_t packetSize = OSReadBigInt16(pResource->data, offset + 36);
    // +2 for snthID which is unused 38
    uint16_t sampleSize = OSReadBigInt16(pResource->data, offset + 40);

    printf("*** Frame count: 0x%08x\n", frameCount);
    printf("*** Compression format: ");
    switch (compressionID)
    {
        case -2:
            printf("variable-rate");
            break;
        case -1:
            printf("%s", compressionFormat);
            break;
        case 0:
            printf("uncompressed");
            break;
        case 3:
            printf("3:1");
            break;
        case 4:
            printf("6:1");
            break;
        default:
            printf("!!! unrecognized");
    }
    printf("\n");
    printf("*** Original sample size: %d\n", sampleSize);
}

void DissectSoundHeader(struct Resource* pResource, uint32_t offset)
{
    uint32_t dataPointer = OSReadBigInt32(pResource->data, offset);
    uint32_t numChannels = OSReadBigInt32(pResource->data, offset + 4);
    uint32_t sampleRate = OSReadBigInt32(pResource->data, offset + 8);
    uint32_t loopStart = OSReadBigInt32(pResource->data, offset + 12);
    uint32_t loopEnd = OSReadBigInt32(pResource->data, offset + 16);
    uint8_t encoding = (uint8_t)*(pResource->data + offset + 20);
    uint8_t baseFrequency = (uint8_t)*(pResource->data + offset + 21);

    printf("*** Channels: %d\n", numChannels);
    printf("*** Sample rate: 0x%08x\n", sampleRate);

    switch (encoding)
    {
        case 0x00:
            printf("*** Loop start: 0x%08x to loop end: 0x%08x\n", loopStart, loopEnd);
            printf("*** Base frequency: 0x%02x\n", baseFrequency);
            break;
        case 0xFE:
            DissectCompressedSoundHeader(pResource, offset + 22);
            break;
        case 0xFF:
            printf("*** !!! Extended sound header, not sure what this is\n");
            break;
        default:
            printf("*** !!! Unrecognized sound header encoding: 0x%02x\n", encoding);
    }
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
            printf("** !!! Unrecognized sound command: 0x%04x (param1: 0x%04x param2: 0x%08x)\n", command, param1, param2);
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
