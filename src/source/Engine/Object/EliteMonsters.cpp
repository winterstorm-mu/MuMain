#include "stdafx.h"
#include "Engine/Object/EliteMonsters.h"
#include "Data/Translation/MultiLanguage.h"

namespace Elites
{
namespace
{
    constexpr int TableSize = LastNumber - FirstNumber + 1;
    constexpr const wchar_t* FilePath = L"Data\\EliteMonsters.txt";

    Entry g_table[TableSize];
    bool g_present[TableSize];

    // Row: <number>\t<appearance>\t"<Name>". Returns false for comments and malformed rows.
    bool ParseRow(const char* line, int& number, int& appearance, char* name, size_t nameSize)
    {
        if (line[0] == '/' && line[1] == '/')
            return false;

        int consumed = 0;
        if (sscanf(line, "%d %d %n", &number, &appearance, &consumed) != 2 || consumed == 0)
            return false;

        const char* open = strchr(line + consumed, '"');
        if (open == nullptr)
            return false;
        const char* close = strrchr(open + 1, '"');
        if (close == nullptr || close == open + 1)
            return false;

        const size_t length = static_cast<size_t>(close - open - 1);
        if (length >= nameSize)
            return false;
        memcpy(name, open + 1, length);
        name[length] = '\0';
        return true;
    }
}

void Load()
{
    memset(g_present, 0, sizeof(g_present));

    FILE* file = _wfopen(FilePath, L"rb");
    if (file == nullptr)
    {
        g_ErrorReport.Write(L"%ls - File not exist, no elite monsters.\r\n", FilePath);
        return;
    }

    char line[256];
    char name[NameCapacity];
    while (fgets(line, sizeof(line), file) != nullptr)
    {
        int number = 0;
        int appearance = 0;
        if (!ParseRow(line, number, appearance, name, sizeof(name)))
            continue;
        if (number < FirstNumber || number > LastNumber)
            continue;

        const int slot = number - FirstNumber;
        g_table[slot].Appearance = appearance;
        CMultiLanguage::ConvertFromUtf8(g_table[slot].Name, name);
        g_table[slot].Name[NameCapacity - 1] = L'\0';
        g_present[slot] = true;
    }
    fclose(file);
}

const Entry* Find(int number)
{
    if (number < FirstNumber || number > LastNumber)
        return nullptr;
    const int slot = number - FirstNumber;
    return g_present[slot] ? &g_table[slot] : nullptr;
}
}
