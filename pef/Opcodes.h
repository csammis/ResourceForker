#ifndef __PEF_OPCODES_H__
#define __PEF_OPCODES_H__

#include <stdbool.h>

#define S64_EXT_16(v) v * ((v & 0x8000) ? -1 : 1)
#define S64_EXT_24(v) v * ((v & 0x800000) ? -1 : 1)

#define CASE_PRINT(x, y) case x: printf(#y);

#define D_FORM PrintDForm(inst, opcode)
#define I_FORM PrintIForm(inst, opcode)
#define B_FORM PrintBForm(inst, opcode)
#define DS_FORM PrintDSForm(inst, opcode)

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

bool PrintOpcode(uint8_t* inst)
{
    uint8_t opcode = (inst[0] & 0xFC) >> 2;
    uint32_t extOpcode = ((inst[2] & 0x07) << 7) | (inst[3] >> 1);
    switch (opcode)
    {
        case 0x00: printf("Illegal!"); break;
        CASE_PRINT(2,  tdi);    D_FORM; break;
        CASE_PRINT(3,  twi);    D_FORM; break;
        CASE_PRINT(7,  mulli);  D_FORM; break;
        CASE_PRINT(8,  subfic); D_FORM; break;
        CASE_PRINT(9,  dozi);   D_FORM; break;
        CASE_PRINT(10, cmpli);  D_FORM; break;
        CASE_PRINT(11, cmpi);   D_FORM; break;
        CASE_PRINT(12, addic);  D_FORM; break;
        CASE_PRINT(13, addic.); D_FORM; break;
        CASE_PRINT(14, addi);   D_FORM; break;
        CASE_PRINT(15, addis);  D_FORM; break;
        CASE_PRINT(16, bc);     B_FORM; break;
        CASE_PRINT(17, sc); break;
        CASE_PRINT(18, b);      I_FORM; break;
        case 0x13: switch(extOpcode)
        {
            case 0x0000: printf("mcrf"); break;
            case 0x0010: printf("bclr"); break;
            case 0x0012: printf("rfid"); break;
            case 0x0021: printf("crnor"); break;
            case 0x0032: printf("rfi"); break;
            case 0x0052: printf("rfsvc"); break;
            case 0x0081: printf("crandc"); break;
            case 0x0096: printf("isync"); break;
            case 0x00C1: printf("crxor"); break;
            case 0x00E1: printf("crnand"); break;
            case 0x0101: printf("crand"); break;
            case 0x0121: printf("creqv"); break;
            case 0x01A1: printf("crorc"); break;
            case 0x01C1: printf("cror"); break;
            case 0x0210: printf("bcctr"); break;
            default: printf("!! Unknown extended opcode 0x%02x for opcode 0x13", extOpcode); break;
        }
        break;
        case 0x14: printf("rlwimi"); break;
        case 0x15: printf("rlwinm"); break;
        case 0x16: printf("rlmi"); break;
        case 0x17: printf("rlwnm"); break;
        CASE_PRINT(24, ori);    D_FORM; break;
        CASE_PRINT(25, oris);   D_FORM; break;
        CASE_PRINT(26, xori);   D_FORM; break;
        CASE_PRINT(27, xoris);  D_FORM; break;
        CASE_PRINT(28, andi.);  D_FORM; break;
        CASE_PRINT(29, andis.); D_FORM; break;
        case 0x1E: printf("FX Dwd Rot **"); break;
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
        case 0x38: printf("lfq"); break;
        case 0x39: printf("lfqu"); break;
        case 58:               DS_FORM; break;
        case 0x3B: printf("FP Single Extended **"); break;
        case 0x3C: printf("stfq"); break;
        case 0x3D: printf("stfqu"); break;
        CASE_PRINT(62, std);   DS_FORM; break;
        case 0x3F: printf("FP Double Extended **"); break;
        default: printf("!! Unknown opcode 0x%02x", opcode); return false;
    }
    return true;
}

#endif
