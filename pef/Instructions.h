#ifndef __PEF_INSTRUCTIONS_H__
#define __PEF_INSTRUCTIONS_H__

#define LABEL_NAME_SIZE 128
struct CodeLabel
{
    uint32_t address;
    char name[LABEL_NAME_SIZE];
    struct CodeLabel* nextLabel;
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

struct CodeLabel* CreateLabelAtTail(uint32_t destination, struct CodeLabel** pHead, struct CodeLabel** pTail)
{
    struct CodeLabel* pNew = malloc(sizeof(struct CodeLabel));
    memset(pNew->name, 0, LABEL_NAME_SIZE);
    pNew->address = destination;
    pNew->nextLabel = NULL;
    snprintf(pNew->name, LABEL_NAME_SIZE, "<%x_dest>", destination);

    if (*pHead == NULL)
    {
        *pHead = pNew;
        *pTail = pNew;
    }
    else
    {
        (*pTail)->nextLabel = pNew;
        *pTail = pNew;
    }

    return pNew;
}

void FreeLabels(struct CodeLabel* pHead)
{
    struct CodeLabel* iter = pHead;
    while (iter)
    {
        struct CodeLabel* tmp = iter;
        iter = iter->nextLabel;
        free(tmp);
    }
}

#endif
