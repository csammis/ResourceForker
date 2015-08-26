#ifndef __PEF_INSTRUCTIONS_H__
#define __PEF_INSTRUCTIONS_H__

#define LABEL_NAME_SIZE 128
typedef struct _CodeLabel
{
    uint32_t address;
    char name[LABEL_NAME_SIZE];
} Label;

#define INSTRUCTION_NAME_SIZE 10
#define INSTRUCTION_PARAM_SIZE 128
typedef struct _CodeInstruction
{
    uint32_t address;
    uint8_t raw[4];
    char opcode[INSTRUCTION_NAME_SIZE];
    char params[INSTRUCTION_PARAM_SIZE];
    char* pExtraInfo;
} Instruction;

Instruction* CreateInstruction(uint32_t address, uint8_t* data)
{
    Instruction* pNew = malloc(sizeof(Instruction));
    memset(pNew->opcode, 0, INSTRUCTION_NAME_SIZE);
    memset(pNew->params, 0, INSTRUCTION_PARAM_SIZE);
    pNew->address = address;
    memcpy(pNew->raw, data, 4);
    pNew->pExtraInfo = NULL;
    return pNew;
}

void PrintInstruction(Instruction* instr)
{
    printf("%8x:\t%02x %02x %02x %02x\t\t", instr->address, instr->raw[0], instr->raw[1], instr->raw[2], instr->raw[3]);
    printf("%-7s\t%s", instr->opcode, instr->params);
    if (instr->pExtraInfo)
        printf("\t\t\t%s", instr->pExtraInfo);
    printf("\n");
}

void FreeInstruction(Instruction* instr)
{
    if (instr)
    {
        if (instr->pExtraInfo) free(instr->pExtraInfo);
        free(instr);
    }
}

Label* CreateLabel(uint32_t destination)
{
    Label* pNew = malloc(sizeof(Label));
    memset(pNew->name, 0, LABEL_NAME_SIZE);
    pNew->address = destination;
    snprintf(pNew->name, LABEL_NAME_SIZE, "<%x_dest>", destination);
    return pNew;
}

#endif
