#include "UI/Combat/MonsterHealthBar.h"

bool UI::Combat::HealthBar::ShouldRenderSelected(float healthStatus)
{
    return healthStatus != 0.0f;
}

float UI::Combat::HealthBar::PlateAlpha(float distanceTiles)
{
    if (distanceTiles <= PlateFadeStartTiles)
        return 1.0f;
    if (distanceTiles >= PlateFadeEndTiles)
        return 0.0f;
    return (PlateFadeEndTiles - distanceTiles) / (PlateFadeEndTiles - PlateFadeStartTiles);
}

int UI::Combat::HealthBar::HealthPercent(float healthStatus)
{
    if (healthStatus < 0.0f)
        return -1;
    return static_cast<int>(healthStatus * 100.0f + 0.5f);
}
