#ifndef __PEF_PEF_H__
#define __PEF_PEF_H__

#include <time.h>
#include "../Common.h"
#include "Opcodes.h"

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
    uint32_t relocInstCount;
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
};

void ProcessLoaderSection(uint8_t* loaderSection, struct LoaderSection* pSection)
{
    uint8_t header[56];
    memcpy(&header, loaderSection, 56);

    pSection->mainSection = OSReadBigInt32(header, 0);
    pSection->mainOffset = OSReadBigInt32(header, 4);
    pSection->initSection = OSReadBigInt32(header, 8);
    pSection->initOffset = OSReadBigInt32(header, 12);
    pSection->termSection = OSReadBigInt32(header, 16);
    pSection->termOffset = OSReadBigInt32(header, 20);
    pSection->importLibraryCount = OSReadBigInt32(header, 24);
    pSection->importSymbolCount = OSReadBigInt32(header, 28);
    pSection->relocSectionCount = OSReadBigInt32(header, 32);
    pSection->relocInstCount = OSReadBigInt32(header, 36);
    pSection->loaderStringOffset = OSReadBigInt32(header, 40);
    pSection->exportHashOffset = OSReadBigInt32(header, 44);
    pSection->exportHashTablePower = OSReadBigInt32(header, 48);
    pSection->exportSymbolCount = OSReadBigInt32(header, 52);

    printf("Loader information:\n");
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
    for (uint32_t i = 0; i < pSection->importLibraryCount; i++)
    {
        memcpy(&libraryDescription, loaderSection + 56 + (i * 24), 24);
        uint32_t nameOffset = OSReadBigInt32(libraryDescription, 0);
        uint32_t oldImpVersion = OSReadBigInt32(libraryDescription, 4);
        uint32_t currentVersion = OSReadBigInt32(libraryDescription, 8);
        uint32_t symbolCount = OSReadBigInt32(libraryDescription, 12);
        uint32_t firstSymbol = OSReadBigInt32(libraryDescription, 16);
        uint8_t options = libraryDescription[20];

        printf("\t\t'%s' imports %u symbols\n", loaderSection + pSection->loaderStringOffset + nameOffset, symbolCount);
        for (uint32_t j = 0; j < symbolCount; j++)
        {
            memcpy(&symbolTableEntry, loaderSection + offsetToImportTable + ((j + firstSymbol) * 4), 4);
            uint8_t symbolClass = symbolTableEntry[0];
            symbolTableEntry[0] = 0x00;
            uint32_t symbolNameOffset = OSReadBigInt32(symbolTableEntry, 0);
            printf("\t\t\t%s ", loaderSection + pSection->loaderStringOffset + symbolNameOffset);
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
        }
    }
    printf("\t%u exported symbols\n", pSection->exportSymbolCount);
}

void ProcessCodeSection(struct SectionData* pSection, struct LoaderSection* pLoader)
{
    uint8_t inst[4];
    uint32_t mainOffset = (pSection->id == pLoader->mainSection) ? pLoader->mainOffset : 0x00000001;
    for (uint32_t i = 0; i < pSection->length; i += 4)
    {
        memcpy(&inst, pSection->data + i, 4);
        if (i == mainOffset)
        {
            printf("main:\n");
        }
        printf("0x%08x\t%02x %02x %02x %02x\t", i, inst[0], inst[1], inst[2], inst[3]);
        PrintOpcode(i, inst);
        printf("\n");
    }
}

void ReadPEFSection(uint16_t sectionIndex, FILE* input, struct LoaderSection* pLoaderSection, struct SectionData* pSection)
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
    pSection->data = NULL;
    switch (pSection->type)
    {
        case 0x04:
            pSection->data = malloc(packedSize);
            pSection->length = packedSize;
            fseek(input, containerOffset, SEEK_SET);
            fread(pSection->data, packedSize, 1, input);
            ProcessLoaderSection(pSection->data, pLoaderSection);
            break;
        default:
            pSection->data = malloc(packedSize);
            pSection->length = packedSize;
            fseek(input, containerOffset, SEEK_SET);
            fread(pSection->data, packedSize, 1, input);
    }
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

    struct LoaderSection loader;
    struct SectionData** sections = malloc(sizeof(struct SectionData*) * sectionCount);
    for (uint16_t i = 0; i < sectionCount; i++)
    {
        sections[i] = malloc(sizeof(struct SectionData));
        ReadPEFSection(i, input, &loader, sections[i]);
    }

    for (uint16_t i = 0; i < sectionCount; i++)
    {
        switch (sections[i]->type)
        {
            case 0:
                ProcessCodeSection(sections[i], &loader);
                break;
            case 4:
                // Loader - already handled
                break;
            default:
                printf("DEBUG: No detailed analysis of section type %d\n", sections[i]->type);
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
