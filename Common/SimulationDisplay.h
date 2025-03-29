#ifndef SIMULATION_DISPLAY_H
#define SIMULATION_DISPLAY_H

#include "raylib.h"
#include "Simulation.h"
#include "Utils.h"
#include <vector>

// Here we collect all the simulation drawing functions
// to keep the actual simulation code clean and readable

//==================================================================
// Draw stars in the background
inline void DrawStars(const Simulation& sim, int64_t drawFrame)
{
    struct Star
    {
        Vector2 position {0,0};
        float lum {};
        float size {};
        int shimmerOff {};
        int shimmerPeriod {};
        float shimmerStre {};
    };

    static std::vector<Star> stars;
    const auto& sp = sim.sp;

    // Make sure stars are initialized
    if (stars.empty())
    {
        stars.resize(400);
        // Create a RandomGenerator with a fixed seed for reproducible stars
        RandomGenerator rng(0xABCDEF0123456789);
        for (auto& star : stars)
        {
            star.position.x = rng.RandRange(0, sp.SCREEN_WIDTH);
            star.position.y = rng.RandRange(0, sp.SCREEN_HEIGHT);
            star.lum = rng.RandRange(0.3f, 0.6f);
            star.size = rng.RandRange(0.5f, 1.0f);
            star.shimmerOff = rng.RandRange(0, 1000);
            star.shimmerPeriod = rng.RandRange(40, 90);
            star.shimmerStre = rng.RandRange(0.4f, 0.6f);
        }
    }

    // Draw each star
    for (auto& star : stars)
    {
        // Use modulo to create a periodic shimmer cycle
        const auto shimmerAngle =
            (double)((drawFrame + star.shimmerOff) % star.shimmerPeriod)
                / (double)star.shimmerPeriod * 2.0 * PI;

        const auto shimmer =
            ((float)std::cos(shimmerAngle) + 1) * 0.5f * star.shimmerStre;

        auto l = std::clamp(star.lum + shimmer, 0.05f, 1.0f);
        auto col = ColorFromNormalized({l, l, l, 1.0f});
        DrawCircleV(star.position, (float)star.size, col);
    }
}

//==================================================================
// Convert simulation coordinates to screen coordinates
// In the simulation y=0 -> bottom, but in the display y=0 -> top
inline Vector2 SimToScreen(const Vector2& simPos, const SimParams& sp)
{
    return Vector2
    {
        simPos.x + sp.SCREEN_WIDTH*0.5f,
        sp.SCREEN_HEIGHT - simPos.y
    };
}

//==================================================================
inline void DrawLander(const Lander& lander, const SimParams& sp)
{
    Color landerColor = WHITE;
    if (lander.mStateIsLanded) landerColor = GREEN;
    if (lander.mStateIsCrashed) landerColor = RED;

    const auto drawX = SimToScreen(lander.mPos, sp).x;
    const auto drawY = SimToScreen(lander.mPos, sp).y - 20;

    // Main body
    DrawRectangleV({drawX - 15, drawY - 15}, {30.f, 30.f}, landerColor);

    // Landing legs
    DrawLineV({drawX - 15, drawY + 15}, {drawX - 25, drawY + 25}, landerColor);
    DrawLineV({drawX + 15, drawY + 15}, {drawX + 25, drawY + 25}, landerColor);

    // Do not draw flames if lander is crashed or landed or out of fuel
    if (lander.mStateIsCrashed || lander.mStateIsLanded || lander.mFuel <= 0)
        return;

    // Use a Random Generator with the current draw coordinates as seed
    // This gives different but consistent flame patterns for each position
    RandomGenerator rng(*(uint64_t*)&drawX ^ (*(uint64_t*)&drawY << 32));

    // Display a little flame if the thruster is on
    if (lander.mControl_UpThrust)
    {
        DrawTriangle(
            {drawX - 8, drawY + 15},
            {drawX + 8, drawY + 15},
            {drawX, drawY + 25 + rng.RandRange(0, 5)},
            ORANGE
        );
    }

    // Display a little flame if the thruster is on
    if (lander.mControl_LeftThrust)
    {
        DrawTriangle(
            {drawX + 15, drawY - 8},
            {drawX + 15, drawY + 8},
            {drawX + 25 + rng.RandRange(0, 5), drawY},
            ORANGE
        );
    }

    // Display a little flame if the thruster is on
    if (lander.mControl_RightThrust)
    {
        DrawTriangle(
            {drawX - 15, drawY - 8},
            {drawX - 15, drawY + 8},
            {drawX - 25 - rng.RandRange(0, 5), drawY},
            ORANGE
        );
    }
}

//==================================================================
inline void DrawLandingPad(const LandingPad& pad, const SimParams& sp)
{
    const auto spos = SimToScreen(pad.mPos, sp);
    const auto px = spos.x;
    const auto py = spos.y;
    const auto w = pad.mPadWidth;
    DrawRectangleV({px - w/2, py}, {w, 10.f}, GREEN);

    // Draw landing lights
    for (int i=0; i < 5; ++i)
    {
        float x = px - ((float)w/2) + ((float)w/4) * i;
        DrawRectangleV({x, py - 5}, {3.f, 5.f}, YELLOW);
    }
}

//==================================================================
inline void DrawTerrain(const Terrain& terrain, const SimParams& sp)
{
    const auto& pts = terrain.mPoints;
    for (size_t i=0; i < Terrain::SEGMENTS_N; ++i)
    {
        // Fill terrain below
        const auto p0 = SimToScreen(pts[i], sp);
        const auto p1 = SimToScreen(pts[i + 1], sp);
        const auto p2 = Vector2{p0.x, sp.SCREEN_HEIGHT};
        const auto p3 = Vector2{p1.x, sp.SCREEN_HEIGHT};
        //const auto p4 = Vector2{p0.x, sp.SCREEN_HEIGHT};
        DrawLineEx(p0, p1, 2.0f, DARKBROWN);
        DrawTriangle(p0, p1, p2, BROWN);
        DrawTriangle(p1, p3, p2, BROWN);
    }
}

//==================================================================
inline void DrawSim(const Simulation& sim)
{
    static int64_t drawFrame = 0;
    drawFrame += 1;
    DrawStars(sim, drawFrame); // Draw stars (background)
    DrawTerrain(sim.mTerrain, sim.sp); // Draw terrain
    DrawLandingPad(sim.mLandingPad, sim.sp); // Draw landing pad
    DrawLander(sim.mLander, sim.sp); // Draw lander
}

#endif