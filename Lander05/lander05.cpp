#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include <limits>
#include <chrono>

#include "raylib.h"
#include "rlgl.h"
#include "Simulation.h"
#include "SimulationDisplay.h"
#include "SimpleNeuralNet.h"
#include "TrainingTaskRES.h" // Use REINFORCE-ES task
#include "DrawUI.h"

static const int SCREEN_WIDTH = 800;
static const int SCREEN_HEIGHT = 600;
static const float RESTART_DELAY = 2.0f;

// Number of training generations/updates to run
static const int MAX_TRAINING_GENERATIONS = 10000;
// REINFORCE-ES Hyperparameters (NOTE: internally scaled by params count)
static const double SIGMA = 0.5;             // Noise standard deviation
static const double ALPHA = 0.40;            // Learning rate
static const size_t NUM_PERTURBATIONS = 50;  // Number of perturbation pairs

//==================================================================
// Network configuration
//==================================================================
static constexpr std::array<int, 4> NETWORK_ARCHITECTURE = {
    SIM_BRAINSTATE_N,             // Input layer: simulation state variables
    (int)((double)SIM_BRAINSTATE_N*1.25), // Hidden layer
    (int)((double)SIM_BRAINSTATE_N*1.25), // Hidden layer
    SIM_BRAINACTION_N             // Output layer: actions (up, left, right)
};

using TrainingTask = TrainingTaskRES<float, NETWORK_ARCHITECTURE>;

// Forward declarations
static void drawUI(Simulation& sim, TrainingTask& trainingTask);

//==================================================================
// Main function
//==================================================================
int main()
{
    // Initialize window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "NNLander05 - REINFORCE-ES Training");
    SetTargetFPS(60);

    // Setup the simulation parameters
    SimParams sp;
    sp.SCREEN_WIDTH = (float)SCREEN_WIDTH;
    sp.SCREEN_HEIGHT = (float)SCREEN_HEIGHT;

    // Create the simulation object with the parameters
    uint32_t seed = 1134; // Initial random seed
    Simulation sim(sp, seed);

    // Create the training task
    TrainingTask::Params par;
    par.maxGenerations = MAX_TRAINING_GENERATIONS;
    par.sigma = SIGMA;
    par.alpha = ALPHA;
    par.numPerturbations = NUM_PERTURBATIONS;
    TrainingTask trainingTask(par, sp);

    // We'll use the central network from trainingTask

    float restartTimer = 0.0f;

    // Variables to track training time
    auto trainingStartTime = std::chrono::steady_clock::now();
    bool hasTrainingCompleted = false;

    // Main game loop
    while (!WindowShouldClose())
    {
        // Run training iterations in the background
        if (!trainingTask.IsTrainingComplete())
        {
            // Run a single generation per frame to avoid blocking the UI too much
            trainingTask.RunIteration();
        }
        else
        if (!hasTrainingCompleted)
        {
            // Training just completed
            auto trainingEndTime = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(trainingEndTime - trainingStartTime).count();
            printf("Training completed in %i seconds\n", (int)duration);
            hasTrainingCompleted = true;
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
            // Animate the simulation using the best network from the training task
            sim.AnimateSim([&](const TrainingTask::NeuralNet::Inputs& states, TrainingTask::NeuralNet::Outputs& actions)
            {
                // states -> centralNet -> actions
                trainingTask.GetCentralNetwork().FeedForward(states, actions); // Use central network
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
static void drawUI(Simulation& sim, TrainingTask& trainingTask)
{
    // Draw neural network visualization
    //if (!sim.mLander.mStateIsLanded && !sim.mLander.mStateIsCrashed)
    {
        // Use the central network from the training task for visualization
        const auto& centralNet = trainingTask.GetCentralNetwork(); // Use central network
        // Pass the flattened parameters to the drawing function
        DrawNeuralNetwork(centralNet);
    }

    const int fsize = 20;

    DrawUIBase(sim, fsize, "ai");

    // Draw training information
    DrawUITrainingStatus(trainingTask.IsTrainingComplete(), fsize);

    DrawText(TextFormat("Generation: %i/%i",
                       (int)trainingTask.GetCurrentGeneration(),
                       (int)trainingTask.GetMaxGenerations()),
            SCREEN_WIDTH - 300, 40, fsize, WHITE);

    const double bestScore = trainingTask.GetBestScore();
    DrawText(TextFormat("Best Score: %.2f", bestScore),
            SCREEN_WIDTH - 300, 70, fsize, bestScore > 500.0f ? GREEN : ORANGE);

    // Display ES parameters
    DrawText(TextFormat("Sigma: %.3f", trainingTask.GetSigma()),
            SCREEN_WIDTH - 300, 100, fsize, WHITE);
    DrawText(TextFormat("Alpha: %.4f", trainingTask.GetAlpha()),
            SCREEN_WIDTH - 300, 130, fsize, WHITE);
    DrawText(TextFormat("Perturbations: %zu", trainingTask.GetNumPerturbations()),
            SCREEN_WIDTH - 300, 160, fsize, WHITE);
}
