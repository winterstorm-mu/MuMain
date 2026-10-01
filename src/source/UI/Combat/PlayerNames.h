#pragma once

// Which players get a name balloon without being selected (F9). Stored as an
// int in config.ini [UI] PlayerNames; see docs/client/interface.md.
namespace UI::PlayerNames
{
enum Mode : int
{
    Off = 0,
    PartyAndGuild = 1,
    All = 2,
};

// Off -> PartyAndGuild -> All -> Off.
Mode NextMode(Mode mode);

// GM observation (/charactername) names everyone on every map. The F9 modes
// never name anyone inside Chaos Castle.
bool ShouldName(Mode mode, bool gmObservation, bool inChaosCastle, bool isHero, bool isPartyMember, bool isGuildMember);
} // namespace UI::PlayerNames
