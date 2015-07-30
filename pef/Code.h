#ifndef __PEF_CODE_H__
#define __PEF_CODE_H__

#include "Structures.h"
#include "Instructions.h"
#include "Opcodes.h"

void ProcessCodeSection(struct SectionData* pSection, struct LoaderSection* pLoader)
{
    uint8_t inst[4];
    uint32_t mainInstruction = (pSection->id == pLoader->mainSection) ? pLoader->mainOffset / 4 : 0xFFFFFFFF;

    uint32_t instructionCount = pSection->length / 4;
    struct CodeInstruction** instructions = malloc(sizeof(struct CodeInstruction) * instructionCount);
    struct CodeLabel** allLabels = malloc(sizeof(struct CodeLabel) * instructionCount);
    uint32_t labelCount = 0;

    for (uint32_t i = 0, j = 0, k = 0; i < pSection->length; i += 4, j++)
    {
        instructions[j] = CreateInstruction(i, pSection->data + i);
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

    printf("\nCode section %d disassembly:\n", pSection->id);
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
