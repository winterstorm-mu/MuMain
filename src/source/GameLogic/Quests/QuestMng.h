//*****************************************************************************
// File: QuestMng.h
//*****************************************************************************
#pragma once

#include "Network/Server/WSclient.h"

#define QM_MAX_ND_ANSWER 10
#define QM_MAX_ANSWER 5
#define QM_MAX_REQUEST_REWARD_TEXT_LEN 64

// Word ids of quest text sent by the server. Shipped QuestWords ids stop at
// 20999, so this range cannot collide with them.
constexpr int QM_CUSTOM_WORDS_ID_BASE = 0x40000000;
// A server-sent quest owns one id for its title and one for its summary.
constexpr int QM_CUSTOM_WORDS_PER_QUEST = 2;
// SQuestProgress::m_byUIType of an entry that only carries text for the T window.
constexpr BYTE QM_UITYPE_TEXT_ONLY = 1;

struct SNPCDialogue
{
    int m_nNPCWords;
    int m_anAnswer[QM_MAX_ND_ANSWER * 2];
};

#pragma pack(push, 1)
struct SQuestProgress
{
    BYTE m_byUIType;
    int m_nNPCWords;
    int m_nPlayerWords;
    int m_anAnswer[QM_MAX_ANSWER];
    int m_nSubject;
    int m_nSummary;
};
#pragma pack(pop)

struct SQuestRequest
{
    DWORD m_dwType;
    WORD m_wIndex;
    DWORD m_dwValue;
#ifdef ASG_ADD_TIME_LIMIT_QUEST
    DWORD m_dwCurValue;
#else  // ASG_ADD_TIME_LIMIT_QUEST
    WORD m_wCurValue;
#endif // ASG_ADD_TIME_LIMIT_QUEST
    ITEM* m_pItem;
};

struct SQuestReward
{
    DWORD m_dwType;
    WORD m_wIndex;
    DWORD m_dwValue;
    ITEM* m_pItem;
};

struct SQuestRequestReward
{
    BYTE m_byRequestCount;
    BYTE m_byGeneralRewardCount;
    BYTE m_byRandRewardCount;
    BYTE m_byRandGiveCount;
    SQuestRequest m_aRequest[5];
    SQuestReward m_aReward[5];
};

enum REQUEST_REWARD_CLASSIFY
{
    RRC_NONE = 0,
    RRC_REQUEST = 1,
    RRC_REWARD = 2
};

struct SRequestRewardText
{
    HFONT m_hFont;
    DWORD m_dwColor;
    wchar_t m_szText[QM_MAX_REQUEST_REWARD_TEXT_LEN];
    REQUEST_REWARD_CLASSIFY m_eRequestReward; //
    DWORD m_dwType;
    WORD m_wIndex;
    ITEM* m_pItem;
};

typedef std::map<DWORD, SNPCDialogue> NPCDialogueMap;
typedef std::map<DWORD, SQuestProgress> QuestProgressMap;
typedef std::map<int, std::wstring> QuestWordsMap;
typedef std::map<DWORD, SQuestRequestReward> QuestRequestRewardMap;
typedef std::list<DWORD> DWordList;

class CQuestMng
{
protected:
    NPCDialogueMap m_mapNPCDialogue;
    QuestProgressMap m_mapQuestProgress;
    QuestWordsMap m_mapQuestWords;
    QuestRequestRewardMap m_mapQuestRequestReward;

    std::map<WORD, bool> m_mapEPRequestRewardState;

    int m_nNextCustomWordsId = QM_CUSTOM_WORDS_ID_BASE;

    int m_nNPCIndex;
    const wchar_t* m_szNPCName; // [MAX_MONSTER_NAME] ;

    DWordList m_listQuestIndexByEtc;
    DWordList m_listCurQuestIndex;

public:
    CQuestMng();
    virtual ~CQuestMng();

    void LoadQuestScript();
    void SetQuestRequestReward(const BYTE* pbyRequestRewardPacket);
    const SQuestRequestReward* GetRequestReward(DWORD dwQuestIndex);

    void SetNPC(int nNPCIndex);
    int GetNPCIndex();
    const wchar_t* GetNPCName();

    void SetCurQuestProgress(DWORD dwQuestIndex);
    // Caches the title and summary of quest (group, number) under index
    // (group << 16) | number. Does not touch the current quest list.
    void AddCustomQuestText(WORD wGroup, WORD wNumber, const wchar_t* pszTitle, const wchar_t* pszSummary);

    const wchar_t* GetWords(int nWordsIndex);
    const wchar_t* GetNPCDlgNPCWords(DWORD dwDlgState);
    const wchar_t* GetNPCDlgAnswer(DWORD dwDlgState, int nAnswer);
    int GetNPCDlgAnswerResult(DWORD dwDlgState, int nAnswer);
    const wchar_t* GetNPCWords(DWORD dwQuestIndex);
    const wchar_t* GetPlayerWords(DWORD dwQuestIndex);
    const wchar_t* GetAnswer(DWORD dwQuestIndex, int nAnswer);
    const wchar_t* GetSubject(DWORD dwQuestIndex);
    const wchar_t* GetSummary(DWORD dwQuestIndex);
    bool IsRequestRewardQS(DWORD dwQuestIndex);
    bool GetRequestRewardText(SRequestRewardText* aDest, int nDestCount, DWORD dwQuestIndex);
    void SetEPRequestRewardState(DWORD dwQuestIndex, bool ProgressState);
    bool IsEPRequestRewardState(DWORD dwQuestIndex);
    bool IsQuestByEtc(DWORD dwQuestIndex);

    void SetQuestIndexByEtcList(DWORD* adwSrcQuestIndex, int nIndexCount);
    bool IsQuestIndexByEtcListEmpty();
    void SendQuestIndexByEtcSelection();
    void DelQuestIndexByEtcList(DWORD dwQuestIndex);

    void SetCurQuestIndexList(DWORD* adwCurQuestIndex, int nIndexCount);
    void AddCurQuestIndexList(DWORD dwQuestIndex);
    void RemoveCurQuestIndexList(DWORD dwQuestIndex);
    bool IsIndexInCurQuestIndexList(DWORD dwQuestIndex);

protected:
    void LoadNPCDialogueScript();
    void LoadQuestProgressScript();
    void LoadQuestWordsScript();
};

extern CQuestMng g_QuestMng;
