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
#include "TrainingTaskRandom.h"
#include "DrawUI.h"

static const int SCREEN_WIDTH = 800;
static const int SCREEN_HEIGHT = 600;
static const float RESTART_DELAY = 2.0f;

// Number of training epochs to run
static const int MAX_TRAINING_EPOCHS = 50000;

// Forward declarations
static void drawUI(Simulation& sim, TrainingTaskRandom& trainingTask);

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
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Lunar Lander - Neural Network Training Demo");
    SetTargetFPS(60);

    // Setup the simulation parameters
    SimParams sp;
    sp.SCREEN_WIDTH = (float)SCREEN_WIDTH;
    sp.SCREEN_HEIGHT = (float)SCREEN_HEIGHT;

    // Create the simulation object with the parameters
    uint32_t seed = 1134; // Initial random seed
    Simulation sim(sp, seed);

    // Create the training task
    TrainingTaskRandom trainingTask(sp, NETWORK_ARCHITECTURE, MAX_TRAINING_EPOCHS);

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
            // Run a small batch, so we don't block the main thread
            for (int i=0; i < 25; ++i)
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
static void drawUI(Simulation& sim, TrainingTaskRandom& trainingTask)
{
    const int fsize = 20;

    DrawUIBase(sim, fsize, "ai");

    // Draw training information
    DrawUITrainingStatus(trainingTask.IsTrainingComplete(), fsize);

    DrawText(TextFormat("Epoch: %i/%i",
                       (int)trainingTask.GetCurrentEpoch(),
                       (int)trainingTask.GetMaxEpochs()),
            SCREEN_WIDTH - 300, 40, fsize, WHITE);

    const double bestScore = trainingTask.GetBestScore();
    DrawText(TextFormat("Best Score: %.2f", bestScore),
            SCREEN_WIDTH - 300, 70, fsize, bestScore < 100.0f ? GREEN : ORANGE);

    // Draw neural network visualization
    if (!sim.mLander.mStateIsLanded && !sim.mLander.mStateIsCrashed)
    {
        SimpleNeuralNet net(NETWORK_ARCHITECTURE);
        DrawNeuralNetwork(net, trainingTask.GetBestNetworkParameters());
    }
}
