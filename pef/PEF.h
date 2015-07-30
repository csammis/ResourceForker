#ifndef __PEF_PEF_H__
#define __PEF_PEF_H__

#include <time.h>
#include "../Common.h"
#include "Structures.h"
#include "PatternData.h"
#include "Code.h"
#include "Relocations.h"

void ReadPEFSection(uint16_t sectionIndex, FILE* input, struct SectionData* pSection)
{
    uint16_t sectionOffset = 40 + (sectionIndex * 28);
    uint8_t section[28];
    fseek(input, sectionOffset, SEEK_SET);
    fread(&section, 28, 1, input);

    uint32_t nameOffset         = OSReadBigInt32(section, 0);
    uint32_t defaultAddress     = OSReadBigInt32(section, 4);
    uint32_t totalSize          = OSReadBigInt32(section, 8);
    uint32_t unpackedSize       = OSReadBigInt32(section, 12);
    uint32_t packedSize         = OSReadBigInt32(section, 16);
    uint32_t containerOffset    = OSReadBigInt32(section, 20);

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
    pSection->baseAddress = 0;
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
    ProcessLoaderSection(sections, loaderSectionIndex, &loader);

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
