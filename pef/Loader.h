#ifndef __PEF_LOADER_H__
#define __PEF_LOADER_H__

#include "Structures.h"
#include "Relocations.h"
#include "Symbol.h"
#include <math.h>

Symbol* InsertImportSymbolIntoNameTable(LoaderSection* pSection, uint8_t* pSymbolTableEntry, int index)
{
    Symbol* symbol = malloc(sizeof(Symbol));
    symbol->symbolClass = pSymbolTableEntry[0] & 0x0F;
    symbol->extra = 0;

    pSymbolTableEntry[0] = 0x00;
    uint32_t symbolNameOffset = OSReadBigInt32(pSymbolTableEntry, 0);
    
    int symbolLength = strlen((const char*)(pSection->data + pSection->loaderStringOffset + symbolNameOffset));
    symbol->mangledName = malloc(symbolLength + 1);
    memcpy(symbol->mangledName, pSection->data + pSection->loaderStringOffset + symbolNameOffset, symbolLength);
    symbol->mangledName[symbolLength] = '\0';
    SetSymbolUnmangledName(symbol);
    AddImportSymbolToNameTable(index, symbol);

    return symbol;
}

Symbol* InsertExportSymbolIntoNameTable(LoaderSection* pSection, uint8_t* pSymbolTableEntry, uint32_t address, uint16_t section, int symbolLength)
{
    Symbol* symbol = malloc(sizeof(Symbol));
    symbol->symbolClass = pSymbolTableEntry[0];
    symbol->extra = section;

    pSymbolTableEntry[0] = 0x00;
    uint32_t symbolNameOffset = OSReadBigInt32(pSymbolTableEntry, 0);

    symbol->mangledName = malloc(symbolLength + 1);
    memcpy(symbol->mangledName, pSection->data + pSection->loaderStringOffset + symbolNameOffset, symbolLength);
    symbol->mangledName[symbolLength] = '\0';
    SetSymbolUnmangledName(symbol);
    AddExportSymbolToNameTable(address, symbol);

    return symbol;
}


void ProcessLoaderSection(Section** sections, uint16_t loaderSectionIndex, LoaderSection* pSection)
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

    InitializeSymbolNameTables(pSection->importSymbolCount, pSection->exportSymbolCount);

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

        printf("\t\t%u symbols imported from '%s'\n", symbolCount, pSection->data + pSection->loaderStringOffset + nameOffset);
        for (uint32_t j = 0; j < symbolCount; j++)
        {
            memcpy(&symbolTableEntry, pSection->data + pSection->importSymbolTableOffset + ((j + firstSymbol) * 4), 4);
            Symbol* symbol = InsertImportSymbolIntoNameTable(pSection, symbolTableEntry, totalSymbolCount);

            printf("\t\t\t%d: %s", totalSymbolCount, symbol->unmangledName);
            PrintSymbolClass(symbol);
            printf("\n");
            totalSymbolCount++;
        }
    }
    
    printf("\t%u exported symbols\n", pSection->exportSymbolCount);
    uint32_t exportHashCount = pow(2, pSection->exportHashTablePower);
    uint32_t exportTableOffset = pSection->exportHashOffset + (exportHashCount * 4);
    uint32_t exportTableSize = pSection->exportSymbolCount * 4;
    uint32_t exportSymbolTableOffset = exportTableOffset + exportTableSize;

    uint32_t currentExportSymbol = 0;
    for (uint32_t i = 0; i < exportHashCount; i++)
    {
        uint8_t hashEntry[4];
        memcpy(&hashEntry, pSection->data + pSection->exportHashOffset + (i * 4), 4);
        uint32_t hashValue = OSReadBigInt32(hashEntry, 0);
        uint32_t chainCount = (hashEntry[0] << 6) | (hashEntry[1] & 0xFC) >> 2;
        uint32_t firstTableIndex = (hashEntry[1] & 0x03) << 16 | hashEntry[2] << 8 | hashEntry[3];

        for (uint32_t j = 0; j < chainCount; j++)
        {
            uint8_t tableEntry[4];
            memcpy(&tableEntry, pSection->data + exportTableOffset + ((j + firstTableIndex) * 4), 4);
            uint16_t symbolLength = OSReadBigInt16(tableEntry, 0);
            uint16_t symbolHash = OSReadBigInt16(tableEntry, 2);

            uint32_t entryOffset = exportSymbolTableOffset + (currentExportSymbol * 10);
            memcpy(&symbolTableEntry, pSection->data + entryOffset, 4);
            
            uint32_t symbolLocation = OSReadBigInt32(pSection->data + entryOffset, 4);
            uint16_t symbolSection = OSReadBigInt16(pSection->data + entryOffset, 8);

            Symbol* symbol = InsertExportSymbolIntoNameTable(pSection, symbolTableEntry, symbolLocation, symbolSection, symbolLength);

            printf("\t\t%d: %s in section %d at offset 0x%08x\n", currentExportSymbol, symbol->unmangledName, symbolSection, symbolLocation);
            currentExportSymbol++;
        }
    }

    if (pSection->relocSectionCount != 0)
    {
        printf("\t%u sections requiring relocation\n", pSection->relocSectionCount);
        uint32_t relocationHeaderTable = (totalSymbolCount * 4) + (pSection->importLibraryCount * 24) + 56;
        ProcessRelocationArea(pSection->data, pSection->relocSectionCount, relocationHeaderTable, pSection->relocInstOffset, sections);
    }
}


#endif
