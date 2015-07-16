#ifndef __PEF_OPCODES_H__
#define __PEF_OPCODES_H__

#include <stdbool.h>

bool PrintOpcode(uint8_t* inst)
{
    uint8_t opcode = (inst[0] & 0xFC) >> 2;
    uint32_t extOpcode = ((inst[2] & 0x07) << 7) | (inst[3] >> 1);
    switch (opcode)
    {
        case 0x00: printf("Illegal!"); break;
        case 0x02: printf("tdi"); break;
        case 0x03: printf("twi"); break;
        case 0x07: printf("mulli"); break;
        case 0x08: printf("subfic"); break;
        case 0x09: printf("dozi"); break;
        case 0x0A: printf("cmpli"); break;
        case 0x0B: printf("cmpi"); break;
        case 0x0C: printf("addic"); break;
        case 0x0D: printf("addic."); break;
        case 0x0E: printf("addi"); break;
        case 0x0F: printf("addis"); break;
        case 0x10: printf("bc"); break;
        case 0x11: printf("sc"); break;
        case 0x12: printf("b"); break;
        case 0x13:
            switch(extOpcode)
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
        case 0x18: printf("ori"); break;
        case 0x19: printf("oris"); break;
        case 0x1A: printf("xori"); break;
        case 0x1B: printf("xoris"); break;
        case 0x1C: printf("andi."); break;
        case 0x1D: printf("andis."); break;
        case 0x1E: printf("FX Dwd Rot **"); break;
        case 0x1F: printf("FX extended ops **"); break;
        case 0x20: printf("lwz"); break;
        case 0x21: printf("lwzu"); break;
        case 0x22: printf("lbz"); break;
        case 0x23: printf("lbzu"); break;
        case 0x24: printf("stw"); break;
        case 0x25: printf("stwu"); break;
        case 0x26: printf("stb"); break;
        case 0x27: printf("stbu"); break;
        case 0x28: printf("lhz"); break;
        case 0x29: printf("lhzu"); break;
        case 0x2A: printf("lha"); break;
        case 0x2B: printf("lhau"); break;
        case 0x2C: printf("sth"); break;
        case 0x2D: printf("sthu"); break;
        case 0x2E: printf("lmw"); break;
        case 0x2F: printf("stmw"); break;
        case 0x30: printf("lfs"); break;
        case 0x31: printf("lfsu"); break;
        case 0x32: printf("lfd"); break;
        case 0x33: printf("lfdu"); break;
        case 0x34: printf("stfs"); break;
        case 0x35: printf("stfsu"); break;
        case 0x36: printf("stfd"); break;
        case 0x37: printf("stfdu"); break;
        case 0x38: printf("lfq"); break;
        case 0x39: printf("lfqu"); break;
        case 0x3A: printf("FX DS-form loads **"); break;
        case 0x3B: printf("FP Single Extended **"); break;
        case 0x3C: printf("stfq"); break;
        case 0x3D: printf("stfqu"); break;
        case 0x3E: printf("FX DS-form stores **"); break;
        case 0x3F: printf("FP Double Extended **"); break;
        default: printf("!! Unknown opcode 0x%02x", opcode); return false;
    }
    return true;
}

#endif
