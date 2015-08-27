#ifndef __PEF_SYMBOL_H__
#define __PEF_SYMBOL_H__

#include <stdbool.h>

#define SYMBOL_NAME_LENGTH 257
typedef struct _Symbol
{
    char* mangledName;
    char unmangledName[SYMBOL_NAME_LENGTH];
} Symbol;

#define APPEND_LITERAL_TO_SYMBOL(x) snprintf(symbol->unmangledName + strlen(symbol->unmangledName), strlen(x) + 1, x)
#define APPEND_TO_SYMBOL(x) snprintf(symbol->unmangledName + strlen(symbol->unmangledName), strlen(x) + 1, "%s", x)

Symbol* CreateSymbolFromTable(LoaderSection* loader, uint32_t symbolIndex)
{
    uint32_t symbolTableEntry = OSReadBigInt32(loader->data, loader->importSymbolTableOffset + (symbolIndex * 4));
    Symbol* symbol = malloc(sizeof(Symbol));
    symbol->mangledName = (char*)(loader->data + loader->loaderStringOffset + (symbolTableEntry & 0x00FFFFFF));
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
            argStart = classEnd + 1;
        }

        // Deal with the special cases for constructors and destructors
        if (strncmp(funcName, "__ct", 4) == 0)
            APPEND_LITERAL_TO_SYMBOL("ctor");
        else if (strncmp(funcName, "__dt", 4) == 0)
            APPEND_LITERAL_TO_SYMBOL("dtor");
        else
            APPEND_TO_SYMBOL(funcName);
    }

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
            memset(typeName, 0, 65);
        }

        argStart++;
    }
    APPEND_LITERAL_TO_SYMBOL(")");

    return symbol;
}

void FreeSymbol(Symbol* symbol)
{
    free(symbol);
}

#endif
