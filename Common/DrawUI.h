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
// Assumes weights are stored row-major in LayerParameters.
inline void DrawNeuralNetwork(const SimpleNeuralNet& net)
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
    const auto& layerParams = net.GetLayerParameters(); // Get layer-based parameters

    // Draw connections first (so they appear behind nodes)
    float prevLayerY = startY;
    auto prevLayerNodes = architecture[0]; // Use int for consistency with loop types

    for (size_t layerIdx=0; layerIdx < layerParams.size(); ++layerIdx) // Loop through layer transitions
    {
        const auto& currentLayerParams = layerParams[layerIdx];
        const auto* pWeights = currentLayerParams.weights.data();
        const auto currLayerNodes = architecture[layerIdx + 1];
        const auto currLayerY = startY + (float)(layerIdx + 1) * layerSpacing;

        // Calculate vertical offset to center the layer
        float prevOffset = (float)(prevLayerNodes - 1) * nodeSpacing / 2;
        float currOffset = (float)(currLayerNodes - 1) * nodeSpacing / 2;

        // Draw connections between layers
        for (int prevNode=0; prevNode < prevLayerNodes; ++prevNode)
        {
            const auto prevX = startX + (float)prevNode * nodeSpacing - prevOffset;
            const auto prevY = prevLayerY;

            for (int currNode=0; currNode < currLayerNodes; ++currNode)
            {
                const auto currX = startX + (float)currNode * nodeSpacing - currOffset;
                const auto currY = currLayerY;

                // Get weight value directly from the layer's weights vector
                // Assuming row-major: weights[currNode * prevLayerNodes + prevNode]
                float weight = pWeights[currNode * prevLayerNodes + prevNode];
                weight = std::clamp(weight, -1.0f, 1.0f); // Clamp for color mapping

                // Color based on weight value using lerp between red (-1) and blue (+1)
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
        const auto nodes = (size_t)architecture[layer];
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
