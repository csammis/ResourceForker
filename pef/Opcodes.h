#ifndef __PEF_OPCODES_H__
#define __PEF_OPCODES_H__

#include <stdbool.h>
#include "Instructions.h"

char NO_OP[4] = { 0x60, 0x00, 0x00, 0x00 };

#define READ_OPCODE(x) ((x[0] & 0xFC) >> 2)

#define INSTR_BUFFER_APPEND(x) snprintf(instrBuffer + strlen(instrBuffer), INSTRUCTION_NAME_SIZE - strlen(instrBuffer), x)

#define S64_EXT_16(v) (int64_t)(v & 0x8000 ? v | 0xFFFFFFFFFFFF0000 : v);
#define S64_EXT_24(v) (int64_t)(v & 0x2000000 ? v | 0xFFFFFFFFFC000000 : v)

#define CASE_PRINT(x, y) case x: snprintf(instrBuffer, INSTRUCTION_NAME_SIZE, "%s", #y);

#define A_FORM PrintAForm(inst, opcode, instrBuffer, paramBuffer)
#define B_FORM PrintBForm(inst, opcode, currentAddress, instrBuffer, paramBuffer, currentLabel)
#define D_FORM PrintDForm(inst, opcode, pInstruction)
#define DS_FORM PrintDSForm(inst, opcode, instrBuffer, paramBuffer)
#define I_FORM PrintIForm(inst, opcode, currentAddress, instrBuffer, paramBuffer, currentLabel)
#define M_FORM PrintMForm(inst, opcode, instrBuffer, paramBuffer)
#define MD_FORM PrintMDForm(inst, opcode, instrBuffer, paramBuffer)
#define X_FORM PrintXForm(inst, opcode, extOpcode, instrBuffer, paramBuffer)
#define XL_FORM PrintXLForm(inst, opcode, extOpcode, pInstruction)
#define XFX_FORM PrintXFXForm(inst, opcode, extOpcode, pInstruction)
#define XO_FORM PrintXOForm(inst, opcode, extOpcode, pInstruction)
#define Q_FORM snprintf(pInstruction->params, 2, "?")

void PrintXForm(uint8_t* inst, uint8_t opcode, uint16_t extOpcode, char* instrBuffer, char* paramBuffer)
{
    uint8_t rst = ((inst[0] & 0x03) << 3) | ((inst[1] & 0xE0) >> 5);
    uint8_t ra = (inst[1] & 0x1F);
    uint8_t rb = (inst[2] & 0xF8) >> 3;

    if (inst[3] & 0x01) INSTR_BUFFER_APPEND(".");

    switch (extOpcode)
    {
        case 4:
        case 20:
        case 54:
        case 68:
        case 83:
        case 84:
        case 86:
        case 146:
        case 150:
        case 178:
        case 210:
        case 214:
        case 242:
        case 246:
        case 278:
        case 306:
        case 310:
        case 370:
        case 402:
        case 434:
        case 438:
        case 498:
        case 512:
        case 535:
        case 566:
        case 567:
        case 595:
        case 598:
        case 659:
        case 663:
        case 695:
        case 851:
        case 854:
        case 915:
        case 982:
        case 983:
        case 1014:
            printf("\tDEBUG: Recognized but unimplemented extended opcode %d for opcode 31", extOpcode);
            break;
        case 599:
        case 631:
        case 727:
        case 759:
            snprintf(paramBuffer, INSTRUCTION_PARAM_SIZE, "fpr%d, r%d, r%d", rst, ra, rb);
            break;
        case 24:
        case 27:
        case 28:
        case 60:
        case 124:
        case 144:
        case 284:
        case 316:
        case 412:
        case 444:
        case 476:
        case 536:
        case 539:
        case 792:
        case 794:
            snprintf(paramBuffer, INSTRUCTION_PARAM_SIZE, "r%d, r%d, r%d", ra, rst, rb);
            break;
        case 0:
        case 32:
            {
                uint8_t bf = ((rst & 0xFC) >> 2);
                uint8_t l = rst & 0x01;
                snprintf(paramBuffer, INSTRUCTION_PARAM_SIZE, "%d, %d, r%d, r%d", bf, l, ra, rb);
            }
            break;
        case 26:
        case 58:
        case 122:
        case 922:
        case 954:
        case 986:
            snprintf(paramBuffer, INSTRUCTION_PARAM_SIZE, "r%d, r%d", ra, rst);
            break;
        case 21:
        case 23:
        case 53:
        case 55:
        case 87:
        case 119:
        case 149:
        case 151:
        case 181:
        case 183:
        case 215:
        case 247:
        case 279:
        case 311:
        case 341:
        case 343:
        case 373:
        case 375:
        case 407:
        case 439:
        case 533:
        case 534:
        case 661:
        case 662:
        case 790:
        case 918:
            snprintf(paramBuffer, INSTRUCTION_PARAM_SIZE, "r%d, r%d, r%d", rst, ra, rb);
            break;
        case 597:
        case 725:
            snprintf(paramBuffer, INSTRUCTION_PARAM_SIZE, "r%d, r%d, %d", rst, ra, rb);
            break;
        case 824:
            snprintf(paramBuffer, INSTRUCTION_PARAM_SIZE, "r%d, r%d, %d", ra, rst, rb);
            break;
        default: printf("\tDEBUG: unrecognized extended opcode %d for opcode 31", extOpcode);
    }
}

void PrintIForm(uint8_t* inst, uint8_t opcode, uint32_t currentAddress, char* instrBuffer, char* paramBuffer,
         Label** currentLabel)
{
    uint32_t target = OSReadBigInt32(inst, 0);
    target = ((target & 0x03FFFFFC) >> 2) << 2;
    int64_t value = S64_EXT_24(target);

    if (inst[3] & 0x01) INSTR_BUFFER_APPEND("l");
    if (inst[3] & 0x02)
    {
        INSTR_BUFFER_APPEND("a");
        currentAddress = 0; // Don't use the offset of the instruction as a base
    }

    int64_t targetAddress = value + currentAddress;

    snprintf(paramBuffer, INSTRUCTION_PARAM_SIZE, "%lld <%llx_dest>", value, targetAddress);
    *currentLabel = CreateLabel(targetAddress);
}

void PrintBForm(uint8_t* inst, uint8_t opcode, uint32_t currentAddress, char* instrBuffer, char* paramBuffer,
         Label** currentLabel)
{
    uint8_t bo = ((inst[0] & 0x03) << 3) | ((inst[1] & 0xE0) >> 5);
    uint8_t bi = (inst[1] & 0x1F);
    uint16_t target = ((OSReadBigInt16(inst, 2) & 0xFFFC) >> 2) << 2;
    int64_t value = S64_EXT_16(target);

    if (inst[3] & 0x01) INSTR_BUFFER_APPEND("l");
    if (inst[3] & 0x02)
    {
        INSTR_BUFFER_APPEND("a");
        currentAddress = 0;
    }

    int64_t targetAddress = value + currentAddress;
    snprintf(paramBuffer, INSTRUCTION_PARAM_SIZE, "%d, %d, %lld <%llx_dest>", bo, bi, value, targetAddress);
    *currentLabel = CreateLabel(targetAddress);
}

void PrintAForm(uint8_t* inst, uint8_t opcode, char* instrBuffer, char* paramBuffer)
{
    uint8_t rst = ((inst[0] & 0x03) << 3) | ((inst[1] & 0xE0) >> 5);
    uint8_t ra = (inst[1] & 0x1F);
    uint8_t rb = (inst[2] & 0xF8) >> 3;
    uint8_t rc = ((inst[2] & 0x07) << 2) | ((inst[3] & 0xC0) >> 6);
    uint8_t xo = (inst[3] & 0x3E) >> 1;

    switch (xo)
    {
        CASE_PRINT(18, fdiv); break;
        CASE_PRINT(20, fsub); break;
        CASE_PRINT(21, fadd); break;
        CASE_PRINT(22, fsqrt); break;
        CASE_PRINT(23, fsel); break;
        CASE_PRINT(24, fre); break;
        CASE_PRINT(25, fmul); break;
        CASE_PRINT(26, frsqrte); break;
        CASE_PRINT(28, fmsub); break;
        CASE_PRINT(29, fmadd); break;
        CASE_PRINT(30, fnmsub); break;
        CASE_PRINT(31, fnmadd); break;
    }

    if (opcode == 59) INSTR_BUFFER_APPEND("s");
    if (inst[3] & 0x0) INSTR_BUFFER_APPEND(".");

    switch (xo)
    {
        case 18:
        case 20:
        case 21:
            snprintf(paramBuffer, INSTRUCTION_PARAM_SIZE, "fpr%d, fpr%d, fpr%d", rst, ra, rb);
            break;
        case 25:
            snprintf(paramBuffer, INSTRUCTION_PARAM_SIZE, "fpr%d, fpr%d, fpr%d", rst, ra, rc);
            break;
        case 23:
        case 28:
        case 29:
        case 30:
        case 31:
            snprintf(paramBuffer, INSTRUCTION_PARAM_SIZE, "fpr%d, fpr%d, fpr%d, fpr%d", rst, ra, rc, rb);
            break;
        case 22:
        case 24:
        case 26:
            snprintf(paramBuffer, INSTRUCTION_PARAM_SIZE, "fpr%d, fpr%d", rst, rb);
            break;
        default: printf("\tDEBUG: unrecognized extended opcode %d for opcode %d", xo, opcode);
    }
}

void PrintXOForm(uint8_t* inst, uint8_t opcode, uint16_t extOpcode, Instruction* pInstruction)
{
    uint8_t rst = ((inst[0] & 0x03) << 3) | ((inst[1] & 0xE0) >> 5);
    uint8_t ra = (inst[1] & 0x1F);
    uint8_t rb = (inst[2] & 0xF8) >> 3;

    char* instrBuffer = pInstruction->opcode;

    if (inst[2] & 0x04) INSTR_BUFFER_APPEND("o");
    if (inst[3] & 0x01) INSTR_BUFFER_APPEND(".");

    switch (extOpcode)
    {
        case 8:
        case 9:
        case 10:
        case 11:
        case 40:
        case 73:
        case 75:
        case 136:
        case 138:
        case 233:
        case 235:
        case 266:
        case 457:
        case 459:
        case 489:
        case 491:
            snprintf(pInstruction->params, INSTRUCTION_PARAM_SIZE, "r%d, r%d, r%d", rst, ra, rb);
            break;
        case 104:
        case 200:
        case 202:
        case 232:
        case 234:
            snprintf(pInstruction->params, INSTRUCTION_PARAM_SIZE, "r%d, r%d", rst, ra);
            break;
        default: printf("DEBUG: unrecognized extended opcode %d for XO-form instruction", extOpcode);
    }
}

void PrintXFXForm(uint8_t* inst, uint8_t opcode, uint16_t extOpcode, Instruction* pInstruction)
{
    uint8_t rs = ((inst[0] & 0x03) << 3) | ((inst[1] & 0xE0) >> 5);

    switch (extOpcode)
    {
        case 19:
            snprintf(pInstruction->params, INSTRUCTION_PARAM_SIZE, "r%d", rs);
            break;
        case 144:
            {
                uint8_t fxm = ((inst[1] & 0x0F) << 4) | ((inst[2] & 0xF0) >> 4);
                snprintf(pInstruction->params, INSTRUCTION_PARAM_SIZE, "%d, r%d", fxm, rs);
            }
            break;
        case 339:
        case 467:
            {
                uint16_t spr = (inst[1] & 0x1F);
                switch (spr)
                {
                    case 1:
                        snprintf(pInstruction->params, INSTRUCTION_PARAM_SIZE, "XER, r%d", rs);
                        break;
                    case 8:
                        snprintf(pInstruction->params, INSTRUCTION_PARAM_SIZE, "LR, r%d", rs);
                        break;
                    case 9:
                        snprintf(pInstruction->params, INSTRUCTION_PARAM_SIZE, "CTR, r%d", rs);
                        break;
                    default:
                        printf("\tDEBUG: unrecognized SPR %d for opcode %d", spr, opcode);
                }
            }
            break;
        default: printf("DEBUG: unrecognized extended opcode %d for XFX-form instruction", extOpcode);
    }
}

void PrintXFLForm(uint8_t* inst, uint8_t opcode, char* instrBuffer, char* paramBuffer)
{
    uint8_t flm = ((inst[0] & 0x01) << 7) | ((inst[1] & 0xFE) >> 1);
    uint8_t frb = (inst[2] & 0xF8) >> 3;

    snprintf(instrBuffer, INSTRUCTION_NAME_SIZE, "mtfsf");
    if (inst[3] & 0x01) INSTR_BUFFER_APPEND(".");
    snprintf(paramBuffer, INSTRUCTION_PARAM_SIZE, "%d, fpr%d", flm, frb);
}

void PrintXLForm(uint8_t* inst, uint8_t opcode, uint16_t extOpcode, Instruction* pInstruction)
{
    uint8_t bt = ((inst[0] & 0x03) << 3) | ((inst[1] & 0xE0) >> 5);
    uint8_t ba = (inst[1] & 0x1F);
    uint8_t bb = (inst[2] & 0xF8) >> 3;

    switch (extOpcode)
    {
        case 0:
            {
                uint8_t bf = (bt & 0x1C) >> 2;
                uint8_t bfa = (ba & 0x1E) >> 1;
                snprintf(pInstruction->params, INSTRUCTION_PARAM_SIZE, "%d, %d", bf, bfa);
            }
            break;
        case 33:
        case 129:
        case 193:
        case 225:
        case 257:
        case 289:
        case 417:
        case 449:
            snprintf(pInstruction->params, INSTRUCTION_PARAM_SIZE, "%d, %d, %d", bt, ba, bb);
            break;
        case 16:
        case 528:
            {
                uint8_t bh = (bb & 0x03);
                snprintf(pInstruction->params, INSTRUCTION_PARAM_SIZE, "%d, %d, %d", bt, ba, bh);
                if (extOpcode == 16 && bh == 0) // Instruction is a return from subroutine
                {
                    pInstruction->pExtraInfo = malloc(10);
                    snprintf(pInstruction->pExtraInfo, 10, "# return;");
                }
            }
            break;
        default: printf("\tDEBUG: unrecognized extended opcode %d for opcode %d", extOpcode, opcode);
    }
}

void PrintFloatingXForm(uint8_t* inst, uint8_t opcode, uint16_t extOpcode, char* instrBuffer, char* paramBuffer)
{
    uint8_t rst = ((inst[0] & 0x03) << 3) | ((inst[1] & 0xE0) >> 5);
    uint8_t frb = (inst[2] & 0xF8) >> 3;
    uint8_t bf = (rst & 0xFE) >> 1;
    uint8_t fra = (inst[1] & 0x1F);

    switch (extOpcode)
    {
        CASE_PRINT(0, fcmpu); break; 
        CASE_PRINT(12, frsp); break;
        CASE_PRINT(14, fctiw); break;
        CASE_PRINT(15, fctiwz); break;
        CASE_PRINT(32, fcmpo); break;
        CASE_PRINT(38, mtfsb1); break;
        CASE_PRINT(40, fneg); break;
        CASE_PRINT(64, mcrfs); break;
        CASE_PRINT(70, mtfsb0); break;
        CASE_PRINT(72, fmr); break;
        CASE_PRINT(134, mtfsfi); break;
        CASE_PRINT(136, fnabs); break;
        CASE_PRINT(264, fabs); break;
        CASE_PRINT(583, mffs); break;
        CASE_PRINT(814, fctid); break;
        CASE_PRINT(815, fctidz); break;
        CASE_PRINT(846, fcfid); break;
        default:
            printf("\tDEBUG: unrecognized extended opcode %d for opcode %d", extOpcode, opcode);
            return;
    }

    if (inst[3] & 0x01) INSTR_BUFFER_APPEND(".");

    switch (extOpcode)
    {
        case 12:
        case 14:
        case 15:
        case 40:
        case 72:
        case 136:
        case 264:
        case 814:
        case 815:
        case 846:
            snprintf(paramBuffer, INSTRUCTION_PARAM_SIZE, "fpr%d, fpr%d", rst, frb);
            break;
        case 0:
        case 32:
            snprintf(paramBuffer, INSTRUCTION_PARAM_SIZE, "%d, fpr%d, fpr%d", bf, fra, frb);
            break;
        case 64:
            {
                uint8_t bfa = (fra & 0xF0) >> 4;
                snprintf(paramBuffer, INSTRUCTION_PARAM_SIZE, "%d, %d", bf, bfa);
            }
            break;
        case 134:
            {
                uint8_t u = (frb & 0xFE) >> 1;
                snprintf(paramBuffer, INSTRUCTION_PARAM_SIZE, "%d, %d", bf, u);
            }
            break;
        case 583:
            snprintf(paramBuffer, INSTRUCTION_PARAM_SIZE, "fpr%d", rst);
            break;
        case 38:
        case 70:
            snprintf(paramBuffer, INSTRUCTION_PARAM_SIZE, "%d", rst);
            break;
    }
}

void PrintMForm(uint8_t* inst, uint8_t opcode, char* instrBuffer, char* paramBuffer)
{
    uint8_t rst = ((inst[0] & 0x03) << 3) | ((inst[1] & 0xE0) >> 5);
    uint8_t ra = (inst[1] & 0x1F);
    uint8_t sh = (inst[2] & 0xF8) >> 3;
    uint8_t mb = ((inst[2] & 0x07) << 2) | ((inst[3] & 0xC0) >> 6);
    uint8_t me = (inst[3] & 0x3E);

    uint64_t mask = 0;
    if (mb <= me)
    {
        int mlen = me - mb;
        do { mask = ((mask << 1) | 1); } while (mlen-- > 0);
        mask <<= mb;
    }

    if (inst[3] & 0x01) INSTR_BUFFER_APPEND(".");
    snprintf(paramBuffer, INSTRUCTION_PARAM_SIZE, "r%d, r%d, %d, 0x%016llx", rst, ra, sh, mask);
}

void PrintDSForm(uint8_t* inst, uint8_t opcode, char* instrBuffer, char* paramBuffer)
{
    uint8_t rst = ((inst[0] & 0x03) << 3) | ((inst[1] & 0xE0) >> 5);
    uint8_t ra = (inst[1] & 0x1F);
    uint8_t xo = (inst[3] & 0x03);
    uint16_t value = (OSReadBigInt16(inst, 2) & 0xFFFC);
    int64_t signextvalue = S64_EXT_16(value);

    if (opcode == 58)
    {
        switch (xo)
        {
            CASE_PRINT(0, ld);  break;
            CASE_PRINT(1, ldu); break;
            CASE_PRINT(2, lwa); break;
        }
        if (ra == 0 || rst == ra)
        {
            printf("\t** Invalid instruction form in opcode 58");
            return;
        }
    }
    else if (opcode == 62)
    {
        if (xo == 1) INSTR_BUFFER_APPEND("u");
        if (ra == 0)
        {
            printf("\t** Invalid instruction form in opcode 62");
            return;
        }
    }
    else if (opcode == 56 || opcode == 57 || opcode == 60 || opcode == 61)
    {
        if (xo != 0)
        {
            printf("\t** Invalid instruction form in opcode %d", opcode);
            return;
        }
    }
    else
    {
        printf("\tDEBUG: Called PrintDSForm with unexpected opcode %d", opcode);
        return;
    }
    snprintf(paramBuffer, INSTRUCTION_PARAM_SIZE, "r%d, %lld(r%d)", rst, signextvalue, ra);
}

uint16_t GetValueFromDForm(uint8_t* inst)
{
    return OSReadBigInt16(inst, 2);
}

int64_t GetSignExtValueFromDForm(uint8_t* inst)
{
    uint16_t value = GetValueFromDForm(inst);
    return S64_EXT_16(value);
}

void PrintDForm(uint8_t* inst, uint8_t opcode, Instruction* pInstruction)
{
    uint8_t rst = ((inst[0] & 0x03) << 3) | ((inst[1] & 0xE0) >> 5);
    uint8_t ra = (inst[1] & 0x1F);

    switch (opcode)
    {
        case 2:
        case 3:
            snprintf(pInstruction->params, INSTRUCTION_PARAM_SIZE, "%d, r%d, %lld", rst, ra, GetSignExtValueFromDForm(inst));
            break;
        case 10:
        case 11:
            {
                uint8_t bf = rst >> 2;
                uint8_t l = rst & 0x01;
                if (opcode == 11)
                    snprintf(pInstruction->params, INSTRUCTION_PARAM_SIZE, "%d, %d, r%d, %lld", bf, l, ra, GetSignExtValueFromDForm(inst));
                else
                    snprintf(pInstruction->params, INSTRUCTION_PARAM_SIZE, "%d, %d, r%d, %u", bf, l, ra, GetValueFromDForm(inst));
            }
            break;
        case 14:
        case 15:
            if (ra == 0)
            {
                snprintf(pInstruction->params, INSTRUCTION_PARAM_SIZE, "r%d, 0, %lld", rst, GetSignExtValueFromDForm(inst));
                break;
            }
        case 7:
        case 8:
        case 12:
        case 13:
            snprintf(pInstruction->params, INSTRUCTION_PARAM_SIZE, "r%d, r%d, %lld", rst, ra, GetSignExtValueFromDForm(inst));
            break;
        case 24:
        case 25:
        case 26:
        case 27:
        case 28:
        case 29:
            snprintf(pInstruction->params, INSTRUCTION_PARAM_SIZE, "r%d, r%d, %d", ra, rst, GetValueFromDForm(inst));
            if (memcmp(inst, NO_OP, 4) == 0)
            {
                pInstruction->pExtraInfo = malloc(11);
                snprintf(pInstruction->pExtraInfo, 11, "# no-op();");
            }
            break;
        case 32:
        case 33:
        case 34:
        case 35:
        case 36:
        case 37:
        case 38:
        case 39:
        case 40:
        case 41:
        case 42:
        case 43:
        case 44:
        case 45:
        case 46:
        case 47:
            snprintf(pInstruction->params, INSTRUCTION_PARAM_SIZE, "r%d, %lld(r%d)", rst, GetSignExtValueFromDForm(inst), ra);
            break;
        case 48:
        case 49:
        case 50:
        case 51:
        case 52:
        case 53:
        case 54:
        case 55:
            snprintf(pInstruction->params, INSTRUCTION_PARAM_SIZE, "fpr%d, %lld(r%d)", rst, GetSignExtValueFromDForm(inst), ra);
            break;
        default:
            snprintf(pInstruction->params, INSTRUCTION_PARAM_SIZE, "DEBUG: unhandled D-form opcode %d", opcode);
            break;
    }
}

void PrintMDForm(uint8_t* inst, uint8_t opcode, char* instrBuffer, char* paramBuffer)
{
    uint8_t rs = ((inst[0] & 0x03) << 3) | ((inst[1] & 0xE0) >> 5);
    uint8_t ra = (inst[1] & 0x1F);
    uint8_t rb = (inst[2] & 0xF8) >> 3;
    uint8_t mb = ((inst[2] & 0x07) << 3) | ((inst[3] & 0xE0) >> 5);
    uint8_t xo = ((inst[3] & 0x1E) >> 1);
    uint8_t sh = (rb << 1) | (xo & 0x01);

    switch (xo)
    {
        CASE_PRINT(0, icl); break;
        CASE_PRINT(1, icr); break;
        CASE_PRINT(2, ic); break;
        CASE_PRINT(3, imi); break;
        CASE_PRINT(8, cl); break;
        CASE_PRINT(9, cr); break;
        default:
            printf("\tDEBUG: unrecognized extended opcode %d for opcode %d", xo, opcode);
            return;
    }

    if (inst[3] & 0x01) INSTR_BUFFER_APPEND(".");

    uint64_t mask = 0;
    if (xo == 0 || xo == 2 || xo == 3 || xo == 8)
    {
        int mlen = 64 - mb;
        do { mask = ((mask << 1) | 1); } while (mlen-- > 0);
        mask <<= mb;
    }
    else if (xo == 1 || xo == 9)
    {
        int mlen = mb;
        do { mask = ((mask << 1) | 1); } while (mlen-- > 0);
    }

    if (xo == 8 || xo == 9)
    {
        snprintf(paramBuffer, INSTRUCTION_PARAM_SIZE, "r%d, r%d, r%d, 0x%016llx", ra, rs, rb, mask);
    }
    else
    {
        snprintf(paramBuffer, INSTRUCTION_PARAM_SIZE, "r%d, r%d, %d, 0x%016llx", ra, rs, sh, mask);
    }
}

void HandleOpcode63(uint8_t* inst, uint8_t opcode, char* instrBuffer, char* paramBuffer)
{
    uint8_t aExtOpcode = (inst[3] & 0x3E) >> 1;
    if (aExtOpcode >= 18 && aExtOpcode <= 31)
    {
        PrintAForm(inst, opcode, instrBuffer, paramBuffer);
        return;
    }

    uint16_t xExtOpcode = (OSReadBigInt16(inst, 2) & 0x07FE) >> 1;
    if (xExtOpcode == 711)
    {
        PrintXFLForm(inst, opcode, instrBuffer, paramBuffer);
    }
    else
    {
        PrintFloatingXForm(inst, opcode, xExtOpcode, instrBuffer, paramBuffer);
    }
}

void HandleOpcodeZero(uint8_t* inst, uint8_t opcode, char* instrBuffer, char* paramBuffer)
{
    uint16_t extOpcode = OSReadBigInt32(inst, 2);
    extOpcode = (extOpcode & 0x07FE) >> 1;
    if (extOpcode == 256)
    {
        snprintf(instrBuffer, INSTRUCTION_NAME_SIZE, "SP Attn");
    }
    else
    {
        snprintf(instrBuffer, INSTRUCTION_NAME_SIZE, "Illegal!");
    }
}

void HandleUnknownOpcode(uint8_t* inst, uint8_t opcode, char* instrBuffer, char* paramBuffer)
{
    snprintf(instrBuffer, INSTRUCTION_NAME_SIZE, "Unknown!");
    snprintf(paramBuffer, INSTRUCTION_PARAM_SIZE, "%d", opcode);
}

bool PrintOpcode(Instruction* pInstruction, Label** currentLabel, bool* isBranch)
{
    uint32_t currentAddress = pInstruction->address;
    uint8_t* inst = pInstruction->raw;

    uint8_t opcode = READ_OPCODE(inst);
    uint16_t extOpcode = ((inst[2] & 0x07) << 7) | (inst[3] >> 1);

    char* instrBuffer = pInstruction->opcode;
    char* paramBuffer = pInstruction->params;

    *isBranch = false;

    switch (opcode)
    {
        case 0x00: HandleOpcodeZero(inst, opcode, instrBuffer, paramBuffer); break;
        CASE_PRINT(2,  tdi);    D_FORM; break;
        CASE_PRINT(3,  twi);    D_FORM; break;
        CASE_PRINT(7,  mulli);  D_FORM; break;
        CASE_PRINT(8,  subfic); D_FORM; break;
        CASE_PRINT(9, Invalid for PowerPC); break;
        CASE_PRINT(10, cmpli);  D_FORM; break;
        CASE_PRINT(11, cmpi);   D_FORM; break;
        CASE_PRINT(12, addic);  D_FORM; break;
        CASE_PRINT(13, addic.); D_FORM; break;
        CASE_PRINT(14, addi);   D_FORM; break;
        CASE_PRINT(15, addis);  D_FORM; break;
        CASE_PRINT(16, bc);     B_FORM; *isBranch = true; break;
        CASE_PRINT(17, sc);     Q_FORM; break;
        CASE_PRINT(18, b);      I_FORM; *isBranch = true; break;
        case 19: switch(extOpcode)
        {
            CASE_PRINT(0, mcrf); break;
            CASE_PRINT(16, bclr); break;
            CASE_PRINT(18, rfid); break;
            CASE_PRINT(33, crnor); break;
            CASE_PRINT(129, crandc); break;
            CASE_PRINT(150, isync); break;
            CASE_PRINT(193, crxor); break;
            CASE_PRINT(225, crnand); break;
            CASE_PRINT(257, crand); break;
            CASE_PRINT(274, hrfid); break;
            CASE_PRINT(289, creqv); break;
            CASE_PRINT(417, crorc); break;
            CASE_PRINT(449, cror); break;
            CASE_PRINT(528, bcctr); break;
            default: printf("DEBUG: Unknown extended opcode %d for opcode 19", extOpcode); return false;
        }
        XL_FORM;
        break;
        CASE_PRINT(20, rlwimi); M_FORM; break;
        CASE_PRINT(21, rlwinm); M_FORM; break;
        CASE_PRINT(22, Invalid for PowerPC); break;
        CASE_PRINT(23, rlwnm);  M_FORM; break;
        CASE_PRINT(24, ori);    D_FORM; break;
        CASE_PRINT(25, oris);   D_FORM; break;
        CASE_PRINT(26, xori);   D_FORM; break;
        CASE_PRINT(27, xoris);  D_FORM; break;
        CASE_PRINT(28, andi.);  D_FORM; break;
        CASE_PRINT(29, andis.); D_FORM; break;
        CASE_PRINT(30, rld);    MD_FORM; break;
        case 31: switch (extOpcode)
        {
            CASE_PRINT(0, cmp);         X_FORM; break;
            CASE_PRINT(4, tw);          X_FORM; break;
            CASE_PRINT(8, subfc);       XO_FORM; break;
            CASE_PRINT(9, mulhdu);      XO_FORM; break;
            CASE_PRINT(10, addc);       XO_FORM; break;
            CASE_PRINT(11, mulhwu);     XO_FORM; break;
            CASE_PRINT(19, mfcr);       XFX_FORM; break;
            CASE_PRINT(20, lwarx);      X_FORM; break;
            CASE_PRINT(21, ldx);        X_FORM; break;
            CASE_PRINT(23, lwzx);       X_FORM; break;
            CASE_PRINT(24, slw);        X_FORM; break;
            CASE_PRINT(26, cntlzw);     X_FORM; break;
            CASE_PRINT(27, sld);        X_FORM; break;
            CASE_PRINT(28, and);        X_FORM; break;
            CASE_PRINT(32, cmpl);       X_FORM; break;
            CASE_PRINT(40, subf);       XO_FORM; break;
            CASE_PRINT(53, ldux);       X_FORM; break;
            CASE_PRINT(54, dcbst);      X_FORM; break;
            CASE_PRINT(55, lwzux);      X_FORM; break;
            CASE_PRINT(58, cntlzd);     X_FORM; break;
            CASE_PRINT(60, andc);       X_FORM; break;
            CASE_PRINT(68, td);         X_FORM; break;
            CASE_PRINT(73, mulhd);      XO_FORM; break;
            CASE_PRINT(75, mulhw);      XO_FORM; break;
            CASE_PRINT(83, mfmsr);      X_FORM; break;
            CASE_PRINT(84, ldarx);      X_FORM; break;
            CASE_PRINT(86, dcbf);       X_FORM; break;
            CASE_PRINT(87, lbzx);       X_FORM; break;
            CASE_PRINT(104, neg);       XO_FORM; break;
            CASE_PRINT(119, lbzux);     X_FORM; break;
            CASE_PRINT(124, nor);       X_FORM; break;
            CASE_PRINT(136, subfe);     XO_FORM; break;
            CASE_PRINT(138, adde);      XO_FORM; break;
            CASE_PRINT(144, mtcrf);     XFX_FORM; break;
            CASE_PRINT(146, mtmsr);     X_FORM; break;
            CASE_PRINT(149, stdx);      X_FORM; break;
            CASE_PRINT(150, stwcx);     X_FORM; break;
            CASE_PRINT(151, stwx);      X_FORM; break;
            CASE_PRINT(178, mtmsrd);    X_FORM; break;
            CASE_PRINT(181, stdux);     X_FORM; break;
            CASE_PRINT(183, stwux);     X_FORM; break;
            CASE_PRINT(200, subfze);    XO_FORM; break;
            CASE_PRINT(202, addze);     XO_FORM; break;
            CASE_PRINT(210, mtsr);      X_FORM; break;
            CASE_PRINT(214, stdcx.);    X_FORM; break;
            CASE_PRINT(215, stbx);      X_FORM; break;
            CASE_PRINT(232, subfm);     XO_FORM; break;
            CASE_PRINT(233, subfme);    XO_FORM; break;
            CASE_PRINT(234, addme);     XO_FORM; break;
            CASE_PRINT(235, mullw);     XO_FORM; break;
            CASE_PRINT(242, mtsrin);    X_FORM; break;
            CASE_PRINT(246, dcbtst);    X_FORM; break;
            CASE_PRINT(247, stbux);     X_FORM; break;
            CASE_PRINT(266, add);       XO_FORM; break;
            CASE_PRINT(278, dcbt);      X_FORM; break;
            CASE_PRINT(279, lhzx);      X_FORM; break;
            CASE_PRINT(284, eqv);       X_FORM; break;
            CASE_PRINT(306, tlbie);     X_FORM; break;
            CASE_PRINT(310, eciwx);     X_FORM; break;
            CASE_PRINT(311, lhzux);     X_FORM; break;
            CASE_PRINT(316, xor);       X_FORM; break;
            CASE_PRINT(339, mfspr);     XFX_FORM; break;
            CASE_PRINT(341, lwax);      X_FORM; break;
            CASE_PRINT(343, lhax);      X_FORM; break;
            CASE_PRINT(370, tlbia);     X_FORM; break;
            CASE_PRINT(371, mftb);      Q_FORM; break;
            CASE_PRINT(373, lwaux);     X_FORM; break;
            CASE_PRINT(375, lhaux);     X_FORM; break;
            CASE_PRINT(402, slbmte);    X_FORM; break;
            CASE_PRINT(407, sthx);      X_FORM; break;
            CASE_PRINT(412, orc);       X_FORM; break;
            CASE_PRINT(434, slbie);     X_FORM; break;
            CASE_PRINT(438, ecowx);     X_FORM; break;
            CASE_PRINT(439, sthux);     X_FORM; break;
            CASE_PRINT(444, or);        X_FORM; break;
            CASE_PRINT(457, divdu);     XO_FORM; break;
            CASE_PRINT(459, divwu);     XO_FORM; break;
            CASE_PRINT(467, mtspr);     XFX_FORM; break;
            CASE_PRINT(476, nand);      X_FORM; break;
            CASE_PRINT(489, divd);      XO_FORM; break;
            CASE_PRINT(491, divw);      XO_FORM; break;
            CASE_PRINT(498, slbia);     X_FORM; break;
            CASE_PRINT(512, mcrxr);     X_FORM; break;
            CASE_PRINT(533, lswx);      X_FORM; break;
            CASE_PRINT(534, lwbrx);     X_FORM; break;
            CASE_PRINT(535, lfsx);      X_FORM; break;
            CASE_PRINT(536, srw);       X_FORM; break;
            CASE_PRINT(539, srd);       X_FORM; break;
            CASE_PRINT(566, tlbsync);   X_FORM; break;
            CASE_PRINT(567, lfsux);     X_FORM; break;
            CASE_PRINT(595, mfsr);      X_FORM; break;
            CASE_PRINT(597, lswi);      X_FORM; break;
            CASE_PRINT(598, sync);      X_FORM; break;
            CASE_PRINT(599, lfdx);      X_FORM; break;
            CASE_PRINT(631, lfdux);     X_FORM; break;
            CASE_PRINT(659, mfsrin);    X_FORM; break;
            CASE_PRINT(661, stswx);     X_FORM; break;
            CASE_PRINT(662, stwbrx);    X_FORM; break;
            CASE_PRINT(663, stfsx);     X_FORM; break;
            CASE_PRINT(695, stfsux);    X_FORM; break;
            CASE_PRINT(725, stswi);     X_FORM; break;
            CASE_PRINT(727, stfdx);     X_FORM; break;
            CASE_PRINT(759, stfdux);    X_FORM; break;
            CASE_PRINT(790, lhbrx);     X_FORM; break;
            CASE_PRINT(792, sraw);      X_FORM; break;
            CASE_PRINT(794, srad);      X_FORM; break;
            CASE_PRINT(824, srawi);     X_FORM; break;
            CASE_PRINT(851, slbmfev);   X_FORM; break;
            CASE_PRINT(854, eieio);     X_FORM; break;
            CASE_PRINT(915, slbmfee);   X_FORM; break;
            CASE_PRINT(918, sthbrx);    X_FORM; break;
            CASE_PRINT(922, extsh);     X_FORM; break;
            CASE_PRINT(954, extsb);     X_FORM; break;
            CASE_PRINT(982, icbi);      X_FORM; break;
            CASE_PRINT(983, stfiwx);    X_FORM; break;
            CASE_PRINT(986, extsw);     X_FORM; break;
            CASE_PRINT(1014, dcbz);     X_FORM; break;
            default: printf("DEBUG: Unknown extended opcode %u for opcode 31", extOpcode); break;
        }
        break;
        CASE_PRINT(32, lwz);   D_FORM; break;
        CASE_PRINT(33, lwzu);  D_FORM; break;
        CASE_PRINT(34, lbz);   D_FORM; break;
        CASE_PRINT(35, lbzu);  D_FORM; break;
        CASE_PRINT(36, stw);   D_FORM; break;
        CASE_PRINT(37, stwu);  D_FORM; break;
        CASE_PRINT(38, stb);   D_FORM; break;
        CASE_PRINT(39, stbu);  D_FORM; break;
        CASE_PRINT(40, lhz);   D_FORM; break;
        CASE_PRINT(41, lhzu);  D_FORM; break;
        CASE_PRINT(42, lha);   D_FORM; break;
        CASE_PRINT(43, lhau);  D_FORM; break;
        CASE_PRINT(44, sth);   D_FORM; break;
        CASE_PRINT(45, sthu);  D_FORM; break;
        CASE_PRINT(46, lmw);   D_FORM; break;
        CASE_PRINT(47, stmw);  D_FORM; break;
        CASE_PRINT(48, lfs);   D_FORM; break;
        CASE_PRINT(49, lfsu);  D_FORM; break;
        CASE_PRINT(50, lfd);   D_FORM; break;
        CASE_PRINT(51, lfdu);  D_FORM; break;
        CASE_PRINT(52, stfs);  D_FORM; break;
        CASE_PRINT(53, stfsu); D_FORM; break;
        CASE_PRINT(54, stfd);  D_FORM; break;
        CASE_PRINT(55, stfdu); D_FORM; break;
        CASE_PRINT(56, lfq);   DS_FORM; break;
        CASE_PRINT(57, lfqu);  DS_FORM; break;
        case 58:               DS_FORM; break;
        case 59:               A_FORM; break;
        CASE_PRINT(60, stfq);  DS_FORM; break;
        CASE_PRINT(61, stfqu); DS_FORM; break;
        CASE_PRINT(62, std);   DS_FORM; break;
        case 63: HandleOpcode63(inst, opcode, instrBuffer, paramBuffer); break;
        default:
            HandleUnknownOpcode(inst, opcode, instrBuffer, paramBuffer);
            return false;
    }

    return true;
}

#endif
