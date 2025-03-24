#ifndef DRAWUI_H
#define DRAWUI_H

#include <vector>
#include "raylib.h"
#include "Simulation.h"
#include "SimpleNeuralNet.h"

//==================================================================
static Color ColorLerp(Color c1, Color c2, float t)
{
    return Color{
        (unsigned char)std::clamp(c1.r + (c2.r - c1.r) * t, 0.0f, 255.0f),
        (unsigned char)std::clamp(c1.g + (c2.g - c1.g) * t, 0.0f, 255.0f),
        (unsigned char)std::clamp(c1.b + (c2.b - c1.b) * t, 0.0f, 255.0f),
        (unsigned char)std::clamp(c1.a + (c2.a - c1.a) * t, 0.0f, 255.0f)
    };
}

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

//==================================================================
inline void DrawNeuralNetwork(
    const SimpleNeuralNet& net,
    const std::vector<float>& params)
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

    const auto& architecture = net.GetArchitecture();

    // Draw connections first (so they appear behind nodes)
    int paramIdx = 0;
    float prevLayerY = startY;
    float prevLayerNodes = architecture[0];

    for (size_t layer = 1; layer < architecture.size(); ++layer)
    {
        float currLayerY = startY + layer * layerSpacing;
        float currLayerNodes = architecture[layer];

        // Calculate vertical offset to center the layer
        float prevOffset = (prevLayerNodes - 1) * nodeSpacing / 2;
        float currOffset = (currLayerNodes - 1) * nodeSpacing / 2;

        // Draw connections between layers
        for (size_t prevNode = 0; prevNode < prevLayerNodes; ++prevNode)
        {
            float prevX = startX + prevNode * nodeSpacing - prevOffset;
            float prevY = prevLayerY;

            for (size_t currNode = 0; currNode < currLayerNodes; ++currNode)
            {
                float currX = startX + currNode * nodeSpacing - currOffset;
                float currY = currLayerY;

                // Get weight value and normalize it to [-1, 1] range
                float weight = params[paramIdx++];
                weight = std::clamp(weight, -1.0f, 1.0f);

                // Color based on weight value using lerp between blue (-1) and red (+1)
                float t = (weight + 1.0f) * 0.5f; // Convert from [-1,1] to [0,1]
                Color lineColor = ColorLerp(Color{255,0,0,255}, Color{0,0,255,255}, t);
                lineColor.a = 128;//(unsigned char)(std::abs(weight) * 255.0f); // Alpha based on weight magnitude

                DrawLineEx(
                    Vector2{prevX, prevY},
                    Vector2{currX, currY},
                    1.0f,
                    lineColor
                );
            }
        }

        prevLayerY = currLayerY;
        prevLayerNodes = currLayerNodes;
    }

    // Draw nodes
    float layerY = startY;
    for (size_t layer = 0; layer < architecture.size(); ++layer)
    {
        float nodes = architecture[layer];
        float offset = (nodes - 1) * nodeSpacing / 2;

        for (size_t node = 0; node < nodes; ++node)
        {
            float x = startX + node * nodeSpacing - offset;
            float y = layerY;

            // Draw node circle
            DrawCircle((int)x, (int)y, nodeRadius, ColorAlpha(WHITE, 0.75f));

            // Draw node number
            DrawText(
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
