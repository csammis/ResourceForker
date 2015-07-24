#ifndef __PEF_INSTRUCTIONS_H__
#define __PEF_INSTRUCTIONS_H__

#define LABEL_NAME_SIZE 128
struct CodeLabel
{
    uint32_t address;
    char name[LABEL_NAME_SIZE];
};

#define INSTRUCTION_NAME_SIZE 10
#define INSTRUCTION_PARAM_SIZE 128

struct CodeInstruction
{
    uint32_t address;
    uint8_t raw[4];
    char opcode[INSTRUCTION_NAME_SIZE];
    char params[INSTRUCTION_PARAM_SIZE];
};

struct CodeInstruction* CreateInstruction(uint32_t address, uint8_t* data)
{
    struct CodeInstruction* pNew = malloc(sizeof(struct CodeInstruction));
    memset(pNew->opcode, 0, INSTRUCTION_NAME_SIZE);
    memset(pNew->params, 0, INSTRUCTION_PARAM_SIZE);
    pNew->address = address;
    memcpy(pNew->raw, data, 4);
    return pNew;
}

struct CodeLabel* CreateLabel(uint32_t destination)
{
    struct CodeLabel* pNew = malloc(sizeof(struct CodeLabel));
    memset(pNew->name, 0, LABEL_NAME_SIZE);
    pNew->address = destination;
    snprintf(pNew->name, LABEL_NAME_SIZE, "<%x_dest>", destination);
    return pNew;
}

#endif
