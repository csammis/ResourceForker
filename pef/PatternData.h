#ifndef __PEF_PATTERNDATA_H__
#define __PEF_PATTERNDATA_H__

#include "Structures.h"

#define PATTERN_DATA_OPCODE(x) ((x & 0xE0) >> 5)

uint32_t ReadOnePatternArgument(uint8_t* pData, bool firstArgument, uint32_t index, uint32_t* pNewIndex)
{
    uint32_t arg = 0;
    if (firstArgument)
    {
        arg = pData[index++] & 0x1F;
        if (arg == 0)
        {
            return ReadOnePatternArgument(pData, false, index, pNewIndex);
        }
    }
    else
    {
        uint8_t piece = 0;
        do
        {
            piece = pData[index++];
            arg <<= 7;
            arg |= (piece & 0x7F);
        } while (piece & 0x80);
    }

    *pNewIndex = index;
    return arg;
}

void InflatePatternDataSection(Section* pSection)
{
    uint32_t dataLocationCount = 0;
    // Copy the section data pointer and create it again with the new total size
    uint8_t* packedData = malloc(pSection->length);
    memcpy(packedData, pSection->data, pSection->length);
    free(pSection->data);
    pSection->data = malloc(pSection->totalLength);

    uint8_t* unpackedData = pSection->data;
    memset(unpackedData, 0, pSection->totalLength);

    for (uint32_t i = 0, instCount = 0; i < pSection->length; instCount++)
    {
        uint8_t opcode = PATTERN_DATA_OPCODE(packedData[i]);
        uint32_t count = ReadOnePatternArgument(packedData, true, i, &i);

        switch (opcode)
        {
            case 0: // Zero
                memset(unpackedData + dataLocationCount, 0, count);
                dataLocationCount += count;
                break;
            case 1: // blockCopy
                memcpy(unpackedData + dataLocationCount, packedData + i, count);
                dataLocationCount += count;
                i += count;
                break;
            case 2: // repeatedBlock
                {
                    uint32_t repeatCount = ReadOnePatternArgument(packedData, false, i, &i);
                    for (uint32_t j = 0; j < repeatCount; j++)
                    {
                        memcpy(unpackedData + dataLocationCount, packedData + i, count);
                        dataLocationCount += count;
                    }
                    i += (repeatCount * count);
                }
                break;
            case 3:
                {
                    uint32_t customSize = ReadOnePatternArgument(packedData, false, i, &i);
                    uint32_t repeatCount = ReadOnePatternArgument(packedData, false, i, &i);

                    for (uint32_t j = 0; j < repeatCount; j++)
                    {
                        // Common data
                        memcpy(unpackedData + dataLocationCount, packedData + i, count);
                        dataLocationCount += count;
                        // Custom data j
                        memcpy(unpackedData + dataLocationCount, packedData + i + count + (j * customSize), customSize);
                        dataLocationCount += customSize;
                    }

                    memcpy(unpackedData + dataLocationCount, packedData + i, count);
                    dataLocationCount += count;
                    i += (count + (customSize * repeatCount));
                }
                break;
            case 4:
                {
                    uint32_t customSize = ReadOnePatternArgument(packedData, false, i, &i);
                    uint32_t repeatCount = ReadOnePatternArgument(packedData, false, i, &i);

                    for (uint32_t j = 0; j < repeatCount; j++)
                    {
                        // Zeros instead of common data
                        memset(unpackedData + dataLocationCount, 0, count);
                        dataLocationCount += count;
                        // Custom data j
                        memcpy(unpackedData + dataLocationCount, packedData + i + count + (j * customSize), customSize);
                        dataLocationCount += customSize;
                    }

                    memset(unpackedData + dataLocationCount, 0, count);
                    dataLocationCount += count;
                    i += (customSize * repeatCount);
                }
                break;
            default:
                printf("DEBUG: Unexpected opcode in packed data at %x: %d, count = %d", i, opcode, count);
        }
    }

    free(packedData);
}

#endif
