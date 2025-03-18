#ifndef SIMULATION_DISPLAY_H
#define SIMULATION_DISPLAY_H

#include "raylib.h"
#include "Simulation.h"

// Here we collect all the simulation drawing functions
// to keep the actual simulation code clean and readable

//==================================================================
inline void DrawLander(const Lander& lander)
{
    Color landerColor = WHITE;
    if (lander.mStateIsLanded) landerColor = GREEN;
    if (lander.mStateIsCrashed) landerColor = RED;

    const auto drawX = lander.mPos.x;
    const auto drawY = lander.mPos.y - 20;

    // Main body
    DrawRectangle(drawX - 15, drawY - 15, 30, 30, landerColor);

    // Landing legs
    DrawLine(drawX - 15, drawY + 15, drawX - 25, drawY + 25, landerColor);
    DrawLine(drawX + 15, drawY + 15, drawX + 25, drawY + 25, landerColor);

    // Do not draw flames if lander is crashed or landed or out of fuel
    if (lander.mStateIsCrashed || lander.mStateIsLanded || lander.mFuel <= 0)
        return;

    // Display a little flame if the thruster is on
    if (lander.mControl_UpThrust)
    {
        DrawTriangle(
            {drawX - 8, drawY + 15},
            {drawX + 8, drawY + 15},
            {drawX, drawY + 25 + GetRandomValue(0, 5)},
            ORANGE
        );
    }

    // Display a little flame if the thruster is on
    if (lander.mControl_LeftThrust)
    {
        DrawTriangle(
            {drawX + 15, drawY - 8},
            {drawX + 15, drawY + 8},
            {drawX + 25 + GetRandomValue(0, 5), drawY},
            ORANGE
        );
    }

    // Display a little flame if the thruster is on
    if (lander.mControl_RightThrust)
    {
        DrawTriangle(
            {drawX - 15, drawY - 8},
            {drawX - 15, drawY + 8},
            {drawX - 25 - GetRandomValue(0, 5), drawY},
            ORANGE
        );
    }
}

//==================================================================
inline void DrawLandingPad(const LandingPad& pad)
{
    const auto px = pad.mPos.x;
    const auto py = pad.mPos.y;
    const auto w = pad.mPadWidth;
    DrawRectangle(px - w/2, py, w, 10, GREEN);

    // Draw landing lights
    for (int i=0; i < 5; ++i)
    {
        float x = px - ((float)w/2) + ((float)w/4) * i;
        DrawRectangle(x, py - 5, 3, 5, YELLOW);
    }
}

//==================================================================
inline void DrawTerrain(const Terrain& terrain)
{
    const auto& pts = terrain.mPoints;
    const auto& sp = terrain.sp;
    for (size_t i=0; i < Terrain::SEGMENTS_N; ++i)
    {
        DrawLineEx(pts[i], pts[i + 1], 2.0f, DARKBROWN);
        // Fill terrain below
        const auto p0 = pts[i];
        const auto p1 = pts[i + 1];
        const auto p2 = Vector2{p0.x, sp.SCREEN_HEIGHT};
        const auto p3 = Vector2{p1.x, sp.SCREEN_HEIGHT};
        //const auto p4 = Vector2{p0.x, sp.SCREEN_HEIGHT};
        DrawTriangle(p0, p1, p2, BROWN);
        DrawTriangle(p1, p3, p2, BROWN);
    }
}

//==================================================================
inline void DrawSim(const Simulation& sim)
{
    DrawTerrain(sim.mTerrain); // Draw terrain
    DrawLandingPad(sim.mLandingPad); // Draw landing pad
    DrawLander(sim.mLander); // Draw lander
}

#endif