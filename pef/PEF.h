#ifndef __PEF_PEF_H__
#define __PEF_PEF_H__

#include <time.h>
#include "../Common.h"
#include "Opcodes.h"
#include "Instructions.h"

struct LoaderSection
{
    uint32_t mainSection;
    uint32_t mainOffset;
    uint32_t initSection;
    uint32_t initOffset;
    uint32_t termSection;
    uint32_t termOffset;
    uint32_t importLibraryCount;
    uint32_t importSymbolCount;
    uint32_t relocSectionCount;
    uint32_t relocInstOffset;
    uint32_t loaderStringOffset;
    uint32_t exportHashOffset;
    uint32_t exportHashTablePower;
    uint32_t exportSymbolCount;
};

struct SectionData
{
    uint8_t id;
    uint8_t type;
    uint8_t* data;
    uint32_t length;
    uint32_t totalLength;
};

void ProcessRelocationArea(uint8_t* data, uint32_t sectionCount, uint32_t headerOffset, uint32_t areaOffset)
{
    for (uint32_t section = 0; section < sectionCount; section++)
    {
        uint32_t thisOffset = headerOffset + (section * 12);
        uint16_t affectedSection = OSReadBigInt16(data, thisOffset);
        uint32_t relocCount = OSReadBigInt32(data, thisOffset + 4);
        uint32_t firstRelocOffset = OSReadBigInt32(data, thisOffset + 8);

        printf("\t\tGoing to read %d relocation instruction blocks for section %d\n", relocCount, affectedSection);

        uint32_t instOffset = areaOffset + firstRelocOffset;

        uint32_t relocAddress = 0; // section base address
        uint32_t importIndex = 0;
        uint32_t sectionC = 0; // section 0
        uint32_t sectionD = 0; // section 1
        
        uint32_t byImportCount = 0;

        for (uint32_t i = 0; i < relocCount; i++)
        {
            uint8_t block[2];
            memcpy(block, data + instOffset + (i * 2), 2);

            printf("\t\t\t");
            if ((block[0] & 0xC0) == 0)
            {
                printf("RelocBySectDWithSkip");
            }
            else
            {
                uint8_t opcode = (block[0] & 0xFE) >> 1;
                switch (opcode & 0xF0)
                {
                    case 0x20:
                        {
                            switch (opcode)
                            {
                                case 0x20: printf("RelocBySectC"); break;
                                case 0x21: printf("kPEFRelocBySectD"); break;
                                case 0x22: printf("kPEFRelocTVector12"); break;
                                case 0x23: printf("kPEFRelocTVector8"); break;
                                case 0x24: printf("kPEFRelocVTable8"); break;
                                case 0x25: printf("kPEFRelocImportRun"); break;
                            }
                        }
                        break;
                    case 0x30:
                        {
                            uint16_t index = ((block[0] & 0x01) << 8) | block[1];
                            switch (opcode)
                            {
                                case 0x30: printf("RelocSmByImport, index %d", index); importIndex = index + 1; byImportCount++; break;
                                case 0x31: printf("RelocSmSetSectC"); break;
                                case 0x32: printf("RelocSmSetSectD"); break;
                                case 0x33: printf("RelocSmBySection"); break;
                            }
                        }
                        break;
                    case 0x40:
                        {
                            if (opcode & 0x80)
                            {
                                printf("kPEFRelocSmRepeat"); break;
                            }
                            else
                            {
                                printf("RelocIncrPosition"); break;
                            }
                        }
                        break;
                    case 0x50:
                        {
                            opcode &= 0xFE; // clear the last bit
                            switch (opcode)
                            {
                                case 0x50: printf("kPEFRelocSetPosition"); break;
                                case 0x52: printf("kPEFRelocLgByImport"); break;
                                case 0x58: printf("kPEFRelocLgRepeat"); break;
                                case 0x5A: printf("kPEFRelocSetOrBySection"); break;
                            }
                        }
                        break;
                }
            }
            printf("\n");
        }
        printf("%d ByImportCount instructions\n", byImportCount);
    }
}

void ProcessLoaderSection(struct SectionData* pSectionData, struct LoaderSection* pSection)
{
    uint8_t* loaderData = pSectionData->data;
    uint8_t header[56];
    memcpy(&header, loaderData, 56);

    pSection->mainSection = OSReadBigInt32(header, 0);
    pSection->mainOffset = OSReadBigInt32(header, 4);
    pSection->initSection = OSReadBigInt32(header, 8);
    pSection->initOffset = OSReadBigInt32(header, 12);
    pSection->termSection = OSReadBigInt32(header, 16);
    pSection->termOffset = OSReadBigInt32(header, 20);
    pSection->importLibraryCount = OSReadBigInt32(header, 24);
    pSection->importSymbolCount = OSReadBigInt32(header, 28);
    pSection->relocSectionCount = OSReadBigInt32(header, 32);
    pSection->relocInstOffset = OSReadBigInt32(header, 36);
    pSection->loaderStringOffset = OSReadBigInt32(header, 40);
    pSection->exportHashOffset = OSReadBigInt32(header, 44);
    pSection->exportHashTablePower = OSReadBigInt32(header, 48);
    pSection->exportSymbolCount = OSReadBigInt32(header, 52);

    printf("\nLoader information:\n");
    if (pSection->mainSection != 0xFFFFFFFF)
    {
        printf("\tMain symbol in section %u at offset 0x%08x\n", pSection->mainSection, pSection->mainOffset);
    }
    else
    {
        printf("\tNo main symbol\n");
    }
    if (pSection->initSection != 0xFFFFFFFF)
    {
        printf("\tInitialization function vector in section %u at offset 0x%08x\n", pSection->initSection, pSection->initOffset);
    }
    else
    {
        printf("\tNo initialization function vector\n");
    }
    if (pSection->termSection != 0xFFFFFFFF)
    {
        printf("\tTermination function vector in section %u at offset 0x%08x\n", pSection->termSection, pSection->termOffset);
    }
    else
    {
        printf("\tNo termination function vector\n");
    }

    printf("\t%u imported symbols from %u libraries\n", pSection->importSymbolCount, pSection->importLibraryCount);
    uint8_t libraryDescription[24];
    uint8_t symbolTableEntry[4];

    uint32_t offsetToImportTable = (pSection->importLibraryCount * 24) + 56;
    uint32_t totalSymbolCount = 0;
    for (uint32_t i = 0; i < pSection->importLibraryCount; i++)
    {
        memcpy(&libraryDescription, loaderData + 56 + (i * 24), 24);
        uint32_t nameOffset = OSReadBigInt32(libraryDescription, 0);
        uint32_t oldImpVersion = OSReadBigInt32(libraryDescription, 4);
        uint32_t currentVersion = OSReadBigInt32(libraryDescription, 8);
        uint32_t symbolCount = OSReadBigInt32(libraryDescription, 12);
        uint32_t firstSymbol = OSReadBigInt32(libraryDescription, 16);
        uint8_t options = libraryDescription[20];

        printf("\t\t'%s' imports %u symbols\n", loaderData + pSection->loaderStringOffset + nameOffset, symbolCount);
        for (uint32_t j = 0; j < symbolCount; j++)
        {
            memcpy(&symbolTableEntry, loaderData + offsetToImportTable + ((j + firstSymbol) * 4), 4);
            uint8_t symbolClass = symbolTableEntry[0];
            symbolTableEntry[0] = 0x00;
            uint32_t symbolNameOffset = OSReadBigInt32(symbolTableEntry, 0);
            printf("\t\t\t%d: %s ", totalSymbolCount, loaderData + pSection->loaderStringOffset + symbolNameOffset);
            switch(symbolClass & 0x0F)
            {
                case 0x00: printf("(code address)"); break;
                case 0x01: printf("(data address)"); break;
                case 0x02: break; // Almost everything I'm looking at is a standard function
                case 0x03: printf("(TOC symbol)"); break;
                case 0x04: printf("(linker glue symbol)"); break;
                default: printf("! unknown symbol flag");
            }
            printf("\n");
            totalSymbolCount++;
        }
    }
    printf("\t%u exported symbols\n", pSection->exportSymbolCount);
    if (pSection->relocSectionCount != 0)
    {
        printf("\t%u sections requiring relocation\n", pSection->relocSectionCount);
        uint32_t relocationHeaderTable = (totalSymbolCount * 4) + (pSection->importLibraryCount * 24) + 56;
        ProcessRelocationArea(loaderData, pSection->relocSectionCount, relocationHeaderTable, pSection->relocInstOffset);
    }
}

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

#define PATTERN_DATA_OPCODE(x) ((x & 0xE0) >> 5)

uint32_t ReadOnePatternArgument(uint8_t* pData, bool firstArgument, uint32_t index, uint32_t* pNewIndex)
{
    uint32_t arg = 0;
    if (firstArgument)
    {
        arg = pData[index++] & 0x1F;
        if (arg == 0)
        {
            return ReadOnePatternArgument(pData, false, index, pNewIndex);
        }
    }
    else
    {
        uint8_t piece = 0;
        do
        {
            piece = pData[index++];
            arg <<= 7;
            arg |= (piece & 0x7F);
        } while (piece & 0x80);
    }

    *pNewIndex = index;
    return arg;
}

void InflatePatternDataSection(struct SectionData* pSection)
{
    uint32_t dataLocationCount = 0;
    // Copy the section data pointer and create it again with the new total size
    uint8_t* packedData = malloc(pSection->length);
    memcpy(packedData, pSection->data, pSection->length);
    free(pSection->data);
    pSection->data = malloc(pSection->totalLength);

    uint8_t* unpackedData = pSection->data;
    memset(unpackedData, 0, pSection->totalLength);

    for (uint32_t i = 0, instCount = 0; i < pSection->length; instCount++)
    {
        uint8_t opcode = PATTERN_DATA_OPCODE(packedData[i]);
        uint32_t count = ReadOnePatternArgument(packedData, true, i, &i);

        switch (opcode)
        {
            case 0: // Zero
                memset(unpackedData + dataLocationCount, 0, count);
                dataLocationCount += count;
                break;
            case 1: // blockCopy
                memcpy(unpackedData + dataLocationCount, packedData + i, count);
                dataLocationCount += count;
                i += count;
                break;
            case 2: // repeatedBlock
                {
                    uint32_t repeatCount = ReadOnePatternArgument(packedData, false, i, &i);
                    for (uint32_t j = 0; j < repeatCount; j++)
                    {
                        memcpy(unpackedData + dataLocationCount, packedData + i, count);
                        dataLocationCount += count;
                    }
                    i += (repeatCount * count);
                }
                break;
            case 3:
                {
                    uint32_t customSize = ReadOnePatternArgument(packedData, false, i, &i);
                    uint32_t repeatCount = ReadOnePatternArgument(packedData, false, i, &i);

                    for (uint32_t j = 0; j < repeatCount; j++)
                    {
                        // Common data
                        memcpy(unpackedData + dataLocationCount, packedData + i, count);
                        dataLocationCount += count;
                        // Custom data j
                        memcpy(unpackedData + dataLocationCount, packedData + i + count + (j * customSize), customSize);
                        dataLocationCount += customSize;
                    }

                    memcpy(unpackedData + dataLocationCount, packedData + i, count);
                    dataLocationCount += count;
                    i += (count + (customSize * repeatCount));
                }
                break;
            case 4:
                {
                    uint32_t customSize = ReadOnePatternArgument(packedData, false, i, &i);
                    uint32_t repeatCount = ReadOnePatternArgument(packedData, false, i, &i);

                    for (uint32_t j = 0; j < repeatCount; j++)
                    {
                        // Zeros instead of common data
                        memset(unpackedData + dataLocationCount, 0, count);
                        dataLocationCount += count;
                        // Custom data j
                        memcpy(unpackedData + dataLocationCount, packedData + i + count + (j * customSize), customSize);
                        dataLocationCount += customSize;
                    }

                    memset(unpackedData + dataLocationCount, 0, count);
                    dataLocationCount += count;
                    i += (customSize * repeatCount);
                }
                break;
            default:
                printf("DEBUG: Unexpected opcode in packed data at %x: %d, count = %d", i, opcode, count);
        }
    }

    free(packedData);
}

void ReadPEFSection(uint16_t sectionIndex, FILE* input, struct SectionData* pSection)
{
    uint16_t sectionOffset = 40 + (sectionIndex * 28);
    uint8_t section[28];
    fseek(input, sectionOffset, SEEK_SET);
    fread(&section, 28, 1, input);

    uint32_t nameOffset = OSReadBigInt32(section, 0);
    uint32_t defaultAddress = OSReadBigInt32(section, 4);
    uint32_t totalSize = OSReadBigInt32(section, 8);
    uint32_t unpackedSize = OSReadBigInt32(section, 12);
    uint32_t packedSize = OSReadBigInt32(section, 16);
    uint32_t containerOffset = OSReadBigInt32(section, 20);

    printf("Section %d:\n", sectionIndex + 1);
    printf("\tSection name offset: 0x%08x\n", nameOffset);
    printf("\tDefault address: 0x%08x\n", defaultAddress);
    printf("\tTotal size: %u\tUnpacked size:%u\tPacked size:%u\n", totalSize, unpackedSize, packedSize);
    printf("\tOffset in container: 0x%08x\n", containerOffset);
    printf("\tType: ");
    switch (section[24])
    {
        case 0x00: printf("Code"); break;
        case 0x01: printf("Unpacked data"); break;
        case 0x02: printf("Pattern-initialized data"); break;
        case 0x03: printf("Constant"); break;
        case 0x04: printf("Loader"); break;
        case 0x05: printf("Debug"); break;
        case 0x06: printf("Executable data"); break;
        case 0x07: printf("Exception"); break;
        case 0x08: printf("Traceback"); break;
        default: printf("! Unknown"); break;
    }
    printf("\n");
    printf("\tSection sharing: ");
    switch (section[25])
    {
        case 0x01: printf("Process"); break;
        case 0x04: printf("Global"); break;
        case 0x05: printf("Protected"); break;
        default: printf("! Unknown"); break;
    }
    printf("\n");

    pSection->id = sectionIndex + 1;
    pSection->type = section[24];
    pSection->data = malloc(packedSize);
    pSection->length = packedSize;
    pSection->totalLength = totalSize;
    fseek(input, containerOffset,SEEK_SET);
    fread(pSection->data, packedSize, 1, input);
}

void ProcessPEF(FILE* input)
{
    uint8_t header[40];
    fread(&header, 40, 1, input);

    if (strncmp((const char*)header, "Joy!peff", 8) != 0)
    {
        printf("! File is not an Apple PEF file.\n");
        return;
    }

    if (strncmp((const char*)(header + 8), "pwpc", 4) != 0)
    {
        printf("! PEF is not built for the PowerPC.\n");
        return;
    }

    uint32_t epmach = OSReadBigInt32(header, 16); // Get it, it's the epoch but for a Mac (1/1/1904)
    time_t timestamp = (time_t)(epmach - 2082844800);
    printf("Build date (GMT): %s", asctime(gmtime(&timestamp)));

    uint16_t sectionCount = OSReadBigInt16(header, 32);
    uint16_t instSectionCount = OSReadBigInt16(header, 34);

    struct SectionData** sections = malloc(sizeof(struct SectionData*) * sectionCount);
    uint16_t loaderSectionIndex = 0xFFFF;

    uint8_t* dataSection = NULL;
    for (uint16_t i = 0; i < sectionCount; i++)
    {
        sections[i] = malloc(sizeof(struct SectionData));
        ReadPEFSection(i, input, sections[i]);
        if (sections[i]->type == 4)
        {
            loaderSectionIndex = i;
        }
        else if (sections[i]->type == 2)
        {
            InflatePatternDataSection(sections[i]);
        }
    }

    if (loaderSectionIndex == 0xFFFF)
    {
        printf("! Couldn't find loader section in PEF.\n");
        return;
    }

    struct LoaderSection loader;
    ProcessLoaderSection(sections[loaderSectionIndex], &loader);

    // Run back through and process sections which depend on loader information.
    for (uint16_t i = 0; i < sectionCount; i++)
    {
        switch (sections[i]->type)
        {
            case 0:
                ProcessCodeSection(sections[i], &loader);
                break;
        }
    }

    for (uint16_t i = 0; i < sectionCount; i++)
    {
        if (sections[i]->data != NULL)
        {
            free(sections[i]->data);
        }
        free(sections[i]);
    }
    free(sections);
}

#endif
