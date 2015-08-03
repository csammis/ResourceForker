#ifndef __PEF_LOADER_H__
#define __PEF_LOADER_H__

#include "Structures.h"
#include "Relocations.h"

void ProcessLoaderSection(struct SectionData** sections, uint16_t loaderSectionIndex, struct LoaderSection* pSection)
{
    pSection->data = sections[loaderSectionIndex]->data;
    uint8_t header[56];
    memcpy(&header, pSection->data, 56);

    pSection->mainSection           = OSReadBigInt32(header, 0);
    pSection->mainOffset            = OSReadBigInt32(header, 4);
    pSection->initSection           = OSReadBigInt32(header, 8);
    pSection->initOffset            = OSReadBigInt32(header, 12);
    pSection->termSection           = OSReadBigInt32(header, 16);
    pSection->termOffset            = OSReadBigInt32(header, 20);
    pSection->importLibraryCount    = OSReadBigInt32(header, 24);
    pSection->importSymbolCount     = OSReadBigInt32(header, 28);
    pSection->relocSectionCount     = OSReadBigInt32(header, 32);
    pSection->relocInstOffset       = OSReadBigInt32(header, 36);
    pSection->loaderStringOffset    = OSReadBigInt32(header, 40);
    pSection->exportHashOffset      = OSReadBigInt32(header, 44);
    pSection->exportHashTablePower  = OSReadBigInt32(header, 48);
    pSection->exportSymbolCount     = OSReadBigInt32(header, 52);

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

    pSection->importSymbolTableOffset = (pSection->importLibraryCount * 24) + 56;
    uint32_t totalSymbolCount = 0;
    for (uint32_t i = 0; i < pSection->importLibraryCount; i++)
    {
        memcpy(&libraryDescription, pSection->data + 56 + (i * 24), 24);
        uint32_t nameOffset = OSReadBigInt32(libraryDescription, 0);
        uint32_t oldImpVersion = OSReadBigInt32(libraryDescription, 4);
        uint32_t currentVersion = OSReadBigInt32(libraryDescription, 8);
        uint32_t symbolCount = OSReadBigInt32(libraryDescription, 12);
        uint32_t firstSymbol = OSReadBigInt32(libraryDescription, 16);
        uint8_t options = libraryDescription[20];

        printf("\t\t'%s' imports %u symbols\n", pSection->data + pSection->loaderStringOffset + nameOffset, symbolCount);
        for (uint32_t j = 0; j < symbolCount; j++)
        {
            memcpy(&symbolTableEntry, pSection->data + pSection->importSymbolTableOffset + ((j + firstSymbol) * 4), 4);
            uint8_t symbolClass = symbolTableEntry[0];
            symbolTableEntry[0] = 0x00;
            uint32_t symbolNameOffset = OSReadBigInt32(symbolTableEntry, 0);
            printf("\t\t\t%d: %s ", totalSymbolCount, pSection->data + pSection->loaderStringOffset + symbolNameOffset);
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
        ProcessRelocationArea(pSection->data, pSection->relocSectionCount, relocationHeaderTable, pSection->relocInstOffset, sections);
    }
}


#endif
