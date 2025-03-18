#include "Simulation.h"

static const int SCREEN_WIDTH = 800;
static const int SCREEN_HEIGHT = 600;

//==================================================================
// Draw UI
//==================================================================
static void drawUI(Simulation& sim)
{
    // Draw info
    DrawText(TextFormat("Fuel: %.0f%%", sim.lander.fuel), 10, 10, 20, WHITE);
    DrawText(TextFormat("Speed: %.1f", sqrt(sim.lander.velocity.x*sim.lander.velocity.x + sim.lander.velocity.y*sim.lander.velocity.y)), 10, 40, 20, WHITE);

    // Draw game state message
    if (sim.lander.state == GameState::LANDED)
    {
        DrawText("SUCCESSFUL LANDING!", SCREEN_WIDTH/2 - 150, 200, 30, GREEN);
        DrawText("Press SPACE to play again", SCREEN_WIDTH/2 - 150, 240, 20, WHITE);
    }
    else if (sim.lander.state == GameState::CRASHED)
    {
        DrawText("CRASHED!", SCREEN_WIDTH/2 - 80, 200, 30, RED);
        DrawText("Press SPACE to try again", SCREEN_WIDTH/2 - 150, 240, 20, WHITE);
    }
    else
    {
        DrawText("UP: Vertical thrust, LEFT/RIGHT: Lateral thrusters",
            SCREEN_WIDTH - 600, 10,
            20, WHITE);
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
        if (sim.lander.state == GameState::PLAYING)
        {
            // Handle input
            sim.lander.thrustingUp = IsKeyDown(KEY_UP);
            sim.lander.thrustingLeft = IsKeyDown(KEY_LEFT);
            sim.lander.thrustingRight = IsKeyDown(KEY_RIGHT);

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
        sim.DrawSim();

        // Draw UI
        drawUI(sim);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
