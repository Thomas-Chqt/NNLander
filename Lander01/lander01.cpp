#include "raylib.h"
#include "rlgl.h"
#include "Simulation.h"
#include "SimulationDisplay.h"

static const int SCREEN_WIDTH = 800;
static const int SCREEN_HEIGHT = 600;

static void drawUI(Simulation& sim);

//==================================================================
// This is just an interface to the user's brain ;)
//==================================================================
static void getUserBrainActions(
    const float* in_simState, size_t in_simStateN,
    float* out_actions, size_t out_actionsN)
{
    // We ignore the input state here, because it's up to the user
    // to see the simulation on screen and decide what to do
    (void)in_simState; (void)in_simStateN;

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
    // Draw info
    DrawText(TextFormat("Fuel: %.0f%%", sim.mLander.mFuel), 10, 10, fsize, WHITE);

    const auto speed = sim.mLander.CalcSpeed();
    const auto speedColor = sim.sp.LANDING_SAFE_SPEED < speed ? RED : GREEN;
    DrawText(TextFormat("Speed: %.1f", speed), 10, 40, fsize, speedColor);

    // Draw game state message
    if (sim.mLander.mStateIsLanded)
    {
        DrawText("SUCCESSFUL LANDING!", SCREEN_WIDTH/2 - 150, 200, fsize+10, GREEN);
        DrawText("Press SPACE to play again", SCREEN_WIDTH/2 - 150, 240, fsize, WHITE);
    }
    else if (sim.mLander.mStateIsCrashed)
    {
        DrawText("STATE_CRASHED!", SCREEN_WIDTH/2 - 80, 200, fsize+10, RED);
        DrawText("Press SPACE to try again", SCREEN_WIDTH/2 - 150, 240, fsize, WHITE);
    }
    else
    {
        DrawText("UP: Vertical thrust, LEFT/RIGHT: Lateral thrusters",
            SCREEN_WIDTH - 600, 10,
            fsize, WHITE);
    }
}

