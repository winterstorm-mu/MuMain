#include <doctest.h>

#include "UI/Combat/PlayerNames.h"

using namespace UI::PlayerNames;

TEST_CASE("F9 cycles off -> party+guild -> all -> off [ui][player_names]")
{
    CHECK(NextMode(Off) == PartyAndGuild);
    CHECK(NextMode(PartyAndGuild) == All);
    CHECK(NextMode(All) == Off);
}

TEST_CASE("who gets a name [ui][player_names]")
{
    // mode, gm, chaosCastle, hero, party, guild
    CHECK_FALSE(ShouldName(Off, false, false, true, true, true));

    CHECK(ShouldName(All, false, false, false, false, false));
    CHECK(ShouldName(All, false, false, true, false, false));

    CHECK(ShouldName(PartyAndGuild, false, false, true, false, false));
    CHECK(ShouldName(PartyAndGuild, false, false, false, true, false));
    CHECK(ShouldName(PartyAndGuild, false, false, false, false, true));
    CHECK_FALSE(ShouldName(PartyAndGuild, false, false, false, false, false));
}

TEST_CASE("Chaos Castle hides F9 names but not GM observation [ui][player_names]")
{
    CHECK_FALSE(ShouldName(All, false, true, false, false, false));
    CHECK_FALSE(ShouldName(PartyAndGuild, false, true, true, true, true));
    CHECK(ShouldName(Off, true, true, false, false, false));
    CHECK(ShouldName(Off, true, false, false, false, false));
}
