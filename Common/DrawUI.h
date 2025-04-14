#ifndef DRAWUI_H
#define DRAWUI_H

#include <vector>
#include "raylib.h"
#include "Simulation.h"
#include "SimpleNeuralNet.h"

//==================================================================
inline void DrawTextF(const char* text, float x, float y, int fsize, Color color)
{
    DrawText(TextFormat(text), (int)x, (int)y, fsize, color);
}

//==================================================================
static void DrawUIBase(Simulation& sim, int fsize, const std::string& ctrl)
{
    const auto screenW = (float)GetScreenWidth();

    // Draw info
    DrawTextF(TextFormat("Fuel: %.0f%%", sim.mLander.mFuel), 10, 10, fsize, WHITE);

    const auto speed = sim.mLander.CalcSpeed();
    const auto speedColor = sim.sp.LANDING_SAFE_SPEED < speed ? RED : GREEN;
    DrawTextF(TextFormat("Speed: %.1f", speed), 10, 40, fsize, speedColor);

    // Draw game state message
    float px = screenW/2 - 150;
    float py = 200;
    if (sim.mLander.mStateIsLanded)
    {
        const auto* pPlayer = "User";
        if (ctrl == "ai")    pPlayer = "AI"; else
        if (ctrl == "fixed") pPlayer = "Fixed-Brain";

        DrawTextF(TextFormat("SUCCESSFUL LANDING! (%s)", pPlayer), px, py, fsize+10, GREEN); py += 40;
        DrawTextF(TextFormat("%s Score: %.2f", pPlayer, sim.CalculateScore()), px, py, fsize+10, SKYBLUE); py += 40;
        DrawTextF("Press SPACE to play again", px, py, fsize, WHITE);
    }
    else if (sim.mLander.mStateIsCrashed)
    {
        DrawTextF("CRASHED!", px, py, fsize+10, RED); py += 40;
        DrawTextF("Press SPACE to try again", px, py, fsize, WHITE);
    }
    else if (ctrl == "ai")
    {
        // Flash at an interval to indicate that we're watching the AI play
        const auto frameCount = (int)(sim.GetElapsedTimeS()*60);
        if ((frameCount % 50) > 10)
            DrawTextF("AI CONTROLLING LANDER", px-90, 10, fsize, ORANGE);
    }
    else if (ctrl == "user")
    {
        DrawTextF("UP: Vertical thrust, LEFT/RIGHT: Lateral thrusters",
            screenW - 600, 10,
            fsize, WHITE);
    }
    else if (ctrl == "fixed")
    {
        // Flash at an interval to indicate that we're watching the AI play
        const auto frameCount = (int)(sim.GetElapsedTimeS()*60);
        if ((frameCount % 50) > 10)
            DrawTextF("FIXED-BRAIN CONTROLLING LANDER", px-50, 10, fsize, ORANGE);
    }
}

//==================================================================
inline void DrawUITrainingStatus(bool isTrainingComplete, int fsize)
{
    const char* pStatus = isTrainingComplete ? "TRAINING COMPLETE" : "TRAINING...";

    DrawText(pStatus, GetScreenWidth() - 300, 10, fsize, YELLOW);
}

//==================================================================
// Draws the neural network structure and connection weights.
template<std::floating_point T, NetArch auto netArch>
inline void DrawNeuralNetwork(const SimpleNeuralNet<T, netArch>& net)
{
    //const float screenW = (float)GetScreenWidth();
    //const float screenH = (float)GetScreenHeight();

    // Network visualization parameters
    const float nodeRadius = 10.0f;
    const float layerSpacing = 70.0f;
    const float nodeSpacing = 24.0f;
    const float startX = 160.0f;
    const float startY = 90.0f;
    const int fsize = 15;

    const typename decltype(net)::Parameters& params = net.GetParameters(); // Get parameters

    // Draw connections first (so they appear behind nodes)

    net.foreachParameters([&](int layerIdx, int row, int col, T& param){
        const float prevLayerY = startY + (float)layerIdx * layerSpacing;
        int prevLayerSize = netArch[layerIdx];

        const int currLayerSize = netArch[layerIdx + 1]; // because first parameters layer correspond to the second architecture layer
        const int currLayerY = prevLayerY + layerSpacing;

        // Calculate vertical offset to center the layer
        float prevOffsetX = (float)(prevLayerSize - 1) * nodeSpacing / 2;
        float currOffsetX = (float)(currLayerSize - 1) * nodeSpacing / 2;

        // Draw connections between layers
        for (int prevNodeIdx = 0; prevNodeIdx < prevLayerSize; ++prevNodeIdx)
        {
            const float prevX = startX + (float)prevNodeIdx * nodeSpacing - prevOffsetX;
            const float prevY = prevLayerY;

            if (col < netArch[layerIdx]) // only the weights, not the bias
            {
                const float currX = startX + (float)row * nodeSpacing - currOffsetX;
                const float currY = currLayerY;
                T clampedParam = std::clamp(param, T(-1.0), T(1.0)); // Clamp for color mapping
                //
                // Color based on weight value using lerp between red (-1) and blue (+1)
                float t = (clampedParam + 1.0f) * 0.5f; // Convert from [-1,1] to [0,1]
                Color lineColor = ColorLerp(Color{255,0,0,255}, Color{0,0,255,255}, t);
                lineColor.a = 128;//(unsigned char)(std::abs(weight) * 255.0f); // Alpha based on weight magnitude

                DrawLineEx(Vector2{prevX, prevY}, Vector2{currX, currY}, 1.0f, lineColor);
            }
        }
    });

    // Draw nodes
    float layerY = startY;
    for (size_t layer = 0; layer < netArch.size(); ++layer)
    {
        const auto nodes = (size_t)netArch[layer];
        const auto offset = (float)(nodes - 1) * nodeSpacing / 2;

        for (size_t node = 0; node < nodes; ++node)
        {
            const auto x = startX + (float)node * nodeSpacing - offset;
            const auto y = layerY;

            // Draw node circle
            DrawCircle((int)x, (int)y, nodeRadius, ColorAlpha(WHITE, 0.75f));

            // Draw node number
            DrawTextF(
                TextFormat("%d", (int)node),
                x - (float)fsize * 0.3f,
                y - (float)fsize * 0.4f,
                fsize,
                BLACK
            );
        }

        layerY += layerSpacing;
    }
}

#endif
