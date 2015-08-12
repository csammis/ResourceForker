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
} Instruction;

Instruction* CreateInstruction(uint32_t address, uint8_t* data)
{
    Instruction* pNew = malloc(sizeof(Instruction));
    memset(pNew->opcode, 0, INSTRUCTION_NAME_SIZE);
    memset(pNew->params, 0, INSTRUCTION_PARAM_SIZE);
    pNew->address = address;
    memcpy(pNew->raw, data, 4);
    return pNew;
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
