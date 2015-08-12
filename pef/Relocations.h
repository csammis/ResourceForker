#ifndef __PEF_RELOCATIONS_H__
#define __PEF_RELOCATIONS_H__

#include "Structures.h"

typedef struct _RelocationState
{
    uint32_t relocAddr;
    uint32_t importIndex;
    uint32_t sectionC;
    uint32_t sectionD;
} RelocationState;

#define ADD_TO_ADDRESS(x, y) OSWriteBigInt32(pSection->data, x, OSReadBigInt32(pSection->data, x) + y)

uint32_t DoOneRelocationInstruction(uint8_t* data, uint32_t instructionOffset, RelocationState* state, Section* pSection, Section** sections)
{
    uint8_t block[2];
    memcpy(block, data + instructionOffset, 2);

    // the 0x05 instructions take 4 bytes instead of 2
    uint32_t instructionSize = 2;

    if ((block[0] & 0xC0) == 0)
    {
        uint16_t skipCount = ((block[0] & 0x3F) << 2) | ((block[1] & 0xC0) >> 6);
        uint8_t relocCount = block[1] & 0x3F;
        state->relocAddr += skipCount * 4;
        for (uint8_t j = 0; j < relocCount; j++, state->relocAddr += 4)
        {
            ADD_TO_ADDRESS(state->relocAddr, state->sectionD);
        }
    }
    else
    {
        uint8_t opcode = (block[0] & 0xFE) >> 1;
        switch (opcode & 0xF0)
        {
            case 0x20:
                {
                    uint8_t runLength = (opcode & 0x0F) + 1;
                    switch (opcode)
                    {
                        case 0x20:
                            for (uint8_t j = 0; j < runLength; j++, state->relocAddr += 4)
                            {
                                ADD_TO_ADDRESS(state->relocAddr, state->sectionC);
                            }
                            break;
                        case 0x21:
                            for (uint8_t j = 0; j < runLength; j++, state->relocAddr += 4)
                            {
                                ADD_TO_ADDRESS(state->relocAddr, state->sectionD);
                            }
                            break;
                        case 0x22:
                            for (uint8_t j = 0; j < runLength; j++, state->relocAddr += 12)
                            {
                                ADD_TO_ADDRESS(state->relocAddr, state->sectionC);
                                ADD_TO_ADDRESS(state->relocAddr + 4, state->sectionD);
                                // No write to offset 8
                            }
                            break;
                        case 0x23:
                            for (uint8_t j = 0; j < runLength; j++, state->relocAddr += 8)
                            {
                                ADD_TO_ADDRESS(state->relocAddr, state->sectionC);
                                ADD_TO_ADDRESS(state->relocAddr + 4, state->sectionD);
                            }
                            break;
                        case 0x24:
                            for (uint8_t j = 0; j < runLength; j++, state->relocAddr += 8)
                            {
                                ADD_TO_ADDRESS(state->relocAddr, state->sectionD);
                                // No write to offset 4
                            }
                            break;
                        case 0x25:
                            for (uint8_t j = 0; j < runLength; j++, state->relocAddr += 4, state->importIndex++)
                            {
                                ADD_TO_ADDRESS(state->relocAddr, state->importIndex); //cstodo hm
                            }
                            break;
                    }
                }
                break;
            case 0x30:
                {
                    uint16_t index = ((block[0] & 0x01) << 8) | block[1];
                    switch (opcode)
                    {
                        case 0x30:
                            OSWriteBigInt32(pSection->data, state->relocAddr, index);
                            state->importIndex = index + 1;
                            state->relocAddr += 4;
                            break;
                        case 0x31:
                            state->sectionC = sections[index]->baseAddress;
                            break;
                        case 0x32:
                            state->sectionD = sections[index]->baseAddress;
                            break;
                        case 0x33:
                            ADD_TO_ADDRESS(state->relocAddr, sections[index]->baseAddress);
                            state->relocAddr += 4;
                            break;
                    }
                }
                break;
            case 0x40:
                {
                    if (opcode & 0x80)
                    {
                        uint8_t blockCount = (block[0] & 0x0F) + 1;
                        uint8_t repeatCount = block[1] + 1;
                        
                        uint32_t originalOffset = instructionOffset;
                        for (uint8_t j = 0, instructionOffset = originalOffset; j < repeatCount; j++)
                        {
                            instructionOffset -= blockCount * 2;
                            while (instructionOffset < originalOffset)
                            {
                                instructionOffset = DoOneRelocationInstruction(data, instructionOffset, state, pSection, sections);
                            }
                        }
                        instructionOffset = originalOffset;
                    }
                    else
                    {
                        uint16_t offset = (((block[0] & 0x0F) << 4) | block[1]) + 1;
                        state->relocAddr += offset;
                    }
                }
                break;
            case 0x50:
                {
                    opcode &= 0xFE; // clear the last bit
                    switch (opcode)
                    {
                        case 0x50:
                            {
                                uint32_t value = OSReadBigInt32(data, instructionOffset) & 0x03FFFFFF;
                                state->relocAddr = value;
                            }
                            break;
                        case 0x52:
                            {
                                uint32_t value = OSReadBigInt32(data, instructionOffset) & 0x03FFFFFF;
                                OSWriteBigInt32(pSection->data, state->relocAddr, value);
                            }
                            break;
                        case 0x58: printf("DEBUG: unproccessed relocation instruction kPEFRelocLgRepeat\n"); break;
                        case 0x5A: printf("DEBUG: unproccessed relocation instruction kPEFRelocSetOrBySection\n"); break;
                    }
                }
                instructionSize = 4;
                break;
        }
    }

    return instructionOffset + instructionSize;
}

void ProcessRelocationArea(uint8_t* data, uint32_t sectionCount, uint32_t headerOffset, uint32_t areaOffset, Section** sections)
{
    for (uint32_t section = 0; section < sectionCount; section++)
    {
        uint32_t thisOffset = headerOffset + (section * 12);
        uint16_t affectedSection    = OSReadBigInt16(data, thisOffset);
        uint32_t relocCount         = OSReadBigInt32(data, thisOffset + 4);
        uint32_t firstRelocOffset   = OSReadBigInt32(data, thisOffset + 8);

        Section* pSection = sections[affectedSection];
        printf("\t\tGoing to read %d relocation instruction blocks for section %d with type %d\n", relocCount, affectedSection, pSection->type);

        uint32_t instructionOffset = areaOffset + firstRelocOffset;
        uint32_t endOffset = instructionOffset + (relocCount * 2);

        RelocationState state;
        state.relocAddr = 0;
        state.importIndex = 0;
        state.sectionC = sections[0]->baseAddress;
        state.sectionD = sections[1]->baseAddress;
        
        while (instructionOffset < endOffset)
        {
            instructionOffset = DoOneRelocationInstruction(data, instructionOffset, &state, pSection, sections);
        }

        FILE* out = fopen("relocdump.dat", "w");
        fwrite(pSection->data, pSection->totalLength, 1, out);
        fclose(out);
    }
}

#endif
