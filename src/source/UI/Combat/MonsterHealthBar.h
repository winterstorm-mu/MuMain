#pragma once

namespace UI::Combat::HealthBar
{
// Distances are in tiles (TERRAIN_SCALE world units) from the hero, in 2D.
inline constexpr float PlateFadeStartTiles = 8.f;
inline constexpr float PlateFadeEndTiles = 12.f;

bool ShouldRenderSelected(float healthStatus);

// 1 up to PlateFadeStartTiles, linear to 0 at PlateFadeEndTiles, 0 beyond.
float PlateAlpha(float distanceTiles);

// round(healthStatus * 100); -1 when healthStatus < 0 (HP unknown).
int HealthPercent(float healthStatus);
}
