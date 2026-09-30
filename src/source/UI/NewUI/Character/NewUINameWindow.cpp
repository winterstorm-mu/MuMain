// NewUINameWindow.cpp: implementation of the CNewUINameWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UI/Chat/Chat.h"
#include "UI/NewUI/Character/NewUINameWindow.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/Object/ZzzInventory.h"
#include "UI/Legacy/UIControls.h"
#include "GameLogic/Events/CSChaosCastle.h"
#include "GameLogic/Items/PersonalShopTitleImp.h"
#include "GameLogic/Events/MatchEvent.h"
#include "World/MapInfra/MapManager.h"
#include "Camera/CameraProjection.h"
#include "Camera/CameraState.h"
#include "UI/Combat/MonsterHealthBar.h"
#include "GameLogic/Social/PartyManager.h"
#include "Data/GameConfig/GameConfig.h"

// DevEditor forward declarations (must be at global scope)
#ifdef _EDITOR
extern "C" bool DevEditor_ShouldRenderItemLabels();
#endif

using namespace SEASON3B;

namespace
{

// Draws a segmented monster HP bar, horizontally centered on centerX with its
// top edge at topY. `steps` is the segment count (HP granularity); `scale`
// horizontally compresses the bar (1.0 == original width). `alpha` (0..1) scales
// the alpha byte of every quad. `fillArgb` is the colour of the filled segments.
constexpr DWORD kHealthFillRed = 0xFFFA0A00u;
constexpr DWORD kHealthFillGold = 0xFFFFC800u;

void DrawHealthBar(int centerX, int topY, float health, int steps, float scale, float alpha,
    DWORD fillArgb = kHealthFillRed)
{
    // Applies `alpha` to the alpha byte of an ARGB constant.
    const auto faded = [alpha](DWORD argb)
    {
        const DWORD a = (DWORD)((float)(argb >> 24) * alpha + 0.5f);
        return (argb & 0x00FFFFFFu) | (a << 24);
    };

    const float borderHeight = 2.f;                  // vertical inset (unscaled)
    const float borderWidth = 2.f * scale;           // horizontal inset
    const float stepSeparatorWidth = 1.f * scale;    // gap between segments
    const float segmentSpan = 80.f * scale;          // total span of the segment track
    const float widthPerStep = segmentSpan / steps;  // derived: fewer steps -> wider segments
    const float stepsWidth = segmentSpan - 2.f * stepSeparatorWidth;
    const float totalWidth = stepsWidth + borderWidth * 2.f;

    const int x = centerX - (int)(totalWidth / 2);
    const int y = topY;

    // Drop shadow.
    EnableAlphaTest();
    RenderColorQuadARGB((float)(x + 1), (float)(y + 1), totalWidth, 5.f, faded(0x80000000u));

    // Dark backing.
    EnableAlphaBlend();
    RenderColorQuadARGB((float)x, (float)y, totalWidth, 5.f, faded(0xFF330000u));

    // Inner track.
    RenderColorQuadARGB((float)(x + borderWidth), (float)(y + borderHeight), stepsWidth, 1.f,
        faded(0xFF320A00u));

    // HealthStatus < 0 is the "HP unknown" sentinel (server sends 0xFF -> -1, and
    // the field is initialized to -1), so render a full bar instead of an empty one.
    const float clampedHealth = (health < 0.f) ? 1.f : health;
    const int stepHP = (int)(clampedHealth * steps);

    // Filled health segments.
    const DWORD fillColor = faded(fillArgb);
    for (int k = 0; k < stepHP; ++k)
    {
        RenderColorQuadARGB(
            (float)(x + borderWidth + (k * widthPerStep)),
            (float)(y + borderHeight),
            widthPerStep - stepSeparatorWidth,
            2.f,
            fillColor);
    }
    DisableAlphaBlend();
}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SEASON3B::CNewUINameWindow::CNewUINameWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;

    m_bShowItemName = false;
    m_bShowMonsterHealthBar = true;
    m_playerNameMode = UI::PlayerNames::All;
}

SEASON3B::CNewUINameWindow::~CNewUINameWindow()
{
    Release();
}

bool SEASON3B::CNewUINameWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_NAME_WINDOW, this);

    SetPos(x, y);

    m_bShowMonsterHealthBar = GameConfig::GetInstance().GetShowMonsterPlates();
    m_playerNameMode = static_cast<UI::PlayerNames::Mode>(GameConfig::GetInstance().GetPlayerNames());

    Show(true);

    return true;
}

void SEASON3B::CNewUINameWindow::Release()
{
    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void SEASON3B::CNewUINameWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool SEASON3B::CNewUINameWindow::UpdateMouseEvent()
{
    return true;
}

bool SEASON3B::CNewUINameWindow::UpdateKeyEvent()
{
    if (SEASON3B::IsPress(VK_MENU) == true)
    {
        m_bShowItemName = !m_bShowItemName;
    }

    if (SEASON3B::IsPress(VK_F8) == true)
    {
        m_bShowMonsterHealthBar = !m_bShowMonsterHealthBar;
        GameConfig::GetInstance().SetShowMonsterPlates(m_bShowMonsterHealthBar);
        GameConfig::GetInstance().Save();
    }

    if (SEASON3B::IsPress(VK_F9) == true)
    {
        m_playerNameMode = UI::PlayerNames::NextMode(m_playerNameMode);
        GameConfig::GetInstance().SetPlayerNames(m_playerNameMode);
        GameConfig::GetInstance().Save();
    }

    return true;
}

bool SEASON3B::CNewUINameWindow::Update()
{
    return true;
}

bool SEASON3B::CNewUINameWindow::Render()
{
    EnableAlphaTest();
    RenderName();
    RenderTimes();
    matchEvent::RenderMatchTimes();
    UI::Chat::RenderBooleans();
    RenderMonsterHealthBars();
    DrawPersonalShopTitleImp();
    DisableAlphaBlend();

    return true;
}

void SEASON3B::CNewUINameWindow::RenderName()
{
    const bool inChaosCastle = gMapManager.InChaosCastle();
    if (g_bGMObservation || (m_playerNameMode != UI::PlayerNames::Off && !inChaosCastle))
    {
        const bool alliesOnly = m_playerNameMode == UI::PlayerNames::PartyAndGuild;
        for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
        {
            CHARACTER* c = &CharactersClient[i];
            OBJECT* o = &c->Object;
            if (!o->Live || !o->Visible || o->Kind != KIND_PLAYER || IsShopTitleVisible(c))
                continue;

            const bool isParty = alliesOnly && g_pPartyManager->IsPartyMemberChar(c);
            const bool isGuild = alliesOnly && Hero->GuildMarkIndex >= 0 && c->GuildMarkIndex == Hero->GuildMarkIndex;
            if (UI::PlayerNames::ShouldName(m_playerNameMode, g_bGMObservation, inChaosCastle, c == Hero, isParty, isGuild))
                UI::Chat::KeepNameAlive(c);
        }
    }

#ifndef GUILD_WAR_EVENT
    if (gMapManager.InChaosCastle() == true && (SelectedNpc != -1 || SelectedCharacter != -1))
    {
        return;
    }
#endif//GUILD_WAR_EVENT

    if (SelectedItem != -1 || SelectedNpc != -1 || SelectedCharacter != -1)
    {
        if (SelectedNpc != -1)
        {
            CHARACTER* c = &CharactersClient[SelectedNpc];
            OBJECT* o = &c->Object;
            UI::Chat::CreateChat(c->ID, L"", c);
        }
        else if (SelectedCharacter != -1)
        {
            CHARACTER* c = &CharactersClient[SelectedCharacter];

            OBJECT* o = &c->Object;
            if (o->Kind == KIND_MONSTER)
            {
                g_pRenderText->SetTextColor(255, 230, 200, 255);
                g_pRenderText->SetBgColor(100, 0, 0, 255);

                // "<name> — <percent>%"; just the name while HP is unknown.
                wchar_t targetLine[MAX_MONSTER_NAME + 16];
                const int percent = UI::Combat::HealthBar::HealthPercent(c->HealthStatus);
                if (percent >= 0)
                    mu_swprintf_s(targetLine, L"%ls \u2014 %d%%", c->ID, percent);
                else
                    mu_swprintf_s(targetLine, L"%ls", c->ID);
                g_pRenderText->RenderText(320, 2, targetLine, 0, 0, RT3_WRITE_CENTER);

                if (UI::Combat::HealthBar::ShouldRenderSelected(c->HealthStatus))
                {
                    // Full-width bar centered under the selected monster's name.
                    DrawHealthBar(320, 15, c->HealthStatus, 20, 1.f, 1.f);
                }
            }
            else
#ifdef ASG_ADD_GENS_SYSTEM
#ifndef PBG_MOD_STRIFE_GENSMARKRENDER
                if (!::IsStrifeMap(World) || Hero->m_byGensInfluence == c->m_byGensInfluence)
#endif //PBG_MOD_STRIFE_GENSMARKRENDER
#endif	// ASG_ADD_GENS_SYSTEM
                {
                    if (IsShopTitleVisible(c) == false)
                    {
                        UI::Chat::CreateChat(c->ID, L"", c);
                    }
                }
        }
        else if (SelectedItem != -1)
        {
#ifdef _EDITOR
            if (DevEditor_ShouldRenderItemLabels())
#endif
                RenderItemName(SelectedItem, &Items[SelectedItem].Object, &Items[SelectedItem].Item, false);
        }
    }

    if (m_bShowItemName || SEASON3B::IsRepeat(VK_MENU))
    {
#ifdef _EDITOR
        bool renderLabels = DevEditor_ShouldRenderItemLabels();
#else
        bool renderLabels = true;
#endif

        if (renderLabels)
        {
            for (int i = 0; i < MAX_ITEMS; i++)
            {
                OBJECT* o = &Items[i].Object;
                if (o->Live)
                {
                    if (o->Visible && i != SelectedItem)
                    {
                        RenderItemName(i, o, &Items[i].Item, true);
                    }
                }
            }
        }
    }
}

void SEASON3B::CNewUINameWindow::RenderMonsterHealthBars()
{
    if (!m_bShowMonsterHealthBar)
        return;

    // Font and colours are set once; the text cache keys on font+string only and
    // colour is a vertex attribute, so a per-monster alpha does not churn it.
    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetBgColor(0, 0, 0, 0);

    for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
    {
        CHARACTER* c = &CharactersClient[i];
        OBJECT* o = &c->Object;

        if (!o->Live || !o->Visible || o->Alpha <= 0.f || c->Dead > 0 || o->Kind != KIND_MONSTER)
            continue;

        const float distanceTiles = VectorDistance2D(o->Position, Hero->Object.Position) / TERRAIN_SCALE;
        const float alpha = UI::Combat::HealthBar::PlateAlpha(distanceTiles);
        if (alpha <= 0.f)
            continue;

        vec3_t Position;
        Vector(o->Position[0], o->Position[1], o->Position[2] + o->BoundingBoxMax[2] + 60.f, Position);

        int ScreenX, ScreenY;
        vec3_t transformPos;
        VectorTransform(Position, g_Camera.Matrix, transformPos);
        if (transformPos[2] >= 0)
            continue;

        CameraProjection::WorldToScreen(g_Camera, Position, &ScreenX, &ScreenY);

        if (ScreenX < -100 || ScreenY < -100
            || ScreenX > (REFERENCE_WIDTH + 100)
            || ScreenY > (REFERENCE_HEIGHT + 100))
            continue;

        // Bar fixed at ~3/7 of the original width, with 8 segments so each one
        // stays close to the original thickness (see DrawHealthBar for geometry).
        DrawHealthBar(ScreenX, ScreenY, c->HealthStatus, 8, 3.f / 7.f, alpha,
            c->Elite ? kHealthFillGold : kHealthFillRed);

        // Name sits above the bar, centred on the same X; elites in gold.
        const BYTE nameAlpha = (BYTE)(alpha * 255.f + 0.5f);
        if (c->Elite)
            g_pRenderText->SetTextColor(255, 200, 0, nameAlpha);
        else
            g_pRenderText->SetTextColor(255, 230, 200, nameAlpha);
        g_pRenderText->RenderText(ScreenX, ScreenY - 12, c->ID, 0, 0, RT3_WRITE_CENTER);
    }
}

float SEASON3B::CNewUINameWindow::GetLayerDepth()
{
    return 1.0f;
}
