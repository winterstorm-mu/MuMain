#pragma once

namespace Elites
{
    // Elite monster numbers occupy [FirstNumber, LastNumber]: the create-monster
    // packet carries the type in 10 bits (WSclient.cpp ReceiveCreateMonsterViewport).
    inline constexpr int FirstNumber = 900;
    inline constexpr int LastNumber = 1023;
    inline constexpr int NameCapacity = 64;

    struct Entry
    {
        int Appearance;
        wchar_t Name[NameCapacity];
    };

    // Reads Data\EliteMonsters.txt. A missing file leaves the table empty.
    void Load();

    // nullptr when `number` is not an elite.
    const Entry* Find(int number);
}
