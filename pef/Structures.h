#ifndef __PEF_STRUCTURES_H__
#define __PEF_STRUCTURES_H__

struct LoaderSection
{
    uint8_t* data;
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

    uint32_t importSymbolTableOffset;
};

struct SectionData
{
    uint8_t id;
    uint8_t type;
    uint8_t* data;
    uint32_t length;
    uint32_t totalLength;
    uint32_t baseAddress;
};

#endif
