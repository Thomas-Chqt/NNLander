#include "raylib.h"
#include "rlgl.h"
#include "Simulation.h"
#include "SimulationDisplay.h"

static const int SCREEN_WIDTH = 800;
static const int SCREEN_HEIGHT = 600;

static void drawUI(Simulation& sim);

//==================================================================
// This is a simple rule-based brain that operates based on
// predeternined rules implemented the programmer based on
// observation of the simulation.
//==================================================================
static void getFixedBrainActions(
    const float* in_simState, size_t in_simStateN,
    float* out_actions, size_t out_actionsN)
{
    // Copy the simulation state variables to more readable names
    const auto landerX  = in_simState[SIM_BRAINSTATE_LANDER_X];
    const auto landerY  = in_simState[SIM_BRAINSTATE_LANDER_Y];
    const auto landerVX = in_simState[SIM_BRAINSTATE_LANDER_VX];
    const auto landerVY = in_simState[SIM_BRAINSTATE_LANDER_VY];
    const auto padX     = in_simState[SIM_BRAINSTATE_PAD_X];
    const auto padY     = in_simState[SIM_BRAINSTATE_PAD_Y];
    const auto padWidth = in_simState[SIM_BRAINSTATE_PAD_WIDTH];

    // Try to keep the lander centered on the pad by applying lateral
    // thrusts if the lander is too far from the center of the pad
    const auto tolerance = padWidth / 4.0f;
    const auto isLanderTooFarLeft = landerX > padX + tolerance;
    const auto isLanderTooFarRight = landerX < padX - tolerance;
    const auto isLanderMovingLeft = landerVX < -0.5f;
    const auto isLanderMovingRight = landerVX > 0.5f;

    if (isLanderTooFarLeft && !isLanderMovingLeft)
        out_actions[SIM_BRAINACTION_LEFT] = 1.0f; // Apply LEFT thrust
    else
    if (isLanderTooFarRight && !isLanderMovingRight)
        out_actions[SIM_BRAINACTION_RIGHT] = 1.0f; // Apply RIGHT thrust

    // Try to keep the lander from crashing by selectively applying
    // vertical thrust when somewhat close to the pad and the lander
    // is dropping too fast
    const auto minEngageHeight = padWidth * 3; // OK to fall below this height

    const auto isLanderDroppingTooFast = landerVY > 1.0f;
    const auto isLanderTooCloseToPad = landerY > padY - minEngageHeight;
    if (isLanderDroppingTooFast && isLanderTooCloseToPad)
        out_actions[SIM_BRAINACTION_UP] = 1.0f; // Apply UP thrust
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
    uint32_t seed = 1134; // Initial random seed
    Simulation sim(sp, seed);

    // Main game loop
    while (!WindowShouldClose())
    {
        // Update if it is not crashed or landed
        if (sim.mLander.mStateIsCrashed == false &&
            sim.mLander.mStateIsLanded == false)
        {
            // Animate the simulation with the fixed brain
            sim.AnimateSim(getFixedBrainActions);
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

