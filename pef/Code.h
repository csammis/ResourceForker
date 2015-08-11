#ifndef __PEF_CODE_H__
#define __PEF_CODE_H__

#include "Structures.h"
#include "Instructions.h"
#include "Opcodes.h"
#include "Loader.h"

uint8_t RESTORE_R2_AFTER_GLUE[4] = { 0x80, 0x41, 0x00, 0x14 }; // lwz r2, 20(r1)
uint8_t BEGIN_PROLOGUE[4]        = { 0x7c, 0x08, 0x02, 0xa6 }; // mfspr LR, r0
uint8_t BEGIN_EPILOGUE_1[2]      = { 0x80, 0x01 };             // lwz r0, x(r1)
uint8_t BEGIN_EPILOGUE_2[2]      = { 0x38, 0x21 };             // addi r1, x(r1)
uint8_t BEGIN_EPILOGUE_3[4]      = { 0x7c, 0x08, 0x03, 0xa6 }; // mtspr LR, r0
uint8_t END_EPILOGUE[4]          = { 0x4e, 0x80, 0x00, 0x20 }; // bclr 20, 0, 0

uint8_t GLUE_PATTERN[5][4] = {
    { 0x90, 0x41, 0x00, 0x14 }, // stw r2, 20(r1)
    { 0x80, 0x0c, 0x00, 0x00 }, // lwz r0, 0(r12)
    { 0x80, 0x4c, 0x00, 0x04 }, // lwz r2, 4(r12)
    { 0x7c, 0x09, 0x03, 0xa6 }, // mtspr CTR, r0
    { 0x4e, 0x80, 0x04, 0x20 }  // bcctr 20, 0, 0
};

typedef struct _PatternState
{
    bool inPrologue;
    uint32_t prologueEnd;
    bool inEpilogue;
    uint32_t epilogueEnd;
    bool inSubroutine;
} PatternState;

char* FindSymbolNameFromGlue(struct SectionData* dataSection, int64_t offset, struct LoaderSection* loaderSection)
{
    uint32_t symbol = OSReadBigInt32(dataSection->data, offset);
    uint32_t symbolTableEntry = OSReadBigInt32(loaderSection->data, loaderSection->importSymbolTableOffset + (symbol * 4));
    return (char*)(loaderSection->data + loaderSection->loaderStringOffset + (symbolTableEntry & 0x00FFFFFF));
}

bool IsPatternEpilogue(struct CodeInstruction** instructions, uint32_t i, uint32_t instructionCount, PatternState* pState)
{
    if (memcmp(instructions[i]->raw, BEGIN_EPILOGUE_1, 2) != 0)
    {
        return false;
    }

    if (memcmp(instructions[++i]->raw, BEGIN_EPILOGUE_2, 2) != 0)
    {
        return false;
    }

    if (memcmp(instructions[++i]->raw, BEGIN_EPILOGUE_3, 4) != 0)
    {
        return false;
    }

    while (++i < instructionCount)
    {
        uint8_t* raw = instructions[i]->raw;
        uint8_t opcode = READ_OPCODE(raw);

        if (opcode == 46 || opcode == 32)
        {
            // lmw or lwz x, x(r1)
            if ((raw[1] & 0x1F) != 1) break;
        }
        else if (memcmp(raw, END_EPILOGUE, 4) == 0)
        {
            pState->inEpilogue = true;
            pState->epilogueEnd = i;
            break;
        }
        else break;
    }
    return pState->inEpilogue;
}

bool IsPatternPrologue(struct CodeInstruction** instructions, uint32_t i, uint32_t instructionCount, PatternState* pState)
{
    if (memcmp(instructions[i]->raw, BEGIN_PROLOGUE, 4) != 0)
    {
        return false;
    }

    while (++i < instructionCount)
    {
        uint8_t* raw = instructions[i]->raw;
        uint8_t opcode = READ_OPCODE(raw);
        if (opcode == 47 || opcode == 36)
        {
            // stmw or stw x, x(r1)
            if ((raw[1] & 0x1F) != 1) break;
        }
        else if (opcode == 37)
        {
            // stwu r1, x(r1) signals the end of a prologue
            uint8_t rs = ((raw[0] & 0x03) << 3) | ((raw[1] & 0xE0) >> 5);
            uint8_t ra = (raw[1] & 0x1F);
            if (rs == 1 && ra == 1)
            {
                pState->inPrologue = true;
                pState->inSubroutine = true;
                pState->prologueEnd = i;
                break;
            }
            else break;
        }
        else break;
    }

    return pState->inPrologue;
}

bool IsPatternJumpToGlue(struct CodeInstruction** instructions, uint32_t i, uint32_t instructionCount)
{
    if (i + 1 >= instructionCount)
    {
        return false;
    }

    uint8_t* raw = instructions[i]->raw;
    return READ_OPCODE(raw) == 18 && ((raw[3] & 0x01) == 0x01) &&        // bl <dest>
        memcmp(instructions[i + 1]->raw, RESTORE_R2_AFTER_GLUE, 4) == 0; // lwz r2, 20(r1)
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

bool PrintLabelAtAddress(struct CodeLabel** labels, uint32_t labelCount, uint64_t address)
{
    for (uint32_t j = 0; j < labelCount; j++)
    {
        if (labels[j]->address == address)
        {
            printf("%016x %s:", labels[j]->address, labels[j]->name);
            return true;
        }
    }
    return false;
}

void PrintInstruction(struct CodeInstruction* instr)
{
    printf("%8x:\t%02x %02x %02x %02x\t\t", instr->address, instr->raw[0], instr->raw[1], instr->raw[2], instr->raw[3]);
    printf("%-7s\t%s\n", instr->opcode, instr->params);
}

void AnnotateInstruction(struct CodeInstruction** instructions, uint32_t i, uint32_t instructionCount, struct SectionData* pDataSection, struct LoaderSection* pLoader,
        struct CodeLabel** labels, uint32_t labelCount, PatternState* pState)
{
    // Check state to see what we're doing now
    if (pState->inPrologue)
    {
        if (i < pState->prologueEnd)
            printf("||");
        else
        {
            printf("|\\");
            pState->inPrologue = false;
        }
    }
    else if (pState->inEpilogue)
    {
        if (i != pState->epilogueEnd)
            printf("||");
        else
        {
            printf(" \\");
            PrintInstruction(instructions[i]);
            printf("\n");
            pState->inEpilogue = false;
            pState->inSubroutine = false;
            return;
        }
    }
    else if (pState->inSubroutine)
    {
        printf("|");
    }

    if (IsPatternJumpToGlue(instructions, i, instructionCount))
    {
        uint32_t target = OSReadBigInt32(instructions[i]->raw, 0);
        target = (target & 0x03FFFFFC) >> 2;
        int64_t value = S64_EXT_24((target << 2)) + (i * 4);

        uint32_t glueIndex = value / 4;
        if (IsPatternGlue(instructions, glueIndex, instructionCount))
        {
            int64_t signextvalue = GetSignExtValueFromDForm(instructions[glueIndex]->raw);
            char* symbolName = FindSymbolNameFromGlue(pDataSection, signextvalue, pLoader);
            snprintf(instructions[i]->params + strlen(instructions[i]->params), 26 + strlen(symbolName), " Glue to symbol %s", symbolName);
        }
    }
    else if (IsPatternGlue(instructions, i, instructionCount))
    {
        int64_t signextvalue = GetSignExtValueFromDForm(instructions[i]->raw);
        uint32_t dataword = OSReadBigInt32(pDataSection->data, signextvalue);
        printf("Looks like glue pointing to offset %lld in data area, containing %d (%s)\n",
                signextvalue, dataword, FindSymbolNameFromGlue(pDataSection, signextvalue, pLoader));
    }
    else if (IsPatternPrologue(instructions, i, instructionCount, pState))
    {
        printf("\nFunction Start ");
        PrintLabelAtAddress(labels, labelCount, i * 4);
        printf("\n");
        printf(" /");
    }
    else if (IsPatternEpilogue(instructions, i, instructionCount, pState))
    {
        printf("/");
    }
    else
    {
        if (PrintLabelAtAddress(labels, labelCount, i * 4))
        {
            printf("\n");
            if (pState->inSubroutine)
                printf("|");
        }
    }

    PrintInstruction(instructions[i]);
}

uint32_t CountBranchInstructions(uint8_t* instructions, uint32_t length)
{
    uint32_t retval = 0;
    for (uint32_t i = 0; i < length; i += 4)
    {
        uint8_t opcode = READ_OPCODE((instructions + i));
        if (opcode == 16 || opcode == 18)
        {
            retval++;
        }
    }
    return retval;
}

void ProcessCodeSection(struct SectionData* pCodeSection, struct SectionData* pDataSection, struct LoaderSection* pLoader)
{
    uint32_t instructionCount = pCodeSection->length / 4;
    struct CodeInstruction** instructions = malloc(sizeof(struct CodeInstruction) * instructionCount);

    uint32_t labelCount = CountBranchInstructions(pCodeSection->data, pCodeSection->length);
    if (pCodeSection->id == pLoader->mainSection)
    {
        labelCount++;
    }
    struct CodeLabel** labels = malloc(sizeof(struct CodeLabel) * labelCount);
    uint32_t currentLabel = 0;
    if (pCodeSection->id == pLoader->mainSection)
    {
        labels[currentLabel] = CreateLabel(pLoader->mainOffset);
        memset(labels[currentLabel]->name, 0, LABEL_NAME_SIZE);
        snprintf(labels[currentLabel]->name, 7, "<main>");
        currentLabel++;
    }

    bool isBranchInstruction = false;
    for (uint32_t i = 0, j = 0; i < pCodeSection->length; i += 4, j++)
    {
        instructions[j] = CreateInstruction(i, pCodeSection->data + i);
        PrintOpcode(instructions[j], &labels[currentLabel], &isBranchInstruction);
        if (isBranchInstruction)
        {
            currentLabel++;
        }
    }

    printf("\nCode section %d disassembly:\n", pCodeSection->id);
    PatternState state;
    state.inPrologue = state.inEpilogue = state.inSubroutine = false;
    for (uint32_t i = 0, addr = 0; i < instructionCount; i++, addr += 4)
    {
        AnnotateInstruction(instructions, i, instructionCount, pDataSection, pLoader, labels, labelCount, &state);
        free(instructions[i]);
    }

    for (uint32_t i = 0; i < labelCount; i++)
    {
        free(labels[i]);
    }
    free(labels);
    free(instructions);
}


#endif
