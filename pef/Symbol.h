#ifndef __PEF_SYMBOL_H__
#define __PEF_SYMBOL_H__

#include <stdbool.h>

#define SYMBOL_NAME_LENGTH 257

typedef struct _Symbol
{
    char* mangledName;
    char* unmangledName;
    uint8_t symbolClass;
    uint16_t extra;
} Symbol;

typedef struct _SymbolNameTableEntry
{
    uint32_t address;
    Symbol* symbol;
} SymbolNameTableEntry;

uint32_t importTableSize;
uint32_t importInsertIndex;
uint32_t exportTableSize;
uint32_t exportInsertIndex;
SymbolNameTableEntry** importSymbolNameTable = NULL;
SymbolNameTableEntry** exportSymbolNameTable = NULL;

void InitializeSymbolNameTables(uint32_t importSymbolCount, uint32_t exportSymbolCount)
{
    importTableSize = importSymbolCount;
    exportTableSize = exportSymbolCount;
    importInsertIndex = exportInsertIndex = 0;

    importSymbolNameTable = malloc(importSymbolCount * sizeof(Symbol*));
    memset(importSymbolNameTable, 0, importSymbolCount * sizeof(Symbol*));
    exportSymbolNameTable = malloc(exportSymbolCount * sizeof(Symbol*));
    memset(exportSymbolNameTable, 0, exportSymbolCount * sizeof(Symbol*));
}

void FreeSymbol(Symbol* symbol)
{
    free(symbol->unmangledName);
    free(symbol);
}

void FreeSymbolNameTables()
{
    if (importSymbolNameTable == NULL)
        return;

    for (uint32_t i = 0; i < importInsertIndex; i++)
    {
        FreeSymbol(importSymbolNameTable[i]->symbol);
        free(importSymbolNameTable[i]);
    }
    free(importSymbolNameTable);

    for (uint32_t i = 0; i < exportInsertIndex; i++)
    {
        FreeSymbol(exportSymbolNameTable[i]->symbol);
        free(exportSymbolNameTable[i]);
    }
    free(exportSymbolNameTable);
}

void AddImportSymbolToNameTable(uint32_t symbolIndex, Symbol* symbol)
{
    SymbolNameTableEntry* entry = malloc(sizeof(SymbolNameTableEntry));
    entry->address = symbolIndex;
    entry->symbol = symbol;
    importSymbolNameTable[importInsertIndex++] = entry;
}

void AddExportSymbolToNameTable(uint32_t exportAddress, Symbol* symbol)
{
    SymbolNameTableEntry* entry = malloc(sizeof(SymbolNameTableEntry));
    entry->address = exportAddress;
    entry->symbol = symbol;
    exportSymbolNameTable[exportInsertIndex++] = entry;
}

Symbol* GetImportSymbol(uint32_t symbolIndex)
{
    for (uint32_t i = 0; i < importInsertIndex; i++)
    {
        if (importSymbolNameTable[i]->address == symbolIndex)
        {
            return importSymbolNameTable[i]->symbol;
        }
    }
    return NULL;
}

Symbol* GetExportSymbol(uint32_t exportAddress)
{
    for (uint32_t i = 0; i < exportInsertIndex; i++)
    {
        if (exportSymbolNameTable[i]->address == exportAddress)
        {
            return exportSymbolNameTable[i]->symbol;
        }
    }
    return NULL;
}

void PrintSymbolClass(Symbol* symbol)
{
    switch (symbol->symbolClass)
    {
        case 0x00: printf(" (code address)"); break;
        case 0x01: printf(" (data address)"); break;
        case 0x02: break; // Almost everything I'm looking at is a standard function
        case 0x03: printf(" (TOC symbol)"); break;
        case 0x04: printf(" (linker glue symbol)"); break;
        default: printf(" ! unknown symbol flag");
    }
}


#define APPEND_LITERAL_TO_SYMBOL(x) snprintf(symbol->unmangledName + strlen(symbol->unmangledName), strlen(x) + 1, x)
#define APPEND_TO_SYMBOL(x) snprintf(symbol->unmangledName + strlen(symbol->unmangledName), strlen(x) + 1, "%s", x)

void SetSymbolUnmangledName(Symbol* symbol)
{
    symbol->unmangledName = malloc(sizeof(char) * SYMBOL_NAME_LENGTH);
    memset(symbol->unmangledName, 0, SYMBOL_NAME_LENGTH);

    char* argStart = 0;
    // Deal with special cases for new and delete before general case
    if (strncmp(symbol->mangledName, "__nw__", 6) == 0)
    {
        argStart = symbol->mangledName + 6;
        APPEND_LITERAL_TO_SYMBOL("operator new");
    }
    else if (strncmp(symbol->mangledName, "__dl__", 6) == 0)
    {
        argStart = symbol->mangledName + 6;
        APPEND_LITERAL_TO_SYMBOL("operator delete");
    }
    else
    {
        char* funcStart = symbol->mangledName;
        char* funcEnd = symbol->mangledName;
        if (*funcEnd == '_' && funcStart == funcEnd)
        {
            funcEnd += 2;
        }
        while (*funcEnd && strncmp(funcEnd, "__", 2) != 0)
        {
            funcEnd++;
        }

        char funcName[256] = { '\0' };
        snprintf(funcName, funcEnd - funcStart + 1, "%s", funcStart);

        if (*funcEnd)
        {
            char* classStart = funcEnd + 2;
            long length = strtol(classStart, &classStart, 10);
            char* classEnd = classStart + length;
            snprintf(symbol->unmangledName, (classEnd - classStart + 1), "%s", classStart);
            APPEND_LITERAL_TO_SYMBOL("::");
            // Only standard procedure pointer symbols might have arguments
            if (symbol->symbolClass == 2)
            {
                argStart = classEnd + 1;
            }
            else
            {
                argStart = 0;
            }
        }

        // Deal with the special cases for constructors and destructors
        if (strncmp(funcName, "__ct", 4) == 0)
            APPEND_LITERAL_TO_SYMBOL("ctor");
        else if (strncmp(funcName, "__dt", 4) == 0)
            APPEND_LITERAL_TO_SYMBOL("dtor");
        else
            APPEND_TO_SYMBOL(funcName);
    }

    if (symbol->symbolClass != 2)
        return;

    APPEND_LITERAL_TO_SYMBOL("(");
    bool isPointer = false, isConst = false, isRef = false, isUnsigned = false, firstParam = true;
    char typeName[65] = { '\0' };
    while (argStart && *argStart)
    {
        switch (*argStart)
        {
            case 'F': break;
            case 'l': snprintf(typeName, 5, "long"); break;
            case 's': snprintf(typeName, 6, "short"); break;
            case 'v': snprintf(typeName, 5, "void"); break;
            case 'i': snprintf(typeName, 4, "int"); break;
            case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
                {
                    long length = strtol(argStart, &argStart, 10);
                    snprintf(typeName, length + 1, "%s", argStart);
                    argStart += length - 1;
                }
                break;

            case 'C': isConst = true; break;
            case 'R': isRef = true; break;
            case 'P': isPointer = true; break;
            case 'U': isUnsigned = true; break;

            default: snprintf(typeName, 2, "?"); break;
        }

        if (strlen(typeName) != 0)
        {
            if (!firstParam)
                APPEND_LITERAL_TO_SYMBOL(", ");

            if (isConst) APPEND_LITERAL_TO_SYMBOL("const ");
            if (isUnsigned) APPEND_LITERAL_TO_SYMBOL("unsigned ");
            APPEND_TO_SYMBOL(typeName);
            if (isPointer) APPEND_LITERAL_TO_SYMBOL(" *");
            if (isRef) APPEND_LITERAL_TO_SYMBOL(" &");

            firstParam = false;
            isPointer = false;
            isConst = false;
            isRef = false;
            isUnsigned = false;
            memset(typeName, 0, 65);
        }

        argStart++;
    }
    APPEND_LITERAL_TO_SYMBOL(")");
}

#endif
