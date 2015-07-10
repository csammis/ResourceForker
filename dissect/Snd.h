#ifndef __DISSECT_SND_H__
#define __DISSECT_SND_H__

#include "../NiceStructures.h"
#include <stdbool.h>
#include "../Common.h"
#include <limits.h>

// IMA4_Decode cobbled together from:
// * http://wiki.multimedia.cx/index.php?title=IMA_ADPCM
// * https://devforums.apple.com/message/10678#10678

int ima_index_table[16] = {
  -1, -1, -1, -1, 2, 4, 6, 8,
  -1, -1, -1, -1, 2, 4, 6, 8
};

int ima_step_table[89] = { 
  7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 
  19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 
  50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 
  130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
  337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
  876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066, 
  2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
  5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899, 
  15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767 
};

void ClampStepIndex(int* pValue)
{
    int value = *pValue;
    if (value > 88) *pValue = 88;
    if (value < 0)  *pValue = 0;
}

void ClampSigned16BitPCM(int* pValue)
{
    int value = *pValue;
    if (value >  32767) *pValue =  32767;
    if (value < -32768) *pValue = -32768;
}

uint32_t IMA4_Decode(struct Resource* pResource, uint32_t offset, uint32_t frameCount, short** outputBuffer)
{
    // 32 data bytes per frame, two samples per byte
    short* output = (short*)malloc(sizeof(uint16_t) * frameCount * 64);
    uint32_t outIndex = 0;
    for (uint32_t i = 0; i < frameCount; i++)
    {
        uint16_t preamble = OSReadBigInt16(pResource->data, offset + (i * 34));
        int predictor = preamble & 0xFF80;
        if (predictor & 0x8000)
        {
            predictor |= 0xFFFF0000;
        }
        int step_index = preamble & 0x007F;
        int step, nibble, diff;

        // Next 32 bytes are the ADPCM nibbles
        for (uint32_t j = 2; j < 34; j++)
        {
            uint8_t byte = pResource->data[offset + (j + (i * 34))];

            nibble = byte & 0x0F; // Lo nibble
            ClampStepIndex(&step_index);
            step = ima_step_table[step_index];
            step_index += ima_index_table[nibble];
            diff = step >> 3;
            if (nibble & 0x04) diff += step;
            if (nibble & 0x02) diff += (step >> 1);
            if (nibble & 0x01) diff += (step >> 2);
            if (nibble & 0x08)
                predictor -= diff;
            else
                predictor += diff;
            ClampSigned16BitPCM(&predictor);
            output[outIndex++] = predictor;

            nibble = byte >> 4; // Hi nibble
            ClampStepIndex(&step_index);
            step = ima_step_table[step_index];
            step_index += ima_index_table[nibble];
            diff = step >> 3;
            if (nibble & 0x04) diff += step;
            if (nibble & 0x02) diff += (step >> 1);
            if (nibble & 0x01) diff += (step >> 2);
            if (nibble & 0x08)
                predictor -= diff;
            else
                predictor += diff;
            ClampSigned16BitPCM(&predictor);
            output[outIndex++] = predictor;
        }
    }

    *outputBuffer = output;
    return outIndex;
}

void DissectCompressedSoundHeader(struct Resource* pResource, uint32_t offset, bool verbose)
{
    uint32_t frameCount = OSReadBigInt32(pResource->data, offset);
    uint8_t aiffBuffer[10];
    for (uint8_t i = 0; i < 10; i++)
    {
        aiffBuffer[9 - i] = *(pResource->data + offset + 4 + i);
    }
    long double aiffSampleRate = 0;
    memcpy(&aiffSampleRate, aiffBuffer, 10); 
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

    if (verbose)
    {
        Indent(8); printf("Frame count: %d\n", frameCount);
        Indent(8); printf("AIFF sample rate: %Lf Hz\n", aiffSampleRate);
        Indent(8); printf("Compression format: ");
    }

    switch (compressionID)
    {
        case -2:
            if (verbose) printf("variable-rate\n");
            break;
        case -1:
            if (verbose) printf("%s\n", compressionFormat);

            if (strncmp(compressionFormat, "ima4", 4) == 0)
            {
                short* outputBuffer = NULL;
                uint32_t outputBufferCount = IMA4_Decode(pResource, offset + 42, frameCount, &outputBuffer);

                uint32_t nameSize = strlen(pResource->name) + 5;
                char* pOutName = malloc(nameSize);
                memset(pOutName, 0, nameSize);
                strncpy(pOutName, pResource->name, strlen(pResource->name));
                strncpy(pOutName + strlen(pResource->name), ".raw", 4);
                FILE* rawout = fopen(pOutName, "wb");
                fwrite(outputBuffer, sizeof(short), outputBufferCount, rawout);
                fclose(rawout);
                
                char txtBuffer[513];
                memset(&txtBuffer, 0, 513);
                snprintf(txtBuffer, 512, "%s.raw is uncompressed signed 16bit PCM, mono, at a bitrate of %Lf Hz\n", pResource->name, aiffSampleRate);
                strncpy(pOutName + strlen(pResource->name), ".txt", 4);
                FILE* txtout = fopen(pOutName, "w");
                fwrite(&txtBuffer, strlen(txtBuffer), 1, txtout);
                fclose(txtout);
                free(outputBuffer);
            }
            break;
        case 0:
            if (verbose) printf("uncompressed\n");
            break;
        case 3:
            if (verbose) printf("3:1\n");
            break;
        case 4:
            if (verbose) printf("6:1\n");
            break;
        default:
            printf("!!! unrecognized compression\n");
    }

    if (verbose)
    {
        Indent(8); printf("Packet size: %d\n", packetSize);
        Indent(8); printf("Original sample size: %d\n", sampleSize);
    }
}

void DissectSoundHeader(struct Resource* pResource, uint32_t offset, bool verbose)
{
    uint32_t dataPointer = OSReadBigInt32(pResource->data, offset);
    uint32_t numChannels = OSReadBigInt32(pResource->data, offset + 4);
    uint32_t sampleRate = OSReadBigInt32(pResource->data, offset + 8);
    uint32_t loopStart = OSReadBigInt32(pResource->data, offset + 12);
    uint32_t loopEnd = OSReadBigInt32(pResource->data, offset + 16);
    uint8_t encoding = (uint8_t)*(pResource->data + offset + 20);
    uint8_t baseFrequency = (uint8_t)*(pResource->data + offset + 21);

    if (verbose)
    {
        Indent(8); printf("Channels: %d\n", numChannels);
        Indent(8); printf("Sample rate: 0x%08x\n", sampleRate);
    }

    switch (encoding)
    {
        case 0x00:
            if (verbose)
            {
                Indent(8); printf("*** Loop start: 0x%08x to loop end: 0x%08x\n", loopStart, loopEnd);
                Indent(8); printf("*** Base frequency: 0x%02x\n", baseFrequency);
            }
            break;
        case 0xFE:
            DissectCompressedSoundHeader(pResource, offset + 22, verbose);
            break;
        case 0xFF:
            Indent(8); printf("!!! Extended sound header, not sure what this is\n");
            break;
        default:
            Indent(8); printf("!!! Unrecognized sound header encoding: 0x%02x\n", encoding);
    }
}

void DissectSingleSoundCommand(struct Resource* pResource, uint16_t rawCommand, uint16_t param1, uint32_t param2, bool verbose)
{
    bool hasAssociatedSoundData = ((rawCommand & 0x8000) != 0);
    uint16_t command = (rawCommand & 0x7FFF);

    switch (command)
    {
        case 0x51:
            if (verbose)
            {
                Indent(6); printf("bufferCmd");
            }
            if (hasAssociatedSoundData == false)
            {
                if (verbose)
                    printf(" with sound header located outside resource at 0x%08x\n", param2);
            }
            else
            {
                if (verbose)
                    printf(" with sound header located at offset 0x%08x\n", param2);
                DissectSoundHeader(pResource, param2, verbose);
            }
            break;
        default:
            if (verbose)
            {
                Indent(6); printf("!!! Unrecognized sound command: 0x%04x (param1: 0x%04x param2: 0x%08x)\n", command, param1, param2);
            }
    }
}

void DissectSingleSoundResource(struct Resource* pResource, bool verbose)
{
    if (verbose)
    {
        printf("Dissecting named resource '%s'\n", pResource->name);
        Indent(2); printf("Sound Resource Header\n");
        Indent(4); printf("Format Type: %d\n", OSReadBigInt16(pResource->data, 0));
    }
    
    uint16_t dataTypeCount = OSReadBigInt16(pResource->data, 2);
    if (verbose)
    {
        Indent(4); printf("Number of data types: %d\n", dataTypeCount);
    }

    uint16_t offset = 4;
    for (uint16_t i = 0; i < dataTypeCount; i++)
    {
        if (verbose)
        {
            Indent(6); printf("Data type: 0x%04x\n", OSReadBigInt16(pResource->data, offset));
            Indent(6); printf("Initialization options: 0x%08x\n", OSReadBigInt32(pResource->data, offset + 2));
        }
        offset += 6;
    }

    if (verbose)
    {
        Indent(2); printf("Sound Commands\n");
    }
    uint16_t commandCount = OSReadBigInt16(pResource->data, offset);
    offset += 2;
    for (uint16_t i = 0; i < commandCount; i++)
    {
        uint16_t command = OSReadBigInt16(pResource->data, offset);
        uint16_t param1 = OSReadBigInt16(pResource->data, offset + 2);
        uint32_t param2 = OSReadBigInt32(pResource->data, offset + 4);

        if (verbose)
        {
            Indent(4); printf("Command %d:\n", i + 1);
        }
        DissectSingleSoundCommand(pResource, command, param1, param2, verbose);
        offset += 8;
    }
}

void DissectSound(struct ResourceType* pResourceType, bool verbose)
{
    for (uint16_t i = 0; i < pResourceType->resourceCount; i++)
    {
        DissectSingleSoundResource(pResourceType->resources[i], verbose);
    }
}

#endif
