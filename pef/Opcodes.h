#ifndef __PEF_OPCODES_H__
#define __PEF_OPCODES_H__

#include <stdbool.h>

#define S64_EXT_16(v) v * ((v & 0x8000) ? -1 : 1)
#define S64_EXT_24(v) v * ((v & 0x800000) ? -1 : 1)

#define CASE_PRINT(x, y) case x: printf(#y);
#define PRINT_RC if (inst[3] & 0x01) printf(".");

#define A_FORM PrintAForm(inst, opcode)
#define B_FORM PrintBForm(inst, opcode)
#define D_FORM PrintDForm(inst, opcode)
#define DS_FORM PrintDSForm(inst, opcode)
#define I_FORM PrintIForm(inst, opcode)
#define M_FORM PrintMForm(inst, opcode)
#define MD_FORM PrintMDForm(inst, opcode)
#define XL_FORM PrintXLForm(inst, opcode, extOpcode)

void PrintIForm(uint8_t* inst, uint8_t opcode)
{
    uint32_t target = OSReadBigInt32(inst, 0);
    target = (target & 0x03FFFFFC) >> 2;
    int64_t value = S64_EXT_24(target);

    if (inst[3] & 0x01) printf("l");
    if (inst[3] & 0x02) printf("a");
    printf("\t%lld", value);
}

void PrintBForm(uint8_t* inst, uint8_t opcode)
{
    uint8_t bo = ((inst[0] & 0x03) << 3) | ((inst[1] & 0xE0) >> 5);
    uint8_t bi = (inst[1] & 0x1F);
    uint16_t target = (OSReadBigInt16(inst, 2) & 0xFFFC) >> 2;
    int64_t value = S64_EXT_16(target);

    if (inst[3] & 0x01) printf("l");
    if (inst[3] & 0x02) printf("a");
    printf("\t%d, %d, %lld", bo, bi, value);
}

void PrintAForm(uint8_t* inst, uint8_t opcode)
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

    if (opcode == 59) printf("s");
    if (inst[3] & 0x0) printf(".");

    switch (xo)
    {
        case 18:
        case 20:
        case 21:
            printf("\tfpr%d, fpr%d, fpr%d", rst, ra, rb);
            break;
        case 25:
            printf("\tfpr%d, fpr%d, fpr%d", rst, ra, rc);
            break;
        case 23:
        case 28:
        case 29:
        case 30:
        case 31:
            printf("\tfpr%d, fpr%d, fpr%d, fpr%d", rst, ra, rc, rb);
            break;
        case 22:
        case 24:
        case 26:
            printf("\tfpr%d, fpr%d", rst, rb);
            break;
    }
}

void PrintXFLForm(uint8_t* inst, uint8_t opcode)
{
    uint8_t flm = ((inst[0] & 0x01) << 7) | ((inst[1] & 0xFE) >> 1);
    uint8_t frb = (inst[2] & 0xF8) >> 3;

    printf("mtfsf");
    if (inst[3] & 0x01) printf(".");
    printf("\t%d, fpr%d", flm, frb);
}

void PrintXLForm(uint8_t* inst, uint8_t opcode, uint16_t extOpcode)
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
                printf("\t%d, %d", bf, bfa);
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
            printf("\t%d, %d, %d", bt, ba, bb);
            break;
        case 16:
        case 528:
            {
                uint8_t bh = (bb & 0x03);
                printf("\t%d, %d, %d", bt, ba, bh);
            }
            break;
    }
}

void PrintXForm(uint8_t* inst, uint8_t opcode, uint16_t extOpcode)
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
    }

    if (inst[3] & 0x01) printf(".");

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
            printf("\tfpr%d, fpr%d", rst, frb);
            break;
        case 0:
        case 32:
            printf("\t%d, fpr%d, fpr%d", bf, fra, frb);
            break;
        case 64:
            {
                uint8_t bfa = (fra & 0xF0) >> 4;
                printf("\t%d, %d", bf, bfa);
            }
            break;
        case 134:
            {
                uint8_t u = (frb & 0xFE) >> 1;
                printf("\t%d, %d", bf, u);
            }
            break;
        case 583:
            printf("\tfpr%d", rst);
            break;
        case 38:
        case 70:
            printf("\t%d", rst);
            break;
    }
}

void PrintMForm(uint8_t* inst, uint8_t opcode)
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

    if (inst[3] & 0x01) printf(".");
    printf("\tr%d, r%d, %d, 0x%016llx", rst, ra, sh, mask);
}

void PrintDSForm(uint8_t* inst, uint8_t opcode)
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
            printf("\t!! Invalid instruction form");
            return;
        }
    }
    else if (opcode == 62)
    {
        if (xo == 1) printf("u");
        if (ra == 0)
        {
            printf("\t!! Invalid instruction form");
            return;
        }
    }
    else if (opcode == 56 || opcode == 57 || opcode == 60 || opcode == 61)
    {
        if (xo != 0)
        {
            printf("\t!! Invalid instruction form");
            return;
        }
    }
    printf("\tr%d, %lld(r%d)", rst, signextvalue, ra);
}

void PrintDForm(uint8_t* inst, uint8_t opcode)
{
    uint8_t rst = ((inst[0] & 0x03) << 3) | ((inst[1] & 0xE0) >> 5);
    uint8_t ra = (inst[1] & 0x1F);
    uint16_t value = OSReadBigInt16(inst, 2);
    int64_t signextvalue = S64_EXT_16(value);

    if (opcode == 2 || opcode == 3) // Fixed point trap instructions
    {
        printf("\t0x%02x, r%d, %lld", rst, ra, signextvalue);
    }
    else if (opcode == 10) // cmpli
    {
        printf("\tr%d, %u", ra, value);
    }
    else if (opcode == 11) // cmpi
    {
        printf("\tr%d, %lld", ra, signextvalue);
    }
    else if (opcode >= 48 && opcode <= 55) // FPR instructions
    {
        printf("\tfpr%d, %lld(r%d)", rst, signextvalue, ra);
    }
    else
    {
        printf("\tr%d, %lld(r%d)", rst, signextvalue, ra);
    }
}

void PrintMDForm(uint8_t* inst, uint8_t opcode)
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
    }

    if (inst[3] & 0x01) printf(".");

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
        printf("\tr%d, r%d, r%d, 0x%016llx", ra, rs, rb, mask);
    }
    else
    {
        printf("\tr%d, r%d, %d, 0x%016llx", ra, rs, sh, mask);
    }
}

void HandleOpcode63(uint8_t* inst, uint8_t opcode)
{
    uint8_t aExtOpcode = (inst[3] & 0x3E) >> 1;
    if (aExtOpcode >= 18 && aExtOpcode <= 31)
    {
        PrintAForm(inst, opcode);
        return;
    }

    uint16_t xExtOpcode = (OSReadBigInt16(inst, 2) & 0x07FE) >> 1;
    if (xExtOpcode == 711)
    {
        PrintXFLForm(inst, opcode);
    }
    else
    {
        PrintXForm(inst, opcode, xExtOpcode);
    }
}

bool PrintOpcode(uint8_t* inst)
{
    uint8_t opcode = (inst[0] & 0xFC) >> 2;
    uint16_t extOpcode = ((inst[2] & 0x07) << 7) | (inst[3] >> 1);
    switch (opcode)
    {
        case 0x00: printf("Illegal!"); break;
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
        CASE_PRINT(16, bc);     B_FORM; break;
        CASE_PRINT(17, sc);             break;
        CASE_PRINT(18, b);      I_FORM; break;
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
            default: printf("!! Unknown extended opcode %d for opcode 19", extOpcode); return false;
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
        case 0x1F: switch (extOpcode)
        {
            case 0x0000: printf("cmp"); break;
            case 0x0004: printf("tw"); break;
            case 0x0008: printf("subfc"); break;
            case 0x0009: printf("mulhdu"); break;
            case 0x000A: printf("addc"); break;
            case 0x000B: printf("mulhwu"); break;
            case 19: printf("mfcr"); break;
            case 20: printf("lwarx"); break;
            case 21: printf("ldx"); break;
            case 23: printf("lwzx"); break;
            case 24: printf("slw"); break;
            case 26: printf("cntlzw"); break;
            case 27: printf("sld"); break;
            case 28: printf("and"); break;
            case 29: printf("maskg"); break;
            case 30: printf("rldicl"); break;
            case 32: printf("cmpl"); break;
            case 40: printf("subf"); break;
            case 53: printf("ldux"); break;
            case 54: printf("dcbst"); break;
            case 55: printf("lwzux"); break;
            case 58: printf("cntlzd"); break;
            case 60: printf("andc"); break;
            case 62: printf("rldicl"); break;
            case 68: printf("td"); break;
            case 73: printf("mulhd"); break;
            case 75: printf("mulhw"); break;
            case 79: printf("tdi"); break;
            case 82: printf("mtsrd"); break;
            case 83: printf("mfmsr"); break;
            CASE_PRINT(84, ldarx); break;
            CASE_PRINT(86, dcbf); break;
            CASE_PRINT(87, lbzx); break;
            CASE_PRINT(94, rldicr); break;
            CASE_PRINT(104, neg); break;
            CASE_PRINT(107, mul); break;
            CASE_PRINT(111, twi); break;
            CASE_PRINT(114, mtsrdin); break;
            CASE_PRINT(118, clf); break;
            CASE_PRINT(119, lbzux); break;
            CASE_PRINT(124, nor); break;
            CASE_PRINT(126, rldicr); break;
            CASE_PRINT(136, subfe); break;
            CASE_PRINT(138, adde); break;
            CASE_PRINT(144, mtcrf); break;
            CASE_PRINT(146, mtmsr); break;
            CASE_PRINT(149, stdx); break;
            CASE_PRINT(150, stwcx); break;
            CASE_PRINT(151, stwx); break;
            CASE_PRINT(152, slq); break;
            CASE_PRINT(153, sle); break;
            CASE_PRINT(158, rldic); break;
            CASE_PRINT(159, rlwimi); break;
            CASE_PRINT(178, mtmsrd); break;
            CASE_PRINT(181, stdux); break;
            CASE_PRINT(183, stwux); break;
            CASE_PRINT(184, sliq); break;
            CASE_PRINT(190, rldic); break;
            CASE_PRINT(191, rlwinm); break;
            CASE_PRINT(200, subfze); break;
            CASE_PRINT(202, addze); break;
            CASE_PRINT(210, mtsr); break;
            CASE_PRINT(214, stdcx.); break;
            CASE_PRINT(215, stbx); break;
            CASE_PRINT(216, sliq); break;
            CASE_PRINT(217, sleq); break;
            CASE_PRINT(222, rldimi); break;
            CASE_PRINT(223, rlmi); break;
            CASE_PRINT(232, subfm); break;
            CASE_PRINT(233, subfme); break;
            CASE_PRINT(234, addme); break;
            CASE_PRINT(235, mullw); break;
            CASE_PRINT(239, mulli); break;
            CASE_PRINT(242, mtsrin); break;
            CASE_PRINT(246, dcbtst); break;
            CASE_PRINT(247, stbux); break;
            CASE_PRINT(248, slliq); break;
            CASE_PRINT(254, rldimi); break;
            CASE_PRINT(255, rlwnm); break;
            CASE_PRINT(264, doz); break;
            CASE_PRINT(266, add); break;
            CASE_PRINT(271, subfic); break;
            CASE_PRINT(277, lscbx); break;
            CASE_PRINT(278, dcbt); break;
            CASE_PRINT(279, lhzx); break;
            CASE_PRINT(284, eqv); break;
            CASE_PRINT(286, rldcl); break;
            CASE_PRINT(287, ori); break;
            CASE_PRINT(303, dozi); break;
            CASE_PRINT(306, tlbie); break;
            CASE_PRINT(310, eciwx); break;
            CASE_PRINT(311, lhzux); break;
            CASE_PRINT(316, xor); break;
            CASE_PRINT(318, rldcr); break;
            CASE_PRINT(319, oris); break;
            CASE_PRINT(331, div); break;
            CASE_PRINT(335, cmpli); break;
            CASE_PRINT(339, mfspr); break;
            CASE_PRINT(341, lwax); break;
            CASE_PRINT(343, lhax); break;
            CASE_PRINT(351, xori); break;
            CASE_PRINT(360, abs); break;
            CASE_PRINT(363, divs); break;
            CASE_PRINT(367, cmpi); break;
            CASE_PRINT(370, tlbia); break;
            CASE_PRINT(371, mftb); break;
            CASE_PRINT(373, lwaux); break;
            CASE_PRINT(375, lhaux); break;
            CASE_PRINT(383, xoris); break;
            CASE_PRINT(399, addic); break;
            CASE_PRINT(402, slbmte); break;
            CASE_PRINT(407, sthx); break;
            CASE_PRINT(412, orc); break;
            CASE_PRINT(415, andi); break;
            CASE_PRINT(431, addic.); break;
            CASE_PRINT(434, slbie); break;
            CASE_PRINT(438, ecowx); break;
            CASE_PRINT(439, sthux); break;
            CASE_PRINT(444, or); break;
            CASE_PRINT(447, andis); break;
            CASE_PRINT(457, divdu); break;
            CASE_PRINT(459, divwu); break;
            CASE_PRINT(463, addi); break;
            CASE_PRINT(467, mtspr); break;
            CASE_PRINT(470, dcbi); break;
            CASE_PRINT(471, lmw); break;
            CASE_PRINT(476, nand); break;
            CASE_PRINT(488, nabs); break;
            CASE_PRINT(489, divd); break;
            CASE_PRINT(491, divw); break;
            CASE_PRINT(495, addis); break;
            CASE_PRINT(498, slbia); break;
            CASE_PRINT(502, cli); break;
            CASE_PRINT(503, stmw); break;
            CASE_PRINT(512, mcrxr); break;
            CASE_PRINT(520, subfc); break;
            CASE_PRINT(521, mulhdu); break;
            CASE_PRINT(522, addc); break;
            CASE_PRINT(523, mulhwu); break;
            CASE_PRINT(531, clcs); break;
            CASE_PRINT(533, lswx); break;
            CASE_PRINT(534, lwbrx); break;
            CASE_PRINT(535, lfsx); break;
            CASE_PRINT(536, srw); break;
            CASE_PRINT(537, rrib); break;
            CASE_PRINT(539, srd); break;
            CASE_PRINT(541, maskir); break;
            CASE_PRINT(552, subf); break;
            CASE_PRINT(566, tlbsync); break;
            CASE_PRINT(567, lfsux); break;
            CASE_PRINT(585, mulhd); break;
            CASE_PRINT(587, mulhw); break;
            CASE_PRINT(595, mfsr); break;
            CASE_PRINT(597, lswi); break;
            CASE_PRINT(598, sync); break;
            CASE_PRINT(599, lfdx); break;
            CASE_PRINT(616, neg); break;
            CASE_PRINT(619, mul); break;
            CASE_PRINT(627, mfsri); break;
            CASE_PRINT(630, dclst); break;
            CASE_PRINT(631, lfdux); break;
            CASE_PRINT(648, subfe); break;
            CASE_PRINT(650, adde); break;
            CASE_PRINT(659, mfsrin); break;
            CASE_PRINT(661, stswx); break;
            CASE_PRINT(662, stwbrx); break;
            CASE_PRINT(663, stfsx); break;
            CASE_PRINT(664, srq); break;
            CASE_PRINT(665, sre); break;
            CASE_PRINT(695, stfsux); break;
            CASE_PRINT(696, sriq); break;
            CASE_PRINT(712, subfze); break;
            CASE_PRINT(714, addze); break;
            CASE_PRINT(725, stswi); break;
            CASE_PRINT(727, stfdx); break;
            CASE_PRINT(728, srlq); break;
            CASE_PRINT(729, sreq); break;
            CASE_PRINT(744, subfme); break;
            CASE_PRINT(745, mulld); break;
            CASE_PRINT(746, addme); break;
            CASE_PRINT(747, mullw); break;
            CASE_PRINT(758, dcba); break;
            CASE_PRINT(759, stfdux); break;
            CASE_PRINT(760, srliq); break;
            CASE_PRINT(776, doz); break;
            CASE_PRINT(778, add); break;
            CASE_PRINT(790, lhbrx); break;
            CASE_PRINT(791, lfqx); break;
            CASE_PRINT(792, sraw); break;
            CASE_PRINT(794, srad); break;
            CASE_PRINT(818, rac); break;
            CASE_PRINT(823, lfqux); break;
            CASE_PRINT(824, srawi); break;
            CASE_PRINT(826, sradi); break;
            CASE_PRINT(827, sradi); break;
            CASE_PRINT(843, div); break;
            CASE_PRINT(851, slbmfev); break;
            CASE_PRINT(854, eieio); break;
            CASE_PRINT(872, abs); break;
            CASE_PRINT(875, divs); break;
            CASE_PRINT(915, slbmfee); break;
            CASE_PRINT(918, sthbrx); break;
            CASE_PRINT(919, stfqx); break;
            CASE_PRINT(920, sraq); break;
            CASE_PRINT(921, srea); break;
            CASE_PRINT(922, extsh); break;
            CASE_PRINT(951, stfqux); break;
            CASE_PRINT(952, sraiq); break;
            CASE_PRINT(954, extsb); break;
            CASE_PRINT(969, divdu); break;
            CASE_PRINT(971, divwu); break;
            CASE_PRINT(982, icbi); break;
            CASE_PRINT(983, stfiwx); break;
            CASE_PRINT(986, extsw); break;
            CASE_PRINT(1000, nabs); break;
            CASE_PRINT(1001, divd); break;
            CASE_PRINT(1003, divw); break;
            CASE_PRINT(1014, dcbz); break;
            default: printf("!! Unknown extended opcode %u for opcode 0x1F", extOpcode); break;
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
        case 63: HandleOpcode63(inst, opcode); break;
        default: printf("!! Unknown opcode 0x%02x", opcode); return false;
    }
    return true;
}

#endif
