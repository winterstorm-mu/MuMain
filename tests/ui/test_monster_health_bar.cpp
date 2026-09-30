#include <doctest.h>

#include "UI/Combat/MonsterHealthBar.h"

TEST_CASE("selected monster health eligibility [ui][health_bar]")
{
    CHECK(UI::Combat::HealthBar::ShouldRenderSelected(1.0f));
    CHECK(UI::Combat::HealthBar::ShouldRenderSelected(0.25f));
    CHECK(UI::Combat::HealthBar::ShouldRenderSelected(-1.0f));
    CHECK_FALSE(UI::Combat::HealthBar::ShouldRenderSelected(0.0f));
}

TEST_CASE("plate alpha fades between 8 and 12 tiles [ui][health_bar]")
{
    using UI::Combat::HealthBar::PlateAlpha;
    CHECK(PlateAlpha(0.0f) == doctest::Approx(1.0f));
    CHECK(PlateAlpha(8.0f) == doctest::Approx(1.0f));
    CHECK(PlateAlpha(10.0f) == doctest::Approx(0.5f));
    CHECK(PlateAlpha(11.0f) == doctest::Approx(0.25f));
    CHECK(PlateAlpha(12.0f) == doctest::Approx(0.0f));
    CHECK(PlateAlpha(30.0f) == doctest::Approx(0.0f));
}

TEST_CASE("health percent [ui][health_bar]")
{
    using UI::Combat::HealthBar::HealthPercent;
    CHECK(HealthPercent(-1.0f) == -1);
    CHECK(HealthPercent(0.0f) == 0);
    CHECK(HealthPercent(1.0f) == 100);
    CHECK(HealthPercent(0.64f) == 64);
    CHECK(HealthPercent(1.0f / 250.0f) == 0);
    CHECK(HealthPercent(3.0f / 250.0f) == 1);
    CHECK(HealthPercent(127.0f / 250.0f) == 51);
}
