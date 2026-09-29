#include "stdafx.h"
#include "QuestTextPacket.h"

#include <cstring>
#include <string>

#include "Data/Translation/MultiLanguage.h"
#include "GameLogic/Quests/QuestMng.h"

namespace
{
constexpr BYTE PacketCodeC2 = 0xC2;
constexpr int ByteShift = 8;

// One character of UTF-8 is at most one wchar_t, so the byte length bounds the output.
std::wstring DecodeField(const char* field, std::size_t fieldLength)
{
    const std::size_t textLength = strnlen(field, fieldLength);
    if (textLength == 0)
        return std::wstring();

    std::wstring text(textLength, L'\0');
    const int32_t decoded = CMultiLanguage::ConvertFromUtf8(text.data(), field, static_cast<int>(textLength));
    text.resize(decoded > 0 ? static_cast<std::size_t>(decoded) : 0);
    return text;
}

bool IsValidQuestText(std::span<const BYTE> packet)
{
    if (packet.size() < sizeof(PMSG_QUEST_TEXT))
        return false;

    const auto* header = reinterpret_cast<const PWMSG_HEADER*>(packet.data());
    const std::size_t declaredSize = (static_cast<std::size_t>(header->SizeH) << ByteShift) | header->SizeL;
    return header->Code == PacketCodeC2 && declaredSize >= sizeof(PMSG_QUEST_TEXT);
}
} // namespace

namespace Network::Quest
{
bool ReceiveQuestText(std::span<const BYTE> packet)
{
    if (!IsValidQuestText(packet))
        return false;

    PMSG_QUEST_TEXT data;
    std::memcpy(&data, packet.data(), sizeof(data));

    const std::wstring title = DecodeField(data.Title, sizeof(data.Title));
    const std::wstring summary = DecodeField(data.Summary, sizeof(data.Summary));
    g_QuestMng.AddCustomQuestText(data.m_wQuestGroup, data.m_wQuestNumber, title.c_str(), summary.c_str());
    return true;
}
} // namespace Network::Quest
