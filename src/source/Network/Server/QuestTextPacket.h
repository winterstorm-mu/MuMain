// Head code F6, sub-code F0: the title and summary of a quest, sent by the
// server so the T quest window needs no client data file for it.
#pragma once

#include <cstddef>
#include <span>

#include "Network/Server/WSclient.h"

constexpr BYTE QUEST_TEXT_SUBCODE = 0xF0;
constexpr std::size_t QUEST_TEXT_TITLE_LENGTH = 64;
constexpr std::size_t QUEST_TEXT_SUMMARY_LENGTH = 512;

#pragma pack(push, 1)
typedef struct
{
    PWMSG_HEADER Header;
    BYTE         SubCode;
    BYTE         Padding;
    WORD         m_wQuestNumber;
    WORD         m_wQuestGroup;
    char         Title[QUEST_TEXT_TITLE_LENGTH];     // UTF-8, NUL-padded
    char         Summary[QUEST_TEXT_SUMMARY_LENGTH]; // UTF-8, NUL-padded
} PMSG_QUEST_TEXT, * LPPMSG_QUEST_TEXT;
#pragma pack(pop)

namespace Network::Quest
{
// Decodes a QuestText packet and caches it in g_QuestMng. Ignores a packet that
// is not C2 or is shorter than PMSG_QUEST_TEXT; returns whether it was cached.
bool ReceiveQuestText(std::span<const BYTE> packet);
} // namespace Network::Quest
