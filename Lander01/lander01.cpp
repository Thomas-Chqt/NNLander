#include "raylib.h"
#include "rlgl.h"
#include "Simulation.h"
#include "SimulationDisplay.h"
#include "DrawUI.h"

static const int SCREEN_WIDTH = 800;
static const int SCREEN_HEIGHT = 600;

static void drawUI(Simulation& sim);

//==================================================================
// This is just an interface to the user's brain ;)
//==================================================================
static void getUserBrainActions(const float* in_simState, float* out_actions)
{
    // We ignore the input state here, because it's up to the user
    // to see the simulation on screen and decide what to do !
    (void)in_simState;

    // Set the actions based on the user input
    out_actions[SIM_BRAINACTION_UP]    = (float)IsKeyDown(KEY_UP);
    out_actions[SIM_BRAINACTION_LEFT]  = (float)IsKeyDown(KEY_LEFT);
    out_actions[SIM_BRAINACTION_RIGHT] = (float)IsKeyDown(KEY_RIGHT);
}

//==================================================================
// Main function
//==================================================================
int main()
{
    // Enable anti-aliasing (MSAA 4X)
    //SetConfigFlags(FLAG_MSAA_4X_HINT);

    // Initialize window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Lunar Lander Simulation");
    SetTargetFPS(60);

    // Setup the simulation parameters
    SimParams sp;
    sp.SCREEN_WIDTH = (float)SCREEN_WIDTH;
    sp.SCREEN_HEIGHT = (float)SCREEN_HEIGHT;

    // Create the simulation object with the parameters
    uint32_t seed = 1134; // Always the same seed
    Simulation sim(sp, seed);

    // Main game loop
    while (!WindowShouldClose())
    {
        // Update if it is not crashed or landed
        if (sim.mLander.mStateIsCrashed == false &&
            sim.mLander.mStateIsLanded == false)
        {
            // Animate the simulation with the user brain
            sim.AnimateSim(getUserBrainActions);
        }
        else
        {
            // Restart game on Space key
            if (IsKeyPressed(KEY_SPACE))
                sim = Simulation(sp, ++seed); // New simulation, with new seed
        }

        // Begin drawing
        BeginDrawing();

        ClearBackground(BLACK);
        // Allow any triangle to be drawn regardless of winding order
        rlDisableBackfaceCulling();

        // Draw the simulation
        DrawSim(sim);
        // Draw UI
        drawUI(sim);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

//==================================================================
static void drawUI(Simulation& sim)
{
    const int fsize = 20;

    DrawUIBase(sim, fsize, "user");
}

