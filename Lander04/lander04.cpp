#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include <limits>

#include "raylib.h"
#include "rlgl.h"
#include "Simulation.h"
#include "SimulationDisplay.h"
#include "SimpleNeuralNet.h"
#include "TrainingTaskGA.h"

static const int SCREEN_WIDTH = 800;
static const int SCREEN_HEIGHT = 600;
static const float RESTART_DELAY = 2.0f;

// Number of training generations to run
static const int MAX_TRAINING_GENERATIONS = 10000;
// Size of population
static const int POPULATION_SIZE = 100;
// Mutation parameters
static const double MUTATION_RATE = 0.1;
static const double MUTATION_STRENGTH = 0.3;

// Forward declarations
static void drawUI(Simulation& sim, TrainingTaskGA& trainingTask);

//==================================================================
// Network configuration
//==================================================================
static const std::vector<int> NETWORK_ARCHITECTURE = {
    SIM_BRAINSTATE_N,             // Input layer: simulation state variables
    (int)(SIM_BRAINSTATE_N*1.25), // Hidden layer
    (int)(SIM_BRAINSTATE_N*1.25), // Hidden layer
    SIM_BRAINACTION_N             // Output layer: actions (up, left, right)
};

//==================================================================
// Main function
//==================================================================
int main()
{
    // Initialize window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Lunar Lander - Genetic Algorithm Training Demo");
    SetTargetFPS(60);

    // Setup the simulation parameters
    SimParams sp;
    sp.SCREEN_WIDTH = (float)SCREEN_WIDTH;
    sp.SCREEN_HEIGHT = (float)SCREEN_HEIGHT;

    // Create the simulation object with the parameters
    uint32_t seed = 1134; // Initial random seed
    Simulation sim(sp, seed);

    // Create the training task
    TrainingTaskGA trainingTask(
        sp,
        NETWORK_ARCHITECTURE,
        MAX_TRAINING_GENERATIONS,
        POPULATION_SIZE,
        MUTATION_RATE,
        MUTATION_STRENGTH
    );

    // Neural net object used for testing (will use the best params as they come
    // from the ongoing training)
    SimpleNeuralNet testNet(NETWORK_ARCHITECTURE);

    float restartTimer = 0.0f;

    // Main game loop
    while (!WindowShouldClose())
    {
        // Run training iterations in the background
        if (!trainingTask.IsTrainingComplete())
        {
            // Run a single generation per frame to avoid blocking the UI too much
            trainingTask.RunIteration();
        }

        // Auto-restart after landing or crashing
        if (sim.mLander.mStateIsLanded || sim.mLander.mStateIsCrashed)
        {
            restartTimer += GetFrameTime();
            if (restartTimer >= RESTART_DELAY || IsKeyPressed(KEY_SPACE))
            {
                // Reset the simulation, keep the same seed
                sim = Simulation(sp, seed);
                seed += 1;
                restartTimer = 0.0f;
            }
        }
        else
        {
            // Animate the simulation with the neural network brain
            const auto& bestParams = trainingTask.GetBestNetworkParameters();
            sim.AnimateSim([&](const float* states, float* actions)
            {
                // states -> testNet(bestParams) -> actions
                testNet.FeedForward(bestParams.data(), states, actions);
            });
        }

        // Begin drawing
        BeginDrawing();

        ClearBackground(BLACK);
        // Allow any triangle to be drawn regardless of winding order
        rlDisableBackfaceCulling();

        // Draw the simulation
        DrawSim(sim);
        // Draw UI
        drawUI(sim, trainingTask);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

//==================================================================
static void drawUI(Simulation& sim, TrainingTaskGA& trainingTask)
{
    const int fsize = 20;
    // Draw info
    DrawText(TextFormat("Fuel: %.0f%%", sim.mLander.mFuel), 10, 10, fsize, WHITE);

    const auto speed = sim.mLander.CalcSpeed();
    const auto speedColor = sim.sp.LANDING_SAFE_SPEED < speed ? RED : GREEN;
    DrawText(TextFormat("Speed: %.1f", speed), 10, 40, fsize, speedColor);

    // Draw training information
    const char* trainingStatus = trainingTask.IsTrainingComplete() ?
                                "TRAINING COMPLETE" : "TRAINING...";
    DrawText(trainingStatus, SCREEN_WIDTH - 300, 10, fsize, YELLOW);

    DrawText(TextFormat("Generation: %i/%i",
                       (int)trainingTask.GetCurrentGeneration(),
                       (int)trainingTask.GetMaxGenerations()),
            SCREEN_WIDTH - 300, 40, fsize, WHITE);

    const double bestScore = trainingTask.GetBestScore();
    DrawText(TextFormat("Best Score: %.2f", bestScore),
            SCREEN_WIDTH - 300, 70, fsize, bestScore < 100.0f ? GREEN : ORANGE);

    DrawText(TextFormat("Population Size: %i",
                       (int)trainingTask.GetPopulationSize()),
            SCREEN_WIDTH - 300, 100, fsize, WHITE);

    // Draw game state message
    float px = SCREEN_WIDTH/2 - 150;
    float py = 200;
    if (sim.mLander.mStateIsLanded)
    {
        DrawText("SUCCESSFUL LANDING!", px, py, fsize+10, GREEN); py += 40;
        DrawText(TextFormat("AI Score: %.2f", sim.CalculateScore()), px, py, fsize+10, SKYBLUE); py += 40;
        DrawText("Wait for restart or press SPACE", px, py, fsize, WHITE);
    }
    else if (sim.mLander.mStateIsCrashed)
    {
        DrawText("CRASHED!", px, py, fsize+10, RED); py += 40;
        DrawText("Wait for restart or press SPACE", px, py, fsize, WHITE);
    }
    else
    {
        // Flash at an interval to indicate that we're watching the AI play
        const auto frameCount = (int)(sim.GetElapsedTimeS()*60);
        if ((frameCount % 50) > 10)
            DrawText("AI CONTROLLING LANDER", px-90, 10, fsize, ORANGE);
    }
}