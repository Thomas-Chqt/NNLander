#ifndef DRAWUI_H
#define DRAWUI_H

#include "raylib.h"
#include "Simulation.h"

//==================================================================
static void DrawUIBase(Simulation& sim, int fsize, const std::string& ctrl)
{
    const auto screenW = (float)GetScreenWidth();

    // Draw info
    DrawText(TextFormat("Fuel: %.0f%%", sim.mLander.mFuel), 10, 10, fsize, WHITE);

    const auto speed = sim.mLander.CalcSpeed();
    const auto speedColor = sim.sp.LANDING_SAFE_SPEED < speed ? RED : GREEN;
    DrawText(TextFormat("Speed: %.1f", speed), 10, 40, fsize, speedColor);

    // Draw game state message
    float px = screenW/2 - 150;
    float py = 200;
    if (sim.mLander.mStateIsLanded)
    {
        DrawText("SUCCESSFUL LANDING!", px, py, fsize+10, GREEN); py += 40;
        DrawText(TextFormat("Fixed-Brain Score: %.2f", sim.CalculateScore()), px, py, fsize+10, SKYBLUE); py += 40;
        DrawText("Press SPACE to play again", px, py, fsize, WHITE);
    }
    else if (sim.mLander.mStateIsCrashed)
    {
        DrawText("CRASHED!", px, py, fsize+10, RED); py += 40;
        DrawText("Press SPACE to try again", px, py, fsize, WHITE);
    }
    else if (ctrl == "ai")
    {
        const float px = screenW/2 - 150;
        // Flash at an interval to indicate that we're watching the AI play
        const auto frameCount = (int)(sim.GetElapsedTimeS()*60);
        if ((frameCount % 50) > 10)
            DrawText("AI CONTROLLING LANDER", px-90, 10, fsize, ORANGE);
    }
    else if (ctrl == "user")
    {
        DrawText("UP: Vertical thrust, LEFT/RIGHT: Lateral thrusters",
            screenW - 600, 10,
            fsize, WHITE);
    }
    else if (ctrl == "fixed")
    {
        const float px = screenW/2 - 150;
        // Flash at an interval to indicate that we're watching the AI play
        const auto frameCount = (int)(sim.GetElapsedTimeS()*60);
        if ((frameCount % 50) > 10)
            DrawText("FIXED-BRAIN CONTROLLING LANDER", px-50, 10, fsize, ORANGE);
    }
}

//==================================================================
inline void DrawUITrainingStatus(bool isTrainingComplete, int fsize)
{
    const char* pStatus = isTrainingComplete ? "TRAINING COMPLETE" : "TRAINING...";

    DrawText(pStatus, GetScreenWidth() - 300, 10, fsize, YELLOW);
}

#endif
