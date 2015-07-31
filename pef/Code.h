#ifndef __PEF_CODE_H__
#define __PEF_CODE_H__

#include "Structures.h"
#include "Instructions.h"
#include "Opcodes.h"

uint8_t RESTORE_R2_AFTER_GLUE[4] = { 0x80, 0x41, 0x00, 0x14 }; // lwz r2, 20(r1)

uint8_t GLUE_PATTERN[5][4] = {
    { 0x90, 0x41, 0x00, 0x14 }, // stw r2, 20(r1)
    { 0x80, 0x0c, 0x00, 0x00 }, // lwz r0, 0(r12)
    { 0x80, 0x4c, 0x00, 0x04 }, // lwz r2, 4(r12)
    { 0x7c, 0x09, 0x03, 0xa6 }, // mtspr CTR, r0
    { 0x4e, 0x80, 0x04, 0x20 }  // bcctr 20, 0, 0
};

bool IsPatternJumpToGlue(struct CodeInstruction** instructions, uint32_t i, uint32_t instructionCount)
{
    if (i + 1 >= instructionCount)
    {
        return false;
    }

    uint8_t* raw = instructions[i]->raw;
    return raw[0] == 0x48 && raw[1] == 0x00 && ((raw[3] & 0x01) == 0x01) && // bl <dest>
        memcmp(instructions[i + 1]->raw, RESTORE_R2_AFTER_GLUE, 4) == 0;
}

bool IsPatternGlue(struct CodeInstruction** instructions, uint32_t i, uint32_t instructionCount)
{
    if (i + 5 >= instructionCount)
    {
        return false;
    }

    for (uint8_t j = 0; j < 5; j++)
    {
        if (memcmp(instructions[i + j + 1]->raw, GLUE_PATTERN[j], 4) != 0)
        {
            return false;
        }
    }

    return true;
}

void GuessThePattern(struct CodeInstruction** instructions, uint32_t i, uint32_t instructionCount, struct SectionData* pDataSection, struct LoaderSection* pLoader)
{
    if (IsPatternJumpToGlue(instructions, i, instructionCount))
    {
        uint32_t target = OSReadBigInt32(instructions[i]->raw, 0);
        target = (target & 0x03FFFFFC) >> 2;
        int64_t value = S64_EXT_24((target << 2)) + (i * 4);

        uint32_t glueIndex = value / 4;
        if (IsPatternGlue(instructions, glueIndex, instructionCount))
        {
            uint16_t gluevalue = OSReadBigInt16(instructions[glueIndex]->raw, 2);
            value = S64_EXT_16(gluevalue);
            uint32_t symbol = OSReadBigInt32(pDataSection->data, value);
            uint32_t symbolTableEntry = OSReadBigInt32(pLoader->data, pLoader->importSymbolTableOffset + (symbol * 4));
            char* symbolName = (char*)(pLoader->data + pLoader->loaderStringOffset + (symbolTableEntry & 0x00FFFFFF));
            snprintf(instructions[i]->params + strlen(instructions[i]->params), 26 + strlen(symbolName), " Glue to symbol index %d %s", symbol, symbolName);
        }
    }
    else if (IsPatternGlue(instructions, i, instructionCount))
    {
        uint16_t value = OSReadBigInt16(instructions[i]->raw, 2);
        int64_t signextvalue = S64_EXT_16(value);
        uint32_t dataword = OSReadBigInt32(pDataSection->data, signextvalue);
        printf("Looks like glue pointing to offset %lld in data area, containing %d\n", signextvalue, dataword);
    }
}

void ProcessCodeSection(struct SectionData* pCodeSection, struct SectionData* pDataSection, struct LoaderSection* pLoader)
{
    uint8_t inst[4];
    uint32_t mainInstruction = (pCodeSection->id == pLoader->mainSection) ? pLoader->mainOffset / 4 : 0xFFFFFFFF;

    uint32_t instructionCount = pCodeSection->length / 4;
    struct CodeInstruction** instructions = malloc(sizeof(struct CodeInstruction) * instructionCount);
    struct CodeLabel** allLabels = malloc(sizeof(struct CodeLabel) * instructionCount);
    uint32_t labelCount = 0;

    for (uint32_t i = 0, j = 0, k = 0; i < pCodeSection->length; i += 4, j++)
    {
        instructions[j] = CreateInstruction(i, pCodeSection->data + i);
        PrintOpcode(instructions[j], &allLabels[j]);

        if (allLabels[j] != NULL)
        {
            labelCount++;
        }
    }

    // Compress the label array
    struct CodeLabel** labels = malloc(sizeof(struct CodeLabel) * labelCount);
    for (uint32_t i = 0, j = 0; i < instructionCount; i++)
    {
        if (allLabels[i] != NULL)
        {
            labels[j++] = allLabels[i];
        }
    }

    printf("\nCode section %d disassembly:\n", pCodeSection->id);
    for (uint32_t i = 0, addr = 0; i < instructionCount; i++, addr += 4)
    {
        struct CodeInstruction* instr = instructions[i];
        struct CodeLabel* label = NULL;

        if (i == mainInstruction)
        {
            printf("\n%016x <main>:\n", instr->address);
        }

        for (uint32_t j = 0; j < labelCount; j++)
        {
            if (labels[j]->address == addr)
            {
                printf("\n%016x %s:\n", labels[j]->address, labels[j]->name);
                break;
            }
        }

        GuessThePattern(instructions, i, instructionCount, pDataSection, pLoader);

        printf("%8x:\t%02x %02x %02x %02x\t\t", instr->address, instr->raw[0], instr->raw[1], instr->raw[2], instr->raw[3]);
        printf("%-7s\t%s\n", instr->opcode, instr->params);
        free(instr);
    }

    for (uint32_t i = 0; i < labelCount; i++)
    {
        free(labels[i]);
    }
    free(instructions);
    free(allLabels);
    free(labels);
}


#endif
