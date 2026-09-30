#include "UI/Combat/PlayerNames.h"

UI::PlayerNames::Mode UI::PlayerNames::NextMode(Mode mode)
{
    switch (mode)
    {
    case Off:
        return PartyAndGuild;
    case PartyAndGuild:
        return All;
    default:
        return Off;
    }
}

bool UI::PlayerNames::ShouldName(Mode mode, bool gmObservation, bool inChaosCastle, bool isHero, bool isPartyMember, bool isGuildMember)
{
    if (gmObservation)
        return true;
    if (inChaosCastle)
        return false;

    switch (mode)
    {
    case All:
        return true;
    case PartyAndGuild:
        return isHero || isPartyMember || isGuildMember;
    default:
        return false;
    }
}
