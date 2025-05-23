#include <vector>
#include <chrono>

#include "raylib.h"
#include "rlgl.h"
#include "Simulation.h"
#include "SimulationDisplay.h"
#include "SimpleNeuralNet.h"
#include "TrainingTaskGA.h"
#include "DrawUI.h"

static const int SCREEN_WIDTH = 800;
static const int SCREEN_HEIGHT = 600;
static const float RESTART_DELAY = 2.0f;

// Number of training generations to run
static const int MAX_TRAINING_GENERATIONS = 10000;
// Size of population
static const int POPULATION_SIZE = 200;
// Mutation parameters
static const double MUTATION_RATE = 0.1;
static const double MUTATION_STRENGTH = 0.3;

//==================================================================
// Network configuration
//==================================================================
static constexpr std::array<int, 4> NETWORK_ARCHITECTURE = {
    SIM_BRAINSTATE_N,             // Input layer: simulation state variables
    (int)((double)SIM_BRAINSTATE_N*1.25), // Hidden layer
    (int)((double)SIM_BRAINSTATE_N*1.25), // Hidden layer
    SIM_BRAINACTION_N             // Output layer: actions (up, left, right)
};

using TrainingTask = TrainingTaskGA<float, NETWORK_ARCHITECTURE>;

// Forward declarations
static void drawUI(Simulation& sim, TrainingTask& trainingTask);

//==================================================================
// Main function
//==================================================================
int main()
{
    // Initialize window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "NNLander - Genetic Algorithm Training");
    SetTargetFPS(60);

    // Setup the simulation parameters
    SimParams sp;
    sp.SCREEN_WIDTH = (float)SCREEN_WIDTH;
    sp.SCREEN_HEIGHT = (float)SCREEN_HEIGHT;

    // Create the simulation object with the parameters
    uint32_t seed = 1134; // Initial random seed
    Simulation sim(sp, seed);

    // Create the training task
    TrainingTask trainingTask(
        sp,
        MAX_TRAINING_GENERATIONS,
        POPULATION_SIZE,
        MUTATION_RATE,
        MUTATION_STRENGTH
    );

    // No separate testNet needed, we'll use the best one from trainingTask

    float restartTimer = 0.0f;

    // Variables to track training time
    auto trainingStartTime = std::chrono::steady_clock::now();
    bool hasTrainingCompleted = false;
    trainingTask.startTraining();

    // Main game loop
    while (!WindowShouldClose())
    {
        if (!hasTrainingCompleted && trainingTask.IsTrainingComplete())
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
                // states -> bestNet -> actions
                trainingTask.GetBestIndividualNetwork().FeedForward(states, actions);
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
        // Use the actual best network from the training task for visualization
        const auto& bestNet = trainingTask.GetBestIndividualNetwork();
        // Pass the flattened parameters to the drawing function
        DrawNeuralNetwork(bestNet);
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

    DrawText(TextFormat("Population Size: %i",
                       (int)trainingTask.GetPopulationSize()),
            SCREEN_WIDTH - 300, 100, fsize, WHITE);
}
