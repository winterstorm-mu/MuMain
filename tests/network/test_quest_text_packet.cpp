// doctest unit tests for the QuestText packet (F6 F0) and the quest text cache
// it fills.
//
// Run: ctest --test-dir <build directory> --build-config Release -R "quest_text"

#include "doctest.h"

#include "GameLogic/Quests/QuestMng.h"
#include "Network/Server/QuestTextPacket.h"

#include <cstring>
#include <string>
#include <vector>

using Network::Quest::ReceiveQuestText;

namespace
{
constexpr std::size_t PacketSize = 586;
constexpr BYTE CodeC2 = 0xC2;
constexpr BYTE HeadCodeF6 = 0xF6;
constexpr WORD TestGroup = 100;
constexpr WORD TestNumber = 1;
constexpr DWORD TestIndex = (static_cast<DWORD>(TestGroup) << 16) | TestNumber;

static_assert(sizeof(PMSG_QUEST_TEXT) == PacketSize);

std::vector<BYTE> QuestTextPacket(WORD group, WORD number, const char* title, const char* summary)
{
    std::vector<BYTE> packet(PacketSize);
    packet[0] = CodeC2;
    packet[1] = static_cast<BYTE>(PacketSize >> 8);
    packet[2] = static_cast<BYTE>(PacketSize & 0xFF);
    packet[3] = HeadCodeF6;
    packet[4] = QUEST_TEXT_SUBCODE;
    packet[6] = static_cast<BYTE>(number & 0xFF);
    packet[7] = static_cast<BYTE>(number >> 8);
    packet[8] = static_cast<BYTE>(group & 0xFF);
    packet[9] = static_cast<BYTE>(group >> 8);
    std::memcpy(&packet[10], title, std::strlen(title));
    std::memcpy(&packet[10 + QUEST_TEXT_TITLE_LENGTH], summary, std::strlen(summary));
    return packet;
}
} // namespace

TEST_CASE("a QuestText packet is cached under (group << 16) | number [network][quest_text]")
{
    // u8 e-acute is the two bytes C3 A9.
    const auto packet = QuestTextPacket(TestGroup, TestNumber, "Caf\xC3\xA9 Run", "Bring 5 items.");

    CHECK(ReceiveQuestText(packet));

    const wchar_t* title = g_QuestMng.GetSubject(TestIndex);
    const wchar_t* summary = g_QuestMng.GetSummary(TestIndex);
    REQUIRE(title != nullptr);
    REQUIRE(summary != nullptr);
    CHECK(std::wstring(title) == L"Café Run");
    CHECK(std::wstring(summary) == L"Bring 5 items.");
    CHECK(g_QuestMng.IsQuestByEtc(TestIndex));
}

TEST_CASE("a re-sent QuestText updates the text [network][quest_text]")
{
    CHECK(ReceiveQuestText(QuestTextPacket(TestGroup, TestNumber, "First", "One")));
    CHECK(ReceiveQuestText(QuestTextPacket(TestGroup, TestNumber, "Second", "Two")));

    CHECK(std::wstring(g_QuestMng.GetSubject(TestIndex)) == L"Second");
    CHECK(std::wstring(g_QuestMng.GetSummary(TestIndex)) == L"Two");
}

TEST_CASE("a malformed QuestText packet is ignored [network][quest_text]")
{
    constexpr WORD otherNumber = 2;
    constexpr DWORD otherIndex = (static_cast<DWORD>(TestGroup) << 16) | otherNumber;

    auto shortPacket = QuestTextPacket(TestGroup, otherNumber, "T", "S");
    shortPacket.resize(PacketSize - 1);
    CHECK_FALSE(ReceiveQuestText(shortPacket));

    auto wrongCode = QuestTextPacket(TestGroup, otherNumber, "T", "S");
    wrongCode[0] = 0xC1;
    CHECK_FALSE(ReceiveQuestText(wrongCode));

    auto smallDeclared = QuestTextPacket(TestGroup, otherNumber, "T", "S");
    smallDeclared[2] = 10;
    smallDeclared[1] = 0;
    CHECK_FALSE(ReceiveQuestText(smallDeclared));

    CHECK(g_QuestMng.GetSubject(otherIndex) == nullptr);
}

TEST_CASE("an index without text is safe to query [network][quest_text]")
{
    constexpr DWORD unknownIndex = 0x7FFF0001;

    CHECK(g_QuestMng.GetSubject(unknownIndex) == nullptr);
    CHECK(g_QuestMng.GetSummary(unknownIndex) == nullptr);
    CHECK_FALSE(g_QuestMng.IsQuestByEtc(unknownIndex));
    CHECK_FALSE(g_QuestMng.IsRequestRewardQS(unknownIndex));
}
