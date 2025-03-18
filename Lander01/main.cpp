#include "raylib.h"
#include "rlgl.h"
#include "Simulation.h"
#include "SimulationDisplay.h"

static const int SCREEN_WIDTH = 800;
static const int SCREEN_HEIGHT = 600;

//==================================================================
// Draw UI
//==================================================================
static void drawUI(Simulation& sim)
{
    const int fsize = 20;
    // Draw info
    DrawText(TextFormat("Fuel: %.0f%%", sim.mLander.mFuel), 10, 10, fsize, WHITE);

    const auto speed = sim.mLander.CalcSpeed();
    const auto speedColor = sim.sp.LANDING_SAFE_SPEED < speed ? RED : GREEN;
    DrawText(TextFormat("Speed: %.1f", speed), 10, 40, fsize, speedColor);

    // Draw game state message
    if (sim.mLander.state == GameState::LANDED)
    {
        DrawText("SUCCESSFUL LANDING!", SCREEN_WIDTH/2 - 150, 200, fsize+10, GREEN);
        DrawText("Press SPACE to play again", SCREEN_WIDTH/2 - 150, 240, fsize, WHITE);
    }
    else if (sim.mLander.state == GameState::CRASHED)
    {
        DrawText("CRASHED!", SCREEN_WIDTH/2 - 80, 200, fsize+10, RED);
        DrawText("Press SPACE to try again", SCREEN_WIDTH/2 - 150, 240, fsize, WHITE);
    }
    else
    {
        DrawText("UP: Vertical thrust, LEFT/RIGHT: Lateral thrusters",
            SCREEN_WIDTH - 600, 10,
            fsize, WHITE);
    }
}

//==================================================================
// Main function
//==================================================================
int main()
{
    // Initialize window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Lunar Lander Simulation");
    SetTargetFPS(60);

    // Setup the simulation parameters
    SimParams sp;
    sp.SCREEN_WIDTH = (float)SCREEN_WIDTH;
    sp.SCREEN_HEIGHT = (float)SCREEN_HEIGHT;
    // Create the simulation object
    Simulation sim(sp);

    // Main game loop
    while (!WindowShouldClose())
    {
        // Update
        if (sim.mLander.state == GameState::PLAYING)
        {
            // Handle input
            sim.mLander.mIsThrustUpActive = IsKeyDown(KEY_UP);
            sim.mLander.mIsThrustLeftActive = IsKeyDown(KEY_LEFT);
            sim.mLander.mIsThrustRightActive = IsKeyDown(KEY_RIGHT);

            // Animate the simulation
            sim.AnimateSim();
        }
        else
        {
            // Restart game on Space key
            if (IsKeyPressed(KEY_SPACE))
            {
                // Reset the simulation
                sim = Simulation(sp);
            }
        }

        // Drawing
        BeginDrawing();
        ClearBackground(BLACK);

        rlDisableBackfaceCulling(); // Don't care about triangles winding order

        // Draw the simulation
        DrawSim(sim);

        // Draw UI
        drawUI(sim);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
